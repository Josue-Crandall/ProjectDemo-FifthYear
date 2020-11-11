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

#ifndef JC_WORKQ_H
#define JC_WORKQ_H
#include "../macros/macros.h"

#include "../thread/thread.h"
#include "../poller/poller.h"
#include "../cvec/cvec.h"
DD_CVEC(CTaskVec,PTask **, PTaskDe(val), STD_ALLOC);

typedef struct QTask {
    PTask *vptr;
    // Internal use data
    void *iData1, *iData2;
    // Arbitrary object data
    // ...
} QTask;
typedef struct {
    CTaskVec tasks;
    Poller poller;
    size_t workerCount;
    PThread *workers;
} WorkQ;

// Note: timeout == 0 implies no timeout, -1 implies infinite timeout
// WARNING: timeout is not per PollerTask, it is Poller wide.
static Ret WorkQInit(WorkQ *que, int timeoutMiliSec);
// Note: Tasks should not block or WorkQ Can't use CPU's cores effectively
// Note: takes ownership of qtask on both success and failure
// qtask: is any QTask specialization
static Ret WorkQPush(WorkQ *que, QTask *qtask);
// Note: Tasks should not block or WorkQ Can't use CPU's cores effectively
// Note: takes ownership of qtask on both success and failure
// qtask: is any QTask specialization
static Ret WorkQPoll(WorkQ *que, QTask *qtask);
// Note: without a call to WorkQStop this will block until another source signals stop
static void WorkQLoop(WorkQ *que);
// Note: safe to double call Stop
static void WorkQStop(WorkQ *que);
// Note: NOT safe to double call destroy
static void WorkQDe(WorkQ *que);

#include "imp.h"

#endif