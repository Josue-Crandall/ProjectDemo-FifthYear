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

#include "sprot.h"

int main(void) { 
    Ret t0(); CHECK(t0());

    printf("pass\n");
    return 0; 
FAILED:
    printf(" ---   FAILED --- \n");
    return 0;
}

Ret t0() {
    RAI(Sprot, s1); RAI(Sprot, s2);
    RAI(Buff, b1); RAI(Buff, b2);
    RAI(Smem, w1); RAI(Smem, w2);
    u8 msgDup[1079]; 
    u8 spk[NACLA_PUB_BYTES], ssk[NACLA_SEC_BYTES], cpk[NACLA_PUB_BYTES], csk[NACLA_SEC_BYTES];

    CHECK(SprotInit(&s1)); CHECK(SprotInit(&s2));
    CHECK(BuffInit(&b1, 0)); CHECK(BuffInit(&b2, 0));
    CHECK(SmemInit(&w1, 0)); CHECK(SmemInit(&w2, 0));
    for(int i = 0; i < 1079; ++i) { msgDup[i] = i * 3; }
    naclKeyPair(spk, ssk); naclKeyPair(cpk, csk);

    
    CHECK(SmemReserve(&w1, NACLH_BYTES));
    { // Handshake
        CHECK(SprotClientHandshake(&s1, spk, csk, &w1) != SPROT_HS_SEND);
        CHECK(SprotServerHandshake(&s2, cpk, ssk, &w1) != SPROT_HS_RECV);
        CHECK(SprotSize(&s1) != SprotSize(&s2));
        //DEBUG_WINDOW(SprotData(&s1), SprotSize(&s1)); // manual zero'd check
        memcpy(SprotData(&s2), SprotData(&s1), SprotSize(&s1));
        CHECK(SprotClientHandshake(&s1, spk, csk, &w1) != SPROT_HS_RECV);
        CHECK(SprotServerHandshake(&s2, cpk, ssk, &w1) != SPROT_HS_SEND);
        CHECK(SprotSize(&s1) != SprotSize(&s2));
        //DEBUG_WINDOW(SprotData(&s2), SprotSize(&s2)); // manual zero'd check
        memcpy(SprotData(&s1), SprotData(&s2), SprotSize(&s1));
        CHECK(SprotClientHandshake(&s1, spk, csk, &w1) != SPROT_HS_DONE);
        CHECK(SprotServerHandshake(&s2, cpk, ssk, &w1) != SPROT_HS_DONE);
    }
    for(int i = 0; i < 3; ++i) { // Encryption
        BuffClear(&b1); BuffPushArr(&b1, msgDup, 1079);
        SmemClear(&w2); SmemPushArr(&w2, msgDup, 1079);

    #define A2B()                                           \
        CHECK(BuffSPEncrypt(&b1, &s1, &b2));                 \
        SPEncryptFinish(&s1, SmemData(&w1));             \
        CHECK(BuffSPDecrypt(&b1, &s2, &b2, SmemData(&w1)));  \
        CHECK(memcmp(BuffData(&b1), msgDup, 1079));         \

    #define B2A()                                           \
        CHECK(BuffSPEncrypt(&b1, &s2, &b2));                 \
        SPEncryptFinish(&s2, SmemData(&w1));             \
        CHECK(BuffSPDecrypt(&b1, &s1, &b2, SmemData(&w1)));  \
        CHECK(memcmp(BuffData(&b1), msgDup, 1079));         \

    #define A2BM()                                           \
        CHECK(BuffSPEncryptUnmut( &b1,&s1, &b2));                 \
        CHECK(BuffSPDecrypt( &b1, &s2,&b2, SmemData(&w1)));  \
        CHECK(memcmp(BuffData(&b1), msgDup, 1079));         \

    #define B2AM()                                           \
        CHECK(BuffSPEncryptUnmut(&b1,&s2,  &b2));                 \
        CHECK(BuffSPDecrypt( &b1, &s1,&b2, SmemData(&w1)));  \
        CHECK(memcmp(BuffData(&b1), msgDup, 1079));         \

        //

    #define SA2B()                                           \
        CHECK(SmemSPEncrypt(&w2,&s1,  &w1));                 \
        SPEncryptFinish(&s1, SmemData(&w1));             \
        CHECK(SmemSPDecrypt( &w2,&s2, &w1, SmemData(&w1)));  \
        CHECK(memcmp(SmemData(&w2), msgDup, 1079));         \

    #define SB2A()                                           \
        CHECK(SmemSPEncrypt( &w2,&s2, &w1));                 \
        SPEncryptFinish(&s2, SmemData(&w1));             \
        CHECK(SmemSPDecrypt( &w2,&s1, &w1, SmemData(&w1)));  \
        CHECK(memcmp(SmemData(&w2), msgDup, 1079));         \

    #define SA2BM()                                           \
        CHECK(SmemSPEncryptUnmut( &w2,&s1, &w1));                 \
        CHECK(SmemSPDecrypt( &w2, &s2,&w1, SmemData(&w1)));  \
        CHECK(memcmp(SmemData(&w2), msgDup, 1079));         \

    #define SB2AM()                                           \
        CHECK(SmemSPEncryptUnmut( &w2,&s2, &w1));                 \
        CHECK(SmemSPDecrypt( &w2,&s1, &w1, SmemData(&w1)));  \
        CHECK(memcmp(SmemData(&w2), msgDup, 1079));         \

        A2B(); A2B(); B2A(); A2B(); A2BM();
        B2AM(); B2AM();
        A2B(); A2B(); B2A(); A2B(); A2BM();
        SA2B(); SA2B(); SB2A(); SA2B(); SA2BM();
        SB2AM(); SB2AM();
        SA2B(); SA2B(); SB2A(); SA2B(); SA2BM();
    }

    Ret err = 0;
CLEAN:
    SmemDe(&w1); SmemDe(&w2);
    BuffDe(&b1); BuffDe(&b2);
    SprotDe(&s1); SprotDe(&s2);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}

