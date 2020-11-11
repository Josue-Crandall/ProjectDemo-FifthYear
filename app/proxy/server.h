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

static Ret performHandshake() {
    while(1) {
        Ret res;
        NEG_CHECK(res = SprotClientHandshake(sprot, serverPK, clientSK, sws));
        if(res == SPROT_HS_SEND) {
            TCPSockSend(toServer, SprotData(sprot), SprotSize(sprot), NET_BLOCK_FULL);
        }
        else if(res == SPROT_HS_RECV) {
            TCPSockRecv(toServer, SprotData(sprot), SprotSize(sprot), NET_BLOCK_FULL);
        }
        else if(res == SPROT_HS_DONE) { return 0; }
        else { DEBUG_LOG("Strange return from SprotClientHandshake\n"); goto FAILED; }
    }
FAILED:
    return -1;
}
static Ret sendLogin() {
    CHECK(BuffReserve(ws, NETWORK_CHUNK_SIZE));
    u8 *R() loginPacket = BuffData(ws);
    *(u64 *)loginPacket = htobe64(atoll(channel));
    BuffSetSize(ws, sizeof(u64));
    CHECK(BuffSPEncrypt(ws, sprot, ws2));
    static_assert(NETWORK_CHUNK_SIZE >= SPROT_BYTES + sizeof(u64), "single send");
    randombytes(loginPacket + sizeof(u64) + SPROT_BYTES, NETWORK_CHUNK_SIZE - SPROT_BYTES - sizeof(u64));
    //DEBUG_WINDOW(loginPacket, NETWORK_CHUNK_SIZE); // manual check for loose 0's
    CHECK(TCPSockSend(toServer, loginPacket, NETWORK_CHUNK_SIZE, NET_BLOCK_FULL));
    SPEncryptFinish(sprot, SmemData(sws));
    return 0;
FAILED:
    return -1;
}
static void serverInit(void) {
    if(useProxy) {
        if(TCPSockProxyInit(toServer, serverIP, serverPort, proxyIP, proxyPort)) {
            exitFailure("Failed to connect to server", serverIP, "via proxy");
        }        
    }
    else {
        if(!useProxy && TCPSockInit(toServer, serverIP, serverPort)) {
            exitFailure("Failed to connect to server", serverIP, "");
        }   
    }
    if(performHandshake()) { exitFailure("Failed to", "handshake", "with server"); }
    if(sendLogin()) { exitFailure("Failed to", "login", "to server"); }
    if(TunnelInit(tunnel, toServer, 0)) { exitFailure("Failed to init", "tunnel", ""); }
}

static Ret TunnelSCB(Buff *msg, void *arg) {
    void *ign = arg;
    return BuffSPEncrypt(msg, sprot, ws);
}
static void TunnelFCB(void *arg) {
    void *ign = arg;
    SPEncryptFinish(sprot, SmemData(sws));
}
static Ret TunnelRCB(Buff *msg, void *arg) {
    void *ign = arg;
    return BuffSPDecrypt(msg, sprot, ws, SmemData(sws));
}
