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

struct PollQSignal { Ret trig; int fd; void *token; }

#define DEF_POLLQ(NAME, CB, DE)                                             \
                                                                            \
static Ret NAME##Init(NAME *pollq, int timeoutMS) {                         \
    memset(pollq, 0, sizeof(pollq));                                        \
    CHECK(NAME##WQInit(&pollq->que));                                       \
    CHECK(PollerInit(&pollq->poller));                                      \
    CHECK(PipeInit(&pollq>pip, NULL));                                      \
    CHECK(PollerReg(&polq->poller, PipeGetReadEnd(&pollq->pip), POLLER_IN, NULL)); \
    pollq->timeoutMS = timeoutMS;                                           \
    return 0;                                                               \
FAILED:                                                                     \
    NAME##De(que);                                                          \
    memset(pollq, 0, sizeof(pollq));                                        \
    return -1;                                                              \
}                                                                           \
                                                                            \
static Ret NAME##Push(NAME *pollq, void *token) {                           \
    return NAME##WQPush(&pollq->que, token);                                \
}                                                                           \
static Ret NAME##SetPoll(NAME *pollq, int fd, Ret trigger, void *token) {   \
    struct PollQSignal sig = {trigger, fd, token};                          \
    CHECK(PipeWrite(&pollq->pip, &sig, sizeof(struct PollQSignal)));        \
    return 0;                                                               \
FAILED:                                                                     \
    NAME##WQTokenDe(token);                                                 \
    return -1;                                                              \
}                                                                           \
static Ret NAME##SetPoll(NAME *pollq, int fd, Ret trigger, void *token) {   \
    return NAME##SignalImp(pollq, fd, trigger, POLLQ_SIG_REG);              \
}                                                                           \
static void NAME##Poll(NAME *pollq) {                                       \
    while(1) {                                                              \
        int rdy;                                                            \
        NEG_CHECK(rdy = PollerPoll(&pollq->poller, pollq->timeoutMS));      \
        for(int i = 0; i < rdy; ++i) {                                      \
            void *token = PollerTok(&pollq->poller, i);                     \
            if(NULL == token) {                                             \
                struct PollQSignal *sig = token;                            \
                if(PollerReg(pollq->poller, sig->fd, sig->trig, sig->tok)) {\
                    NAME##WQTokenDe(sig->tok);                              \
                    goto FAILED;                                            \
                }                                                           \
            }                                                               \
            else {                                                          \
                
                // And problems since I need to know fd of task after I register it
                // ? This could just be a table

                // ? keep table modify, add internal function which removes or rearms
                // ? then just performs CB here
                CHECK(PollerRm(&pollq->poller, ))

                // issue with (? solution) is that it requires two serialized
                // pipe sends in all cases in order to complete a poll'd transaction 

            }                                                               \
        }                                                                   \
                                                                            \
    }                                                                       \
FAILED:                                                                     \
    NAME##WQStop(&pollq->que);                                              \
    return;                                                                 \
}                                                                           \
static void NAME##Stop(NAME *pollq) { NAME##WQStop(&pollq->que); }          \
                                                                            \
static void NAME##De(NAME *pollq) {                                         \
    PipeDe(&pollq->pip);                                                    \
    PollerDe(&pollq->poller);                                               \
    NAME##WQDe(&pollq->que);                                                \
}                                                                           \

