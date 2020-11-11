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

#ifndef JC_VEC_H
#define JC_VEC_H
#include "../macros/macros.h"

#include "../alloc/alloc.h"

#include <string.h>

#define DEC_VEC(NAME, VAL_T)                                                    \
                                                                                \
typedef struct NAME { VAL_T *data; usize len, cap; } NAME;		                \
typedef struct NAME##Gen { VAL_T *val; VAL_T *end; } NAME##Gen;                 \
typedef Ret (*NAME##PredCB)(void *self, VAL_T *val);                            \
typedef struct NAME##Pred { NAME##PredCB cb; } NAME##Pred;                      \
                                                                                \
static Ret NAME##Init(NAME *vec, usize cap);                                    \
                                                                                \
static Ret NAME##Reserve(NAME *vec, usize amt);                                 \
static Ret NAME##PushArr(NAME *vec, VAL_T *data, usize amt);                    \
static Ret NAME##Push(NAME *vec, VAL_T val);                                    \
static VAL_T NAME##Pop(NAME *vec);                                              \
static usize NAME##Rm(NAME *vec, usize start, usize finish, void *ppred);       \
                                                                                \
static NAME##Gen NAME##Beg(NAME *vec);                                          \
static Ret NAME##Next(NAME##Gen *gen);                                          \
                                                                                \
static VAL_T* NAME##Data(NAME *vec);                                            \
static void NAME##SetSize(NAME *vec, usize newLen);                             \
static usize NAME##Size(NAME *vec);                                             \
static Ret NAME##Alive(NAME *vec);                                              \
                                                                                \
static void NAME##Clear(NAME *vec);                                             \
static void NAME##De(NAME *vec);                                                \

#include "imp.h"

#define DD_VEC(NAME, VAL_T, VAL_DE, ALLOC)  \
DEC_VEC(NAME, VAL_T);                       \
DEF_VEC(NAME, VAL_T, VAL_DE, ALLOC);        \

#endif