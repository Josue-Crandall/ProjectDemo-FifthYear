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
#include <unistd.h>

#define WORKQ_DEFAULT_QUEUE_LENGTH 16

static Ret WorkQPush(WorkQ *que, QTask *qtask) {   
    // Can occur when que has stopped even without an error
    if(CTaskVecPush(&que->tasks, &qtask->vptr)) { goto FAILED; }
    return 0;
FAILED:
    PTaskDe(&qtask->vptr);
    return -1;
}
static void WorkQStop(WorkQ *que) { 
    PollerStop(&que->poller);
    CTaskVecStop(&que->tasks);
}
static Ret refaceCB(PTask **ptask) {
    QTask *task = (QTask *)ptask;
    task->vptr = task->iData1;
    WorkQPush(task->iData2, task);
    return PTASK_RET_SPARE;
}
static PWhen * refaceTriggers (PTask **ptask) {
    QTask *task = (QTask *)ptask;
    return ((PTask *)task->iData1)->when(ptask);
}
static void refaceDe (PTask **ptask) {
    QTask *task = (QTask *)ptask;
    PTask *vptr = task->iData1;
    if(vptr->de) { vptr->de(ptask); }
}
static Ret WorkQPoll(WorkQ *que, QTask *qtask) {    
    qtask->iData1 = qtask->vptr;
    qtask->iData2 = que;
    static PTask reface = {refaceCB, refaceTriggers, refaceDe};
    qtask->vptr = &reface;
    // Can occur when que has stopped even without an error
    if(PollerPush(&que->poller, &qtask->vptr)) { goto FAILED; }
    return 0;
FAILED:
    PTaskDe(&qtask->vptr);
    return -1;
}

static void * qRunner(void *arg) {
    WorkQ *que = arg;

    PTask **task;

    while( (task = CTaskVecPop(&que->tasks))) {
        //DEBUG_LOG("WorkQ worker spin.\n");
        Ret res = (*task)->cb(task);
        if(res < 0) { WorkQStop(que); goto DONE; }
        else if(PTASK_RET_DONE == res) { PTaskDe(task); }
        else if(PTASK_RET_REPOLL == res && WorkQPoll(que, (QTask *)task)) { goto DONE; }
    }

DONE:
    return NULL;
}
static Ret WorkQInit(WorkQ *que, int timeoutMiliseconds) {
    // Order of initialization matters here for error handling
    memset(que, 0, sizeof(*que));
    
    CHECK(CTaskVecInit(&que->tasks, WORKQ_DEFAULT_QUEUE_LENGTH));
    CHECK(PollerInit(&que->poller, timeoutMiliseconds));

    long activeProcessorCount;
    CHECK( (activeProcessorCount = sysconf(_SC_NPROCESSORS_ONLN)) < 1);
    CHECK((unsigned long)activeProcessorCount > SIZE_MAX); // 0.o
    CHECK(NULL == (que->workers = calloc(activeProcessorCount, sizeof(PThread))));
    que->workerCount = activeProcessorCount;

    for(usize i = 0, end = que->workerCount; i < end; ++i) {
        CHECK(PThreadInit(que->workers + i, qRunner, que));
    #ifdef __linux__
        CHECK(PThreadSetAffinity(que->workers + i, i));
    #endif
    }

    return 0;
FAILED:
    if(que->workerCount) { CTaskVecStop(&que->tasks); }
    WorkQDe(que);
    memset(que, 0, sizeof(*que));
    return -1;
}
static void WorkQDe(WorkQ *que) {
    if(que->workers) {
        PThread *iter = que->workers;
        PThread *end = iter + que->workerCount;
        while(iter != end) { PThreadJoin(iter++, NULL); }
        free(que->workers);
    }
    PollerDe(&que->poller);
    CTaskVecDe(&que->tasks);
}
static void WorkQLoop(WorkQ *que) { PollerLoop(&que->poller); }
