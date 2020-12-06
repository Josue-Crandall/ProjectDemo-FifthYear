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

#ifndef JC_CVEC_H
#define JC_CVEC_H
#include "../macros/macros.h"

#include "../arc/arc.h"
#include "../vec/vec.h"
#include "../thread/thread.h"

#include <string.h>

#define DEC_CVEC(NAME, VAL_T, VAL_DE, ALLOC)                \
                                                            \
DD_VEC(NAME##Vec, VAL_T, VAL_DE, ALLOC);					\
                                                            \
typedef struct NAME { 						    			\
    NAME##Vec vec; 											\
    PMutex mutex; 											\
    PCond cond; 											\
    Arc8 done;												\
} NAME;														\
static Ret NAME##Init(NAME *cvec, size_t cap);              \
static Ret NAME##Push(NAME *  cvec, VAL_T val);             \
static VAL_T NAME##Pop(NAME *cvec);                         \
static void NAME##Stop(NAME *cvec);                         \
/* WARNING: NAME##De is NOT threadsafe */                   \
static void NAME##De(NAME *cvec);                           \

#include "imp.h"

#define DD_CVEC(NAME, VAL_T, VAL_DE, ALLOC)		            \
DEC_CVEC(NAME, VAL_T, VAL_DE, ALLOC);                       \
DEF_CVEC(NAME, VAL_T, VAL_DE, ALLOC);                       \

#endif
