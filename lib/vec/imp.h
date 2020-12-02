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

#include "../array/array.h"

#include <string.h>

#define DEF_VEC(NAME, VAL_T, VAL_DE, ALLOC)                                     \
                                                                                \
DEF_ARRAY(NAME##Arr, VAL_T, ALLOC);                                             \
static void NAME##ValDe(VAL_T val) { val = val; { VAL_DE; } }                   \
                                                                                \
static Ret NAME##Init(NAME *vec, usize cap) {                                   \
    CHECK(NAME##ArrInit(&vec->arr, cap));                                       \
    vec->len = 0;                                                               \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static Ret NAME##Reserve(NAME *vec, usize amt) { return NAME##ArrReserve(&vec->arr, amt); } \
static Ret NAME##GrowImp(NAME *vec, usize amt) {                                \
    usize newLen = vec->len;                                                    \
    ADD_CHECK(newLen, amt, SIZE_MAX);                                           \
    CHECK(NAME##ArrReserve(&vec->arr, newLen));                                 \
    vec->len = newLen;                                                          \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static Ret NAME##PushArr(NAME *vec, VAL_T *data, usize len) {                   \
    CHECK(NAME##GrowImp(vec, len));                                             \
    memcpy(NAME##Data(vec) + vec->len - len, data, sizeof(VAL_T) * len);        \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static Ret NAME##Push(NAME *vec, VAL_T val) {                                   \
    CHECK(NAME##GrowImp(vec, 1));                                               \
    NAME##Data(vec)[vec->len - 1] = val;                                        \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static VAL_T NAME##Pop(NAME *vec) {                                             \
    if(0 == vec->len) { RAI(VAL_T, res); return res; }                          \
    return NAME##Data(vec)[--vec->len];                                         \
}                                                                               \
static usize NAME##Rm(NAME *vec, usize start, usize finish, NAME##Pred *pred) { \
    assert(start <= NAME##Size(vec));                                           \
    assert(finish <= NAME##Size(vec));                                          \
                                                                                \
    VAL_T *iter = NAME##Data(vec) + start;                                      \
    VAL_T *end = NAME##Data(vec) + finish;                                      \
                                                                                \
    for(NOP; iter != end; ++iter) {                                             \
        if((*pred)(pred, iter)) { NAME##ValDe(*iter); break; }                  \
    }                                                                           \
                                                                                \
    VAL_T * newEnd = iter;                                                      \
                                                                                \
    if(iter != end) {                                                           \
        while(++iter != end) {                                                  \
            if((*pred)(pred, iter)) { NAME##ValDe(*iter); }                     \
            else { *newEnd++ = *iter; }                                         \
        }                                                                       \
    }                                                                           \
                                                                                \
    usize count = end - newEnd;                                                 \
    vec->len -= count;                                                          \
    return count;                                                               \
}                                                                               \
                                                                                \
static VAL_T *NAME##Data(NAME *vec) { return NAME##ArrData(&vec->arr); }        \
static void NAME##SetSize(NAME *vec, usize newLen) { vec->len = newLen; }       \
static usize NAME##Size(NAME *vec) { return vec->len; }                         \
                                                                                \
static void NAME##Clear(NAME *vec) {                                            \
    VAL_T *iter = NAME##Data(vec);                                              \
    VAL_T *end = iter + NAME##Size(vec);                                        \
    for(NOP; iter != end; ++iter) { NAME##ValDe(*iter); }                       \
    vec->len = 0;                                                               \
}                                                                               \
static Ret NAME##Alive(NAME *vec) { return NAME##ArrAlive(&vec->arr); }         \
static void NAME##De(NAME *vec) {                                               \
    if(NAME##Alive(vec)) {                                                      \
        NAME##Clear(vec);                                                       \
        NAME##ArrDe(&vec->arr);                                                 \
    }                                                                           \
}                                                                               \
