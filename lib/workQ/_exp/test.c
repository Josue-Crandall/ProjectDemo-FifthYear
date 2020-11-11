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

#include "workQ.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

SPRAI(WorkQ, q);

static void de(void *arg) { free(arg); }
static void cb(void *arg) {
    int *val = arg;
    if(*val) {
        printf("val %d\n", *val);
        sleep(1);
        --*val;
        WorkQPush(q, cb, de, val);
    }
    else { 
        free(val);
        WorkQStop(q); 
    }
}

int main(void) {
    CHECK(WorkQInit(q));

    for(int i = 0; i < 12; ++i) {
        int *arg;
        NULL_CHECK(arg = malloc(sizeof(int)));
        *arg = 3;
        CHECK(WorkQPush(q, cb, de, arg));
    }

FAILED:
    WorkQDe(q);
    return 0;
}
