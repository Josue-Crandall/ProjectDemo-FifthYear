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

typedef struct SprotState { u8 inKey[NACLS_KEY_BYTES], outKey[NACLS_KEY_BYTES]; } SprotState;
#define SP_CLIENT_SEND_SIZE (NACLA_PUB_BYTES + NACLA_BYTES)
#define SP_SERVER_SEND_SIZE (NACLS_KEY_BYTES * 2 + NACLA_BYTES)
static_assert(SP_CLIENT_SEND_SIZE <= NETWORK_CHUNK_SIZE, "Fitting into a single network send");
static_assert(SP_SERVER_SEND_SIZE <= NETWORK_CHUNK_SIZE, "Fitting into a single network send");
typedef struct SprotHSData {
    u8 tx[MAX(sizeof(SprotState), NETWORK_CHUNK_SIZE)];
    u8 epk[NACLA_PUB_BYTES], esk[NACLA_SEC_BYTES];
    u8 iKey[NACLS_KEY_BYTES], oKey[NACLS_KEY_BYTES];
    Ret state;
} SprotHSData;
static Ret SprotInit(Sprot *sprot) { return SmemInit(sprot, sizeof(SprotHSData)); }
static u8 *SprotData(Sprot *sprot) { return SmemData(sprot); }
static usize SprotSize(Sprot *sprot) { return SmemSize(sprot); }
static void SprotDe(Sprot *sprot) { SmemDe(sprot); }
static void SPEncryptFinish(Sprot *sprot, u8 sws[NACLH_BYTES]) {
    SprotState *R() data = (SprotState *R())SmemData(sprot);
    naclRatchet(data->outKey, sws);
}

// handshake imp
#include <string.h>

static Ret SprotClientHandshake(Sprot *sprot, u8 pk[NACLA_PUB_BYTES], u8 sk[NACLA_SEC_BYTES], Smem *ws) {
    SprotHSData *R() data = (SprotHSData *R()) SmemData(sprot); 

    if(0 == SmemSize(sprot)) {
        SmemSetSize(sprot, NACLA_PUB_BYTES); 
        naclKeyPair(data->tx, data->esk);
        CHECK(SmemAEncrypt(sprot, pk, sk, ws));
        data->state = SPROT_HS_SEND;
        randombytes(data->tx + SP_CLIENT_SEND_SIZE, NETWORK_CHUNK_SIZE - SP_CLIENT_SEND_SIZE);
        SmemSetSize(sprot, NETWORK_CHUNK_SIZE);
        return SPROT_HS_SEND;
    }
    else if (data->state == SPROT_HS_SEND) {
        data->state = SPROT_HS_RECV;
        return SPROT_HS_RECV;
    }
    else if(data->state == SPROT_HS_RECV) {
        SmemSetSize(sprot, SP_SERVER_SEND_SIZE);
        CHECK(SmemADecrypt(sprot, pk, data->esk, ws));
        memcpy(data->iKey, data->tx, NACLS_KEY_BYTES);
        memcpy(data->oKey, data->tx + NACLS_KEY_BYTES, NACLS_KEY_BYTES);
        SmemSetSize(sprot, sizeof(SprotState));
        SprotState *result = (SprotState *)data;
        memcpy(result->inKey, data->iKey, NACLS_KEY_BYTES);
        memcpy(result->outKey, data->oKey, NACLS_KEY_BYTES);
        explicit_bzero((u8 *)data + sizeof(SprotState), sizeof(SprotHSData) - sizeof(SprotState));
        data->state = SPROT_HS_DONE;
        return SPROT_HS_DONE;
    }

FAILED:
    explicit_bzero(SmemData(sprot), sizeof(SprotHSData));
    SmemSetSize(sprot, 0);
    return -1;
}
static Ret SprotServerHandshake(Sprot *sprot, u8 pk[NACLA_PUB_BYTES], u8 sk[NACLA_SEC_BYTES], Smem *ws) {
    SprotHSData *R() data = (SprotHSData *R()) SmemData(sprot); 

    if(0 == SmemSize(sprot)) {
        SmemSetSize(sprot, NETWORK_CHUNK_SIZE);
        data->state = SPROT_HS_RECV;
        return SPROT_HS_RECV;
    }
    else if (data->state == SPROT_HS_RECV) {
        SmemSetSize(sprot, SP_CLIENT_SEND_SIZE);
        CHECK(SmemADecrypt(sprot, pk, sk, ws));
        memcpy(data->epk, data->tx, NACLA_PUB_BYTES);
        randombytes(data->iKey, NACLS_KEY_BYTES);
        randombytes(data->oKey, NACLS_KEY_BYTES);
        // opposite order intentional
        memcpy(data->tx, data->oKey, NACLS_KEY_BYTES);
        memcpy(data->tx + NACLS_KEY_BYTES, data->iKey, NACLS_KEY_BYTES);
        SmemSetSize(sprot, NACLS_KEY_BYTES * 2);
        CHECK(SmemAEncrypt(sprot, data->epk, sk, ws));
        randombytes(data->tx + SP_SERVER_SEND_SIZE, NETWORK_CHUNK_SIZE - SP_SERVER_SEND_SIZE);
        SmemSetSize(sprot, NETWORK_CHUNK_SIZE);
        data->state = SPROT_HS_SEND;
        return SPROT_HS_SEND;
    }
    else if(data->state == SPROT_HS_SEND) {
        SmemSetSize(sprot, sizeof(SprotState));
        SprotState *result = (SprotState *)data;
        memcpy(result->inKey, data->iKey, NACLS_KEY_BYTES);
        memcpy(result->outKey, data->oKey, NACLS_KEY_BYTES);
        explicit_bzero((u8 *)data + sizeof(SprotState), sizeof(SprotHSData) - sizeof(SprotState));
        data->state = SPROT_HS_DONE;
        return SPROT_HS_DONE;
    }
    
FAILED:
    explicit_bzero(SmemData(sprot), sizeof(SprotHSData));
    SmemSetSize(sprot, 0);
    return -1;
}

//

#define DEF_SPROT_FNS(TYPE)                                                                         \
static Ret TYPE##SPEncrypt(TYPE *R() text, Sprot *sprot, TYPE *R() ws) {                            \
    SprotState *R() data = (SprotState *R())SmemData(sprot);                                        \
    CHECK(TYPE##Push(text, 0));                                                                     \
    CHECK(TYPE##SEncrypt(text, data->outKey, ws));                                                  \
    return 0;                                                                                       \
FAILED:                                                                                             \
    return -1;                                                                                      \
}                                                                                                   \
static Ret TYPE##SPEncryptUnmut(TYPE *R() text, Sprot *sprot, TYPE *R() ws) {                       \
    SprotState *R() data = (SprotState *R())SmemData(sprot);                                        \
    CHECK(TYPE##Push(text, 1));                                                                     \
    CHECK(TYPE##SEncrypt(text, data->outKey, ws));                                                  \
    return 0;                                                                                       \
FAILED:                                                                                             \
    return -1;                                                                                      \
}                                                                                                   \
static Ret TYPE##SPDecrypt(TYPE *R() text, Sprot *sprot, TYPE *R() ws, u8 sws[NACLH_BYTES]) {       \
    SprotState *R() data = (SprotState *R()) SmemData(sprot);                                       \
    CHECK(TYPE##Size(text) < SPROT_BYTES);                                                          \
    CHECK(TYPE##SDecrypt(text, data->inKey, ws));                                                   \
    if(0 == TYPE##Pop(text)) { naclRatchet(data->inKey, sws); }                                     \
    return 0;                                                                                       \
FAILED:                                                                                             \
    return -1;                                                                                      \
}                                                                                                   \

#define DD_SPROT_FNS(TYPE)  \
DEC_SPROT_FNS(TYPE);        \
DEF_SPROT_FNS(TYPE);        \
