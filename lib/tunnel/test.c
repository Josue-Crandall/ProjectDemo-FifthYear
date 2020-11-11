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

#include "tunnel.h"

#include "../nacl/nacl.h"
#include "../alloc/alloc.h"
#include "../file/file.h"
#include "../api/api.h"
#include "../net/net.h"

#include <string.h>

static u8 msg[41079];
static u8 key[NACLS_KEY_BYTES];
static Buff wsthing;
static Buff *ws = &wsthing;
static TCPSock s1, s2;
static TCPSock *sock1 = &s1, *sock2 = &s2;

Ret sndCB(Buff *buff, void *arg) { 
    Buff *ign = buff; void *ign2 = arg;
    return BuffSEncrypt(buff, key, ws);
}
void sndFin(void *arg) { void * ign = arg; }
Ret rcvCB(Buff *buff, void *arg) { 
    Buff *ign = buff; void *ign2 = arg; 
    return BuffSDecrypt(buff, key, ws); 
}
DD_TUNNEL(Tunnel, Buff, sndCB, sndFin, rcvCB, NACLS_BYTES);
Ret t0() {
    PRAI(Tunnel, t);
    PRAI(Tunnel, t2);
    PRAI(Buff, dst);
    CHECK(TunnelInit(t, sock1, 0));
    CHECK(TunnelInit(t2, sock2, 0));
    CHECK(BuffInit(dst, 0));

    CHECK(TunnelSendP(t, msg, sizeof(msg), NET_BLOCK_FULL));
    CHECK(TunnelRecvP(t2, dst, NET_BLOCK_FULL));
    CHECK(sizeof(msg) != BuffSize(dst));
    CHECK(memcmp(BuffData(dst), msg, sizeof(msg)));

    CHECK(TunnelSendP(t2, msg, sizeof(msg), NET_BLOCK_FULL));
    CHECK(TunnelRecvP(t, dst, NET_BLOCK_FULL));
    CHECK(sizeof(msg) != BuffSize(dst));
    CHECK(memcmp(BuffData(dst), msg, sizeof(msg)));

    Ret ret = 0;
CLEAN:
    BuffDe(dst);
    TunnelDe(t2);
    TunnelDe(t);
    return ret;
FAILED:
    ret = -1;
    goto CLEAN;
}

#define NETWORK_TEST_PORT "55555"
int main(void) {
    TCPLis temp = {0};

    randombytes(msg, sizeof(msg));

    CHECK(BuffInit(ws, 0));
    randombytes(key, sizeof(key));

    CHECK(TCPLisInit(&temp, 0, NETWORK_TEST_PORT));
    CHECK(TCPSockInit(&s1, "127.0.0.1", NETWORK_TEST_PORT));
    CHECK(TCPLisAccept(&temp, &s2, 1));


    CHECK(t0());
    printf("pass\n");
    Ret ret = 0;
CLEAN:
    TCPSockDe(&s1);
    TCPSockDe(&s2);
    TCPLisDe(&temp);
    BuffDe(ws);
    return ret;
FAILED:
    printf(" --- FAILED ---\n");
    ret = -1;
    goto CLEAN;
}

