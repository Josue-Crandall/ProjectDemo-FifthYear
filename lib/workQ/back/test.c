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

static WorkQ *q;
void signalStop(int ing) {
    int ign = ing;
    printf("Registered CTRL-C");
    WorkQStop(q);
}

typedef struct {
    TaskFace *vptr;
    WorkQ *userq;
    int *count;
} Counter;
Ret countCB(void *  task) {
    Counter *  counter = task;
    printf("Time: %d\n", (*counter->count)++);
    sleep(1);
    WorkQPush(counter->userq, task);
    return TASKF_RET_SPARE;
}
PTrig * countTriggers(void *  task) { 
    void *  taskign = task;
    return NULL; 
}
void countDe(void *  task) { Counter *  counter = task; }
TaskFace countInterface = {&countCB, &countTriggers, &countDe};
static Counter counterTask = {&countInterface, NULL, NULL};

typedef struct {
    TaskFace * vptr;
    // for internal use
    void * iData1, * iData2;
    //
    int *count;
    PTrig trigger[2];
} EchoTask;
Ret echoCB(void *  ptask) {
    EchoTask * task = ptask;

    char temp;
    while(1 == read(0, &temp, 1)) { 
        printf("%d: %c\n", ++(*task->count), temp); 
    }

    return 2;
}

PTrig * echoTrigger(void *  ptask) {
    EchoTask * task = ptask;
    return task->trigger;
}
void echoDe(void *  ptask) {
    if(ptask) { 
        EchoTask * task = ptask;
        free(task->count);
    }
}
TaskFace echoTaskInterface = { echoCB, echoTrigger, echoDe };
static EchoTask task = { &echoTaskInterface, 0, 0, NULL, { { 0, PTRIG_IN }, {0, 0}} };

Ret t0() {
    Ret err = 0;
    WorkQ que = {0};
    q = &que;
    counterTask.count = NULL;

    CHECK(setSignalHandler(SIGINT, signalStop));
    CHECK(WorkQInit(&que, 127, 5000));
    
    CHECK(NULL == (counterTask.count = malloc(sizeof(int))));
    *counterTask.count = 0;
    counterTask.userq = &que;
    CHECK(WorkQPush(&que, &counterTask));

    CHECK(NULL == (task.count = malloc(sizeof(int))));
    *task.count = 0;
    CHECK(WorkQPoll(&que, &task));

CLEAN:
    WorkQDe(&que);
    free(counterTask.count);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}

int main(void) {
    CHECK(fcntl(0, F_SETFL, O_NONBLOCK));
    printf("Compiled.\n");
    CHECK(t0());
    return 0;
FAILED:
    printf("--- FAILED TEST ---\n");
    return 0;
}