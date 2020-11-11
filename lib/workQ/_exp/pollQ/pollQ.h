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

#ifndef JC_POLLQ_H
#define JC_POLLQ_H
#include "../macros/macros.h"

#include "../poller/poller.h"
#include "../workQ/workQ.h"
#include "../pipe/pipe.h"
#include "../table/table.h"

/*

set of fd -> if any proc we want to trigger the cb once

we poll on fd's
    we get a list of ready fds

    1+ fd's will point to the cb's

    We must deregister all fd's which point to the same cb

# ?? MultiPoller
    fd's/triggers + token in --> token ready out
    # reg's and dereg's as a group internally



*/

DD_TABLE(TaskTable, int, QTask, res = (usize)key, res = lhs == rhs, NOP, QTaskDe(&val), STD_ALLOC);

typedef struct PollQ {
    WorkQ que;
    Poller poller;
    Pipe pip;
    int timeoutMiliSec;
} PollQ;

/* Note: timeout == 0 implies no timeout, -1 implies infinite timeout */
static Ret PollQInit(PollQ *pollq, int timeoutMiliSec);

/* Note: CB should not block or NAME Can't use CPU's cores effectively */
/* Note: takes ownership of token on both success and failure */
/* WARNING: Token must NOT be null  */
static Ret PollQPush(PollQ *pollq, QFn cb, QFn de, void *arg);
// trigger: POLLER_IN, POLLER_OUT, POLLER_IO
static Ret PollQPoll(PollQ *pollq, QFn cb, QFn de, void *arg, int fd, Ret trigger);

// Pushs poll'd events which are ready in a loop
// loop continues until PollQStop is called somewhere
static void PollQLoop(PollQ *pollq);

// Safe to call more than once
static void PollQStop(PollQ *pollq);
// Unsafe to call more than once
static void PollQDe(PollQ *pollq);

#endif

/*




*/