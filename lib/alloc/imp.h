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

//////// Allocators

#include "../thread/thread.h"
#include "../timer/timer.h"
#include "../maf/maf.h"

#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>

//// Constant Macros
// Note: All up front to allow for assertions.
#define ALLOC_PAGE_SIZE() ((usize)4096)
// Note: For threads to return to the global pool if they have too much
#define ALLOC_MAX_STORAGE_BYTES() (4294967296LLU) // 4 GB
#define ALLOC_ROUND_TO_AT_LEAST_PAGE_SIZE(VAL) ((((VAL)-1)|(ALLOC_PAGE_SIZE()-1))+1)
#define ALLOC_SMALL_ALLOC_CUTOFF() (ALLOC_PAGE_SIZE() >> 2)
#define ALLOC_BYTES_TO_INDEX(BYTES) (MafLog2OfPow2(BYTES) - MafLog2OfPow2(PTR_ALIGN_SIZE()))
#define ALLOC_BUCKET_COUNT() (MafLog2OfPow2(ALLOC_PAGE_SIZE() >> 1) - MafLog2OfPow2(PTR_ALIGN_SIZE()))
#define ALLOC_INDEX_TO_SIZE(INDEX) (PTR_ALIGN_SIZE() << (INDEX))
#define ALLOC_INDEX_TO_COUNT(INDEX) (ALLOC_PAGE_SIZE() / ALLOC_INDEX_TO_SIZE(INDEX))
#define ALLOC_INDEX_TO_MAX_STORED(INDEX) (ALLOC_MAX_STORAGE_BYTES() / ALLOC_INDEX_TO_SIZE(INDEX))
#define ALLOC_ALIGN_ROUND(BYTES) (MafRoundUpNearest2Pow(((BYTES)-1) | (PTR_ALIGN_SIZE() - 1)))
#define ALLOC_DEFAULT_STORAGE_CAPACITY() (ALLOC_PAGE_SIZE() / sizeof(void *))

//// ~Constant Macros

//// Assertions
#ifdef JC_DEBUG
    #include "../maf/maf.h"
    #include "../arc/arc.h"

    #include <unistd.h>
#endif

#ifdef JC_DEBUG

static Arc JC_DEBUG_ALLOCATOR_COUNTER = ARCX_INIT(0);
static void JC_DEBUG_ALLOCATOR_COUNTER_CHECKER(void) {
    if(ArcRead(&JC_DEBUG_ALLOCATOR_COUNTER)) {
        DEBUG_LOG("!!! WARNING: JC_ALLOC Malloc count != Free count !!!\n");
    }
}
#endif

static void allocatorAssertions(void) {
#ifdef JC_DEBUG
    static Arc RUN_ASSERTIONS = ARCX_INIT(1);
    if(ArcDec(&RUN_ASSERTIONS)) {
        long pageSize = sysconf(_SC_PAGE_SIZE);
        assert(pageSize > 0 && (ull)pageSize == (ull)ALLOC_PAGE_SIZE());
        assert(MafIsPowTwo(ALLOC_PAGE_SIZE()));
        assert(ALLOC_PAGE_SIZE() % sizeof(void *) == 0);
        assert(ALLOC_PAGE_SIZE() > 1); // -_-;
        assert(PTR_ALIGN_SIZE() > 0);
        assert(MafLog2OfPow2(PTR_ALIGN_SIZE()));
        assert(ALLOC_PAGE_SIZE() >= PTR_ALIGN_SIZE());
        assert(ALLOC_PAGE_SIZE() % PTR_ALIGN_SIZE() == 0);
        assert(ALLOC_INDEX_TO_MAX_STORED(0) > ALLOC_INDEX_TO_COUNT(0));
        assert(ALLOC_MAX_STORAGE_BYTES() % ALLOC_PAGE_SIZE() == 0);

        if(atexit(JC_DEBUG_ALLOCATOR_COUNTER_CHECKER)) { exit(-1); }
    }
#endif
}
//// ~Assertions

#define DEF_ALLOCATOR(NAME, IMP)                                        \
static void * NAME##Malloc(usize count, usize size) {                   \
    allocatorAssertions();                                              \
    void *res;                                                          \
    MULL_CHECK(count, size, USIZE_MAX);                                 \
    if(!count) { return 0; }                                            \
    NULL_CHECK(res = IMP##Malloc(count));                               \
    return res;                                                         \
FAILED:                                                                 \
    return 0;                                                           \
}                                                                       \
static void * NAME##Calloc(usize count, usize size) {                   \
    allocatorAssertions();                                              \
    void *res;                                                          \
    MULL_CHECK(count, size, USIZE_MAX);                                 \
    if(!count) { return 0; }                                            \
    NULL_CHECK(res = IMP##Malloc(count));                               \
    memset(res, 0, count);                                              \
    return res;                                                         \
FAILED:                                                                 \
    return 0;                                                           \
}                                                                       \
static Ret NAME##Realloc(void **ptr, usize prior, usize count, usize size) { \
    allocatorAssertions();                                              \
    if(!*ptr) { *ptr = NAME##Malloc(count, size); return *ptr ? 0 : -1; } \
    void *res;                                                          \
    MULL_CHECK(count, size, USIZE_MAX);                                 \
    if(!count) { res = 0; goto DONE; }                                  \
    if(IMP##RealAllocSize(prior) == IMP##RealAllocSize(count)) { return 0; } \
    NULL_CHECK(res = IMP##Malloc(count));                               \
    memcpy(res, *ptr, prior);                                           \
DONE:                                                                   \
    IMP##Free(*ptr, prior);                                             \
    *ptr = res;                                                         \
    return 0;                                                           \
FAILED:                                                                 \
    return -1;                                                          \
}                                                                       \
static void NAME##Free(void *ptr, usize prior) { if(ptr) { IMP##Free(ptr, prior); } } \

#define DD_ALLOCATOR(NAME, IMP) \
DEC_ALLOCATOR(NAME);            \
DEF_ALLOCATOR(NAME, IMP);       \

#include "impimp.h"

//////// ~Allocators

//// Alloc related procs
#include <sys/mman.h>


// Note: For implementation ptr should be masked off so caller doesn't have to.
//       Unsure for tag, but will do so for along with an assert.
#define PTR_PTR_MASK() (~PTR_TAG_MAX())
#define PTR_TAG_MASK() (PTR_TAG_MAX())
static_assert(sizeof(uptr) <= sizeof(usize), "Casting type in Ptr fns");
static void *PtrPtr(void *ptr) { return (void *)((uptr)ptr & PTR_PTR_MASK()); }
static usize PtrTag(void *ptr) { return (usize)((uptr)ptr & PTR_TAG_MASK()); }
static void PtrSetPtr(void **ptr, void *val) {
    *ptr = (void *)(((uptr)val & (PTR_PTR_MASK())) | ((uptr)*ptr & PTR_TAG_MASK()));
}
static void PtrSetTag(void **ptr, usize tag) {
    assert(tag <= PTR_TAG_MAX());
    *ptr = (void *)(((uptr)*ptr & (PTR_PTR_MASK())) | ((uptr)tag & PTR_TAG_MASK()));
}
static void PtrSetMembers(void **ptr, void *val, usize tag) {
    assert(tag <= PTR_TAG_MAX());
    *ptr = (void *)(((uptr)val & (PTR_PTR_MASK())) | ((uptr)tag & PTR_TAG_MASK()));
}
static Ret lockAllMemory() {
    CHECK(mlockall(MCL_CURRENT | MCL_FUTURE));
    return 0;
FAILED:
    return -1;
}
static void unlockAllMemory() { CHECK_IGN(munlockall()); }
//// ~Alloc related procs
