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

#include "oprot.h"
#include "../macros/term.h"

#include <string.h>

int main(void) {
    printf("compiled\n");

    Ret t0(); CHECK(t0());

    printf("pass\n");
    return 0;
FAILED:
    printf(" ---  %sFAILED TEST%s ---\n", ANSI_COLOR_BOLD_RED, ANSI_COLOR_DEFAULT);
    return 0;
}
Ret t0() {
    //(21000000 - 4561200)
    CHECK(OprotMake("First", "Second", 1024 * 437, NULL));
    RAI(Oprot, p1); RAI(Oprot, p2);
    RAI(Buff, b1); RAI(Buff, b2);
    RAI(Smem, s1); RAI(Oprot, s2); RAI(Oprot, s3);

    CHECK(BuffInit(&b1, 0));CHECK(BuffInit(&b2, 0));
    CHECK(SmemInit(&s1, 0));CHECK(SmemInit(&s2, NACLH_BYTES)); CHECK(SmemInit(&s3, NACLH_BYTES));
    for(int i = 0 ; i < 2; ++i) {
        CHECK(OprotInit(&p1, "First"));
        CHECK(OprotInit(&p2, "Second"));
        for(usize i = 1; i < 437; ++i) {
            u8 msg[i + OPROT_BYTES];
            BuffClear(&b1); SmemClear(&s1);
            for(usize j = 0; j < i; ++j) { msg[j] = j; }
            CHECK(BuffPushArr(&b1, msg, sizeof(msg)));
            CHECK(SmemPushArr(&s1, msg, sizeof(msg)));
            
            CHECK(BuffOPEncrypt(&b1,&p1,  &b2, SmemData(&s2)));
            CHECK(BuffOPDecrypt(&b1,&p2,  &b2, SmemData(&s2)));
            CHECK(BuffSize(&b1) != sizeof(msg));
            CHECK(memcmp(msg, BuffData(&b1), sizeof(msg)));

            CHECK(BuffOPEncryptUnmut(&b1,&p2,  &b2));
            CHECK(BuffOPDecrypt(&b1,&p1, &b2, SmemData(&s2)));
            CHECK(BuffSize(&b1) != sizeof(msg));
            CHECK(memcmp(msg, BuffData(&b1), sizeof(msg)));

            CHECK(SmemOPEncrypt(&s1,&p2,  &s2, SmemData(&s3)));
            CHECK(SmemOPDecrypt(&s1,&p1,  &s2, SmemData(&s3)));
            CHECK(SmemSize(&s1) != sizeof(msg));
            CHECK(memcmp(msg, SmemData(&s1), sizeof(msg)));

            CHECK(SmemOPEncryptUnmut(&s1,&p1,  &s2));
            CHECK(SmemOPDecrypt(&s1, &p2,  &s2, SmemData(&s3)));
            CHECK(SmemSize(&s1) != sizeof(msg));
            CHECK(memcmp(msg, SmemData(&s1), sizeof(msg)));
        }
        CHECK(OprotDe(&p1));
        CHECK(OprotDe(&p2));
    }
    CHECK(OprotInit(&p1, "First"));
    CHECK(OprotInit(&p2, "Second"));

    Ret err = 0;
CLEAN:
    SmemDe(&s1); SmemDe(&s2); SmemDe(&s3);
    BuffDe(&b1); BuffDe(&b2);
    if(OprotDe(&p1)) { err = -7; };
    if(OprotDe(&p2)) { err = -7; }
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}