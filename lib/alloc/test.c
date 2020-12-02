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

#include "alloc.h"

#include <stdio.h>
#include <pthread.h>

Ret test1() {
    int *test;

    NULL_CHECK(test = JC_ALLOCMalloc(1, sizeof(int)));
    dprintf(2, "%zu lower bits free, %zu max value\nBase ptr ", PTR_FREE_LOWER_TAG_BITS(), PTR_TAG_MAX());
    BIN_DEBUG_WINDOW(&test, sizeof(int *));

    int *hold = test;
    PtrSetTag((void **)&test, PTR_TAG_MAX());
    dprintf(2, "Set tag to all 1's "); BIN_DEBUG_WINDOW(&test, sizeof(int *));
    PtrSetTag((void **)&test, 2);
    dprintf(2, "Set tag to 2 "); BIN_DEBUG_WINDOW(&test, sizeof(int *));    
    PtrSetTag((void **)&test, 63 - 32);
    dprintf(2, "Tag boundry with a 111...0 pattern "); BIN_DEBUG_WINDOW(&test, sizeof(int *));    

    PtrSetPtr((void **)&test, (void *)0);
    dprintf(2, "Set ptr bits to all 0's"); BIN_DEBUG_WINDOW(&test, sizeof(int *)); 
    
    PtrSetPtr((void **)&test, (void *)1);
    dprintf(2, "should be unchanged"); BIN_DEBUG_WINDOW(&test, sizeof(int *));  
    PtrSetPtr((void **)&test, (void *)32);
    dprintf(2, "should be unchanged"); BIN_DEBUG_WINDOW(&test, sizeof(int *)); 
    PtrSetPtr((void **)&test, (void *)64);
    dprintf(2, "should be 1 clobber"); BIN_DEBUG_WINDOW(&test, sizeof(int *)); 
    PtrSetPtr((void **)&test, (void *)(64 * 2));
    dprintf(2, "should be 01 clobber"); BIN_DEBUG_WINDOW(&test, sizeof(int *)); 

    // debug
    test = hold;

    Ret res = 0;
CLEAN:
    JC_ALLOCFree(PtrPtr(test), sizeof(int));
    return res;
FAILED:
    res = -1;
    goto CLEAN;
}

int main2(void) {
    int *a = 0, *b = 0, *c = 0;

    CHECK(MLOCK_ALLOCRealloc((void **)&b, 0, 7, sizeof(int)));
    NULL_CHECK(a = JC_ALLOCMalloc(5, sizeof(int)));
    NULL_CHECK(c = MLOCK_ALLOCMalloc(5, sizeof(int)));

    //

    CHECK(test1());

    printf("test done\n");

FAILED:
    JC_ALLOCFree(a, 5 * sizeof(int));
    MLOCK_ALLOCFree(b, 7 * sizeof(int));
    MLOCK_ALLOCFree(c, 5 * sizeof(int));
    pthread_exit(0);
}

#define TEST_SIZE 13

static void mlockTest0(void) {
    for(int j = 0; j < 7; ++j) {
        int *temp[TEST_SIZE];
        for(usize i = 0; i < TEST_SIZE; ++i) {
            NULL_CHECK(temp[i] = MLOCK_ALLOCMalloc(1, sizeof(int)));
            *temp[i] = i * 3 + 1;
        }
        for(usize i = 0; i < TEST_SIZE; ++i) { CHECK(*temp[i] != (int)i * 3 + 1); }
        for(usize i = 0; i < TEST_SIZE; ++i) { MLOCK_ALLOCFree(temp[i], sizeof(int)); }
    }
    return;
FAILED:
    exit(0);
}
static void bigBranchTest0(void) {
    for(int j = 0; j < 7; ++j) {
        int *temp[TEST_SIZE];
        for(usize i = 0; i < TEST_SIZE; ++i) {
            NULL_CHECK(temp[i] = JC_ALLOCMalloc(1, ALLOC_PAGE_SIZE() + 1));
            *temp[i] = i * 3 + 1;
        }
        for(usize i = 0; i < TEST_SIZE; ++i) { CHECK(*temp[i] != (int)i * 3 + 1); }
        for(usize i = 0; i < TEST_SIZE; ++i) { JC_ALLOCFree(temp[i], ALLOC_PAGE_SIZE() + 1); }
    }

    for(int j = 0; j < 7; ++j) {
        int *temp[TEST_SIZE];
        for(usize i = 0; i < TEST_SIZE; ++i) {
            NULL_CHECK(temp[i] = JC_ALLOCMalloc(1, ALLOC_PAGE_SIZE() - 1));
            *temp[i] = i * 3 + 1;
        }
        for(usize i = 0; i < TEST_SIZE; ++i) { CHECK(*temp[i] != (int)i * 3 + 1); }
        for(usize i = 0; i < TEST_SIZE; ++i) { JC_ALLOCFree(temp[i], ALLOC_PAGE_SIZE() - 1); }
    }

    return;
FAILED:
    exit(0);
}

static void alignToIndexTest(void) {
    for(usize i = 1; i <= 64; ++i) { CHECK(ALLOC_BYTES_TO_INDEX(ALLOC_ALIGN_ROUND(i)) != 0); }
    for(usize i = 65; i <= 128; ++i) { CHECK(ALLOC_BYTES_TO_INDEX(ALLOC_ALIGN_ROUND(i)) != 1); }
    for(usize i = 129; i <= 256; ++i) { CHECK(ALLOC_BYTES_TO_INDEX(ALLOC_ALIGN_ROUND(i)) != 2); }
    for(usize i = 257; i <= 512; ++i) { CHECK(ALLOC_BYTES_TO_INDEX(ALLOC_ALIGN_ROUND(i)) != 3); }
    for(usize i = 513; i <= 1024; ++i) { CHECK(ALLOC_BYTES_TO_INDEX(ALLOC_ALIGN_ROUND(i)) != 4); }
    return;
FAILED:
    exit(0);
}

#undef TEST_SIZE
#define TEST_SIZE 15
#define TEST_SIZE_ITERATIONS 7

static void smallBranchTest0(void) {
    for(int j = 0; j < TEST_SIZE_ITERATIONS; ++j) {
        int *temp[TEST_SIZE];
        for(usize i = 0; i < TEST_SIZE; ++i) {
            NULL_CHECK(temp[i] = JC_ALLOCMalloc(1, sizeof(int)));
            *temp[i] = (int)i * 3 + 1;
        }
        for(usize i = 0; i < TEST_SIZE; ++i) { 
            CHECK(*temp[i] != (int)i * 3 + 1);
        }

        for(usize i = 0; i < TEST_SIZE; ++i) { 
            JC_ALLOCFree(temp[i], sizeof(int));
        }
    }

    for(usize z = sizeof(int); z <= 1 ; z += sizeof(int)) {
        for(int j = 0; j < 7 ; ++j) {
            int *temp[TEST_SIZE];
            int *temp2[TEST_SIZE];

            for(usize i = 0; i < TEST_SIZE; ++i) {
                NULL_CHECK(temp[i] = JC_ALLOCMalloc(1, z));
                *temp[i] = (int)i * 3 + 1;
                NULL_CHECK(temp2[i] = JC_ALLOCMalloc(1, z + sizeof(int)));
                *temp2[i] = (int)i * 3 + 2;
            }
            for(usize i = 0; i < TEST_SIZE; ++i) { 
                CHECK(*temp[i] != (int)i * 3 + 1); 
                CHECK(*temp2[i] != (int)i * 3 + 2); 
            }
            for(usize i = 0; i < TEST_SIZE; ++i) { 
                JC_ALLOCFree(temp[i], z); 
                JC_ALLOCFree(temp2[i], z + sizeof(int)); 
            }
        }
    }

    return;
FAILED:
    exit(0);
}


int main(void) {
    mlockTest0();
    bigBranchTest0();
    alignToIndexTest();
    smallBranchTest0();

    printf("Made it to end of main1\n");
    //main2();
FAILED:
    pthread_exit(0);
}

// Note: should test the alloc/realloc/calloc pathways as I've redone them now
