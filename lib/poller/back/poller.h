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
#include "../vec/vec.h"

#include <poll.h>

typedef struct PWhen {
    int fd;
#define PWHEN_IN     0b001
#define PWHEN_OUT    0b010
#define PWHEN_IO     0b011
    Ret when;
} PWhen;
typedef struct PTask PTask;
struct PTask {
    // WARNING: No (*de) call on PTASK_RET_ERROR
    #define PTASK_RET_ERROR   -1 // < 0 checked
    #define PTASK_RET_DONE    0
    #define PTASK_RET_SPARE   1
    #define PTASK_RET_REPOLL  2
    Ret (*cb) (PTask **task);
    // list, terminated with trigger->when == 0
    // Note: list must have a least two elements
    PWhen *(*when) (PTask **task);
    void (*de) (PTask **task);
};

DD_VEC(PollFDs, struct pollfd, NOP, STD_ALLOC);
typedef struct PollTaskTracker { PTask **task; usize len; } PollTaskTracker;
static void PTaskDe(PTask **ptask) { if(ptask && (*ptask)->de) { (*ptask)->de(ptask); } }
DD_VEC(PollTaskTrackers, PollTaskTracker, PTaskDe(val.task), STD_ALLOC);

typedef struct Poller {
    int timeoutMiliSec;
    Pipe pip;
    PollFDs fds;
    PollTaskTrackers trackers;
} Poller;

// WARNING: Multiple PTask's registering the same file descriptor is undefined behavior
// WARNING: edge triggered, callback must exhaust the file descriptor before repolling

// Note: Timeout is poller wide, not per task
// Note: After the timeout all callbacks are called ready or not.
// Note: 0 will make epoll spin, -1 will mean no timeout
static Ret PollerInit(Poller *poller, int timeoutMiliSec);
// Note: takes ownership of ptask on both success and failure
// ptask: is any specialization of PTask
static Ret PollerPush(Poller *poller, PTask **task);
static void PollerLoop(Poller *poller);
static void PollerStop(Poller *poller);
// Note: Not threadsafe if called from non PollerLoop thread.
static void PollerSetTimeout(Poller *poller, int timeoutMiliSec);
static void PollerDe(Poller *poller);

#include "imp.h"

#endif
