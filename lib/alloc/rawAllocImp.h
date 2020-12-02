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

static void *RAW_ALLOC_IMPMalloc(usize bytes) {
    void *res;
#ifdef JC_DEBUG
    CHECK(posix_memalign(&res, ALLOC_PAGE_SIZE(), bytes));
#else
    CHECK(MAP_FAILED == (res = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)));
#endif
    return res;
FAILED:
    return 0;
}

static void RAW_ALLOC_IMPFree(void *ptr, usize bytes) {
#ifdef JC_DEBUG
    bytes = bytes; free(ptr);
#else
    CHECK_IGN(ptr && munmap(ptr, bytes));
#endif
}

static usize RAW_ALLOC_IMPRealAllocSize(usize bytes) { return bytes; }
