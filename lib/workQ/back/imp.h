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

static Ret WorkQPush(WorkQ *que, void *ptask) {   
    PTask *R() task = ptask;
    // Can occur when que has stopped even without an error
    if(CTaskVecPush(&que->tasks, task)) { goto FAILED; }
    return 0;
FAILED:
    task->vptr->taskDe(task);
    return -1;
}
static void WorkQStop(WorkQ *que) { CTaskVecStop(&que->tasks); }
extern TaskFace workQReface;
static Ret WorkQPoll(WorkQ *que, void *qtask) {
    QTask *R() task = qtask;
    task->iData1 = task->vptr;
    task->iData2 = que;
    task->vptr = &que->workQReface;
    // Can occur when que has stopped even without an error
    if(PollerPoll(&que->poller, task)) { goto FAILED; }
    return 0;
FAILED:
    ((TaskFace *R())task->iData1)->taskDe(task);
    return -1;
}

//// Imp
#include <stdlib.h>
#include <unistd.h>

static void * qRunner(void *arg) {
    WorkQ *R() que = arg;

    PTask *R() task;

    while( (task = CTaskVecPop(&que->tasks))) {
    #ifdef JCDEBUG
        fprintf(stderr, "WorkQ worker spin.\n");
    #endif

        Ret res = task->vptr->cb(task);
        if(res < 0) {
            WorkQStop(que);
            goto DONE;
        }
        if(TASKF_RET_DONE == res) { task->vptr->taskDe(task); }
        else if(TASKF_RET_REPOLL == res && WorkQPoll(que, task)) { goto DONE; }
    }

DONE:
    return NULL;
}
static Ret refaceCB(void *qtask) {
    QTask *R() task = qtask;
    task->vptr = task->iData1;
    WorkQPush(task->iData2, task);
    return TASKF_RET_SPARE;
}
static PTrig * refaceTriggers (void *qtask) {
    QTask *R() task = qtask;
    return ((TaskFace *R())task->iData1)->getTriggers(task);
}
static void refaceDe (void *qtask) {
    QTask *R() task = qtask;
    if(task) { ((TaskFace *R())task->iData1)->taskDe(task); }
}
static Ret WorkQInit(WorkQ *que, size_t expectedQueueLength, int timeoutMiliseconds) {
    // Order of initialization matters here for error handling
    memset(que, 0, sizeof(*que));
    
    CHECK(CTaskVecInit(&que->tasks, expectedQueueLength));
    CHECK(PollerInit(&que->poller, timeoutMiliseconds));

    long activeProcessorCount;
    CHECK( (activeProcessorCount = sysconf(_SC_NPROCESSORS_ONLN)) < 0);
    CHECK((unsigned long)activeProcessorCount > SIZE_MAX); // 0.o
    CHECK(NULL == (que->workers = calloc(activeProcessorCount, sizeof(PThread))));
    que->workerCount = activeProcessorCount;

    for(usize i = 0, end = que->workerCount; i < end; ++i) {
        CHECK(PThreadInit(que->workers + i, qRunner, que));
    #ifdef __linux__
        CHECK(PThreadSetAffinity(que->workers + i, i));
    #endif
    }

    que->workQReface.cb = &refaceCB;
    que->workQReface.getTriggers = &refaceTriggers;
    que->workQReface.taskDe = &refaceDe;

    return 0;
FAILED:
    if(que->workerCount) {
        CTaskVecStop(&que->tasks);
        PThread *R() end = que->workers + que->workerCount;
        while(que->workers != end) {
            PThreadJoin(que->workers, NULL);
            ++que->workers;
        }
        free(que->workers);
    }
    PollerDe(&que->poller);
    CTaskVecDe(&que->tasks);

    memset(que, 0, sizeof(*que));
    return -1;
}
static void WorkQDe(WorkQ *que) {
    if(que->workerCount) {
        // Order matters here
        PThread *R() iter = que->workers;
        PThread *R() end = iter + que->workerCount;
        while(iter != end) { PThreadJoin(iter++, NULL); }
        free(que->workers);
        PollerDe(&que->poller);
        CTaskVecDe(&que->tasks);
    }
}
