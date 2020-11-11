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

static Ret lastMsgWasMutable;
static Ret e2eEncrypt(Buff *msg) {
    usize ptLen = BuffSize(msg);
    
    // Must have a server control byte
    CHECK(0 == ptLen);

    // spare cb from e2e encryption
    Ret cb = BuffPop(msg);

    if(SERVER_CB_LOG == cb) {
        lastMsgWasMutable = 1;
        if(oprotPath) { CHECK(BuffOPEncrypt(msg, oprot, ws, SmemData(sws))); }
        if(rprotPath) { CHECK(BuffRPEncrypt(msg, rprot, ws, sws)); }
    }
    else {
        lastMsgWasMutable = 0;
        if(oprotPath) { CHECK(BuffOPEncryptUnmut(msg, oprot, ws)); }
        if(rprotPath) { CHECK(BuffRPEncryptUnmut(msg, rprot, ws)); }
    }
    
    if(e2ePath) { CHECK(BuffSEncrypt(msg, SmemData(e2eKey), ws)); }

    CHECK(BuffPush(msg, (u8)cb));

    return 0;
FAILED:
    return -1;
}
static void e2eEncryptFinish() {
    if(rprotPath && lastMsgWasMutable) { RPEncryptFinish(rprot, SmemData(sws)); }
}
static Ret e2eDecrypt(Buff *msg) {
    if(e2ePath) { CHECK(BuffSDecrypt(msg, SmemData(e2eKey), ws)); }
    if(rprotPath) { CHECK(BuffRPDecrypt(msg, rprot, ws, sws)); }
    if(oprotPath) { CHECK(BuffOPDecrypt(msg, oprot, ws, SmemData(sws))); }
    return 0;
FAILED:
    return -1;
}
static Ret readFromClientCB() {
    Ret res;

    NEG_CHECK(res = BuffTCPRecvP(ws2, toClient, NET_BLOCK_POLL));
    if(res) { return 1; }
    CHECK(e2eEncrypt(ws2));
    CHECK(TunnelSendP(tunnel, BuffData(ws2), BuffSize(ws2), NET_BLOCK_FULL));
    e2eEncryptFinish();

    return 0;
FAILED:
    return -1;
}
static Ret readFromServerCB() {
    Ret res;

    NEG_CHECK(res = TunnelRecvP(tunnel, ws2, NET_BLOCK_POLL));
    if(res) { return 1; }
    if(e2eDecrypt(ws2)) { fprintf(stderr, "WARNING: auth failure on msg sent by server, discarding\n"); }
    else { CHECK(TCPSockSendP(toClient, BuffData(ws2), BuffSize(ws2), NET_BLOCK_FULL)); }

    return 0;
FAILED:
    return -1;
}
static Ret taskCB(PTask **task) {
    void *ign = task;
    while(1) { Ret res; NEG_CHECK(res = readFromClientCB()); if(res) { break; } }
    while(1) { Ret res; NEG_CHECK(res = readFromServerCB()); if(res) { break; } }
    return PTASK_RET_REPOLL;
FAILED:
    return PTASK_RET_ERROR;
}
static PWhen taskTrigs[3]; PWhen *taskTrig(PTask **task) { void *ign = task; return taskTrigs; }
static PTask taskFace = { &taskCB, &taskTrig, NULL };
static PTask *taskPtr = &taskFace;
static void taskInit() {
    taskTrigs[0].fd = TCPSockGetFD(toServer); taskTrigs[0].when = PWHEN_IN;
    taskTrigs[1].fd = TCPSockGetFD(toClient); taskTrigs[1].when = PWHEN_IN;
    if(PollerPush(mpoll, &taskPtr)) { exitFailure("Failed to set", "proxy task", "poll"); }
}