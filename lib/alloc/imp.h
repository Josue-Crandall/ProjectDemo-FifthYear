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

static void *stdMalloc(usize count, usize size) {
    MULL_CHECK(count, size, SIZE_MAX);
    void *res = malloc(count);
    return res;
FAILED:
    return 0;
}
static void *stdCalloc(usize count, usize size) {
    return calloc(count, size);
}
static Ret stdRealloc(void **ptr, usize count, usize size) {
    MULL_CHECK(count, size, SIZE_MAX);
    void *temp; NULL_CHECK(temp = realloc(*ptr, count));
    *ptr = temp;
    return 0;
FAILED:
    return -1;
}
static void stdFree(void *ptr) { free(ptr); }
static isize getPageSize() {
    static_assert(sizeof(long) <= sizeof(isize), "sysconf returns long, returning as isize");
    static_assert(sizeof(isize) == sizeof(usize), "treating positive isize as usize later");
    isize pageSize;
    NEG_CHECK(pageSize = sysconf(_SC_PAGE_SIZE));
    return pageSize;
FAILED:
    return -1;
}
static void *mlockMalloc(usize count, usize size) {
    void *res = 0;
    MULL_CHECK(count, size, SIZE_MAX);
    
    isize pageSize;
    NEG_CHECK(pageSize = getPageSize());

    if(posix_memalign(&res, pageSize, count)) {
        res = 0;
        DEBUG_LOG("posix_memalign failed\n");
        goto FAILED;
    }

    CHECK(mlock(res, count));

    return res;
FAILED:
    stdFree(res);
    return 0;
}
static void mlockFree(void *ptr, usize bytes) {
    if(ptr) {
        explicit_bzero(ptr, bytes);
        if(munlock(ptr, bytes)) { DEBUG_LOG("mlockFree has failed munlock\n"); }
        stdFree(ptr);
    }
}
static void *mlockCalloc(usize count, usize size) {
    void *res = 0;
    NULL_CHECK(res = mlockMalloc(count, size));
    memset(res, 0, count * size);
    return res;
FAILED:
    mlockFree(res, count * size);
    return 0;
}
static Ret mlockRealloc(void **ptr, usize priorSize, usize count, usize size) {
    void *res = 0;
    
    NULL_CHECK(res = mlockMalloc(count, size));
    if(*ptr) {
        memcpy(res, *ptr, priorSize);
        mlockFree(*ptr, priorSize);
    }

    *ptr = res;
    return 0;
FAILED:
    mlockFree(res, 0);
    return -1;
}
static Ret lockAllMemory() {
    CHECK(mlockall(MCL_CURRENT | MCL_FUTURE));
    return 0;
FAILED:
    return -1;
}
static void unlockAllMemory() {
    if(munlockall()) { DEBUG_LOG("munlockall has failed\n"); }
}
