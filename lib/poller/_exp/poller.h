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

#include <sys/epoll.h>

#define POLLER_BUFFER_SIZE (128ULL)
static_assert(POLLER_BUFFER_SIZE <= INT_MAX, "For return value on PollerPoller");
typedef struct Poller {
    Ret alive;
    int epfd;
	struct epoll_event events[POLLER_BUFFER_SIZE];
} Poller;

// WARNING: Registering the same file descriptor more than once (without deregistering) is undefined behavior
// WARNING: edge triggered, callback must exhaust the file descriptor before repollering

static Ret PollerInit(Poller *poller);

// Trigger flags:
#define POLLER_IN   (1)
#define POLLER_OUT  (2)
#define POLLER_IO   (3)
static Ret PollerReg(Poller *poller, int fd, Ret trigger);
static Ret PollerMod(Poller *poller, int fd, Ret trigger);
static Ret PollerRm(Poller *poller, int fd);
// timeoutMiliSec: 0 will make epoller spin, -1 means no timeout
// return: number ready, or -1 on failure
static int PollerPoll(Poller *poller, int timeoutMiliSec);
// Note: Are ready after PollerPoll
//       Why returns out trigger flags, 
//       What returns the ready fd.
static Ret PollerWhy(Poller *poller, int index);
static int PollerWhat(Poller *poller, int index);

static void PollerDe(Poller *poller);

#include "imp.h"

#endif
