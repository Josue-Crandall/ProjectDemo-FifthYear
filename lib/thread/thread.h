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

#ifndef JC_THREAD_H
#define JC_THREAD_H
#include "../macros/macros.h"

#include <signal.h>
#include <pthread.h>
#include <unistd.h>

typedef struct { pthread_t thread; u8 alive; } PThread;
// If thread is NULL thread is set to detatched
// Note: Child threads inherit signal disposition // as well as inherit capacitilities && sched_setaffinity
static Ret PThreadInit(PThread *thread, void * (*task)(void *taskArg), void *taskArg);
// Note: result can be NULL
static void PThreadJoin(PThread *thread, void **result);
static Ret PThreadAlive(PThread *thread);
static Ret PThreadSetAffinity(PThread *thread, size_t coreID);

typedef struct { pthread_mutex_t mutex; u8 alive; } PMutex;
#define PMUTEX_STATIC_INIT {PTHREAD_MUTEX_INITIALIZER, 0}
static Ret PMutexInit(PMutex *mutex);
static void PMutexLock(PMutex *mutex);
static Ret PMutexTryLock(PMutex *mutex);
static void PMutexUnlock(PMutex *mutex);
static void PMutexDe(PMutex *mutex);

typedef struct { pthread_cond_t cond; u8 alive; } PCond;
#define PCOND_STATIC_INIT {PTHREAD_COND_INITIALIZER, 0}
static Ret PCondInit(PCond *cond);
static void PCondWait(PCond *cond, PMutex *mutex);
static void PCondSignal(PCond *cond, Ret broadcast);
static void PCondDe(PCond *cond);

typedef struct POnce { pthread_once_t once; } POnce;
#define PONCE_STATIC_INIT   { PTHREAD_ONCE_INIT }
static void POnceDo(POnce *once, void (*cb)(void));
typedef struct PLocal { pthread_key_t key; } PLocal;
// WARNING!!: May fail and loop infinitely. 
//            No obvious way to avoid this and not have proc'd POnce.
static Ret PLocalInit(PLocal *local, void (*destructor)(void *));
static void *PLocalGet(PLocal *local);
static Ret PLocalSet(PLocal *local, void *val);

//// Signals
static Ret setSignalHandler(int signalType, void(*handler)(int));

//// Exec
//Note: if the program name argument has no slashes PATH is searched
//      (execv unlike execvp will treat "b" like "./b" and not search path)
//      (Were going to use execvp and execvpe)
//
//Note: fork'd children do not(untested?) automatically die when the parent exits
//Note: fork inherits signal dispositions
//Note: fork inherits open file descriptors
//Note: execs however reset signal dispositions // Except in linux an ignored SIGCHLD is still ignored
//Note: execs inherit open file descriptors
//
//WARNING: sets SIGCHLD
//Note: envp may be null
static pid_t forkedExec(char *file, char **argv, char **envp);
//WARNING: sets SIGCHLD
//Note: envp may be null
//Note: returns -1 on failure, or the result of the executed program o.w.
static int runProgram(char *file, char **argv, char **envp);

#include "imp.h"

#endif
