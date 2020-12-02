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

#ifndef JC_ALLOC_H
#define JC_ALLOC_H
#include "../macros/macros.h"

// Note: JCAlloc using an instantiated version of DD_VEC().

//// Static Allocators
// Note: Unlike malloc et al... Alloctors here are defined to return
//       0 on size 0 calls, not 0 or a free'able pointer.

// Note: Value choosen in order to store capacity in the pointer as a power of two
#define PTR_ALIGN_SIZE() (sizeof(usize) * 8)
#define PTR_FREE_LOWER_TAG_BITS() (MafLog2OfPow2(PTR_ALIGN_SIZE()))
#define PTR_TAG_MAX() (PTR_ALIGN_SIZE() - 1)
static void *PtrPtr(void *ptr);
static void PtrSetPtr(void **ptr, void *val);
static usize PtrTag(void *ptr);
static void PtrSetTag(void **ptr, usize tag);
static void PtrSetMembers(void **ptr, void *val, usize tag);

#define DEC_ALLOCATOR(NAME)                                                    \
static void * NAME##Malloc(usize count, usize size);                           \
static void * NAME##Calloc(usize count, usize size);                           \
static Ret NAME##Realloc(void **ptr, usize prior, usize count, usize size);    \
static void NAME##Free(void *ptr, usize prior);                                \

static Ret lockAllMemory();
static void unlockAllMemory();

// Note: Forward declared for use in other allocators.
DEC_ALLOCATOR(RAW_ALLOC);

#include "imp.h"

// Note: ALL allocators are aligned to at least PTR_ALIGN_SIZE()
DD_ALLOCATOR(STD_ALLOC, STD_ALLOC_IMP);
// Note: RAW_ALLOC is page aligned.
DEF_ALLOCATOR(RAW_ALLOC, RAW_ALLOC_IMP);
DD_ALLOCATOR(MLOCK_ALLOC, MLOCK_ALLOC_IMP);
DD_ALLOCATOR(JC_ALLOC, JC_ALLOC_IMP);

#endif


// tcmalloc, jemalloc 
// /proc/sys/vm/mmap_min_addr 65536 // u16 max here
// MALLOC_ARENA_MAX 
