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
#include "../cvec/cvec.h"

typedef void (*QFn)(void *arg);
typedef struct QTask { QFn cb, de; void *arg; } QTask;
static void QTaskDe(QTask *task) { if(task->de) { task->de(task->arg); } }
DD_CVEC(QTasks,QTask,QTaskDe(&val),STD_ALLOC);

typedef struct WorkQ { 
    QTasks tasks;
    PThread *workers;
    long count;
} WorkQ;

static Ret WorkQInit(WorkQ *que);
/* Note: cb should not block or NAME Can't use CPU's cores effectively */   \
/* Note: Takes ownership of task on both success and failure */
static Ret WorkQPush(WorkQ *que, QFn cb, QFn de, void *arg);
/* Note: safe to call multiple times */                                     \
static void WorkQStop(WorkQ *que);
/* Note: UNSAFE to call multiple times */                                   \
/* Note: blocks until some source calls NAME##Stop */                       \
static void WorkQDe(WorkQ *que);

#include "imp.h"

#endif
