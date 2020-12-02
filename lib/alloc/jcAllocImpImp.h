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

//// DD_VEC(JCAllocBucket, void *, NOP, RAW_ALLOC);
// Note: avoiding forward declartion by macro instantiation.
// min replaced with ALLOC_DEFAULT_STORAGE_CAPACITY()
typedef struct JCAllocBucketArr { void * *data; } JCAllocBucketArr; static Ret JCAllocBucketArrInit(JCAllocBucketArr *arr, usize cap); static Ret JCAllocBucketArrReserve(JCAllocBucketArr *arr, usize amt); static void ** JCAllocBucketArrData(JCAllocBucketArr *arr); static usize JCAllocBucketArrCap(JCAllocBucketArr *arr); static Ret JCAllocBucketArrAlive(JCAllocBucketArr *arr); static void JCAllocBucketArrDe(JCAllocBucketArr *arr);
typedef struct JCAllocBucket { JCAllocBucketArr arr; usize len; } JCAllocBucket; typedef Ret (*JCAllocBucketPred)(void *pred, void * *val); static Ret JCAllocBucketInit(JCAllocBucket *vec, usize cap); static Ret JCAllocBucketReserve(JCAllocBucket *vec, usize amt); static Ret JCAllocBucketPushArr(JCAllocBucket *vec, void * *data, usize len); static Ret JCAllocBucketPush(JCAllocBucket *vec, void * val); static void * JCAllocBucketPop(JCAllocBucket *vec); static usize JCAllocBucketRm(JCAllocBucket *vec, usize start, usize finish, JCAllocBucketPred *pred); static void * *JCAllocBucketData(JCAllocBucket *vec); static void JCAllocBucketSetSize(JCAllocBucket *vec, usize newLen); static usize JCAllocBucketSize(JCAllocBucket *vec); static void JCAllocBucketClear(JCAllocBucket *vec); static Ret JCAllocBucketAlive(JCAllocBucket *vec); static void JCAllocBucketDe(JCAllocBucket *vec);
static Ret JCAllocBucketArrInit(JCAllocBucketArr *arr, usize cap) { cap = MafRoundUpNearest2Pow(cap | (ALLOC_DEFAULT_STORAGE_CAPACITY() - 1)); NULL_CHECK(arr->data = RAW_ALLOCMalloc(cap, sizeof(void *))); PtrSetTag((void **)&arr->data, MafLog2OfPow2(cap)); return 0; FAILED: return -1; } static Ret JCAllocBucketArrReserve(JCAllocBucketArr *arr, usize amt) { usize cap = JCAllocBucketArrCap(arr); if(amt > cap) { usize newCap = cap; do { UNSIGNED_LSHIFT_CHECK(newCap, usize); } while(newCap < amt); void *newData = JCAllocBucketArrData(arr); CHECK(RAW_ALLOCRealloc((void **)&newData, cap * sizeof(void *), newCap, sizeof(void *))); PtrSetMembers((void **)&arr->data, newData, MafLog2OfPow2(newCap)); } return 0; FAILED: return -1; } static void ** JCAllocBucketArrData(JCAllocBucketArr *arr) { return PtrPtr(arr->data); } static Ret JCAllocBucketArrAlive(JCAllocBucketArr *arr) { return arr->data ? 1 : 0; } static usize JCAllocBucketArrCap(JCAllocBucketArr *arr) { return MafTwoPowOf(PtrTag(arr->data)); } static void JCAllocBucketArrDe(JCAllocBucketArr *arr) { RAW_ALLOCFree(PtrPtr(arr->data), JCAllocBucketArrCap(arr) * sizeof(void *)); }
static void JCAllocBucketValDe(void * val) { val = val; { ; } } static Ret JCAllocBucketInit(JCAllocBucket *vec, usize cap) { CHECK(JCAllocBucketArrInit(&vec->arr, cap)); vec->len = 0; return 0; FAILED: return -1; } static Ret JCAllocBucketReserve(JCAllocBucket *vec, usize amt) { return JCAllocBucketArrReserve(&vec->arr, amt); } static Ret JCAllocBucketGrowImp(JCAllocBucket *vec, usize amt) { usize newLen = vec->len; ADD_CHECK(newLen, amt, SIZE_MAX); CHECK(JCAllocBucketArrReserve(&vec->arr, newLen)); vec->len = newLen; return 0; FAILED: return -1; } static Ret JCAllocBucketPushArr(JCAllocBucket *vec, void * *data, usize len) { CHECK(JCAllocBucketGrowImp(vec, len)); memcpy(JCAllocBucketData(vec) + vec->len - len, data, sizeof(void *) * len); return 0; FAILED: return -1; } static Ret JCAllocBucketPush(JCAllocBucket *vec, void * val) { CHECK(JCAllocBucketGrowImp(vec, 1)); JCAllocBucketData(vec)[vec->len - 1] = val; return 0; FAILED: return -1; } static void * JCAllocBucketPop(JCAllocBucket *vec) { if(0 == vec->len) { RAI(void *, res); return res; } return JCAllocBucketData(vec)[--vec->len]; } static usize JCAllocBucketRm(JCAllocBucket *vec, usize start, usize finish, JCAllocBucketPred *pred) { assert(start <= JCAllocBucketSize(vec)); assert(finish <= JCAllocBucketSize(vec)); void * *iter = JCAllocBucketData(vec) + start; void * *end = JCAllocBucketData(vec) + finish; for(NOP; iter != end; ++iter) { if((*pred)(pred, iter)) { JCAllocBucketValDe(*iter); break; } } void * * newEnd = iter; if(iter != end) { while(++iter != end) { if((*pred)(pred, iter)) { JCAllocBucketValDe(*iter); } else { *newEnd++ = *iter; } } } usize count = end - newEnd; vec->len -= count; return count; } static void * *JCAllocBucketData(JCAllocBucket *vec) { return JCAllocBucketArrData(&vec->arr); } static void JCAllocBucketSetSize(JCAllocBucket *vec, usize newLen) { vec->len = newLen; } static usize JCAllocBucketSize(JCAllocBucket *vec) { return vec->len; } static void JCAllocBucketClear(JCAllocBucket *vec) { void * *iter = JCAllocBucketData(vec); void * *end = iter + JCAllocBucketSize(vec); for(NOP; iter != end; ++iter) { JCAllocBucketValDe(*iter); } vec->len = 0; } static Ret JCAllocBucketAlive(JCAllocBucket *vec) { return JCAllocBucketArrAlive(&vec->arr); } static void JCAllocBucketDe(JCAllocBucket *vec) { if(JCAllocBucketAlive(vec)) { JCAllocBucketClear(vec); JCAllocBucketArrDe(&vec->arr); } }
//// ~ DD_VEC(JCAllocBucket, void *, NOP, RAW_ALLOC);

// Note: Arenas are an array of ALLOC_BUCKET_COUNT() JCAllocBuckets

static void *JCAllocGlobalGet(JCAllocBucket *localArena, usize index);
static void JCAllocGlobalRet(JCAllocBucket *localArena, usize index);
static void JCAllocGlobalRetAll(JCAllocBucket *localArena);
#include "jcAllocImpImpGlobalArena.h"

static JCAllocBucket *JCAllocGetLocalArena();
static void *JCAllocGetLocalGet(JCAllocBucket *arena, usize index);
static void *JCAllocLocalMake(JCAllocBucket *arena, usize index);
static void JCAllocLocalRet(JCAllocBucket *arena, void *ptr, usize index);
#include "jcAllocImpImpLocalArena.h"

static void *JCAllocGetNode(usize bytes) {
    assert(bytes <= ALLOC_SMALL_ALLOC_CUTOFF());
    usize index = ALLOC_BYTES_TO_INDEX(bytes);
    JCAllocBucket *arena = JCAllocGetLocalArena();

    void *res = JCAllocGetLocalGet(arena, index);
    if(res) { return res; }

    res = JCAllocGlobalGet(arena, index);
    if(res) { return res; }

    return JCAllocLocalMake(arena, index);
}

static void JCAllocRetNode(void *ptr, usize bytes) {
    if(!ptr) { return; }
    assert(bytes <= ALLOC_SMALL_ALLOC_CUTOFF());
    usize index = ALLOC_BYTES_TO_INDEX(bytes);
    JCAllocBucket *arena = JCAllocGetLocalArena();

    if(ALLOC_INDEX_TO_MAX_STORED(index) == JCAllocBucketSize(arena + index)) {
        JCAllocGlobalRet(arena, index);
    }

    JCAllocLocalRet(arena, ptr, index);
}
