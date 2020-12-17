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

static_assert(STR_EXTRA_BASE_CAPACITY + 1 > STR_EXTRA_BASE_CAPACITY, "overflow check");

#include "../file/file.h"

#include <errno.h>

static Ret StrInit(Str *str, char* base) {
    memset(str, 0, sizeof(Str));

    usize baseLen = strlen(base);
    usize allocLen = baseLen;
    ADD_CHECK(allocLen, STR_EXTRA_BASE_CAPACITY + 1, usize);
    CHECK(BuffInit(str, allocLen));
    BuffSetSize(str, baseLen + 1);
    memcpy(BuffData(str), base, baseLen);
    BuffData(str)[baseLen] = 0;
    
    return 0;
FAILED:
    BuffDe(str);
    memset(str, 0, sizeof(Str));
    return -1;
}
static Ret StrFromFile(Str *str, char *path) {
    CHECK(BuffInit(str, 0));
    CHECK(BuffLoadFile(str, path));
    CHECK(StrCPush(str, '\0'));
    return 0;
FAILED:
    BuffDe(str);
    memset(str, 0, sizeof(Str));
    return -1;
}
static char * StrIng(Str *str) { return (char *)BuffData(str); }
static usize StrLen(Str *str) { return BuffSize(str) - 1; }
static void StrDe(Str *str) { BuffDe(str); }
static void StrPop(Str *str, usize amt) {
    usize len = BuffSize(str);
    BuffData(str)[len - (amt + 1)] = 0;
    BuffSetSize(str, len - amt);
}
static Ret StrPush(Str *str, const char *const cstr) {
    BuffPop(str);
    usize cstrLen = strlen(cstr);
    ADD_CHECK(cstrLen, 1, usize);
    CHECK(BuffPushArr(str, (u8 *)cstr, cstrLen));
    return 0;
FAILED:
    CHECK_IGN(BuffPush(str, '\0'));
    return -1;
}
static Ret StrCPush(Str *str, char ch) {
    CHECK(BuffPush(str, 0));
    BuffData(str)[BuffSize(str) - 2] = ch;
    return 0;
FAILED:
    
    return -1;
}
static void StrClear(Str *str) {
    BuffData(str)[0] = 0;
    BuffSetSize(str, 1);
}
static usize strHash(const char *cstr) {
    usize res = 0;
    while(*cstr) {
        res = (res << 1) | (res >> (sizeof(usize) * 8 - 1));
        res += *cstr++;
    }
    return res;
}
static usize StrHash(Str *str) {
    return strHash((char *)BuffData(str));
}
static Ret StrEq(Str *R() lhs, Str *R() rhs) {
    return !strcmp((char *)BuffData(lhs), (char *)BuffData(rhs));
}
static Ret StrCEq(Str *R() lhs, char *rhs) {
    return !strcmp((char *)BuffData(lhs), rhs);
}
static Ret StrReadLine(Str *R() str, int fd) {
    StrClear(str);

    while(1) {
        char temp;
        ssize_t amt = read(fd, &temp, sizeof(char));
        if(0 == amt) { return 1; }
        else if(amt < 0 && errno != EINTR) { DEBUG_LOG("StrReadLine failed\n"); goto FAILED; }
        else if(temp == '\n') { return 0; }
        else { CHECK(StrCPush(str, temp)); }
    }

FAILED:
    return -1;
}

