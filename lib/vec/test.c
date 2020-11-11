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

#include <stdio.h>

#include "vec.h"
#include "../macros/macros.h"
#include <stdlib.h>
#include <time.h>

DD_VEC(Vec, usize *, free(val), STD_ALLOC);

Ret p1(void *pred, usize **val) {
    void *ign = pred;
    return **val > 7;
}
struct { VecPredCB cb; } pred = {&p1};

Ret test0() {
    PRAI(Vec, v1);
    PRAI(Vec, v2);
    VecInit(v1, 0);
    VecInit(v2, 72);

    for(usize i = 0; i < 37; ++i) {
        VecPush(v1, stdMalloc(1, sizeof(usize)));
        *VecData(v1)[i] = i;
    }

    for(VecGen gen = VecBeg(v1); VecNext(&gen); NOP) { printf("%zu, ", **gen.val); }
    printf("\n");

    VecRm(v1, 0, VecSize(v1), &pred);

    for(VecGen gen = VecBeg(v1); VecNext(&gen); NOP) { printf("%zu, ", **gen.val); }
    printf("\n");

    Ret ret = 0;
CLEAN:
    VecDe(v1);
    VecDe(v2);
    return ret;
FAILED:
    ret = -1;
    goto CLEAN;
}

int main(void) {
    usize x = 5; UNSIGNED_LSHIFT_CHECK(x, usize); CHECK(x != 10);

    CHECK(test0());
    printf("pass\n");
    return 0;
FAILED:
    printf("---- FAILED ---\n");
    return 0;
}
