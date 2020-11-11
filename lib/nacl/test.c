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

#include "nacl.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h> // srand

int main(void) {
    srand(time(0));

    printf("compiled\n");
    for(int i = 0; i < 1; ++i) {
        Ret t0(); CHECK(t0());
        Ret t1(); CHECK(t1());
        Ret t2(); CHECK(t2());
        Ret t3(); CHECK(t3());
    } 
    printf("passing\n");
    return 0;

FAILED:
    printf("--- FAILED --- \n");
    return 0;
}

Ret t0() {
    usize testSize = 16; // Manual inspection size

    Buff text, ws;
    CHECK(BuffInit(&text,0)); CHECK(BuffInit(&ws,0));
    CHECK(BuffReserve(&text, testSize + NACLA_BYTES));

    // give it a few shakes
    for(int i = 0; i < 50000; ++i) {
    testSize = rand() % (NACL_MAX_VALIDATED_LEN + 1); // Testing size
    //

    u8 pk[NACLA_PUB_BYTES], sk[NACLA_SEC_BYTES];
    u8 ZEROS[testSize + crypto_box_ZEROBYTES]; memset(ZEROS, 0, sizeof(ZEROS));
    u8 textDup[testSize];
    naclKeyPair(pk, sk);

    BuffClear(&text);
    for(usize i = 0; i < testSize; ++i) { BuffPush(&text, i); }
    memcpy(textDup, BuffData(&text), sizeof(textDup));
    
    CHECK(BuffAEncrypt(&text, pk, sk, &ws));
    // check zero'd pt in ws
    CHECK(memcmp(ZEROS, BuffData(&ws), (testSize + crypto_box_ZEROBYTES)));

    // Checking for loose zeros by manual inspection
    //DEBUG_WINDOW(BuffData(&text), BuffSize(&text)); // check

    // manual auth checker
    //BuffData(&text)[34] = 3; // check

    CHECK(BuffADecrypt(&text, pk, sk, &ws));
    CHECK(memcmp(BuffData(&text), textDup, sizeof(textDup)));
    // check zero'd pt in ws
    CHECK(memcmp(ZEROS, BuffData(&ws) + (testSize + crypto_box_ZEROBYTES), (testSize + crypto_box_ZEROBYTES)));

    //
    } // shake brace
    //

    printf("passed NACL A test\n");

    Ret err = 0;
CLEAN:
    BuffDe(&text); BuffDe(&ws);
    return err;

FAILED:
    err = -1;
    goto CLEAN;
}

Ret t1() {
    usize testSize = 16; // Manual inspection size

    Buff text, ws;
    CHECK(BuffInit(&text,0)); CHECK(BuffInit(&ws,0));
    CHECK(BuffReserve(&text, testSize + NACLA_BYTES));

    // give it a few shakes
    for(int i = 0; i < 50000; ++i) {
    testSize = rand() % (NACL_MAX_VALIDATED_LEN + 1); // Testing size
    //

    u8 key[NACLS_KEY_BYTES];
    u8 ZEROS[testSize + crypto_secretbox_ZEROBYTES]; memset(ZEROS, 0, sizeof(ZEROS));
    u8 textDup[testSize];
    randombytes(key, sizeof(key));

    BuffClear(&text);
    for(usize i = 0; i < testSize; ++i) { BuffPush(&text, i); }
    memcpy(textDup, BuffData(&text), sizeof(textDup));
    
    CHECK(BuffSEncrypt(&text, key, &ws));
    // check zero'd pt in ws
    CHECK(memcmp(ZEROS, BuffData(&ws), (testSize + crypto_secretbox_ZEROBYTES)));

    // Checking for loose zeros by manual inspection
    //DEBUG_WINDOW(BuffData(&text), BuffSize(&text)); // check

    // manual auth checker
    //BuffData(&text)[34] = 3; // check

    CHECK(BuffSDecrypt(&text, key, &ws));
    CHECK(memcmp(BuffData(&text), textDup, sizeof(textDup)));
    // check zero'd pt in ws
    CHECK(memcmp(ZEROS, BuffData(&ws) + (testSize + crypto_secretbox_ZEROBYTES), (testSize + crypto_secretbox_ZEROBYTES)));

    //
    } // shake brace
    //

    printf("passed NACL S test\n");

    Ret err = 0;
CLEAN:
    BuffDe(&text); BuffDe(&ws);
    return err;

FAILED:
    err = -1;
    goto CLEAN;
}

Ret t2() {
    usize testSize = 16; // Manual inspection size

    RAI(Smem, text); RAI(Smem, ws);
    CHECK(SmemInit(&text, 0));
    CHECK(SmemInit(&ws, 0));
    //CHECK(SmemReserve(&text, NACL_MAX_VALIDATED_LEN * 8));
    //CHECK(SmemReserve(&ws, NACL_MAX_VALIDATED_LEN * 8));
    // NACL_MAX_VALIDATED_LEN*8

    // give it a few shakes
    for(int i = 0; i < 50000; ++i)
    { 
    testSize = rand() % (NACL_MAX_VALIDATED_LEN + 1); // Testing size

    //
    u8 pk[NACLA_PUB_BYTES], sk[NACLA_SEC_BYTES];
    u8 ZEROS[testSize + crypto_box_ZEROBYTES]; memset(ZEROS, 0, sizeof(ZEROS));
    u8 textDup[testSize];
    naclKeyPair(pk, sk);
    SmemClear(&text);
    for(usize i = 0; i < testSize; ++i) { SmemPush(&text, i); }
    memcpy(textDup, SmemData(&text), sizeof(textDup));


    //????
    //printf("in !!!!!!!!\n"); DEBUG_WINDOW(SmemData(&text), 8);

    CHECK(SmemAEncrypt(&text, pk, sk, &ws));
    CHECK(memcmp(ZEROS, SmemData(&ws), (testSize + crypto_box_ZEROBYTES)));


    // check zero'd pt in ws


    // Checking for loose zeros by manual inspection
    //DEBUG_WINDOW(SmemData(&text), SmemSize(&text)); // check

    // manual auth checker
    //SmemData(&text)[34] = 3; // check

    CHECK(SmemADecrypt(&text, pk, sk, &ws));

    //????
    //printf("out!!!!!!!!!!!\n "); DEBUG_WINDOW(SmemData(&text), 8);

    // check zero'd pt in ws
    CHECK(memcmp(ZEROS, SmemData(&ws) + (testSize + crypto_box_ZEROBYTES), (testSize + crypto_box_ZEROBYTES)));

    //????
    //CHECK(memcmp(SmemData(&text), textDup, sizeof(textDup)));

    //
    } // shake brace
    //

    printf("passed NACL A_s test\n");

    Ret err = 0;
CLEAN:
    SmemDe(&text);
    SmemDe(&ws);
    return err;

FAILED:
    err = -1;
    goto CLEAN;
}

Ret t3() {
    usize testSize = 16; // Manual inspection size

    Smem text, ws;
    CHECK(SmemInit(&text, NACL_MAX_VALIDATED_LEN* 2)); CHECK(SmemInit(&ws, NACL_MAX_VALIDATED_LEN * 2));

    // give it a few shakes
    for(int i = 0; i < 50000; ++i)
    { 
    testSize = rand() % (NACL_MAX_VALIDATED_LEN + 1); // Testing size
    //

    u8 key[NACLS_KEY_BYTES];
    u8 ZEROS[testSize + crypto_secretbox_ZEROBYTES]; memset(ZEROS, 0, sizeof(ZEROS));
    u8 textDup[testSize];
    randombytes(key, sizeof(key));

    SmemClear(&text); memset(SmemData(&text),0, SmemSize(&text));
    for(usize i = 0; i < testSize; ++i) { SmemPush(&text, i); }
    memcpy(textDup, SmemData(&text), sizeof(textDup));
    
    CHECK(SmemSEncrypt(&text, key, &ws));
    // check zero'd pt in ws
    CHECK(memcmp(ZEROS, SmemData(&ws), (testSize + crypto_secretbox_ZEROBYTES)));

    // Checking for loose zeros by manual inspection
    //DEBUG_WINDOW(SmemData(&text), SmemSize(&text)); // check

    // manual auth checker
    //SmemData(&text)[34] = 3; // check

    CHECK(SmemSDecrypt(&text, key, &ws));
    CHECK(memcmp(SmemData(&text), textDup, sizeof(textDup)));
    // check zero'd pt in ws
    CHECK(memcmp(ZEROS, SmemData(&ws) + (testSize + crypto_secretbox_ZEROBYTES), (testSize + crypto_secretbox_ZEROBYTES)));

    //
    } // shake brace
    //

    printf("passed NACL S_s test\n");

    Ret err = 0;
CLEAN:
    SmemDe(&text); SmemDe(&ws);
    return err;

FAILED:
    err = -1;
    goto CLEAN;
}