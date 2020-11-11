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

#include "../file/file.h"

typedef struct RProtState {
    u8 sws[NACLS_KEY_BYTES + NACLA_BYTES];

    u8 status;
    #define RP_STATUS_SENT_IN                    (1)
    #define RP_STATUS_SENT_OUT                   (2)
    #define RP_STATUS_RATCHET_READY              (4)
    #define RP_STATUS_ADVANCE_KEYS               (8)
    #define RP_STATUS_GET(VAL, STATUS_TYPE)      ((VAL) & ((u8)(STATUS_TYPE)))
    #define RP_STATUS_SET_ON(VAL, STATUS_TYPE)   ((VAL) |= (u8)(STATUS_TYPE))
    #define RP_STATUS_SET_OFF(VAL, STATUS_TYPE)  ((VAL) &= ~((u8)(STATUS_TYPE)))
    u8 outKey[NACLS_KEY_BYTES], inKey[NACLS_KEY_BYTES];
    u8 outSec[NACLS_KEY_BYTES], inSec[NACLS_KEY_BYTES];
    u8 ephPK[NACLA_PUB_BYTES], ephSK[NACLA_SEC_BYTES];
    u8 ignPK[NACLA_PUB_BYTES], ignSK[NACLA_SEC_BYTES];
} RProtState;
typedef struct __attribute__((packed)) {
    u8 newEPK[NACLA_PUB_BYTES];
    u8 newSecret[NACLS_KEY_BYTES + NACLA_BYTES];
    u8 status;
} RPControl;
static Ret RprotMake(char *R() filePath1, char *R() filePath2) {
    int f1 = -1, f2 = -1;
    RAI(Smem, buff);
    u8 *buf;

    NEG_CHECK(f1 = fileOpen(filePath1));
    NEG_CHECK(f2 = fileOpen(filePath2));
    CHECK(fileTrunc(f1, 0));
    CHECK(fileTrunc(f2, 0));
    CHECK(SmemInit(&buff, MAX(NACLS_KEY_BYTES * 2, NACLA_PUB_BYTES + NACLA_SEC_BYTES) ));
    buf = SmemData(&buff);

    buf[0] = RP_STATUS_SENT_IN;
    CHECK(fileWrite(f1, buf, 1));
    CHECK(fileWrite(f2, buf, 1));
    randombytes(buf, NACLS_KEY_BYTES * 2);
    CHECK(fileWrite(f1, buf, NACLS_KEY_BYTES * 2));
    CHECK(fileWrite(f2, buf + NACLS_KEY_BYTES, NACLS_KEY_BYTES));
    CHECK(fileWrite(f2, buf, NACLS_KEY_BYTES));
    memset(buf, 0, NACLS_KEY_BYTES * 2);
    CHECK(fileWrite(f1, buf, NACLS_KEY_BYTES * 2));
    CHECK(fileWrite(f2, buf, NACLS_KEY_BYTES * 2));
    CHECK(fileWrite(f1, buf, NACLA_PUB_BYTES));
    CHECK(fileWrite(f2, buf, NACLA_PUB_BYTES));
    CHECK(fileWrite(f1, buf, NACLA_SEC_BYTES));
    CHECK(fileWrite(f2, buf, NACLA_SEC_BYTES));
    naclKeyPair(buf, buf + NACLA_PUB_BYTES);
    CHECK(fileWrite(f1, buf, NACLA_PUB_BYTES + NACLA_SEC_BYTES));
    CHECK(fileWrite(f2, buf, NACLA_PUB_BYTES + NACLA_SEC_BYTES));

    Ret err = 0;
CLEAN:
    SmemDe(&buff);
    FD_CLOSE(f2);
    FD_CLOSE(f1);
    return 0;
FAILED:
    err = -1;
    goto CLEAN;
}
static Ret RprotInit(Rprot *rprot, char *filePath) {
    int file = -1;
    RProtState *data;

    CHECK(SmemInit(rprot, sizeof(RProtState)));
    NEG_CHECK(file = fileOpen(filePath));
    data = (RProtState *)SmemData(rprot);

    CHECK(fileRead(file, &data->status, 1));
    CHECK(fileRead(file, data->outKey, NACLS_KEY_BYTES));
    CHECK(fileRead(file, data->inKey, NACLS_KEY_BYTES));
    CHECK(fileRead(file, data->outSec, NACLS_KEY_BYTES));
    CHECK(fileRead(file, data->inSec, NACLS_KEY_BYTES));
    CHECK(fileRead(file, data->ephPK, NACLA_PUB_BYTES));
    CHECK(fileRead(file, data->ephSK, NACLA_SEC_BYTES));
    CHECK(fileRead(file, data->ignPK, NACLA_PUB_BYTES));
    CHECK(fileRead(file, data->ignSK, NACLA_SEC_BYTES));

    Ret err = 0;
CLEAN:
    FD_CLOSE(file);
    return err;
FAILED:
    err = -1;
    SmemDe(rprot);    
    memset(rprot, 0, sizeof(*rprot));
    goto CLEAN;
}
static Ret RprotDe(Rprot *rprot, char *path) {
    if(SmemAlive(rprot)) {
        RProtState *data = (RProtState *)SmemData(rprot);
        int file = -1;

        NEG_CHECK(file = fileOpen(path));

        CHECK(fileWrite(file, &data->status, 1));
        CHECK(fileWrite(file, data->outKey, NACLS_KEY_BYTES));
        CHECK(fileWrite(file, data->inKey, NACLS_KEY_BYTES));
        CHECK(fileWrite(file, data->outSec, NACLS_KEY_BYTES));
        CHECK(fileWrite(file, data->inSec, NACLS_KEY_BYTES));
        CHECK(fileWrite(file, data->ephPK, NACLA_PUB_BYTES));
        CHECK(fileWrite(file, data->ephSK, NACLA_SEC_BYTES));
        CHECK(fileWrite(file, data->ignPK, NACLA_PUB_BYTES));
        CHECK(fileWrite(file, data->ignSK, NACLA_SEC_BYTES));
    
        Ret err = 0;
    CLEAN:
        SmemDe(rprot);
        FD_CLOSE(file);
        return err;
    FAILED:
        err = -1;
        goto CLEAN;
    }
    else {
        return 0;
    }
}

#define DEF_ProtREncryptImp(TYPE)                                               \
static Ret TYPE##ProtREncryptImp(TYPE *R() text, Rprot *rprot, TYPE *R() ws, Smem *R() sws) { \
    RProtState *prot = (RProtState *)SmemData(rprot);                           \
                                                                                \
    usize ptLen = TYPE##Size(text);                                             \
    usize ctLen = ptLen;                                                        \
                                                                                \
    ADD_CHECK(ctLen, RPROT_BYTES, usize);                                       \
    CHECK(TYPE##Reserve(text, ctLen));                                          \
                                                                                \
    RPControl *ctrl = (RPControl *)(TYPE##Data(text) + ptLen);                  \
                                                                                \
    if(RP_STATUS_GET(prot->status, RP_STATUS_ADVANCE_KEYS)) {                   \
        if(!RP_STATUS_GET(prot->status, RP_STATUS_SENT_IN)) {                   \
            randombytes(prot->inSec, NACLS_KEY_BYTES);                          \
            memcpy(prot->sws, prot->inSec, NACLS_KEY_BYTES);                    \
            SmemSetSize(rprot, NACLS_KEY_BYTES);                                \
            CHECK(SmemAEncrypt(rprot, prot->ephPK, prot->ignSK, sws));          \
            memcpy(ctrl->newSecret, prot->sws, NACLS_KEY_BYTES + NACLA_BYTES);  \
            explicit_bzero(prot->sws, NACLS_KEY_BYTES + NACLA_BYTES);           \
        }                                                                       \
        else { memset(ctrl->newSecret, 0, NACLS_KEY_BYTES + NACLA_BYTES); }     \
        if(!RP_STATUS_GET(prot->status, RP_STATUS_SENT_OUT)) {                  \
            naclKeyPair(ctrl->newEPK, prot->ephSK);                             \
        }                                                                       \
        else { memset(ctrl->newEPK, 0, NACLA_PUB_BYTES); }                      \
    }                                                                           \
    else { memset(ctrl, 0, sizeof(ctrl) - 1); }                                 \
    ctrl->status = prot->status;                                                \
                                                                                \
    TYPE##SetSize(text, ptLen + sizeof(RPControl));                             \
    CHECK(TYPE##SEncrypt(text, prot->outKey, ws));                              \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \

static void RPEncryptFinish(Rprot *rprot, u8 sws[NACLH_BYTES]) {
    RProtState *prot = (RProtState *)SmemData(rprot);

    RP_STATUS_SET_ON(prot->status, RP_STATUS_SENT_OUT | RP_STATUS_SENT_IN);

    naclRatchet(prot->outKey, sws);

    if(RP_STATUS_GET(prot->status, RP_STATUS_RATCHET_READY)) {
        naclMix(prot->outKey, prot->outSec, sws);
        RP_STATUS_SET_OFF(prot->status, RP_STATUS_RATCHET_READY);
    }
}

#define DEF_RPROT_FNS(TYPE)                                                     \
DEF_ProtREncryptImp(TYPE);                                                      \
static Ret TYPE##RPEncrypt(TYPE *R() text, Rprot *rprot, TYPE *R() ws, Smem *R() sws) { \
    RProtState *prot = (RProtState *)SmemData(rprot);                           \
    RP_STATUS_SET_ON(prot->status, RP_STATUS_ADVANCE_KEYS);                     \
    return TYPE##ProtREncryptImp(text, rprot, ws, sws);                         \
}                                                                               \
static Ret TYPE##RPEncryptUnmut(TYPE *R() text, Rprot *rprot, TYPE *R() ws) {   \
    RProtState *prot = (RProtState *)SmemData(rprot);                           \
    RP_STATUS_SET_OFF(prot->status, RP_STATUS_ADVANCE_KEYS);                    \
    return TYPE##ProtREncryptImp(text, rprot, ws, NULL);                        \
}                                                                               \
static Ret TYPE##RPDecrypt(TYPE *R() text, Rprot *rprot, TYPE *R() ws, Smem *R() sws) { \
    RProtState *prot = (RProtState *)SmemData(rprot);                       \
                                                                            \
    usize ctLen = TYPE##Size(text);                                         \
                                                                            \
    CHECK(ctLen < RPROT_BYTES);                                             \
    CHECK(TYPE##SDecrypt(text, prot->inKey, ws));                           \
                                                                            \
    RPControl *ctrl = (RPControl *)(TYPE##Data(text) + ctLen - RPROT_BYTES);\
                                                                            \
    if(RP_STATUS_GET(ctrl->status, RP_STATUS_ADVANCE_KEYS)) {               \
        SmemReserve(sws, NACLH_BYTES);                                      \
        naclRatchet(prot->inKey, SmemData(sws));                            \
                                                                            \
        if(RP_STATUS_GET(ctrl->status, RP_STATUS_RATCHET_READY)) {          \
            naclMix(prot->inKey, prot->inSec, SmemData(sws));               \
        }                                                                   \
                                                                            \
        if(!RP_STATUS_GET(ctrl->status, RP_STATUS_SENT_OUT)) {              \
            memcpy(prot->ephPK, ctrl->newEPK, NACLA_PUB_BYTES);             \
            RP_STATUS_SET_OFF(prot->status, RP_STATUS_SENT_IN);             \
        }                                                                   \
                                                                            \
        if(!RP_STATUS_GET(ctrl->status, RP_STATUS_SENT_IN)) {               \
            memcpy(prot->sws, ctrl->newSecret, sizeof(ctrl->newSecret));    \
            SmemSetSize(rprot, sizeof(ctrl->newSecret));                    \
            CHECK(SmemADecrypt(rprot, prot->ignPK, prot->ephSK, sws));      \
            memcpy(prot->outSec, prot->sws, NACLS_KEY_BYTES);               \
            explicit_bzero(prot->sws, sizeof(ctrl->newSecret));             \
                                                                            \
            RP_STATUS_SET_OFF(prot->status, RP_STATUS_SENT_OUT);            \
            RP_STATUS_SET_ON(prot->status, RP_STATUS_RATCHET_READY);        \
        }                                                                   \
    }                                                                       \
                                                                            \
    TYPE##SetSize(text, ctLen - RPROT_BYTES);                               \
    return 0;                                                               \
FAILED:                                                                     \
    return -1;                                                              \
}

#define DD_RPROT_FNS(TYPE)  \
DEC_RPROT_FNS(TYPE);        \
DEF_RPROT_FNS(TYPE);        \
