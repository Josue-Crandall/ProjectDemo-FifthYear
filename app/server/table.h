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

static void ClientDe(Client *client) {
    if(client) {
        BuffDe(&client->rcvBuff);
        SprotDe(&client->sprot);
        TCPSockDe(&client->sock);
        TunnelDe(&client->tunnel);
        free(client);
    }
}
static void killChannel(u64 id) {
    PMutexLock(tableMtx);
    CTableRm(table, id, 0);
    PMutexUnlock(tableMtx);
}
static void giveChannelClient(Client *client, u64 id) {
    Channel **chan = 0, *newChan = 0;
    PMutexLock(tableMtx);

    Ret insert = 1;
    chan = CTableAt(table, id, &insert);
    if(insert < 0) { DEBUG_LOG("Failed to push to channel table\n"); goto FAILED; }
    else if(insert != 0) {
        // channel does not yet exist
        NULL_CHECK(newChan = ChannelCreate(id));
        CHECK(WorkQPoll(que, &newChan->qtask));
        *chan = newChan;
    }

    channelDonate(*chan, client);

CLEAN:
    PMutexUnlock(tableMtx);
    return;    
FAILED:
    ClientDe(client);
    ChannelDe(newChan);
    goto CLEAN;
}
static void tableDonate(ClientTask *clientTask) {
    Client *client;
    NULL_CHECK(client = calloc(1, sizeof(Client)));
    client->state = CLIENT_CSTATE_DEFAULT;
    client->sock = clientTask->sock; memset(&clientTask->sock, 0, sizeof(clientTask->sock));
    client->sprot = clientTask->sprot; memset(&clientTask->sprot, 0, sizeof(clientTask->sprot));
    client->rcvBuff = clientTask->rcvBuff; memset(&clientTask->rcvBuff, 0, sizeof(clientTask->rcvBuff));
    giveChannelClient(client, clientTask->channel);
    return;
FAILED:
    ClientDe(client);
}
static Ret TunnelSCB(Buff *msg, void *arg) {
    Client *client = arg;
    return BuffSPEncrypt(msg, &client->sprot, client->ws);
}
static void TunnelFCB(void *arg) {
    Client *client = arg;
    SPEncryptFinish(&client->sprot, SmemData(client->sws));
}
static Ret TunnelRCB(Buff *msg, void *arg) {
    Client *client = arg;
    return BuffSPDecrypt(msg, &client->sprot, client->ws, SmemData(client->sws));
}
static void tableInit() {
    if(CTableInit(table, 0)) { exitFailure("Failed to init", "channel table", ""); }
    if(PMutexInit(tableMtx)) { exitFailure("Failed to init", "channel table mutex", ""); }
}