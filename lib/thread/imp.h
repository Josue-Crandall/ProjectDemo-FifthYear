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

#ifdef __linux__
    #include <sched.h>
#endif

#include <unistd.h> // fork, execs
#include <sys/wait.h> // waitpid, WNOHANG
#include <stdlib.h> // exit
#include <errno.h>

static Ret PThreadInit(PThread *thread, void * (*task)(void *taskArg), void *taskArg) {
    Ret err;
    PThread ignore;
    if(NULL == thread) { thread = &ignore;}
        
    pthread_attr_t attr;
    CHECK(pthread_attr_init(&attr));
    CHECK(pthread_attr_setdetachstate(&attr, thread == &ignore ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE));
    CHECK(pthread_create(&thread->thread, &attr, task, taskArg));

    thread->alive = 1;
    err = 0;
CLEAN:
    if(pthread_attr_destroy(&attr)) { DEBUG_LOG("WARNING: pthread_attr_destroy has failed\n"); }
    return err;
FAILED:
    thread->alive = 0;
    goto CLEAN;
}
#ifdef __linux__
Ret PThreadSetAffinity(PThread *thread, size_t coreID) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreID, &cpuset);
    CHECK(pthread_setaffinity_np(thread->thread, sizeof(cpuset), &cpuset));

    return 0;
FAILED:
    return -1;
}
#else
Ret PThreadSetAffinity(PThread *thread, size_t coreID) {
    void *ign = thread; size_t ign2 = coreID; return 0;
}
#endif

static Ret PThreadAlive(PThread *thread) { return thread->alive; }
static void PThreadJoin(PThread *thread, void **result) {
    if(thread->alive) {
        if(pthread_join(thread->thread, result)) { DEBUG_LOG("WARNING: PThread join has failed\n"); }
    }
}
static Ret PMutexInit(PMutex *mutex) {
    CHECK(pthread_mutex_init(&mutex->mutex, 0));
    mutex->alive = 1;
    return 0;
FAILED:
    mutex->alive = 0;
    return -1;
}
static void PMutexLock(PMutex *mutex) {
    CHECK(pthread_mutex_lock(&mutex->mutex));
FAILED: ;
}
static Ret PMutexTryLock(PMutex *mutex) {
    int err;
    if( (err = pthread_mutex_trylock(&mutex->mutex))) { goto FAILED; }
    return 0;
FAILED:
    #ifdef JC_DEBUG
        if(err != EBUSY) { DEBUG_LOG("PMutexTryLock has failed\n"); }
    #endif
    return -1;
}
static void PMutexUnlock(PMutex *mutex) {
    CHECK(pthread_mutex_unlock(&mutex->mutex));
FAILED: ;
}
static void PMutexDe(PMutex *mutex) {
    CHECK(pthread_mutex_destroy(&mutex->mutex));
FAILED: ;
}
static Ret PCondInit(PCond *cond) {
    CHECK(pthread_cond_init(&cond->cond, 0));
    cond->alive = 1;
    return 0;
FAILED:
    cond->alive = 0;
    return -1;
}
static void PCondWait(PCond *cond, PMutex *mutex) {
    CHECK(pthread_cond_wait(&cond->cond, &mutex->mutex));
FAILED: ;
}
// Note: Signal before letting go of mutex ?
static void PCondSignal(PCond *cond, Ret broadcast) {
    if(broadcast) { CHECK(pthread_cond_broadcast(&cond->cond)); }
    else { CHECK(pthread_cond_signal(&cond->cond)); }
FAILED: ;
}
static void PCondDe(PCond *cond) {
    if(cond->alive) { CHECK(pthread_cond_destroy(&cond->cond)); }
FAILED:;
}
static void POnceDo(POnce *once, void (*cb)(void)) {
    CHECK(pthread_once(&once->once, cb));
    return;
FAILED:
    DEBUG_LOG("POnceDo has failed\n");
}
static Ret PLocalInit(PLocal *local, void (*destructor)(void *)) {
AGAIN:
    CHECK(pthread_key_create(&local->key, destructor));
    return 0;
FAILED:
    return -1;
}
static void *PLocalGet(PLocal *local) {
    return pthread_getspecific(local->key);
}
static Ret PLocalSet(PLocal *local, void *val) {
    CHECK(pthread_setspecific(local->key, val));
    return 0;
FAILED:
    return -1;
}
static Ret setSignalHandler(int signalType, void(*handler)(int)) {
    struct sigaction action;
    action.sa_handler = handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);

    CHECK(sigaction(signalType, &action, 0));
    return 0;

FAILED:
    return -1;
}
static void sigCHLDHandler(int ign) { 
    int storedErrno = errno;
    
    ign = ign;
    pid_t res;
WAIT_PID:
    res = waitpid(-1, NULL, WNOHANG);
    if(res > 0 || (res < 0 && errno == EINTR)) { goto WAIT_PID; }

    errno = storedErrno;
}
static pid_t forkedExec(char *file, char **argv, char **envp) {
    CHECK(setSignalHandler(SIGCHLD, sigCHLDHandler));

    pid_t execPid;
    NEG_CHECK(execPid = fork());
    if(execPid) { return execPid; }
    else {
        if(envp) { if(execvpe(file, argv, envp)) { DEBUG_LOG("execvpe has failed\n"); } }
        else { if(execvp(file, argv)) { DEBUG_LOG("execvp has failed\n"); } }
        DEBUG_LOG("Forked Exec has failed!\n");
        exit(-1);
    }

FAILED:
    return -1;
}

static int runProgram(char *file, char **argv, char **envp) {
    pid_t execPid;

    NEG_CHECK(execPid = fork());
    if(execPid) { 
        int status;
        EINTR_CHECK(waitpid(execPid, &status, 0));
        return status;
    }
    else {
        if(envp) { if(execvpe(file, argv, envp)) { DEBUG_LOG("execvpe has failed\n"); } }
        else { if(execvp(file, argv)) { DEBUG_LOG("execvp has failed\n"); } }
        DEBUG_LOG("Forked Exec has failed!\n");
        exit(-1);
    }

FAILED:
    return -1;
}

