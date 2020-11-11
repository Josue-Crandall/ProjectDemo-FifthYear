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


#include "../net/net.h"
#include "../api/api.h"

#include <endian.h>

#define DEF_TUNNEL(NAME,BUFF_T,SEND_CB, SEND_FIN,RECV_CB,CB_BYTES)                  \
static_assert(sizeof(u64) < sizeof(u64) + (usize)CB_BYTES, "overflow check");       \
static_assert(sizeof(u64) + (usize)CB_BYTES < sizeof(u64) + (usize)CB_BYTES + 1 , "overflow check");   \
static_assert(NETWORK_CHUNK_SIZE < USIZE_MAX, "overflow check");                    \
static_assert(sizeof(u64) + (usize)CB_BYTES + 1 < (usize)NETWORK_CHUNK_SIZE, "Must be able to send at least one byte");   \
                                                                                    \
static Ret NAME##Init(NAME *tunnel, TCPSock *sock, void * arg) {                    \
    memset(tunnel, 0, sizeof(*tunnel));                                             \
    tunnel->sock = sock; tunnel->arg = arg;                                         \
    CHECK(BUFF_T##Init(&tunnel->sndBuff, NETWORK_CHUNK_SIZE));                      \
    CHECK(BUFF_T##Init(&tunnel->rcvBuff, NETWORK_CHUNK_SIZE));                      \
    return 0;                                                                       \
FAILED:                                                                             \
    memset(tunnel, 0, sizeof(*tunnel));                                             \
    return 0;                                                                       \
}                                                                                   \
static void NAME##De(NAME *tunnel) {                                                \
    BUFF_T##De(&tunnel->sndBuff);                                                   \
    BUFF_T##De(&tunnel->rcvBuff);                                                   \
}                                                                                   \
static Ret NAME##SendPFirstFill(NAME *tunnel, u8 *src, u64 len) {                   \
    u8 *sndBuffData = BUFF_T##Data(&tunnel->sndBuff);                               \
    *(u64 *)sndBuffData = htobe64(len);                                             \
                                                                                    \
    u64 leftToFill = NETWORK_CHUNK_SIZE - sizeof(u64) - CB_BYTES;                   \
    if(len >= leftToFill) {                                                         \
        memcpy(sndBuffData + sizeof(u64), src, leftToFill);                         \
        tunnel->sndPlace = leftToFill;                                              \
    }                                                                               \
    else {                                                                          \
        memcpy(sndBuffData + sizeof(u64), src, len);                                \
        devURandom(sndBuffData + sizeof(u64) + len, leftToFill - len);              \
        tunnel->sndPlace = len;                                                     \
    }                                                                               \
                                                                                    \
    BUFF_T##SetSize(&tunnel->sndBuff, NETWORK_CHUNK_SIZE - CB_BYTES);               \
    CHECK(SEND_CB(&tunnel->sndBuff, tunnel->arg));                                  \
                                                                                    \
    return 0;                                                                       \
FAILED:                                                                             \
    return -1;                                                                      \
}                                                                                   \
static Ret NAME##SendPRefill(NAME *tunnel, u8 *src, u64 len) {                      \
    u8 *sndBuffData = BUFF_T##Data(&tunnel->sndBuff);                               \
    u64 leftToFill = NETWORK_CHUNK_SIZE - CB_BYTES;                                 \
    u64 leftInSrc = len - tunnel->sndPlace;                                         \
                                                                                    \
    if(leftToFill > leftInSrc) {                                                    \
        memcpy(sndBuffData, src + tunnel->sndPlace, leftInSrc);                     \
        devURandom(sndBuffData + leftInSrc, leftToFill - leftInSrc);                \
        tunnel->sndPlace = len;                                                     \
    }                                                                               \
    else {                                                                          \
        memcpy(sndBuffData, src + tunnel->sndPlace, leftToFill);                    \
        tunnel->sndPlace += leftToFill;                                             \
    }                                                                               \
                                                                                    \
    BUFF_T##SetSize(&tunnel->sndBuff, NETWORK_CHUNK_SIZE - CB_BYTES);               \
    CHECK(SEND_CB(&tunnel->sndBuff, tunnel->arg));                                  \
                                                                                    \
    return 0;                                                                       \
FAILED:                                                                             \
    return -1;                                                                      \
}                                                                                   \
static Ret NAME##SendP(NAME *tunnel, u8 *src, u64 len, Ret blockType) {             \
    if(0 == tunnel->sndPlace) { CHECK(NAME##SendPFirstFill(tunnel, src, len)); }    \
                                                                                    \
    Ret res;                                                                        \
SEND:                                                                               \
    res = TCPSockSend(tunnel->sock, BUFF_T##Data(&tunnel->sndBuff), NETWORK_CHUNK_SIZE, blockType); \
    if(res) {  return res; }                                                        \
    else {                                                                          \
        SEND_FIN(tunnel->arg);                                                      \
        if(tunnel->sndPlace != len) {                                               \
            CHECK(NAME##SendPRefill(tunnel, src, len));                             \
            goto SEND;                                                              \
        }                                                                           \
    }                                                                               \
                                                                                    \
    Ret err = 0;                                                                    \
CLEAN:                                                                              \
    tunnel->sndPlace = 0;                                                           \
    return err;                                                                     \
FAILED:                                                                             \
    err = -1;                                                                       \
    goto CLEAN;                                                                     \
}                                                                                   \
static Ret NAME##parseRecvLen(NAME *tunnel, BUFF_T *dst) {                          \
    u64 dstLen;                                                                     \
                                                                                    \
    u8 *rcvData = BUFF_T##Data(&tunnel->rcvBuff);                                   \
    CHECK( (dstLen = be64toh(*(u64 *)rcvData)) > MIN(SSIZE_MAX, NETWORK_MAX_RECV_SIZE));  \
    rcvData += sizeof(u64);                                                         \
                                                                                    \
    CHECK(BUFF_T##Reserve(dst, dstLen));                                            \
    BUFF_T##SetSize(dst, dstLen);                                                   \
    u8 *dstData = BUFF_T##Data(dst);                                                \
                                                                                    \
    u64 bytesInRecvBuff = NETWORK_CHUNK_SIZE - CB_BYTES - sizeof(u64);              \
                                                                                    \
    if(bytesInRecvBuff > dstLen) {                                                  \
        memcpy(dstData, rcvData, dstLen);                                           \
        tunnel->rcvPlace = dstLen;                                                  \
    }                                                                               \
    else {                                                                          \
        memcpy(dstData, rcvData, bytesInRecvBuff);                                  \
        tunnel->rcvPlace = bytesInRecvBuff;                                         \
    }                                                                               \
                                                                                    \
    return 0;                                                                       \
FAILED:                                                                             \
    return -1;                                                                      \
}                                                                                   \
static Ret NAME##parseRecv(NAME *tunnel, BUFF_T *dst) {                             \
    u64 bytesInRcvBuf = NETWORK_CHUNK_SIZE - CB_BYTES;                              \
    usize dstSize = BUFF_T##Size(dst);                                              \
    u64 bytesLeftToTransfer = dstSize - tunnel->rcvPlace;                           \
    u8 *dstData = BUFF_T##Data(dst) + tunnel->rcvPlace;                             \
    u8 *rcvData = BUFF_T##Data(&tunnel->rcvBuff);                                   \
                                                                                    \
    if(bytesInRcvBuf >= bytesLeftToTransfer) {                                      \
        memcpy(dstData, rcvData, bytesLeftToTransfer);                              \
        tunnel->rcvPlace = dstSize;                                                 \
    }                                                                               \
    else {                                                                          \
        memcpy(dstData, rcvData, bytesInRcvBuf);                                    \
        tunnel->rcvPlace += bytesInRcvBuf;                                          \
    }                                                                               \
                                                                                    \
    return 0;                                                                       \
FAILED:                                                                             \
    return -1;                                                                      \
}                                                                                   \
static Ret NAME##RecvP(NAME *tunnel, BUFF_T *dst, Ret blockType) {                  \
    Ret err;                                                                        \
RECV:                                                                               \
    err = TCPSockRecv(tunnel->sock, BUFF_T##Data(&tunnel->rcvBuff), NETWORK_CHUNK_SIZE, blockType); \
    if(err) { return err; }                                                         \
    else {                                                                          \
        BUFF_T##SetSize(&tunnel->rcvBuff, NETWORK_CHUNK_SIZE);                      \
        CHECK(RECV_CB(&tunnel->rcvBuff, tunnel->arg));                              \
    }                                                                               \
                                                                                    \
    if(0 == tunnel->rcvPlace) { CHECK(NAME##parseRecvLen(tunnel, dst)); }           \
    else { CHECK(NAME##parseRecv(tunnel, dst)); }                                   \
                                                                                    \
    if(BUFF_T##Size(dst) != tunnel->rcvPlace) { goto RECV; }                        \
                                                                                    \
CLEAN:                                                                              \
    tunnel->rcvPlace = 0;                                                           \
    return err;                                                                     \
FAILED:                                                                             \
    BUFF_T##Clear(dst);                                                             \
    err = -1;                                                                       \
    goto CLEAN;                                                                     \
}                                                                                   \
