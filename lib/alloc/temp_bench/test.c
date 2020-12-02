#include "../macros/macros.h"
#include "../alloc/alloc.h"
#include "../timer/timer.h"
#include "../workQ/workQ.h"
#include "../arc/arc.h"

#include <stdlib.h>
#include <stdio.h>


/*

// can malloc 10 distinct powers of 2, 10 distinct alignments
// 5,  6,  7,  8,  9
// 10, 11, 12, 13, 14

*/

#define TEST_AMT (7048 * 4)
#define TEST_ITERATIONS (64 * 20)
#define TEST_TYPE long
static void t0() {
    volatile TEST_TYPE *ptrs[TEST_AMT];
    for(TEST_TYPE i = 1; i < TEST_AMT; ++i) {
        NULL_CHECK(ptrs[i] = JC_ALLOCMalloc(sizeof(void *), sizeof(TEST_TYPE)));
        *ptrs[i] = i;
    }
    for(TEST_TYPE i = 1; i < TEST_AMT; ++i) {
        if(*ptrs[i] != i) {
            fprintf(stderr, "%llu -> %llu\n", (ull)i, (ull)*ptrs[i]);
            goto FAILED;
        }
        JC_ALLOCFree(ptrs[i], sizeof(TEST_TYPE));
    }
    return;
FAILED:
    printf("t0 failed\n");
    exit(-1);
}
static void t1() {
    volatile TEST_TYPE *ptrs[TEST_AMT];
    for(TEST_TYPE i = 1; i < TEST_AMT; ++i) {
        NULL_CHECK(ptrs[i] = malloc(sizeof(TEST_TYPE)));
        *ptrs[i] = i;
    }
    for(TEST_TYPE i = 1; i < TEST_AMT; ++i) {
        if(*ptrs[i] != i) {
            fprintf(stderr, "%llu -> %llu\n", (ull)i, (ull)*ptrs[i]);
            goto FAILED;
        }
        free(ptrs[i]);
    }
    return;
FAILED:
    printf("t1 failed\n");
    exit(-1);
}
static void t2() {
    volatile TEST_TYPE *ptrs[TEST_AMT];
    for(TEST_TYPE i = 1; i < TEST_AMT; ++i) {
        CHECK(posix_memalign((void **)&ptrs[i], sizeof(void *) * 1, sizeof(TEST_TYPE)));
        *ptrs[i] = i;
    }
    for(TEST_TYPE i = 1; i < TEST_AMT; ++i) {
        if(*ptrs[i] != i) {
            fprintf(stderr, "%llu -> %llu\n", (ull)i, (ull)*ptrs[i]);
            goto FAILED;
        }
        free(ptrs[i]);
    }
    return;
FAILED:
    printf("t2 failed\n");
    exit(-1);
}

#define THREAD_COUNT 2
SPRAI(WorkQ, q);
static Arc arc = ATOMIC_VAR_INIT(THREAD_COUNT);
static Ret arenaTestCB0(PTask **task) { 
    void *ign = task;
    for(int i = 0; i < TEST_ITERATIONS; ++i) { t0(); } 
    if(ArcDec(&arc)) { WorkQStop(q); ArcWrite(&arc, THREAD_COUNT); }
    return PTASK_RET_SPARE; 
}
static Ret arenaTestCB1(PTask **task) { 
    void *ign = task;
    for(int i = 0; i < TEST_ITERATIONS; ++i) { t1(); } 
    if(ArcDec(&arc)) { WorkQStop(q); ArcWrite(&arc, THREAD_COUNT); }
    return PTASK_RET_SPARE; 
}
static Ret arenaTestCB2(PTask **task) { 
    void *ign = task;
    for(int i = 0; i < TEST_ITERATIONS; ++i) { t2(); } 
    if(ArcDec(&arc)) { ArcWrite(&arc, THREAD_COUNT); WorkQStop(q); }
    return PTASK_RET_SPARE; 
}


int main(void) {
    PRAI(Timer, t);

#define ARENA_MECHANISM_RUN(FN) \
    { \
        CHECK(WorkQInit(q, -1)); \
        TimerStart(t); \
        PTask ptask = {FN, NULL, NULL}; \
        QTask qtask = {&ptask, 0, 0}; \
        for(usize i = 0; i < THREAD_COUNT; ++i) { CHECK(WorkQPush(q, &qtask)); } \
        WorkQLoop(q); \
        TimerStop(t); TimerDisplay(t); fprintf(stderr, " %s\n", #FN); \
        WorkQDe(q); \
    } \

    ARENA_MECHANISM_RUN(arenaTestCB1);
    ARENA_MECHANISM_RUN(arenaTestCB0);
    ARENA_MECHANISM_RUN(arenaTestCB2);

    ///*
    // Sequential runs
    for(usize j = 0; j < 1; ++j) {
        TimerStart(t); for(int i = 0; i < TEST_ITERATIONS * THREAD_COUNT; ++i) { t1(); } TimerStop(t); TimerDisplay(t); fprintf(stderr, " single thread std alloc\n");
        TimerStart(t); for(int i = 0; i < TEST_ITERATIONS * THREAD_COUNT; ++i) { t0(); } TimerStop(t); TimerDisplay(t); fprintf(stderr, " single thread wierd alloc\n");
        TimerStart(t); for(int i = 0; i < TEST_ITERATIONS * THREAD_COUNT; ++i) { t2(); } TimerStop(t); TimerDisplay(t); fprintf(stderr, " single memalign alloc\n");
    }

    exit(0);
FAILED:
    printf("Failed in main?\n");
    exit(0);
}

// 0x00000000000003eb60

