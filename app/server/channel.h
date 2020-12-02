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

static Ret handleNewClients(Channel *chan) {
    Client *client;

    while(1) {
        Ret res;
        usize len;
        NEG_CHECK(res = PipeRead(&chan->pip, (u8 *)&client, &len));
        if(res > 0) { return 0; }
        else {
            assert(len == sizeof(Client *));
            CHECK(TunnelInit(&client->tunnel, &client->sock, client));
            client->ws = &chan->ws;
            client->sws = &chan->sws;
            client->id = chan->idTicker++;
            client->logPlace = 0;
            CHECK(ClientVecPush(&chan->clients, client));
            client->state = CLIENT_CSTATE_ONBOARD;

        #ifdef JC_DEBUG
            fprintf(stderr, "Client joined channel %llu\n", (ull)chan->id);
        #endif
        }
    }

FAILED:
    ClientDe(client);
    return -1;
}
static Ret deadClientPredicate(void *self, Client **client) {
    void *ign = self;
    if((*client)->state == CLIENT_CSTATE_DEAD) { DEBUG_LOG("Client dropped from channel.\n"); }
    return (*client)->state == CLIENT_CSTATE_DEAD;
}
static void clearDeadClients(Channel *chan) {
    static ClientVecPred deadClientPredPtr = &deadClientPredicate;
    ClientVecRm(&chan->clients, 0, ClientVecSize(&chan->clients), &deadClientPredPtr);
}
static void MsgDe(Msg *msg) { if(msg) { free(msg->data); } }
static Ret logMsg(Channel *chan, Buff *msgBuff, usize id) {
    Msg msg = {0};
    usize tempLen;
    u64 msgLen;

    CHECK(!(tempLen = BuffSize(msgBuff)));
    CHECK(tempLen > UINT64_MAX);
    msgLen = tempLen;
    NULL_CHECK(msg.data = malloc(msgLen));
    memcpy(msg.data, BuffData(msgBuff), msgLen);
    // -1 removing control byte from the echo'd message
    msg.len = msgLen - 1;
    msg.sender = id;
    // + MSG_DURATION_SECONDS storing when to purge the msg
    msg.ts = time(NULL) + MSG_DURATION_SECONDS;

    CHECK(MsgVecPush(&chan->log, msg));

#ifdef JC_DEBUG
    fprintf(stderr, "Got message len %zu, logged %c\n", msg.len, msg.data[msg.len] == '1' ? 't' : 'f');
#endif

    return 0;
FAILED:
    MsgDe(&msg);
    return -1;
}
static Ret handleClientReads(Channel *chan) {
    Client **iter = ClientVecData(&chan->clients);
    Client **end = iter + ClientVecSize(&chan->clients);
    while(iter != end) {
        while(1) {
            Ret res = TunnelRecvP(&(*iter)->tunnel, &(*iter)->rcvBuff, NET_BLOCK_NONE);
            if(res < 0) { (*iter)->state = CLIENT_CSTATE_DEAD; break; }
            else if(res > 0) { break; }
            else if(logMsg(chan, &(*iter)->rcvBuff, (*iter)->id)) { (*iter)->state = CLIENT_CSTATE_DEAD; break; }
        }

        ++iter;
    }

    clearDeadClients(chan);
    return 0;
FAILED:
    return -1;
}
static Ret handleClientSends(Channel *chan) {
    Client **iter = ClientVecData(&chan->clients);
    Client **end = iter + ClientVecSize(&chan->clients);
    while(iter != end) {
        for(usize logEnd = MsgVecSize(&chan->log); (*iter)->logPlace < logEnd; ++(*iter)->logPlace) {
            Msg *msg = MsgVecData(&chan->log) + (*iter)->logPlace;

            if((*iter)->id != msg->sender) {    
                Ret res = TunnelSendP(&(*iter)->tunnel, msg->data, msg->len, NET_BLOCK_NONE);
                if(res < 0) { (*iter)->state = CLIENT_CSTATE_DEAD; break; }
                else if(res > 0) { (*iter)->state = CLIENT_CSTATE_SNDBLK; break; }
            }
        }

        ++iter;
    }

    clearDeadClients(chan);
    return 0;
FAILED:
    return -1;
}
static Ret resetChannelTriggers(Channel *chan) {
    PTrigVecClear(&chan->trigs);
    PWhen temp = { PipeGetReadEnd(&chan->pip), PWHEN_IN };
    CHECK(PTrigVecPush(&chan->trigs, temp));

    {
        Client **iter = ClientVecData(&chan->clients);
        Client **end = iter + ClientVecSize(&chan->clients);
        while(iter != end) {
            temp.fd = TCPSockGetFD(&(*iter)->sock);
            temp.when = PWHEN_IN;
            if((*iter)->state == CLIENT_CSTATE_SNDBLK) {
                temp.when |= PWHEN_OUT;
                (*iter)->state = CLIENT_CSTATE_ONBOARD;
            }
            CHECK(PTrigVecPush(&chan->trigs, temp));

            ++iter;
        }
    }

    temp.fd = 0; temp.when = 0;
    CHECK(PTrigVecPush(&chan->trigs, temp));
    return 0;
FAILED:
    return -1;
}
static Ret msgIsNotLogged(void *pred, Msg *msg) { void *ign = pred; return msg->data[msg->len] != '1'; }
struct OldMsgPred { MsgVecPred pred; time_t currentTime; };
static Ret msgIsOld(void *ppred, Msg *msg) {
    struct OldMsgPred *pred = ppred;
    return msg->ts < pred->currentTime;
}
static void pruneLog(Channel *chan) {
    usize lastPlace = MsgVecSize(&chan->log);
    usize amtRm = 0;
    
    {
        Client **iter = ClientVecData(&chan->clients);
        Client **end = iter + ClientVecSize(&chan->clients);
        while(iter != end) {
            if(lastPlace > (*iter)->logPlace) { lastPlace = (*iter)->logPlace; }
            ++iter;
        }
    }

    static const MsgVecPred msgIsNotLoggedPredPtr = &msgIsNotLogged;
    amtRm += MsgVecRm(&chan->log, 0, lastPlace, (MsgVecPred *)&msgIsNotLoggedPredPtr); lastPlace -= amtRm;
    struct OldMsgPred oldMsgPred = {&msgIsOld, time(0)};
    amtRm += MsgVecRm(&chan->log, 0, lastPlace, &oldMsgPred.pred);

    {
        Client **iter = ClientVecData(&chan->clients);
        Client **end = iter + ClientVecSize(&chan->clients);
        while(iter != end) {
            (*iter)->logPlace -= amtRm;
            ++iter;
        }
    }
}
static Ret channelCB(PTask **task) {
    Channel *chan = (Channel *)task;

    CHECK(handleNewClients(chan));
    CHECK(handleClientReads(chan));
    CHECK(handleClientSends(chan));
    pruneLog(chan);

    if(0 == ClientVecSize(&chan->clients) && 0 == MsgVecSize(&chan->log)) { goto CLEAN_FINISH; }
    else {
        CHECK(resetChannelTriggers(chan));
        assert(PTrigVecSize(&chan->trigs) > 1);
        return PTASK_RET_REPOLL;
    }

CLEAN_FINISH:
FAILED:
    killChannel(chan->id);
    return PTASK_RET_SPARE;
}
static PWhen * channelTrigs(PTask **task) {
    Channel *chan = (Channel *)task;
    return PTrigVecData(&chan->trigs);
}
static PTask channelFace = {&channelCB,&channelTrigs,NULL};
static Channel * ChannelCreate(u64 id) {
    Channel *res = 0;
    NULL_CHECK(res = calloc(1, sizeof(Channel)));
    res->qtask.vptr = &channelFace;
    res->id = id;
    CHECK(PipeInit(&res->pip, 0));
    CHECK(PTrigVecInit(&res->trigs, 0));
    PWhen trigs[2] = { {PipeGetReadEnd(&res->pip), PWHEN_IN}, {0, 0} };
    CHECK(PTrigVecPushArr(&res->trigs, trigs, 2));
    CHECK(ClientVecInit(&res->clients, 0));
    CHECK(MsgVecInit(&res->log, 0));
    CHECK(BuffInit(&res->ws, 0));
    CHECK(SmemInit(&res->sws, NACLH_BYTES));

    return res;
FAILED:
    ChannelDe(res);
    return 0;
}
static void ChannelDe(Channel *channel) {
    if(channel) {
        SmemDe(&channel->sws);
        BuffDe(&channel->ws);
        PTrigVecDe(&channel->trigs);
        ClientVecDe(&channel->clients);
        MsgVecDe(&channel->log);
        PipeDe(&channel->pip);
        free(channel);
    }
}
static void channelDonate(Channel *chan, Client *client) {
    CHECK(PipeWrite(&chan->pip, (u8 *)&client, sizeof(Client *)));
    return;
FAILED:
    ClientDe(client);
}
