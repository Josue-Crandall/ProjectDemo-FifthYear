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

#include "thread.h"
#include "../macros/macros.h"
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>

volatile PMutex x;

int main(void) {    
    Ret test0(); CHECK(test0());
    Ret test1(); CHECK(test1());
    // printf("--- PASSING ---\n");
    return 0;
FAILED:
    //printf("--- FAILED ---\n");
    return 0;
}

Ret test0() {
    static char* HW_ARGS[1] = {NULL};
    for(int i = 0; i < 3; ++i) {
        NEG_CHECK(forkedExec("./HW.elf", HW_ARGS, NULL));
    }
    sleep(1);

    return 0;
FAILED:
    return -1;
}

static PMutex m;
static PCond c;

static int count = 0;

void* runner(void *arg) {
    PMutexLock(&m);
    int amt = count;
    if(count == 25) { PCondSignal(&c, 0); }
    else { ++count; }
    PMutexUnlock(&m);

    printf("amt %d\n", amt);

    if(amt == 25) return 0;
    else return runner(arg);
}

Ret test1() {
    Ret haveLock = 0;
    Ret err;

    CHECK(PMutexInit(&m));
    CHECK(PCondInit(&c));

    PMutexLock(&m);
    haveLock = 1;

    PThread threads[15];

    for(int i = 0; i < 15; ++i) { CHECK(PThreadInit(&threads[i], runner, 0));}
    while(count != 25) { PCondWait(&c, &m); }
    err = 0;
CLEAN:
    if(haveLock) { PMutexUnlock(&m); }
    for(int i = 0; i < 15; ++i) { PThreadJoin(&threads[i], NULL); }
    PCondDe(&c);
    PMutexDe(&m);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}



/*
// Bad POnce / PLocal test
static POnce once = PONCE_STATIC_INIT;
static PLocal local;

void initLocal(void) { 
    printf("Were initializing local :D\n");
    PLocalInit(&local); 
}

void *task(void *thing) {
    thing = thing;
    POnceDo(&once, initLocal);
    int *val;
    while(0 == (val = PLocalGet(&local))) {
        int *x = stdMalloc(1, sizeof(int));
        CHECK(!x);
        *x = (int)thing;
        CHECK(PLocalSet(&local, x));
    }

    while(*val) {
        printf("Got %d\n", *val);
        sleep(1);
        --*val;
    }
    printf("0!\n");

FAILED:
    return 0;
}

int main(void) {
    CHECK(PThreadInit(NULL, task, (void *)5));
    CHECK(PThreadInit(NULL, task, (void *)4));
    CHECK(PThreadInit(NULL, task, (void *)7));
    CHECK(PThreadInit(NULL, task, (void *)8));    
    sleep(10);
FAILED:
    return 0;
}

*/