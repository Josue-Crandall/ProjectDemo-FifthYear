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

#define DEF_CVEC(NAME, VAL_T, VAL_DE, ALLOC)                \
static Ret NAME##Init(NAME *cvec, size_t cap) {             \
    memset(cvec, 0, sizeof(*cvec));                         \
                                                            \
    CHECK(NAME##VecInit(&cvec->vec, cap));                  \
    CHECK(PMutexInit(&cvec->mutex));                        \
    CHECK(PCondInit(&cvec->cond));                          \
    Arc8Init(&cvec->done, 0);                               \
    return 0;                                               \
                                                            \
FAILED:                                                     \
    NAME##VecDe(&cvec->vec);                                \
    PMutexDe(&cvec->mutex);                                 \
    PCondDe(&cvec->cond);                                   \
    memset(cvec, 0, sizeof(*cvec));                         \
    return -1;                                              \
}			                                                \
static Ret NAME##Push(NAME *  cvec, VAL_T val) {            \
    PMutexLock(&cvec->mutex);                               \
    if(Arc8Read(&cvec->done)) { goto FAILED; }              \
    CHECK(NAME##VecPush(&cvec->vec, val));                  \
                                                            \
    Ret err = 0;                                            \
CLEAN:                                                      \
    PCondSignal(&cvec->cond, 0);                            \
    PMutexUnlock(&cvec->mutex);                             \
    return err;                                             \
FAILED:                                                     \
    err = -1;                                               \
    goto CLEAN;                                             \
}			                                                \
static VAL_T NAME##Pop(NAME *cvec) {                        \
    VAL_T res;                                              \
                                                            \
    PMutexLock(&cvec->mutex);                               \
    while(1) {                                              \
        if(Arc8Read(&cvec->done)) { goto FAILED; }          \
        if(0 == NAME##VecSize(&cvec->vec)) {                \
            PCondWait(&cvec->cond, &cvec->mutex);           \
        }                                                   \
        else {                                              \
            res = NAME##VecPop(&cvec->vec);                 \
            break;                                          \
        }                                                   \
    }                                                       \
                                                            \
CLEAN:                                                      \
    PMutexUnlock(&cvec->mutex);                             \
    return res;                                             \
FAILED:                                                     \
    memset(&res, 0, sizeof(VAL_T));                         \
    PCondSignal(&cvec->cond, 0);                            \
    goto CLEAN;                                             \
}	                	                                    \
static void NAME##Stop(NAME *cvec) {                        \
    Arc8Write(&cvec->done, 1);                              \
    PCondSignal(&cvec->cond, 0);                            \
}						                                    \
static void NAME##De(NAME *cvec) {                          \
    NAME##VecDe(&cvec->vec);                                \
    PMutexDe(&cvec->mutex);                                 \
    PCondDe(&cvec->cond);                                   \
}								                            \
