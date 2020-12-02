/*
	CC v.01 a language project.
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

#ifndef JC_ARRAY_H
#define JC_ARRAY_H
#include "../macros/macros.h"

#define DEC_ARRAY(NAME, VAL_T)                                                  \
                                                                                \
typedef struct NAME { VAL_T *data; } NAME;                                      \
                                                                                \
static Ret NAME##Init(NAME *arr, usize cap);                                    \
static Ret NAME##Reserve(NAME *arr, usize amt);                                 \
static VAL_T* NAME##Data(NAME *arr);                                            \
static usize NAME##Cap(NAME *arr);                                              \
static Ret NAME##Alive(NAME *arr);                                              \
static void NAME##De(NAME *arr);                                                \

#include "imp.h"

#define DD_ARRAY(NAME, VAL_T, ALLOC)            \
DEC_ARRAY(NAME, VAL_T);                         \
DEF_ARRAY(NAME, VAL_T, ALLOC);                  \


#endif