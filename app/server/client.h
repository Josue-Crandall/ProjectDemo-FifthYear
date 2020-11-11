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

static_assert(sizeof(u64) < sizeof(u64) + SPROT_BYTES, "overflow check");
static_assert((sizeof(u64) + SPROT_BYTES) <= NETWORK_CHUNK_SIZE, "Single read and send w/o packet breaking");
static Ret clientTaskInit(ClientTask *task) {
    CHECK(SprotInit(&task->sprot));
    CHECK(SmemInit(&task->sws, NETWORK_CHUNK_SIZE));
    CHECK(BuffInit(&task->rcvBuff, NETWORK_CHUNK_SIZE));
    CHECK(BuffInit(&task->ws, 0));
    task->state = CLIENT_STATE_SHAKE;
    return 0;
FAILED:
    return -1;
}
static Ret clientTaskShake(ClientTask *task) {
    Ret res;
    NEG_CHECK(res = SprotServerHandshake(&task->sprot, clientPK, serverSK, &task->sws));
    if(res == SPROT_HS_SEND) { task->state = CLIENT_STATE_SEND; }
    else if(res == SPROT_HS_RECV) { task->state = CLIENT_STATE_RECV; }
    else if(res == SPROT_HS_DONE) { task->state = CLIENT_STATE_DONE; }
    else { DEBUG_LOG("SprotServerHandshake strange return\n"); goto FAILED; }
    return 0;
FAILED:
    return -1;
}
static Ret clientTaskHSSend(ClientTask *task) {
    Ret res;
    NEG_CHECK(res = TCPSockSend(&task->sock, SprotData(&task->sprot), SprotSize(&task->sprot), NET_BLOCK_NONE));
    if(res) { task->trigs[0].when = PWHEN_OUT; return 1; }
    else { task->trigs[0].when = PWHEN_IN; task->state = CLIENT_STATE_SHAKE; return 0; }
FAILED:
    return -1;
}
static Ret clientTaskHSRecv(ClientTask *task) {
    Ret res;
    NEG_CHECK(res = TCPSockRecv(&task->sock, SprotData(&task->sprot), SprotSize(&task->sprot), NET_BLOCK_NONE));
    if(res) { return 1; }
    else { task->state = CLIENT_STATE_SHAKE; return 0; }
FAILED:
    return -1;
}
static Ret clientTaskCB(PTask **task) {
    ClientTask *ctask = (ClientTask *)task;
SWITCH:
    switch(ctask->state) {
        case CLIENT_STATE_DEFAULT: { CHECK(clientTaskInit(ctask)); } goto SWITCH;
        case CLIENT_STATE_SHAKE: { CHECK(clientTaskShake(ctask)); } goto SWITCH;
        case CLIENT_STATE_SEND: { 
            Ret res; NEG_CHECK(res = clientTaskHSSend(ctask)); if(res) { return PTASK_RET_REPOLL; }
        } goto SWITCH;
        case CLIENT_STATE_RECV: { 
            Ret res; NEG_CHECK(res = clientTaskHSRecv(ctask)); if(res) { return PTASK_RET_REPOLL; }
        } goto SWITCH;
        case CLIENT_STATE_DONE: {
            Ret res; NEG_CHECK(res = TCPSockRecv(&ctask->sock, BuffData(&ctask->rcvBuff), NETWORK_CHUNK_SIZE, NET_BLOCK_NONE));
            if(res) { return PTASK_RET_REPOLL; }
            BuffSetSize(&ctask->rcvBuff, sizeof(u64) + SPROT_BYTES);
            CHECK(BuffSPDecrypt(&ctask->rcvBuff, &ctask->sprot, &ctask->ws, SmemData(&ctask->sws)));
            memcpy(&ctask->channel, BuffData(&ctask->rcvBuff), sizeof(u64));
            tableDonate(ctask);
            return PTASK_RET_DONE;
        } goto SWITCH;
    }
FAILED:
    return PTASK_RET_DONE;
}
static PWhen *clientTaskTrig(PTask **task) {
    ClientTask *ctask = (ClientTask *)task;
    return ctask->trigs;
}
static void clientTaskDe(PTask **task) { 
    ClientTask *ctask = (ClientTask *)task;
    if(ctask) {
        BuffDe(&ctask->rcvBuff);
        BuffDe(&ctask->ws);
        SmemDe(&ctask->sws);
        SprotDe(&ctask->sprot);
        TCPSockDe(&ctask->sock);
        free(ctask);
    }
}
static PTask clientTaskFace = { &clientTaskCB, &clientTaskTrig, &clientTaskDe };
void clientDonate(TCPSock *newClient) {
    ClientTask *newTask = 0;
    NULL_CHECK(newTask = calloc(1, sizeof(ClientTask)));
    newTask->qtask.vptr = &clientTaskFace;
    newTask->trigs[0].fd = TCPSockGetFD(newClient);
    newTask->trigs[0].when = PWHEN_IN;
    newTask->sock = *newClient;
    CHECK(WorkQPoll(que, &newTask->qtask));
    return;
FAILED:
    free(newTask);
    TCPSockDe(newClient);
}
