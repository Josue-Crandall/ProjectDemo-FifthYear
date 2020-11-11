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

#include "base64.h"
#include "../rng/rng.h"
#include <string.h>

#define TEST(TNAME, TYPE, VAL, VAL2) \
Ret TNAME() { \
    PRAI(TYPE, name1); PRAI(TYPE, name2); \
    CHECK(TYPE##Init(name1, 4096));CHECK(TYPE##Init(name2, 4096)); \
    \
    for(usize i = 0; i < 4096; ++i) { \
        devURandom(TYPE##Data(name1), i); \
        u8 dup[i]; memcpy(dup, TYPE##Data(name1), i); \
        CHECK(TYPE##BinToB64(name1, dup, i, VAL)); \
        CHECK(TYPE##B64ToBin(name2, TYPE##Data(name1), TYPE##Size(name1), VAL2)); \
        CHECK(TYPE##Size(name2) != i); \
        CHECK(memcmp(dup, TYPE##Data(name2), i)); \
    } \
    \
    Ret ret = 0; \
CLEAN: \
    TYPE##De(name1); TYPE##De(name2); \
    return ret; \
FAILED: \
    ret = -1; \
    goto CLEAN; \
} \


TEST(x, Buff, 0, 0);
TEST(x3, Buff, 1, 0);
TEST(x4, Buff, 1, 1);
TEST(sx, Smem, 0, 0);
TEST(sx3, Smem, 1, 0);
TEST(sx4, Smem, 1, 1);

int main(void) {
    printf("Starting test ... \n");
    CHECK(x());
    CHECK(x3());
    CHECK(x4());
    CHECK(sx());
    CHECK(sx3());
    CHECK(sx4());
    printf("Passed test!\n");
    return 0;
FAILED:
    printf("Failed test!\n");
    return 0;
}
