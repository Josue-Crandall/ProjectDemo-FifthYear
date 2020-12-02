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

static void *MLOCK_ALLOC_IMPMalloc(usize bytes) {
    void *res;
    bytes = ALLOC_ROUND_TO_AT_LEAST_PAGE_SIZE(bytes);
    NULL_CHECK(res = RAW_ALLOC_IMPMalloc(bytes));
    CHECK(mlock(res, bytes));
    return res;
FAILED:
    RAW_ALLOC_IMPFree(res, bytes);
    return 0;
}

static void MLOCK_ALLOC_IMPFree(void *ptr, usize bytes) {
    if(ptr) {
        bytes = ALLOC_ROUND_TO_AT_LEAST_PAGE_SIZE(bytes);
        explicit_bzero(ptr, bytes);
        CHECK_IGN(munlock(ptr, bytes));
        RAW_ALLOC_IMPFree(ptr, bytes);
    }
}

static usize MLOCK_ALLOC_IMPRealAllocSize(usize bytes) {
    return ALLOC_ROUND_TO_AT_LEAST_PAGE_SIZE(bytes);
}
