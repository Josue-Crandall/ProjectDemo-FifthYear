/*
	Alice v.08 a chat client and server.
	Copyright (C) 2020 Josue Crandall

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

static int TCPSockGetFD(TCPSock *sock) { return sock->sock.fd; }
static int TCPLisGetFD(TCPLis *lis) { return lis->fd; }
static void UDPLisDe(UDPLis *lis) { if(lis->alive) { FD_CLOSE(lis->fd); } }
static void UDPSockDe(UDPSock *sock) { if(sock->alive) { FD_CLOSE(sock->fd); } }
static void TCPLisDe(TCPLis *lis) { if(lis->alive) { FD_CLOSE(lis->fd); } }
static void TCPSockDe(TCPSock *sock) { if(sock->sock.alive) { EINTR_CHECK_IGN(shutdown(sock->sock.fd, SHUT_RDWR)); FD_CLOSE(sock->sock.fd); } }
static u16 getSockBindPort(int sockFD) {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    CHECK(getsockname(sockFD, (struct sockaddr *)&sin, &len));
    return ntohs(sin.sin_port);
FAILED:
    return 0;
}

#include <sys/types.h> // send (along with sys/socket.h)
#include <arpa/inet.h> // inet_pton
#include <fcntl.h> // fcntl
#include <stdlib.h> // strtoul
#include <string.h> // strlen
#include <time.h>
#include <endian.h>

#define PORT_STRING_BASE 10

//// support procedures
static Ret initSockAddr(sockaddr_in *addr, char *R() ip, char *R() port) {
    memset(addr, 0, sizeof(sockaddr_in));
    addr->sin_family = AF_INET;
    if(port) { CHECK(0 == (addr->sin_port = htons((u16)strtoul(port, NULL, PORT_STRING_BASE)))); }
    else { addr->sin_port = 0; }
    if(ip) { CHECK(inet_pton(AF_INET, ip, &addr->sin_addr) < 0); }
    else { addr->sin_addr.s_addr = htonl(INADDR_ANY); }
    return 0;
FAILED:
    return -1;
}
// 0 means no timeout
static Ret NetsockSetCloseOnExecute(int fd) {
#ifndef NETSOCK_SET_CLOSE_ON_EXEC
    int ign = fd;
    return 0;
#else
    int setting;
    EINTR_CHECK(setting = fcntl(fd, F_GETFL, 0));
    setting |=  O_CLOEXEC;
    EINTR_CHECK(fcntl(fd, F_SETFL, setting));
    return 0;
FAILED:
    return -1;
#endif
}
static Ret NetsockSetTimeout(int fd, time_t seconds) {
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    { EINTR_CHECK(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))); }
    { EINTR_CHECK(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout))); }
    return 0;
FAILED:
    return -1;
}
static Ret NetsockSetReuseAddr(int fd) {
    int valTrue = 1;
    EINTR_CHECK(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &valTrue, sizeof(valTrue))); 
    return 0;
FAILED:
    return -1;
}

static Ret NetsockSetNonblocking(int fd, Ret on) {
    int setting;
    EINTR_CHECK(setting = fcntl(fd, F_GETFL, 0));
    if (on) { setting |=  O_NONBLOCK; }
    else { setting &= ~O_NONBLOCK; }    
    EINTR_CHECK(fcntl(fd, F_SETFL, setting));
    return 0;
FAILED:
    return -1;	
}
static Ret UDPLisInit(UDPLis *lis,  char *R() bindIP,  char *R() bindPort) {
    UDPAddr addr;
    lis->fd = -1;

    CHECK(initSockAddr(&addr, bindIP, bindPort));
    NEG_CHECK(lis->fd = socket(AF_INET, SOCK_DGRAM, 0));
    CHECK(NetsockSetReuseAddr(lis->fd));
    CHECK(bind(lis->fd, (struct sockaddr *)&addr, sizeof(addr)));
    CHECK(NetsockSetCloseOnExecute(lis->fd));
    CHECK(NetsockSetTimeout(lis->fd, NETSOCK_DEFAULT_TIMEOUT));
    lis->alive = 1;
    return 0;

FAILED:
    FD_CLOSE(lis->fd);
    lis->alive = 0;
    return -1;
}
static Ret UDPSockInit(UDPSock *sock,  char *R() bindIP,  char *R() bindPort) {
    UDPAddr addr;
    sock->fd = -1;

    CHECK(initSockAddr(&addr, bindIP, bindPort));
    NEG_CHECK(sock->fd = socket(AF_INET, SOCK_DGRAM, 0));
    CHECK(connect(sock->fd, (struct sockaddr *)&addr, sizeof(addr)));
    CHECK(NetsockSetCloseOnExecute(sock->fd));
    CHECK(NetsockSetTimeout(sock->fd, NETSOCK_DEFAULT_TIMEOUT));
    sock->alive = 1;
    return 0;

FAILED:
    FD_CLOSE(sock->fd);
    sock->alive = 0;
    return -1;
}
static usize UDPSockSend(UDPSock *sock, u8 *src, usize len) {
    isize amt = 0;

    CHECK(len > MAX_UDP_PACKET_SIZE);

SEND:
    amt = send(sock->fd, src, len, MSG_DONTWAIT);
    if(amt < 0) {
        if(errno == EINTR) { goto SEND; }
        else { amt = 0; }
    }

FAILED:
    return len - amt;
}
static usize UDPLisSend(UDPLis *lis, u8 *src, usize len,  UDPAddr *dst) {
    isize amt = 0;

    CHECK(len > MAX_UDP_PACKET_SIZE);

SENDTO:
    amt = sendto(lis->fd, src, len, MSG_DONTWAIT, dst, sizeof(*dst));
    if(amt < 0) {
        if(errno == EINTR) { goto SENDTO; }
        else { amt = 0; }
    }

FAILED:
    return len - amt;
}
static usize UDPSockRecv(UDPSock *sock, u8 dst[MAX_UDP_PACKET_SIZE], Ret blocking) {
    isize amt;

RECV:
    amt = recv(sock->fd, dst, MAX_UDP_PACKET_SIZE, blocking ? 0 : MSG_DONTWAIT);
    if(amt < 0) {
        if(errno == EINTR) { goto RECV; }
        else { amt = 0; }
    }

    return amt;
}
static usize UDPLisRecv(UDPLis *lis, u8 dst[MAX_UDP_PACKET_SIZE], UDPAddr *sender, Ret blocking) {
    socklen_t len;
    socklen_t *lenPtr;
    if(sender) {
        memset(sender, 0, sizeof(*sender));
        len = sizeof(*sender);
        lenPtr = &len;
    }
    else { lenPtr = NULL; }

    isize amt;

RECVFROM:
    amt = recvfrom(lis->fd, dst, MAX_UDP_PACKET_SIZE, blocking ? 0 : MSG_DONTWAIT, (struct sockaddr *)sender, lenPtr);
    if(amt < 0) {
        if(errno == EINTR) { goto RECVFROM; }
        else { amt = 0; goto FAILED; }
    }

CLEAN:
    return amt;
FAILED:
    if(sender) { memset(sender, 0, sizeof(*sender)); }
    goto CLEAN;
}
static Ret TCPLisInit(TCPLis *lis,  char *R() bindIP,  char *R() bindPort) {
    TCPAddr addr;
    lis->fd = -1;

    CHECK(initSockAddr(&addr, bindIP, bindPort));
    NEG_CHECK(lis->fd = socket(AF_INET, SOCK_STREAM, 0));
    CHECK(NetsockSetReuseAddr(lis->fd));
    CHECK(bind(lis->fd, (struct sockaddr *R())&addr, sizeof(addr)));
    CHECK(listen(lis->fd, TCP_BACK_LOG_SIZE));
    CHECK(NetsockSetCloseOnExecute(lis->fd));
    CHECK(NetsockSetTimeout(lis->fd, NETSOCK_DEFAULT_TIMEOUT));
    CHECK(NetsockSetNonblocking(lis->fd, 1));

    lis->alive = 1;
    return 0;

FAILED:
    FD_CLOSE(lis->fd);
    lis->alive = 0;
    return -1;    
}
static Ret TCPLisAccept(TCPLis *lis, TCPSock *out, Ret blocking) {
    int fd;

    CHECK(NetsockSetNonblocking(lis->fd, !blocking));

ACCEPT:
    fd = accept(lis->fd, NULL, NULL);
    if(fd < 0) {
        if(errno == EWOULDBLOCK || errno == EAGAIN) { 
            out->sock.alive = 0;
            return 1; 
        }
        else if(errno == EINTR) { goto ACCEPT; }
        else { 
            DEBUG_LOG("TCPLis accept failed\n");
            goto FAILED;
        }
    }

    if(NetsockSetCloseOnExecute(fd)) {
        DEBUG_LOG("TCPLis accept failed to set close on exec\n");
        FD_CLOSE(fd);
        goto FAILED;        
    }
    if(NetsockSetTimeout(fd, NETSOCK_DEFAULT_TIMEOUT)) {
        DEBUG_LOG("TCPLis accept failed to set socket timeout\n");
        FD_CLOSE(fd);
        goto FAILED;
    }
    out->sndPlace = 0;
    out->rcvPlace = 0;
    out->sock.fd = fd;
    out->sock.alive = 1;
    return 0;
FAILED:
    out->sock.alive = 0;
    return -1;
}
static Ret TCPSockInit(TCPSock *sock, char *R() serverIP, char *R() serverPort) {
    sockaddr_in addr;
    sock->sock.fd = -1;

    CHECK(initSockAddr(&addr, serverIP, serverPort));
    NEG_CHECK(sock->sock.fd = socket(AF_INET, SOCK_STREAM, 0));
    CHECK(NetsockSetTimeout(sock->sock.fd, NETSOCK_DEFAULT_TIMEOUT));
    CHECK(connect(sock->sock.fd, (struct sockaddr *R())&addr, sizeof(addr)));
    CHECK(NetsockSetCloseOnExecute(sock->sock.fd));

    sock->sndPlace = 0;
    sock->rcvPlace = 0;
    sock->sock.alive = 1;
    return 0;

FAILED:
    FD_CLOSE(sock->sock.fd);
    sock->sock.alive = 0;
    return -1;        
}
static Ret socketSendImp(int fd,  u8 *data, usize *plen, int flags, Ret async) {
    usize len = *plen;
    CHECK(len > SSIZE_MAX);

    while(len) {
        ssize_t amt;
    CALL:
        amt = send(fd, data, len, flags);
        if(amt < 0) {
            if(errno == EINTR) { goto CALL; }
            else if (errno == EWOULDBLOCK || errno == EAGAIN) {
                if(async || len == *plen) { *plen = len; return 1; }
                else { 
                    flags &= ~MSG_DONTWAIT;
                    goto CALL;
                }
            }
            else { 
                DEBUG_LOG("socketSendImp has failed\n");
                goto FAILED;
            }
        }

        data += amt;
        len -= amt;
    }

    return 0;
FAILED:
    return -1;
}
static Ret parseBlockType(Ret *type, int *flag) {
    *flag = MSG_NOSIGNAL;
    switch(*type) {
        case NET_BLOCK_NONE:
            *flag |= MSG_DONTWAIT;
            *type = 1;
            break;
        case NET_BLOCK_POLL:
            *flag |= MSG_DONTWAIT;
            *type = 0;
            break;
        case NET_BLOCK_FULL:
            *type = 0;
            break;
        default:
            return -1;
    }
    return 0;
}
static Ret TCPSockSend(TCPSock *sock, u8 *src, usize len, Ret blockType) {
    int flag;
    CHECK(parseBlockType(&blockType, &flag));

    usize left = len - sock->sndPlace;
    Ret res = socketSendImp(sock->sock.fd, src + sock->sndPlace, &left, flag, blockType);
    if(res > 0) { sock->sndPlace = len - left; }
    else { sock->sndPlace = 0; }
    return res;

FAILED:
    sock->sndPlace = 0;
    return -1;
}
static Ret TCPSockSendP(TCPSock *sock,  u8 *src, usize len, Ret blockType) {
    int flag;
    CHECK(parseBlockType(&blockType, &flag));

    if(sock->sndPlace < sizeof(u64)) {
        u64 lenBytes = htobe64(len);
        usize left = sizeof(u64) - sock->sndPlace;
        Ret res = socketSendImp(sock->sock.fd, (u8 *R())&lenBytes + sock->sndPlace, &left, flag | MSG_MORE, blockType);
        if(res > 0) { sock->sndPlace = sizeof(u64) - left; return 1; }
        else if (res < 0) { return -1; }
        else { sock->sndPlace = sizeof(u64); } 
    }

    if(!blockType) { flag &= ~MSG_DONTWAIT; }
    usize left = len - (sock->sndPlace - sizeof(u64));
    Ret res = socketSendImp(sock->sock.fd, src + (sock->sndPlace - sizeof(u64)), &left, flag, blockType);
    if(res > 0) { sock->sndPlace = sizeof(u64) + len - left; }
    else { sock->sndPlace = 0; }
    return res;

FAILED:
    sock->sndPlace = 0;
    return -1;
}
static Ret socketRecvImp(int fd, u8 *data, usize *plen, int flags, Ret async) {
    usize len = *plen;
    CHECK(len > SSIZE_MAX);

    while(len) {
        ssize_t amt;

    CALL:
        amt = recv(fd, data, len, flags);
        if(amt < 1) {
            if(0 == amt) { goto FAILED; }
            else if(errno == EINTR) { goto CALL; }
            else if (errno == EWOULDBLOCK || errno == EAGAIN) {
                if(async || len == *plen) { *plen = len; return 1; }
                else { 
                    flags &= ~MSG_DONTWAIT;
                    goto CALL;
                }
            }
            else { 
                DEBUG_LOG("socketRecvImp has failed\n");
                goto FAILED;
            }
        }

        data += amt;
        len -= amt;
    }

    return 0;

FAILED:
    return -1;
}
static Ret TCPSockRecv(TCPSock *sock, u8 *dst, usize len, Ret blockType) {
    int flag;
    CHECK(parseBlockType(&blockType, &flag));
    usize left = len - sock->rcvPlace;

    Ret res = socketRecvImp(sock->sock.fd, dst + sock->rcvPlace, &left, flag, blockType);
    if(res > 0) { sock->rcvPlace = len - left; }
    else { sock->rcvPlace = 0; }

    return res;

FAILED:
    sock->rcvPlace = 0;
    return -1;
}
static Ret TCPSockProxyInit(TCPSock *sock, char *R() serverIP, char *R() serverPort,
                      char *R() proxyIP, char *R() proxyPort)
{
    CHECK(TCPSockInit(sock, proxyIP, proxyPort));

    u8 buf[262];
    usize bufLen;
    usize serverIPLen;
    CHECK( (serverIPLen = strlen(serverIP)) > 255);
    u16 serverPortNumber;
    CHECK(0 == (serverPortNumber = htons((u16)strtoul(serverPort, 0,PORT_STRING_BASE))));

    // Greet proxy
    buf[0] = 5; // Socks version 5
    buf[1] = 1; // 1 auth method excepted
    buf[2] = 0; // no auth
    bufLen = 3;
    CHECK(TCPSockSend(sock, buf, bufLen, NET_BLOCK_FULL));

    // Read response to greeting
    bufLen = 2;
    CHECK(TCPSockRecv(sock, buf, bufLen, NET_BLOCK_FULL));
    CHECK(buf[0] != 0x5); // proxy is not socks v5
    CHECK(buf[1] == 0xFF); // proxy did not accept no auth method

    // Send connection request
    buf[0] = 5; // version 5
    buf[1] = 1; // command code: tcp/ip
    buf[2] = 0; // reserved 0
    buf[3] = 3; // domain name // 1 is ipv4, 4 is ipv6
    buf[4] = serverIPLen;
    bufLen = 5;
    memcpy(&buf[bufLen], serverIP, serverIPLen); bufLen += serverIPLen;
    memcpy(&buf[bufLen], &serverPortNumber, sizeof(serverPortNumber)); bufLen += sizeof(serverPortNumber);
    CHECK(TCPSockSend(sock, buf, bufLen, NET_BLOCK_FULL));

    // Proxy's response
    bufLen = 4;
    CHECK(TCPSockRecv(sock, buf, bufLen, NET_BLOCK_FULL));
    CHECK(buf[0] != 5); // proxy's response version doesn't match up
    CHECK(buf[1]); // status code signals an error:
        //0x00: request granted
        //0x01: general failure
        //0x02: connection not allowed by ruleset
        //0x03: network unreachable
        //0x04: host unreachable
        //0x05: connection refused by destination host
        //0x06: TTL expired
        //0x07: command not supported / protocol error
        //0x08: address type not supported
    CHECK(buf[2]); // reserved 0 Ret isn't 0 ?

    // Read in and throw away address
    switch(buf[3]) {
        case 1:
            // IPv4
            bufLen = 4;
            CHECK(TCPSockRecv(sock, buf, bufLen, NET_BLOCK_FULL));
            break;
        case 3:
            // Name
            bufLen = 1;
            CHECK(TCPSockRecv(sock, buf, bufLen, NET_BLOCK_FULL));
            bufLen = buf[0];
            CHECK(TCPSockRecv(sock, buf, bufLen, NET_BLOCK_FULL));
            break;
        case 4:
            // IPv6
            bufLen = 16;
            CHECK(TCPSockRecv(sock, buf, bufLen, NET_BLOCK_FULL));
            break;
        default:
            DEBUG_LOG("Proxy gave a wierd type return for address");
            goto FAILED;
    }
    
    // throw away port
    bufLen = 2;
    CHECK(TCPSockRecv(sock, buf, bufLen, NET_BLOCK_FULL));
    
    return 0;

FAILED:
    TCPSockDe(sock);
    memset(sock, 0, sizeof(*sock));
    return -1;
}

#define TCPSOCK_DEF_FNS(TYPE)                                           \
static Ret TYPE##TCPRecvP(TYPE *dst, TCPSock *sock, Ret blockType) {    \
    int flag;                                           \
    CHECK(parseBlockType(&blockType, &flag));           \
                                                        \
    if(sock->rcvPlace < sizeof(u64)) {                  \
        CHECK(TYPE##Reserve(dst, sizeof(u64)));         \
        usize left = sizeof(u64) - sock->rcvPlace;      \
        Ret res = socketRecvImp(sock->sock.fd, TYPE##Data(dst) + sock->rcvPlace, &left, flag | MSG_MORE, blockType);    \
        if(res > 0) { sock->rcvPlace = sizeof(u64) - left; return 1; }          \
        else if (res < 0) { return -1; }                                        \
        else { sock->rcvPlace = sizeof(u64); }                                  \
        *(u64 *)TYPE##Data(dst) = be64toh(*(u64 *)TYPE##Data(dst));             \
        CHECK(*(u64 *)TYPE##Data(dst) > MIN(SSIZE_MAX, NETWORK_MAX_RECV_SIZE)); \
        CHECK(TYPE##Reserve(dst, *(u64 *)TYPE##Data(dst)));                     \
        TYPE##SetSize(dst, *(u64 *)TYPE##Data(dst));    \
    }                                                   \
                                                        \
    if(!blockType) { flag &= ~MSG_DONTWAIT; }           \
    usize left = TYPE##Size(dst) - (sock->rcvPlace - sizeof(u64));  \
    Ret res = socketRecvImp(sock->sock.fd, TYPE##Data(dst) + (sock->rcvPlace - sizeof(u64)), &left, flag, blockType);   \
    if(res > 0) { sock->rcvPlace = sizeof(u64) + TYPE##Size(dst) - left; }  \
    else { sock->rcvPlace = 0; }                        \
    return res;                                         \
                                                        \
FAILED:                                                 \
    sock->rcvPlace = 0;                                 \
    return -1;                                          \
}

#define TCPSOCK_DD_FNS(TYPE) \
TCPSOCK_DEC_FNS(TYPE);       \
TCPSOCK_DEF_FNS(TYPE);       \
