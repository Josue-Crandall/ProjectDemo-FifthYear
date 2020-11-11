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

#ifndef JC_NET_H
#define JC_NET_H
#include "../macros/macros.h"

#include <errno.h> // EINTR
#include <unistd.h> // close
#include <sys/socket.h>
//#include <netinet/in.h>
#include <netinet/ip.h> // "superset of previous"

#include "../api/api.h"

#define TCP_BACK_LOG_SIZE               128ULL
// Note: 0 is no timeout
#define NETSOCK_DEFAULT_TIMEOUT         15ULL
// Note: not used by net.h functions
#define NETWORK_CHUNK_SIZE              (512ULL)
#define NETWORK_MAX_RECV_SIZE           (8589935000ULL) // 8 gibybyte
#define NETSOCK_SET_CLOSE_ON_EXEC       1 // Note: undefine to stop

#define MAX_UDP_PACKET_SIZE 508 // Rets 576 - 60Ret ip header, 8Ret-udp header
                                // Note for me: ipv6 is 1500 intead of 576, and ipsec/additional headers can kill 508 sized udp packets
static_assert(USIZE_MAX >= MAX_UDP_PACKET_SIZE, "Thats a cool platform you got there.");

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr_in TCPAddr;
typedef struct sockaddr_in UDPAddr;

typedef struct { int fd; Ret alive; } NetSock;
typedef NetSock TCPLis;
typedef NetSock UDPLis;
typedef NetSock UDPSock;
typedef struct { NetSock sock; usize sndPlace, rcvPlace; } TCPSock;

// WARNING: calling network functions after EWOULDBLOCK with distinct parameters
//  before finishing is undefined behavior
// Question: what does MSG_NOSIGNAL do on UDP sends/recv's 0.o

// Note: Fully async
#define NET_BLOCK_NONE      0
// Note: Atomically block, but otherwise async. Useful for false poll wakeups.
#define NET_BLOCK_POLL      1
// Note: Block, only return on timeout and failure.
#define NET_BLOCK_FULL      2

static Ret TCPSockInit(TCPSock *sock, char *R() serverIP, char *R() serverPort);
// Socks5proxy
static Ret TCPSockProxyInit(TCPSock *sock, char *R() serverIP, char *R() serverPort,
                      char *R() proxyIP, char *R() proxyPort);
// Note: Return values for TCPSock network functions:
// 0: message sent, -1: error, including connection closed, 1: EWOULDBLOCK || EAGAIN
// Note: SSIZE_MAX is the largest size for TCP network calls
static Ret TCPSockSend(TCPSock *sock, u8 *src, usize len, Ret blockType);
static Ret TCPSockRecv(TCPSock *sock, u8 *dst, usize len, Ret blockType);
// Note: Send Packet
static Ret TCPSockSendP(TCPSock *sock,  u8 *src, usize len, Ret blockType);
#define TCPSOCK_DEC_FNS(TYPE)                                           \
static Ret TYPE##TCPRecvP(TYPE *buff, TCPSock *sock, Ret blockType);
// Instantiated for Buff and Smem

static int TCPSockGetFD(TCPSock *sock);
static void TCPSockDe(TCPSock *sock);

// bindIP: may be NULL, bindPort may be NULL
static Ret TCPLisInit(TCPLis *lis,  char *R() bindIP,  char *R() bindPort);
// Return values: 0: client accepted, 1: EWOULDBLOCK || EAGAIN, -1: error
static Ret TCPLisAccept(TCPLis *lis, TCPSock *out, Ret blocking);
static int TCPLisGetFD(TCPLis *lis);
static void TCPLisDe(TCPLis *lis);

// bindIP: may be NULL, bindPort may be NULL
static Ret UDPLisInit(UDPLis *lis,  char *R() bindIP,  char *R() bindPort);
// Note: Return values for UDP network functions
// 0: message sent, -1: error, including connection closed, 1: EWOULDBLOCK || EAGAIN
// Note: MAX_UDP_PACKET_SIZE max for len for UDP functions
// sender: May be null
static usize UDPLisRecv(UDPLis *lis, u8 dst[MAX_UDP_PACKET_SIZE], UDPAddr *sender, Ret blocking);
// Returns: amount of bytes left unsent
static usize UDPLisSend(UDPLis *lis, u8 *src, usize len,  UDPAddr *dst);
static void UDPLisDe(UDPLis *lis);

static Ret UDPSockInit(UDPSock *sock,  char *R() bindIP,  char *R() bindPort);
// Note: Return values for UDP network functions
// 0: message sent, -1: error, including connection closed, 1: EWOULDBLOCK || EAGAIN
// Note: MAX_UDP_PACKET_SIZE max for len for UDP functions
// Returns: amount of bytes left unsent
static usize UDPSockSend(UDPSock *sock, u8 *src, usize len);
// Note: MAX_UDP_PACKET_SIZE max for len
static usize UDPSockRecv(UDPSock *sock, u8 dst[MAX_UDP_PACKET_SIZE], Ret blocking);
static void UDPSockDe(UDPSock *sock);

//// Misc.
static u16 getSockBindPort(int sockFD);

#include "imp.h"

TCPSOCK_DD_FNS(Buff);
TCPSOCK_DD_FNS(Smem);

#endif
