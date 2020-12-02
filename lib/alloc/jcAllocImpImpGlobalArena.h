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

static PMutex JCAllocGlobalMutex = PMUTEX_STATIC_INIT;
static usize JCAllocGlobalArenaReady;
static JCAllocBucket *JCAllocGlobalArena;

static void JCAllocGlobalArenaInit(void) {
    if(JCAllocGlobalArenaReady != ALLOC_BUCKET_COUNT()) {
        if(!JCAllocGlobalArena) { 
            NULL_CHECK(JCAllocGlobalArena = RAW_ALLOCMalloc(ALLOC_BUCKET_COUNT(), sizeof(JCAllocBucket)));
        }

        for(NOP; JCAllocGlobalArenaReady < ALLOC_BUCKET_COUNT(); ++JCAllocGlobalArenaReady) {
            CHECK(JCAllocBucketInit(JCAllocGlobalArena + JCAllocGlobalArenaReady, 0));
        }
    }
    return;
FAILED:
    SleepMilisec(100);
    JCAllocGlobalArenaInit();
}
static void *JCAllocGlobalGet(JCAllocBucket *localArena, usize index) {
    localArena += index;
    usize count = ALLOC_INDEX_TO_COUNT(index);

    if(PMutexTryLock(&JCAllocGlobalMutex)) { return 0; }
    JCAllocGlobalArenaInit();

    for(usize i = 0; i < count; ++i) {
        void *node = JCAllocBucketPop(JCAllocGlobalArena + index);
        if(!node) { break; }

        while(JCAllocBucketPush(localArena, node)) {
            DEBUG_LOG("JCAlocBucketPush in JCAllocGlobalGet failed ... retrying\n");
            SleepMilisec(100);
        }
    }

    PMutexUnlock(&JCAllocGlobalMutex);

    return JCAllocBucketPop(localArena);
}
static void JCAllocGlobalRet(JCAllocBucket *localArena, usize index) {
    localArena += index;
    usize localArenaSize = JCAllocBucketSize(localArena);
    if(localArenaSize) {
        void **localArenaData = JCAllocBucketData(localArena);

        PMutexLock(&JCAllocGlobalMutex);
        while(JCAllocBucketPushArr(JCAllocGlobalArena + index, localArenaData, localArenaSize)) {
            DEBUG_LOG("Failing to alloc for global push... retrying\n");
            SleepMilisec(100);
        }
        PMutexUnlock(&JCAllocGlobalMutex);
        
        JCAllocBucketClear(localArena);
    }
}
static void JCAllocGlobalRetAll(JCAllocBucket *localArena) {
    usize index = 0;
    while(index < ALLOC_BUCKET_COUNT() && !JCAllocBucketSize(localArena + index)) { ++index; }
    if(index < ALLOC_BUCKET_COUNT()) {
        PMutexLock(&JCAllocGlobalMutex);
        
        do {
            while(JCAllocBucketPushArr(JCAllocGlobalArena + index,
                JCAllocBucketData(localArena + index),
                JCAllocBucketSize(localArena + index)))
            {
                DEBUG_LOG("Failing to alloc for global push... retrying\n");
                SleepMilisec(100);
            }
            JCAllocBucketClear(localArena + index);
        }
        while(++index < ALLOC_BUCKET_COUNT());
        
        PMutexUnlock(&JCAllocGlobalMutex);
    }
}
