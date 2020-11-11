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

#define WORKQ_EXPECTED_QUEUE_LENGTH 16

void * workQRunner(void *arg) {
    WorkQ *que = arg;
    QTask task;
    while(1) {
        task = QTasksPop(&que->tasks);
        if(NULL == task.cb) { break; }
        else { task.cb(task.arg); }
    }
    return NULL;
}

static Ret WorkQInit(WorkQ *que) {
    Ret needsStop = 0;
    memset(que, 0, sizeof(WorkQ));
    CHECK(QTasksInit(&que->tasks, WORKQ_EXPECTED_QUEUE_LENGTH));

    CHECK( (que->count = sysconf(_SC_NPROCESSORS_ONLN)) < 1);
    CHECK((unsigned long)que->count > USIZE_MAX); /* 0.o */
    NULL_CHECK(que->workers = STD_ALLOCCalloc(que->count, sizeof(PThread)));

    needsStop = 1;

    for(long i = 0; i < que->count; ++i) {
        CHECK(PThreadInit(que->workers + i, workQRunner, que));
        CHECK(PThreadSetAffinity(que->workers + i, i));
    }

    return 0;
FAILED:
    if(needsStop) { QTasksStop(&que->tasks); }
    WorkQDe(que);
    memset(que, 0, sizeof(WorkQ));
    return -1;
}
static Ret WorkQPush(WorkQ *que, QFn cb, QFn de, void *arg) {
    QTask temp = {cb, de, arg};
    if(QTasksPush(&que->tasks, temp)) { goto FAILED; }
    return 0;
FAILED:
    QTaskDe(&temp);
    return -1;
}
static void WorkQStop(WorkQ *que) { QTasksStop(&que->tasks); }
static void WorkQDe(WorkQ *que) {
    if(que->workers) {
        PThread *iter = que->workers, *end = iter + que->count;
        while(iter != end) { PThreadJoin(iter++, NULL); }
        STD_ALLOCFree(que->workers, que->count * sizeof(PThread));
        QTasksDe(&que->tasks);
    }
}
