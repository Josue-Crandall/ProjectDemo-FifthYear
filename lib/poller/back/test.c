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

#include "poller.h"
#include "../macros/term.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    TaskFace *R() vptr;
    int *count;
    PTrig triggers[2];
} EchoTask;
static Ret echoCB(void *const R() ptask) {
    EchoTask *R() task = ptask;
    char temp;
    while(1 == read(0, &temp, 1)) { 
        printf("%d: %c\n", ++(*task->count), temp); 
    }
    return TASKF_RET_REPOLL;
}
static PTrig * echoTrigger(void *const R() ptask) {
    EchoTask *R() task = ptask;
    return task->triggers;
}
static void echoDe(void *const R() ptask) {
    EchoTask *R() task = ptask;
    if(task) { free(task->count); }
}
static TaskFace echoTaskInterface = { echoCB, echoTrigger, echoDe };
static EchoTask task = { &echoTaskInterface, NULL, {{0,PTRIG_IN},{0, 0}}};

Ret t0() {
    Ret err = 0;
    Poller p = {0};

    CHECK(PollerInit(&p, 5000));
    CHECK(NULL == (task.count = malloc(sizeof(int))));
    *task.count = 0;
    CHECK(PollerPoll(&p, &task));

    sleep(10);

CLEAN:
    PollerDe(&p);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}

Ret t1() {
    MPoll mpoll;
    CHECK(MPollInit(&mpoll, 5000));

    CHECK(NULL == (task.count = malloc(sizeof(int))));
    *task.count = 0;
    CHECK(MPollSetPoll(&mpoll, &task));

    MPollLaunch(&mpoll);

    Ret err = 0;
CLEAN:
    MPollDe(&mpoll);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}

int main(void) {
    CHECK(fcntl(0, F_SETFL, O_NONBLOCK));
    printf("compiled\n");
    
    CHECK(t0());
    //CHECK(t1());
    
    return 0;
FAILED:
    printf(" ---   %sFAILED%s ---- \n", ANSI_COLOR_BOLD_RED, ANSI_COLOR_DEFAULT);
    return 0;
}

