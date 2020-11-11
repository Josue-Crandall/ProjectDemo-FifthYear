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

#ifndef JC_MACROS_H
#define JC_MACROS_H

//////// This region is reserved for defines before any includes.
#ifdef __linux__
    #define _GNU_SOURCE
#endif
////////

#include <stddef.h>     // size_t, ptrdiff_t, max_align_t, offsetof(struct name, member), NULL
#include <stdint.h>     // uint8_t ...
#include <sys/types.h>  // ssize_t (posix)
#include <limits.h>     // INT_MAX, etc...
#include <string.h>     // memcpy, memset
#include <assert.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef size_t   usize;
#define USIZE_MAX SIZE_MAX
typedef ssize_t  isize;
#define ISIZE_MAX SSIZE_MAX
typedef signed long long ill;
typedef unsigned long long ull;
typedef int_fast8_t Ret;
typedef max_align_t umax;
typedef uintptr_t uptr;
typedef intptr_t iptr;

#define NOP // no op
#define R() restrict
#define C() ,

static_assert(CHAR_BIT == 8, "Assumed.");

#ifdef JC_DEBUG

#include <stdio.h>

#define DEBUG_LOG(PRINTF_BODY) fprintf(stderr,PRINTF_BODY);

#define DEBUG_WINDOW(START, LEN) {                          \
    u8 *R() DEBUG_WINDOW_ITER = (u8 *R())(START);           \
    u8 *R() DEBUG_WINDOW_END  = DEBUG_WINDOW_ITER + (LEN);  \
    usize DEBUG_WINDOW_COUNT = 0;                           \
                                                            \
    fprintf(stderr, "Debug window for: %s, len %td\n", #START, DEBUG_WINDOW_END - DEBUG_WINDOW_ITER); \
                                                            \
    while(DEBUG_WINDOW_ITER != DEBUG_WINDOW_END) {          \
        fprintf(stderr,"%02X ", *DEBUG_WINDOW_ITER++);      \
        if(++DEBUG_WINDOW_COUNT == 4) { fprintf(stderr," "); }      \
        else if(DEBUG_WINDOW_COUNT == 8) {                  \
            fprintf(stderr,"\n");                           \
            DEBUG_WINDOW_COUNT = 0;                         \
        }                                                   \
    }                                                       \
    fprintf(stderr,"\n");                                   \
}                                                           \

#define CHECK(EXP) {                                        \
    ill CHECK_RET = (ill)(EXP);                             \
    if(CHECK_RET) {                                         \
        fprintf(stderr,"%s failed check with (%lli)\n", #EXP, CHECK_RET);  \
        goto FAILED;                                        \
    }                                                       \
}                                                           \

#define EINTR_CHECK(EXP) {                                                  \
    while(1) {                                                              \
        ill CHECK_RET = (ill)(EXP);                                         \
        if(CHECK_RET < 0) {                                                 \
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) { continue; }      \
            fprintf(stderr,"%s failed eintr check with (%lli)\n", #EXP, CHECK_RET); \
            goto FAILED;                                                    \
        }                                                                   \
        break;                                                              \
    }                                                                       \
}                                                                           \

#define EINTR_CHECK_IGN(EXP) {                                              \
    while(1) {                                                              \
        ill CHECK_RET = (ill)(EXP);                                         \
        if(CHECK_RET < 0) {                                                 \
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) { continue; } \
            fprintf(stderr,"%s failed eintr check with (%lli)\n", #EXP, CHECK_RET); \
        }                                                                   \
        break;                                                              \
    }                                                                       \
}                                                                           \


#else
    #define NDEBUG

#define DEBUG_LOG(PRINTF_BODY) NOP
#define DEBUG_WINDOW(START, END) NOP
#define CHECK(EXP) { if(EXP) { goto FAILED; } }
#define EINTR_CHECK(EXP) {                      \
    while(1) {                                  \
        if((EXP) < 0) {                         \
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) { continue; }    \
            goto FAILED;                        \
        }                                       \
        break;                                  \
    }                                           \
}                                               \

#define EINTR_CHECK_IGN(EXP) {                         \
    while(1) {                                         \
        if(((EXP) < 0) && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) { continue; }  \
        break;                                         \
    }                                                  \
}                                                      \

#endif

#define NULL_CHECK(EXP) CHECK(!(EXP))
#define FD_CLOSE(FD) { if((FD) > -1) { EINTR_CHECK_IGN(close(FD)); } }
#define NEG_CHECK(EXP) CHECK((EXP) < 0)
// Note: ADD_CHECK must work for ADD_CHECK(A,A,TYPE_A)
#define ADD_CHECK(LHS, RHS, LIM) { CHECK(LHS > (LIM) - (RHS)); (LHS) += (RHS); }
#define MULL_CHECK(LHS, RHS, LIM) { if(RHS) { CHECK(LHS > (LIM) / (RHS)); } (LHS) *= (RHS); }
#define UNSIGNED_LSHIFT_CHECK(VAL, TYPE) { CHECK(((TYPE)1 << ((sizeof(TYPE) * CHAR_BIT) - 1)) & VAL); (VAL) <<= 1; }
#define SWAP(A,B,TYPE) { TYPE SWAP_TEMP = (A); (A) = (B); (B) = SWAP_TEMP; }
#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define MIN(A,B) ((A) > (B) ? (B) : (A))
#define RAI(TYPE,NAME) TYPE NAME; memset(& NAME, 0, sizeof(TYPE));
#define PRAI(TYPE,NAME) RAI(TYPE, NAME##RAI_DATA); TYPE *NAME = & NAME##RAI_DATA;
#define SPRAI(TYPE, NAME) static TYPE NAME##SPRAI_DATA; TYPE *NAME = &NAME##SPRAI_DATA;
// Note: untested
#define ROUND_TO_MULTIPLE(VAL, MULTIPLE, TYPE, LIM) { TYPE ROUND_TO_MULTIPLE_TEMP = (MULTIPLE) - ((VAL) % (MULTIPLE)); ADD_CHECK(VAL, ROUND_TO_MULTIPLE_TEMP, LIM); }
#define HOST_LITTLE_ENDIAN() (*(u16 *)"\x01" == (u16)1)
#define POW2_ROUND(VAL, TYPE) {                                                 \
    --(VAL);                                                                    \
    for(TYPE ROUND_SHIFT_TEMP = 1, ROUND_END_TEMP = sizeof(TYPE) * CHAR_BIT;    \
        ROUND_SHIFT_TEMP < ROUND_END_TEMP; ROUND_SHIFT_TEMP <<= 1)              \
    {                                                                           \
        (VAL) |= (VAL) >> ROUND_SHIFT_TEMP;                                     \
    }                                                                           \
    ++(VAL);                                                                    \
}                                                                               \

#endif
