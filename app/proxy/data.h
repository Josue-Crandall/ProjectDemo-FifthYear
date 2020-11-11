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

//// Conf vars
#define CONF_PATH "proxy.conf"
#define SERVER_CB_LOG '1'
#define SERVER_CB_NOLOG '0'

static char *serverIP, *serverPort;
static Ret useProxy;
static char *proxyIP, *proxyPort;
static u8 *serverPK, *clientSK;
static char *channel;
static char *lisIP, *lisPort;
static char *e2ePath, *rprotPath, *oprotPath;
////

//// Data vars
SPRAI(Conf, conf);
SPRAI(TCPSock, toServer);
SPRAI(TCPSock, toClient);
SPRAI(TCPLis, lis);
SPRAI(Smem, pk);
SPRAI(Smem, sk);
SPRAI(Smem, sws);
SPRAI(Sprot, sprot);
SPRAI(Buff, ws);
SPRAI(Buff, ws2);
SPRAI(Poller, mpoll);
SPRAI(Smem, e2eKey);
SPRAI(Oprot, oprot);
SPRAI(Rprot, rprot);

static Ret TunnelSCB(Buff *msg, void *arg);
static void TunnelFCB(void *arg);
static Ret TunnelRCB(Buff *msg, void *arg);
DD_TUNNEL(Tunnel, Buff, TunnelSCB, TunnelFCB, TunnelRCB, SPROT_BYTES);
SPRAI(Tunnel, tunnel);

#ifdef JCDEBUG
    #define SMEM_JCDEBUG
#endif
#ifdef SMEM_JCDEBUG
    static Smem DEBUG_SMEM_HELPER;
#endif

static void deInitData(void) {
    ConfDe(conf);
    TCPSockDe(toServer);
    TCPSockDe(toClient);
    TCPLisDe(lis);
    SmemDe(sws); SmemDe(pk); SmemDe(sk);
    PollerDe(mpoll);
    SprotDe(sprot);
    TunnelDe(tunnel);
    SmemDe(e2eKey);
    RprotDe(rprot, rprotPath);
    OprotDe(oprot);

#ifdef SMEM_JCDEBUG
    SmemDe(&DEBUG_SMEM_HELPER);
#endif
}

static void dataInit(void) {
#ifdef SMEM_JCDEBUG
    if(SmemInit(&DEBUG_SMEM_HELPER, 0)) { exit(-1); }
#endif

    if(atexit(deInitData)) { exitFailure("Failed to", "set exit handler", ""); }
    if(SmemInit(sws, MAX(NACLH_BYTES,MAX(NACLA_PUB_BYTES, NACLA_SEC_BYTES)))) { exitFailure("Failed to allocate ", "mlocked workspace", "");}
    if(SmemInit(pk, NACLA_PUB_BYTES)) { exitFailure("Failed to allocate", "mlocked workspace", ""); }
    if(SmemInit(sk, NACLA_SEC_BYTES)) { exitFailure("Failed to allocate", "mlocked workspace", ""); }
    if(SprotInit(sprot)) { exitFailure("Failed to init", "server protocol data", ""); }
    if(BuffInit(ws, 0)) { exitFailure("Failed to init", "workspace", ""); }
    if(BuffInit(ws2, 0)) { exitFailure("Failed to init", "second workspace", ""); }
    if(PollerInit(mpoll, -1)) { exitFailure("Failed to init", "poller", ""); }
    if(SmemInit(e2eKey, NACLS_KEY_BYTES)) { exitFailure("Failed to init", "e2eKey buffer", ""); }
}
