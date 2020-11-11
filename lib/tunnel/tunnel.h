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

#ifndef JC_TUNNEL_H
#define JC_TUNNEL_H
#include "../macros/macros.h"

#define DEC_TUNNEL(NAME,BUFF_T)                                                     \
                                                                                    \
typedef struct NAME {                                                               \
    TCPSock *sock; void *R() arg;                                                   \
    u64 sndPlace, rcvPlace;                                                         \
    BUFF_T sndBuff, rcvBuff;                                                        \
} NAME;                                                                             \
                                                                                    \
static Ret NAME##Init(NAME *tunnel, TCPSock *sock, void * arg);                     \
static Ret NAME##SendP(NAME *tunnel, u8 *src, u64 len, Ret blockType);              \
static Ret NAME##RecvP(NAME *tunnel, BUFF_T *dst, Ret blockType);                   \
static void NAME##De(NAME *tunnel);                                                 \

#include "imp.h"

#define DD_TUNNEL(NAME,BUFF_T,SEND_CB, SEND_FIN,RECV_CB,CB_BYTES)                   \
DEC_TUNNEL(NAME,BUFF_T);                                                            \
DEF_TUNNEL(NAME,BUFF_T,SEND_CB, SEND_FIN,RECV_CB,CB_BYTES);                         \

#endif