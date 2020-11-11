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

#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h> // sysconf

//// static allocators
// STD_ALLOC
#define STD_ALLOCMalloc(COUNT, SIZE)                    stdMalloc(COUNT, SIZE)
#define STD_ALLOCCalloc(COUNT, SIZE)                    stdCalloc(COUNT, SIZE)
#define STD_ALLOCRealloc(PTR, PRIOR, COUNT, SIZE)       stdRealloc(PTR, COUNT, SIZE)
#define STD_ALLOCFree(PTR, SIZE)                        stdFree(PTR)
// MLOCK_ALLOC
#define MLOCK_ALLOCMalloc(COUNT, SIZE)                  mlockMalloc(COUNT, SIZE)
#define MLOCK_ALLOCCalloc(COUNT, SIZE)                  mlockCalloc(COUNT, SIZE)
#define MLOCK_ALLOCRealloc(PTR, PRIOR, COUNT, SIZE)     mlockRealloc(PTR, PRIOR, COUNT, SIZE)
#define MLOCK_ALLOCFree(PTR, SIZE)                      mlockFree(PTR, SIZE)
// Related memory proc
static Ret lockAllMemory();
static void unlockAllMemory();

#include "imp.h"

#endif


// jemalloc 