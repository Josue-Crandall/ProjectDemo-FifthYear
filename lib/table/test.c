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

#include <stdlib.h>
#include "table.h"
//#include "backup.h"
#include <time.h>

DD_TABLE(Table, int *, int *, res = (usize)*key, res = *lhs == *rhs, free(key), free(val), STD_ALLOC);

int main(void) {
    printf("Compiled!\n");

    void t0(); t0();

    return 0;
}

void t0() {
    PRAI(Table, t);
    PRAI(Table, t2);
    CHECK(TableInit(t, 0));
    CHECK(TableInit(t2, 0));

    for(int i = 0; i < 72; ++i) {
        int *key = malloc(sizeof(int));
        *key = i;
        int *val = malloc(sizeof(int));
        *val = i * 3;
        CHECK(TablePush(t, key, val, 0));
        CHECK(!TablePush(t, key, val, 0));
    }

    //for(TableGen gen = TableBeg(t); TableNext(&gen); NOP) {
    //    printf("(%d,%d) ", *gen.keys[gen.i], *gen.vals[gen.i]);// scatter check
    //} printf("\n");

    for(int i = 37; i < 72; ++i) { TableRm(t, &i, 0); }

    for(TableGen gen = TableBeg(t); TableNext(&gen); NOP) {
        CHECK(*gen.vals[gen.i] > 36 * 3);
    }


    for(int i = 0; i < 3; ++i) {
        CHECK(TableAt(t2, &i, 0));
        Ret tres = 1;
        int *ptemp = stdMalloc(1, sizeof(int));
        *ptemp = i;
        int **temp = TableAt(t2, ptemp, &tres);
        CHECK(tres < 1);
        *temp = stdMalloc(1, sizeof(int));
        **temp = i;
    }
    for(int i = 0; i < 3; ++i) {
        Ret tres = 1;
        int **temp = TableAt(t2, &i, &tres);
        CHECK(tres != 0);
    }
    for(int i = 0; i < 3; ++i) {
        int **temp = TableAt(t2, &i, 0);
        CHECK(temp == NULL);
    }

    TableDe(t);
    TableDe(t2);
    return;
FAILED:
    exit(0);
}