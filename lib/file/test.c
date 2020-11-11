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

#include "file.h"
#include <string.h>
#include <stdlib.h>

int main(void) {
    printf("compiled.\n--- starting testing ---\n");

    Ret test0(); CHECK(test0());
    Ret test1(); CHECK(test1());
    Ret test2(); CHECK(test2());
    Ret test3(); CHECK(test3());
    Ret test4(); CHECK(test4());
    Ret test5(); CHECK(test5());

    printf("--- PASSED --- \n");
    return 0;

FAILED:
    printf("--- FAILED --- \n");
    return 0;
}

static const u8 TEST_TXT_VALUES[6] = {'1','2','3','4','5','\n'};
static const u8 TEST_TXT_VALUES_X3[18] = {'1','2','3','4','5','\n','1','2','3','4','5','\n','1','2','3','4','5','\n'};

Ret test0() {
    CHECK(6 != getFileSize("test.txt"));
    CHECK(getFileSize("not_there.txt") > -1);

    u8 buf[6], buf2[12];
    CHECK(fileLoad("test.txt", buf, sizeof(buf)));
    CHECK(memcmp(TEST_TXT_VALUES, buf, sizeof(buf)));
    CHECK(!fileLoad("test.txt", buf2, 12));

    return 0;
FAILED:
    return 1;
}
Ret test1() {
    Ret err = 0;

    int fd = -1, fd2 = -1;
    u8 buf[6];
    u8 buf2[12];
    Buff buf3 = {0};
    Smem buf4 = {0};

    CHECK( (fd = fileOpen("test.txt")) < 0);
    CHECK( (fd2 = fileOpen("test.txt")) < 0);
    CHECK(fileRead(fd, buf, sizeof(buf)));
    CHECK(memcmp(TEST_TXT_VALUES, buf, sizeof(buf)));
    CHECK(!fileRead(fd2, buf2, sizeof(buf2)));

    CHECK(BuffInit(&buf3, 0));
    CHECK(BuffLoadFile(&buf3, "test.txt"));
    CHECK(BuffSize(&buf3) != 6);
    CHECK(memcmp(TEST_TXT_VALUES, BuffData(&buf3), 5));

    CHECK(SmemInit(&buf4, 0));
    CHECK(SmemLoadFile(&buf4, "test.txt"));
    CHECK(SmemSize(&buf4) != 6);
    CHECK(memcmp(TEST_TXT_VALUES, SmemData(&buf4), 5));

CLEAN:
    FD_CLOSE(fd);
    FD_CLOSE(fd2);
    BuffDe(&buf3);
    SmemDe(&buf4);
    return err;
FAILED:
    err = 1;
    goto CLEAN;
}
Ret test2() { 
    Ret err = 0;

    int fd = -1, fd2 = -1;
    u8 buf[6];
    u8 buf2[12];
    u8 *buf3 = NULL;

    CHECK( (fd = fileOpen("test.txt")) < 0);
    CHECK(fileWrite(fd, (u8 *)TEST_TXT_VALUES, sizeof(TEST_TXT_VALUES)));

CLEAN:
    FD_CLOSE(fd);
    FD_CLOSE(fd2);
    free(buf3);
    return err | test0() | test1();
FAILED:
    err = 1;
    goto CLEAN; 
}
Ret test3() { 
    Ret err = 0;

    CHECK(fileSave("test.txt", (u8 *)TEST_TXT_VALUES, sizeof(TEST_TXT_VALUES)));

CLEAN:
    return err | test0() | test1();
FAILED:
    err = 1;
    goto CLEAN; 
}
Ret test4() {
    Ret err = 0;

    int fd = -1;
    u8 buf[sizeof(TEST_TXT_VALUES_X3)];

    for(int i = 0; i < 3; ++i) { CHECK(fileAppend("test2.txt", (u8 *)TEST_TXT_VALUES, sizeof(TEST_TXT_VALUES))); }
    CHECK(fileLoad("test2.txt", buf, sizeof(buf)));
    CHECK(memcmp(TEST_TXT_VALUES_X3, buf, sizeof(buf)));
    CHECK( (fd = fileOpen("test2.txt")) < 0);
    CHECK(sizeof(TEST_TXT_VALUES_X3) != fileSeekEnd(fd));
    for(unsigned i = 0; i < sizeof(TEST_TXT_VALUES_X3); ++i) { CHECK(fileSeek(fd, i)); }

    CHECK(fileSeek(fd, 0));
    CHECK(fileTrunc(fd, 5));
    CHECK(5 != fileSeekEnd(fd));

CLEAN:
    FD_CLOSE(fd);
    CHECK(fileRm("test2.txt"));
    return err;
FAILED:
    err = 1;
    goto CLEAN;
}
Ret test5() {
    RAI(Buff, buf); RAI(Buff, ws);
    RAI(Smem, buf2); RAI(Smem, ws2);
    CHECK(BuffInit(&buf, 0)); CHECK(BuffInit(&ws, 0));
    CHECK(SmemInit(&buf2, 0)); CHECK(SmemInit(&ws2, 0));

    for(int i = 0; i < 4; ++i) {
        CHECK(BuffLoadB64File(&buf, "b64.txt", &ws));
        CHECK(BuffSize(&buf) != 7);
        CHECK(memcmp("12345z\n", BuffData(&buf), 7));
        CHECK(BuffSaveB64File(&buf, "b64.txt", &ws));

        CHECK(SmemLoadB64File(&buf2, "b64.txt", &ws2));
        CHECK(SmemSize(&buf2) != 7);
        CHECK(memcmp("12345z\n", SmemData(&buf2), 7));
        CHECK(SmemSaveB64File(&buf2, "b64.txt", &ws2));        
    }

    Ret err = 0;
CLEAN:
    BuffDe(&buf); BuffDe(&ws);
    SmemDe(&buf2); SmemDe(&ws2);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}