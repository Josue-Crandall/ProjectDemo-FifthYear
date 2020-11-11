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

#include "../api/api.h"
#include "../file/file.h"
#include "fwatch.h"

#define TEST_FILE_NAME "fwatch.h"

static Ret displayFile(Buff *fb) {
    system("clear");
    CHECK(BuffLoadFile(fb, TEST_FILE_NAME));
    BuffPush(fb, 0);
    printf("%s:\n%s\n", TEST_FILE_NAME, BuffData(fb));
    return 0;
FAILED:
    return -1;
}

int main(void) {
    PRAI(FWatch, fw1); 
    PRAI(Buff, fb);
    CHECK(FWatchInit(fw1, TEST_FILE_NAME));
    CHECK(BuffInit(fb, 0));

    CHECK(displayFile(fb));
    while(1) {
        Ret res;
        NEG_CHECK(res = FWatchEmpty(fw1));
        sleep(1);
        CHECK(displayFile(fb));
    }

    // handle ctrl-c V
    //system("clear");

FAILED:
    BuffDe(fb);
    FWatchDe(fw1);
    return 0; 
}