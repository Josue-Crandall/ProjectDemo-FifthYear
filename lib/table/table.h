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

#ifndef JC_TABLE_H
#define JC_TABLE_H
#include "../macros/macros.h"

#define DEC_TABLE(NAME, KEY_T, VAL_T)                                       \
typedef struct NAME {                                                       \
    Ret *states;                                                            \
    KEY_T *keys;                                                            \
    VAL_T *vals;                                                            \
    usize mask, count, a, b;                                                \
} NAME;                                                                     \
typedef struct NAME##Gen {                                                  \
    usize i, cap; Ret *states; KEY_T *keys; VAL_T *vals;                    \
} NAME##Gen;                                                                \
static Ret NAME##Init(NAME *table, usize cap);                              \
static Ret NAME##Push(NAME *table, KEY_T key, VAL_T val, Ret replace);      \
/* WARNING: Ret *insert , NULL or valid as input not 0 or 1 */              \
static VAL_T *NAME##At(NAME *table, KEY_T key, Ret *insert);                \
static void NAME##Rm(NAME *table, KEY_T key, VAL_T *R() out);               \
static NAME##Gen NAME##Beg(NAME *table);                                    \
static Ret NAME##Next(NAME##Gen *gen);                                      \
static void NAME##Clear(NAME *table);                                       \
static void NAME##De(NAME *table);                                          \

#include "imp.h"

#define DD_TABLE(NAME, KEY_T, VAL_T, HASH, KEY_EQ, KEY_DE, VAL_DE, ALLOC)   \
DEC_TABLE(NAME, KEY_T, VAL_T)                                               \
DEF_TABLE(NAME, KEY_T, VAL_T, HASH, KEY_EQ, KEY_DE, VAL_DE, ALLOC)          \

#endif