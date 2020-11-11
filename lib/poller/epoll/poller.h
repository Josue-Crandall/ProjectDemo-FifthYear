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

#ifndef JC_POLLER_H
#define JC_POLLER_H
#include "../macros/macros.h"

#include "../pipe/pipe.h"
#include "../thread/thread.h"

typedef struct PTrig {
    int fd;
#define PTRIG_IN     0b001
#define PTRIG_OUT    0b010
    Ret when;
} PTrig;
typedef struct TaskFace {
    #define TASKF_RET_STOP   -1 // < 0
    #define TASKF_RET_DONE    0
    #define TASKF_RET_SPARE   1
    #define TASKF_RET_REPOLL  2
    Ret (*cb) (void *task);
    // list, terminated with trigger->when == 0
    PTrig * (*getTriggers) (void *task);
    void (*taskDe) (void *task);
} TaskFace;
typedef struct PTask {
    TaskFace *R() vptr;
    u8 data[];
} PTask;
typedef struct Poller {
    Pipe pipe;
    PThread thread;
    int epfd, pollwideTimeout; // miliseconds
} Poller;

// WARNING: Multiple PTask's registering the same file descriptor is undefined behavior
// WARNING: edge triggered, callback must exhaust the file descriptor before repolling

// Note: Timeout is poller wide, not per task
// Note: After the timeout all callbacks are called ready or not.
// Note: 0 will make epoll spin, -1 will mean no timeout
static Ret PollerInit(Poller *poller, int pollwideTimeoutMiliseconds);
// Note: takes ownership of ptask on both success and failure
// ptask: is any specialization of PTask
static Ret PollerPoll(Poller *poller, void *R() ptask);
static void PollerDe(Poller *poller);

struct Nothing {};
#include "../table/table.h"
DD_TABLE(Tasks, PTask *, struct Nothing, res = (usize)key, res = lhs == rhs, if(key) { key->vptr->taskDe(key); }, NOP, STD_ALLOC);

//// Managed Poll
typedef struct MPoll {
    int epfd;
    int timeoutMiliseconds;
    Ret alive;
    Tasks tasks;
} MPoll;

// Note: Purpose of MPoll is to use the calling thread
// to perform the polling tasks (instead of a blackground thread).

static Ret MPollInit(MPoll *mpoll, int pollwideTimeoutMiliseconds);
static Ret MPollSetPoll(MPoll *mpoll, void *ptask);
// Keeps going either until error or a task returns -1
static void MPollLaunch(MPoll *mpoll);
static void MPollSetTimeout(MPoll *mpoll, int newTimeoutMiliseconds);
static void MPollDe(MPoll *mpoll);

#include "imp.h"

#endif
