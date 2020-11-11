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

//// Conf
#define CONF_PATH "server.conf"
#define QUE_TIMEOUT_MILISECONDS (1000 * 60 * 60 * 4) // aprox 4 hours
#define MSG_DURATION_SECONDS (60 * 60 * 24 * 8) // > 1wk

static char *serverIP, *serverPort;
static u8 *clientPK, *serverSK;

SPRAI(Smem, pkData);
SPRAI(Smem, skData);
SPRAI(Smem, confSWS);

////

SPRAI(Conf, conf);
SPRAI(TCPLis, lis);
SPRAI(WorkQ, que);
static Ret serverSetup;

typedef struct ClientTask {
    QTask qtask;
    PWhen trigs[2];
#define CLIENT_STATE_DEFAULT 0
#define CLIENT_STATE_SHAKE   1
#define CLIENT_STATE_SEND    2
#define CLIENT_STATE_RECV    3
#define CLIENT_STATE_DONE    4
    Ret state;
    TCPSock sock;
    Sprot sprot;
    Smem sws;
    Buff rcvBuff, ws;
    u64 channel;
} ClientTask;
static Ret TunnelSCB(Buff *msg, void *arg);
static void TunnelFCB(void *arg);
static Ret TunnelRCB(Buff *msg, void *arg);
DD_TUNNEL(Tunnel, Buff, TunnelSCB, TunnelFCB, TunnelRCB, SPROT_BYTES);
typedef struct Client {
#define CLIENT_CSTATE_DEFAULT 5
#define CLIENT_CSTATE_ONBOARD 6
#define CLIENT_CSTATE_DEAD    7
#define CLIENT_CSTATE_SNDBLK  8
    Ret state;
    TCPSock sock;
    Sprot sprot;
    Buff rcvBuff;
    Tunnel tunnel;
    usize logPlace, id;
    Buff *ws;
    Smem *sws;
} Client;
static void ClientDe(Client *client);
typedef struct Msg {
    u8 *data;
    u64 len, sender;
    time_t ts;
} Msg;
static void MsgDe(Msg *msg);
DD_VEC(MsgVec, Msg, MsgDe(&val), STD_ALLOC);
DD_VEC(PTrigVec, PWhen, NOP, STD_ALLOC);
DD_VEC(ClientVec, Client *, ClientDe(val), STD_ALLOC);
typedef struct Channel {
    QTask qtask;
    u64 id;
    Pipe pip;
    PTrigVec trigs;
    ClientVec clients;
    MsgVec log;
    usize idTicker;
    Buff ws;
    Smem sws;
} Channel;
static void killChannel(u64 id); // called by channel
static void ChannelDe(Channel *channel); // internal to table
DD_TABLE(CTable, u64, Channel *, res = (usize)key, res = lhs == rhs, NOP, ChannelDe(val), STD_ALLOC);
static CTable tableData, *table = &tableData;
static PMutex tableMtxData, *tableMtx = &tableMtxData;

#ifdef JCDEBUG
    #define SMEM_JCDEBUG
#endif
#ifdef SMEM_JCDEBUG
    static Smem DEBUG_SMEM_HELPER;
#endif

static void deInitData(void) {
    TCPLisDe(lis);
    ConfDe(conf);
    SmemDe(pkData);
    SmemDe(skData);
    SmemDe(confSWS);
    CTableDe(table);
    PMutexDe(tableMtx);

    if(!serverSetup) {  WorkQStop(que); WorkQDe(que); }

#ifdef SMEM_JCDEBUG
    SmemDe(&DEBUG_SMEM_HELPER);
#endif
}
static void dataInit(void) {
#ifdef SMEM_JCDEBUG
    if(SmemInit(&DEBUG_SMEM_HELPER, 0)) { exit(-1); }
#endif

    if(atexit(deInitData)) { exitFailure("Failed to", "set exit handler", ""); }

    if(WorkQInit(que, QUE_TIMEOUT_MILISECONDS)) { exitFailure("Failed ", "WorkQInit", ""); }
}
