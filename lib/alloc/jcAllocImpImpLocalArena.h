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

static POnce JCAllocLocalOnce = PONCE_STATIC_INIT;
static PLocal JCAllocLocalArena;

static void JCAllocLocalArenaDe(void *arg) {
    if(arg) {
        JCAllocBucket *localArena = arg;
        JCAllocGlobalRetAll(localArena);
        for(usize i = 0; i < ALLOC_BUCKET_COUNT(); ++i) { JCAllocBucketDe(localArena + i); }
        RAW_ALLOCFree(localArena, sizeof(JCAllocBucket) * ALLOC_BUCKET_COUNT());
    }
}
static void JCAllocLocalArenaInit(void) {
    CHECK(PLocalInit(&JCAllocLocalArena, JCAllocLocalArenaDe));
    return;
FAILED:
    SleepMilisec(100);
    JCAllocLocalArenaInit();
}
static JCAllocBucket *JCAllocGetLocalArena() {
    POnceDo(&JCAllocLocalOnce, JCAllocLocalArenaInit);

    JCAllocBucket *localArena;

    if(0 == (localArena = PLocalGet(&JCAllocLocalArena))) {
        NULL_CHECK(localArena = RAW_ALLOCCalloc(ALLOC_BUCKET_COUNT(), sizeof(JCAllocBucket)));
        for(usize i = 0; i < ALLOC_BUCKET_COUNT(); ++i) { 
            CHECK(JCAllocBucketInit(localArena + i, 0)); 
            CHECK(PLocalSet(&JCAllocLocalArena, localArena));
        }
    }

    return localArena;

FAILED:
    JCAllocLocalArenaDe(localArena);
    SleepMilisec(100);
    return JCAllocGetLocalArena();
}

static void *JCAllocGetLocalGet(JCAllocBucket *arena, usize index) {
    return JCAllocBucketPop(arena + index);
}
static void *JCAllocLocalMake(JCAllocBucket *arena, usize index) {
    usize size = ALLOC_INDEX_TO_SIZE(index);
    assert(ALLOC_PAGE_SIZE() % size == 0);
    u8 *page = RAW_ALLOCMalloc(1, ALLOC_PAGE_SIZE());
    if(!page) { return 0; }
    
    for(u8 *iter = page + size, *end = page + ALLOC_PAGE_SIZE(); iter != end; iter += size) {
        while(JCAllocBucketPush(arena + index, iter)) {
            DEBUG_LOG("Local alloc bucket push failed ... retrying\n");
            SleepMilisec(100);
        }
    }
    
    return page;
}

static void JCAllocLocalRet(JCAllocBucket *arena, void *ptr, usize index) {
    while(JCAllocBucketPush(arena + index, ptr)) {
        DEBUG_LOG("Local alloc bucket push failed ... retrying\n");
        SleepMilisec(100);
    }
}
