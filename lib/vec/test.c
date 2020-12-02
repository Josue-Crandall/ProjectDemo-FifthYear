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

#include "../timer/timer.h"
SPRAI(Timer, t);

#include <stdio.h>

#include "vec.h"
#include "../macros/macros.h"
#include <stdlib.h>
#include <time.h>

DD_VEC(Vec, usize *, STD_ALLOCFree(val, sizeof(usize *)), MLOCK_ALLOC);

Ret p1(void *pred, usize **val) {
    void *ign = pred;
    return **val > 7;
}
static VecPred predPtr = &p1;

Ret test0() {
    PRAI(Vec, v1);
    PRAI(Vec, v2);

    CHECK(VecInit(v1, 0));
    CHECK(VecInit(v2, 72));

    for(usize i = 0; i < 37; ++i) {
        //printf("%zu\n", i);
        VecPush(v1, STD_ALLOCMalloc(1, sizeof(usize)));
        *VecData(v1)[i] = i;
    }

    for(usize **iter = VecData(v1), **end = iter + VecSize(v1); iter != end; ++iter) {
        //printf("%zu, ", **iter);
    }

    //printf("\n");

    VecRm(v1, 0, VecSize(v1), &predPtr);

    for(usize **iter = VecData(v1), **end = iter + VecSize(v1); iter != end; ++iter) {
        //printf("%zu, ", **iter);
    }
    //printf("\n");

    Ret res = 0;
CLEAN:
    VecDe(v1);
    VecDe(v2);
    return res;
FAILED:
    res = -1;
    goto CLEAN;
}

int main(void) {
    usize x = 5; UNSIGNED_LSHIFT_CHECK(x, usize); CHECK(x != 10);

    printf("%zu\n", sizeof(Vec));

    TimerStart(t);
    for(int i = 0; i < 6751 * 16; ++i) { CHECK(test0()); }
    TimerStop(t);
    TimerDisplay(t);
    printf("pass\n");
    return 0;
FAILED:
    printf("---- FAILED ---\n");
    return 0;
}
