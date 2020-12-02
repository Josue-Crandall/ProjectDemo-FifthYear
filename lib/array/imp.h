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

#include "../alloc/alloc.h"
#include "../maf/maf.h"

#define ARRAY_MIN_ALLOCATED_SIZE (8)
static_assert(ARRAY_MIN_ALLOCATED_SIZE > 0, "Infinite loop on reserve if 0");

#define DEF_ARRAY(NAME, VAL_T, ALLOC)                                           \
static Ret NAME##Init(NAME *arr, usize cap) {                                   \
    cap = MafRoundUpNearest2Pow(cap | (ARRAY_MIN_ALLOCATED_SIZE - 1));          \
    NULL_CHECK(arr->data = ALLOC##Malloc(cap, sizeof(VAL_T)));                  \
    PtrSetTag((void **)&arr->data, MafLog2OfPow2(cap));                         \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static Ret NAME##Reserve(NAME *arr, usize amt) {                                \
    usize cap = NAME##Cap(arr);                                                 \
    if(amt > cap) {                                                             \
        usize newCap = cap;                                                     \
        do { UNSIGNED_LSHIFT_CHECK(newCap, usize); } while(newCap < amt);       \
                                                                                \
        void *newData = NAME##Data(arr);                                        \
        CHECK(ALLOC##Realloc((void **)&newData, cap * sizeof(VAL_T), newCap, sizeof(VAL_T))); \
        PtrSetMembers((void **)&arr->data, newData, MafLog2OfPow2(newCap));     \
    }                                                                           \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static VAL_T* NAME##Data(NAME *arr) { return PtrPtr(arr->data); }               \
static Ret NAME##Alive(NAME *arr) { return arr->data ? 1 : 0; }                 \
static usize NAME##Cap(NAME *arr) { return MafTwoPowOf(PtrTag(arr->data)); }    \
static void NAME##De(NAME *arr) { ALLOC##Free(PtrPtr(arr->data), NAME##Cap(arr) * sizeof(VAL_T)); } \

