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

#define POLL_EVENT_QUEUE_SIZE               (128)

static Ret PollerPoll(Poller *poller, void *ptask) {
    PTask *task = ptask;
	CHECK(PipeWrite(&poller->pipe, (u8 *)&task, sizeof(task)))
	return 0;
FAILED:
	task->vptr->taskDe(task);
	return -1;
}

#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>

static Ret removeTriggers(int epfd, PTask *task) {	
	for(PTrig *trigger = task->vptr->getTriggers(task); trigger->when; ++trigger) {
		CHECK(epoll_ctl(epfd, EPOLL_CTL_DEL, trigger->fd, NULL));
	}
	return 0;
FAILED:
	return -1;
}
static Ret addTriggers(int epfd, PTask *task) {
	for(PTrig *trigger = task->vptr->getTriggers(task); trigger->when; ++trigger) {
		struct epoll_event setting;
		setting.events = EPOLLET;
		if(trigger->when & PTRIG_IN) { setting.events |= EPOLLIN; }
		if(trigger->when & PTRIG_OUT) { setting.events |= EPOLLOUT; }
		setting.data.ptr = task;
		CHECK(epoll_ctl(epfd, EPOLL_CTL_ADD, trigger->fd, &setting));
	}
	return 0;
FAILED:
	return -1;
}
static Ret activateCB(Tasks *tasks, PTask *task, int epfd) {
	if(0 == TasksAt(tasks, task, 0)) { return 0; }
	CHECK(removeTriggers(epfd, task));

	Ret res = task->vptr->cb(task);
	if (res < 0) { goto FAILED; }
	if(TASKF_RET_DONE == res) { TasksRm(tasks, task, NULL); }
	else if(TASKF_RET_SPARE == res) { TasksRm(tasks, task, TBL_FLG_SPARE_ENTRY); }
	else { CHECK(addTriggers(epfd, task)); }

	return 0;
FAILED:
	return -1;
}
static Ret processNewTask(Tasks *tasks, PTask *task, int epfd) {
    static struct Nothing nothing;
	if(TasksPush(tasks, task, nothing, 0)) {
		DEBUG_LOG("TasksPush has failed\n");
		task->vptr->taskDe(task);
		goto FAILED;
	}
	CHECK(addTriggers(epfd, task));

	return 0;
FAILED:
	return -1;
}
static void * pollerRunner(void *arg) {
	Poller *poller = arg;
	Tasks tasks = {0};
	u8 pipeBuffer[PIPE_BUF];
	usize pipeReadLen;

	CHECK(TasksInit(&tasks, POLL_EVENT_QUEUE_SIZE));

	while(1) {
		struct epoll_event events[POLL_EVENT_QUEUE_SIZE];
		int numRdy;

	EPOLL:
		numRdy = epoll_wait(poller->epfd, events, POLL_EVENT_QUEUE_SIZE, poller->pollwideTimeout);
		if(numRdy < 0) {
			if(errno == EINTR) { goto EPOLL; }
			else {
				DEBUG_LOG("epoll_wait in Poller has failed\n");
				goto FAILED;
			}
		}
		DEBUG_LOG("Poller spin!\n");
		if(numRdy) {
			for(struct epoll_event *iter = events, *end = iter + numRdy; iter != end; ++iter) {				
				if(0 == iter->data.ptr) {
					Ret res;
				PIPE_READ:
					res = PipeRead(&poller->pipe, pipeBuffer, &pipeReadLen);
					if(res < 0) { goto CLEAN_EXIT; }
					if(0 == res) {
						CHECK(processNewTask(&tasks, *(PTask **)pipeBuffer, poller->epfd));
						goto PIPE_READ;
					}				
				}
				else { if(activateCB(&tasks, iter->data.ptr, poller->epfd)) { goto FAILED; } }
			}
		}
		else { 
			for(TasksGen gen = TasksBeg(&tasks); TasksNext(&gen); NOP) {
				if(activateCB(&tasks, gen.keys[gen.i], poller->epfd)) { goto FAILED; }
			}
		}
	}

FAILED:
	PipeCloseReadEnd(&poller->pipe);
CLEAN_EXIT:
	TasksDe(&tasks);
	return 0;
}
static Ret PollerInit(Poller *poller, int pollwideTimeoutMiliseconds) {
	memset(poller, 0, sizeof(*poller)); poller->epfd = -1;

	CHECK(PipeInit(&poller->pipe, NULL));
	CHECK( (poller->epfd = epoll_create1(EPOLL_CLOEXEC)) < 0);

	// Register interest in read end of the pipe
	struct epoll_event settings;
	settings.events = EPOLLIN | EPOLLET; settings.data.ptr = NULL;
	CHECK(epoll_ctl(poller->epfd, EPOLL_CTL_ADD, PipeGetReadEnd(&poller->pipe), &settings));
	poller->pollwideTimeout = pollwideTimeoutMiliseconds;
	PThreadInit(&poller->thread, pollerRunner, poller);

	return 0;

FAILED:
	PipeCloseWriteEnd(&poller->pipe);
	PThreadJoin(&poller->thread, NULL);
	PipeDe(&poller->pipe);
	FD_CLOSE(poller->epfd);

	memset(poller, 0, sizeof(*poller)); poller->epfd = -1;
	return -1;
}
static void PollerDe(Poller *poller) {
	if(PThreadAlive(&poller->thread)) {
		PipeCloseWriteEnd(&poller->pipe);
		PThreadJoin(&poller->thread, NULL);
		PipeDe(&poller->pipe);
		FD_CLOSE(poller->epfd);
	}
}

/////
 
static Ret MPollInit(MPoll *mpoll, int pollwideTimeoutMiliseconds) {
	CHECK( (mpoll->epfd = epoll_create1(EPOLL_CLOEXEC)) < 0);
	CHECK(TasksInit(&mpoll->tasks, POLL_EVENT_QUEUE_SIZE));
	mpoll->timeoutMiliseconds = pollwideTimeoutMiliseconds;
	mpoll->alive = 1;
	return 0;
FAILED:
	mpoll->epfd = -1;
	mpoll->alive = 0;
	return -1;
}
static void MPollDe(MPoll *mpoll) {
	if(mpoll->alive) { 
		FD_CLOSE(mpoll->epfd); 
		TasksDe(&mpoll->tasks);
	}
}
static Ret MPollSetPoll(MPoll *mpoll, void *ptask) {
	CHECK(processNewTask(&mpoll->tasks, ptask, mpoll->epfd));
	return 0;
FAILED:
	return -1;
}
static void MPollLaunch(MPoll *mpoll) {
	while(1) {
		int numRdy;
		struct epoll_event events[POLL_EVENT_QUEUE_SIZE];
	EPOLL:
		numRdy = epoll_wait(mpoll->epfd, events, POLL_EVENT_QUEUE_SIZE, mpoll->timeoutMiliseconds);
		if(numRdy < 0) {
			if(errno == EINTR) { goto EPOLL; }
			else {
				DEBUG_LOG("epoll_wait in ManagedPollPoll has failed\n");
				goto FAILED;
			}
		}
		DEBUG_LOG("MPoll spin!\n");
		if(numRdy) {
			for(struct epoll_event *iter = events, *end = iter + numRdy; iter != end; ++iter) {				
				if(activateCB(&mpoll->tasks, iter->data.ptr, mpoll->epfd)) { goto FAILED; }
			}
		}
		else {
			for(TasksGen gen = TasksBeg(&mpoll->tasks); TasksNext(&gen); NOP) {
				if(activateCB(&mpoll->tasks, gen.keys[gen.i], mpoll->epfd)) { goto FAILED; }
			}
		}
	}

FAILED:
	return;
}
static void MPollSetTimeout(MPoll *mpoll, int newTimeoutMiliseconds) {
	mpoll->timeoutMiliseconds = newTimeoutMiliseconds;
}
