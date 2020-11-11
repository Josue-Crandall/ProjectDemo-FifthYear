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

#include <errno.h>
#include <unistd.h>

static Ret PollerInit(Poller *poller) {
	NEG_CHECK(poller->epfd = epoll_create1(EPOLL_CLOEXEC));
	poller->alive = 1;
	return 0;
FAILED:
	FD_CLOSE(poller->epfd);
	poller->alive = 0;
	return -1;
}
static void PollerDe(Poller *poller) { if(poller->alive) { FD_CLOSE(poller->epfd); } }
static Ret PollerReg(Poller *poller, int fd, Ret trigger) {
#define POLL_MODIFY_IMP(CTL_CONSTANT)							\
	struct epoll_event setting;									\
	setting.events = EPOLLET;									\
	if(trigger & POLLER_IN) { setting.events |= EPOLLIN; }		\
	if(trigger & POLLER_OUT) { setting.events |= EPOLLOUT; }	\
	setting.data.fd = fd;										\
	CHECK(epoll_ctl(poller->epfd, CTL_CONSTANT, fd, &setting)); \
	return 0;													\
FAILED:															\
	return -1;													\

	POLL_MODIFY_IMP(EPOLL_CTL_ADD);
}
static Ret PollerMod(Poller *poller, int fd, Ret trigger) {
	POLL_MODIFY_IMP(EPOLL_CTL_MOD);
#undef POLL_MODIFY_IMP
}
static Ret PollerRm(Poller *poller, int fd) {
	CHECK(epoll_ctl(poller->epfd, EPOLL_CTL_DEL, fd, NULL));
	return 0;
FAILED:
	return -1;
}
// return: number ready, or -1 on failure
static int PollerPoll(Poller *poller, int timeoutMiliSec) {
	int res;

EPOLL_WAIT:
	res = epoll_wait(poller->epfd, poller->events, POLLER_BUFFER_SIZE, timeoutMiliSec);
	if(res < 0) {
		if(res == EINTR) { goto EPOLL_WAIT; }
		else { DEBUG_LOG("epoller_wait in PollerPoll has failed\n"); return -1; }
	}
	DEBUG_LOG("PollerPoll spin!\n");
	assert((unsigned long long)res <= POLLER_BUFFER_SIZE);
	return res;
}
static Ret PollerWhy(Poller *poller, int index) {
	Ret res = poller->events[index].events & EPOLLIN ? POLLER_IN : 0;
	if(poller->events[index].events & EPOLLOUT) { res |= POLLER_OUT; }
	return res;
}
static int PollerWhat(Poller *poller, int index) {
	return poller->events[index].data.fd;
}
