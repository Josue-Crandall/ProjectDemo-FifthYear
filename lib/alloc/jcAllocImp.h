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

#include "jcAllocImpImp.h"

static void *JC_ALLOC_IMPMalloc(usize bytes) {
    NULL_CHECK(bytes = ALLOC_ALIGN_ROUND(bytes));
    void *res;
    if(bytes > ALLOC_SMALL_ALLOC_CUTOFF()) { res = RAW_ALLOC_IMPMalloc(bytes); }
    else { res = JCAllocGetNode(bytes); }

#ifdef JC_DEBUG
    if(res) { ArcInc(&JC_DEBUG_ALLOCATOR_COUNTER); }
#endif

    return res;
FAILED:
    return 0;
}

static void JC_ALLOC_IMPFree(void *ptr, usize bytes) {
    if(ptr) {
        bytes = ALLOC_ALIGN_ROUND(bytes);
        if(bytes > ALLOC_SMALL_ALLOC_CUTOFF()) { RAW_ALLOC_IMPFree(ptr, bytes); }
        else { JCAllocRetNode(ptr, bytes); }

    #ifdef JC_DEBUG
        ArcDec(&JC_DEBUG_ALLOCATOR_COUNTER);
    #endif
    }
}

static usize JC_ALLOC_IMPRealAllocSize(usize bytes) {
    return ALLOC_ALIGN_ROUND(bytes);
}