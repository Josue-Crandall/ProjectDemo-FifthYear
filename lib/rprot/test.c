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

#include "rprot.h"
#include "../macros/term.h"

#include <string.h>
#include <stdlib.h>

int main(void) {
    printf("compiled\n");

    Ret t0(); CHECK(t0());
    Ret t1(); CHECK(t1());
    Ret t2(); CHECK(t2());

    printf("pass\n");
    return 0;
FAILED:
    printf(" ---  %sFAILED TEST%s ---\n", ANSI_COLOR_BOLD_RED, ANSI_COLOR_DEFAULT);
    return 0;
}

Ret t0() {
    RAI(Rprot, p1); RAI(Rprot, p2);
    RAI(Buff, msg); RAI(Buff, ws); RAI(Buff, msg3);
    RAI(Smem, smsg); RAI(Smem, sws); RAI(Smem, hsws);

    CHECK(BuffInit(&msg, 0)); CHECK(BuffInit(&ws, 0)); CHECK(BuffInit(&msg3, 0));
    CHECK(SmemInit(&smsg, 0)); CHECK(SmemInit(&sws, 0)); CHECK(SmemInit(&hsws, NACLH_BYTES));

    for(int i = 0; i < 2; ++i) {
        CHECK(RprotMake("k1", "k2"));

        for(int j = 0; j < 7; ++j) {
            
            CHECK(RprotInit(&p1, "k1"));
            CHECK(RprotInit(&p2, "k2"));

            for(int TEST_SIZE = 1; TEST_SIZE < 17; ++TEST_SIZE) {
                u8 msgDup[TEST_SIZE]; BuffClear(&msg); BuffClear(&msg3); SmemClear(&smsg);
                for(int i = 0; i < TEST_SIZE; ++i) {
                    msgDup[i] = i;
                    CHECK(BuffPush(&msg, i));
                    CHECK(BuffPush(&msg3, i));
                    CHECK(SmemPush(&smsg, i));
                }

            #define A2B() \
                CHECK(BuffRPEncrypt(&msg,&p1, &ws, &hsws));              \
                RPEncryptFinish(&p1, SmemData(&hsws));                \
                CHECK(BuffRPDecrypt(&msg,&p2,&ws,&hsws));              \
                CHECK(memcmp(BuffData(&msg), msgDup, sizeof(msgDup)));  \
                CHECK(SmemRPEncrypt(&smsg,&p1, &sws, &hsws));                \
                RPEncryptFinish(&p1, SmemData(&hsws));                \
                CHECK(SmemRPDecrypt(&smsg,&p2,&sws,&hsws));                \
                CHECK(memcmp(SmemData(&smsg), msgDup, sizeof(msgDup)));
            #define B2A() \
                CHECK(BuffRPEncrypt(&msg,&p2,&ws,&hsws));              \
                RPEncryptFinish(&p2,SmemData(&hsws));                \
                CHECK(BuffRPDecrypt(&msg,&p1,  &ws, &hsws));              \
                CHECK(memcmp(BuffData(&msg), msgDup, sizeof(msgDup)));  \
                CHECK(SmemRPEncrypt(&smsg,&p2,&sws,&hsws));                \
                RPEncryptFinish(&p2, SmemData(&hsws));                \
                CHECK(SmemRPDecrypt(&smsg,&p1,&sws,&hsws));                \
                CHECK(memcmp(SmemData(&smsg), msgDup, sizeof(msgDup)));
            #define CA2B() \
                CHECK(BuffRPEncryptUnmut(&msg,&p1,&ws));         \
                CHECK(BuffRPDecrypt(&msg,&p2,&ws,&hsws));              \
                CHECK(memcmp(BuffData(&msg), msgDup, sizeof(msgDup)));  \
                CHECK(SmemRPEncryptUnmut(&smsg,&p1,&sws));           \
                CHECK(SmemRPDecrypt(&smsg,&p2,&sws,&hsws));                \
                CHECK(memcmp(SmemData(&smsg), msgDup, sizeof(msgDup)));
            #define CB2A() \
                CHECK(BuffRPEncryptUnmut(&msg,&p2,&ws));         \
                CHECK(BuffRPDecrypt(&msg,&p1,&ws,&sws));              \
                CHECK(memcmp(BuffData(&msg), msgDup, sizeof(msgDup)));  \
                CHECK(SmemRPEncryptUnmut(&smsg,&p2,&sws));           \
                CHECK(SmemRPDecrypt(&smsg,&p1,&sws,&hsws));                \
                CHECK(memcmp(SmemData(&smsg), msgDup, sizeof(msgDup)));
            #define AA2BB() \
                CHECK(BuffRPEncrypt(&msg,&p1,&ws,&sws));              \
                RPEncryptFinish(&p1, SmemData(&hsws));                \
                CHECK(SmemRPEncrypt(&smsg,&p1,&sws,&hsws));                \
                RPEncryptFinish(&p1, SmemData(&hsws));                \
                CHECK(BuffRPDecrypt(&msg,&p2,&ws,&sws));              \
                CHECK(memcmp(BuffData(&msg), msgDup, sizeof(msgDup)));  \
                CHECK(SmemRPDecrypt(&smsg,&p2,&sws,&hsws));                \
                CHECK(memcmp(SmemData(&smsg), msgDup, sizeof(msgDup)));
            #define BB2AA() \
                CHECK(BuffRPEncrypt(&msg,&p2,&ws,&sws));              \
                RPEncryptFinish(&p2, SmemData(&hsws));                \
                CHECK(SmemRPEncrypt(&smsg,&p2,&sws,&hsws));                \
                RPEncryptFinish(&p2, SmemData(&hsws));                \
                CHECK(BuffRPDecrypt(&msg,&p1,&ws,&sws));              \
                CHECK(memcmp(BuffData(&msg), msgDup, sizeof(msgDup)));  \
                CHECK(SmemRPDecrypt(&smsg,&p1,&sws,&hsws));                \
                CHECK(memcmp(SmemData(&smsg), msgDup, sizeof(msgDup)));
            #define BAB2ABA() \
                CHECK(BuffRPEncrypt(&msg,&p1,  &ws, &sws));              \
                RPEncryptFinish(&p1, SmemData(&hsws));                \
                CHECK(SmemRPEncrypt(&smsg,&p1,&sws,&hsws));                \
                RPEncryptFinish(&p1, SmemData(&hsws));                \
                CHECK(BuffRPEncrypt(&msg3,&p2,  &ws, &sws));             \
                RPEncryptFinish(&p2, SmemData(&hsws));                \
                CHECK(BuffRPDecrypt(&msg,&p2,  &ws, &sws));              \
                CHECK(memcmp(BuffData(&msg), msgDup, sizeof(msgDup)));  \
                CHECK(SmemRPDecrypt(&smsg,&p2,  &sws, &hsws));                \
                CHECK(memcmp(SmemData(&smsg), msgDup, sizeof(msgDup))); \
                CHECK(BuffRPDecrypt(&msg3,&p1,  &ws, &sws));             \
                CHECK(memcmp(BuffData(&msg3), msgDup, sizeof(msgDup)));  

                A2B(); CA2B(); A2B(); B2A(); CB2A(); A2B();
                AA2BB(); AA2BB(); BB2AA(); AA2BB(); BAB2ABA();
                A2B(); CB2A(); A2B(); B2A(); B2A(); CA2B(); A2B();
                BAB2ABA();
                
                A2B();
                B2A();
                A2B(); A2B();
                B2A();
                B2A();
                A2B();
                B2A();B2A();
                A2B();
                B2A();

                A2B();A2B();
                B2A();B2A();
                A2B();
                B2A();B2A();B2A();
                A2B();
                B2A();

                AA2BB();
                BB2AA();
                A2B();A2B();
                BAB2ABA();
            }
            if(RprotDe(&p1, "k1")) { DEBUG_LOG("k1 destroy failed!\n"); }
            if(RprotDe(&p2, "k2")) { DEBUG_LOG("k2 destroy failed!\n"); }
            
        }
    }

    CHECK(RprotInit(&p1, "k1"));
    CHECK(RprotInit(&p2, "k2"));
    Ret err = 0;
CLEAN:
    if(RprotDe(&p1, "k1")) { DEBUG_LOG("k1 destroy failed!\n"); }
    if(RprotDe(&p2, "k2")) { DEBUG_LOG("k2 destroy failed!\n"); }
    BuffDe(&msg); BuffDe(&ws); BuffDe(&msg3);
    SmemDe(&smsg); SmemDe(&sws); SmemDe(&hsws);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}

Ret t1() {
    return 0;
} 

Ret t2() {
    return 0;
}