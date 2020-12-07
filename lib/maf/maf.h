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

#ifndef JC_MAF_H
#define JC_MAF_H
#include "../macros/macros.h"

static Ret MafIsPowTwo(usize val) {
    static_assert(sizeof(usize) == sizeof(unsigned long), "type casting arg");
    return 1 == __builtin_popcountl(val);
}
static usize MafLog2OfPow2(usize size) {
    static_assert(sizeof(usize) == sizeof(unsigned long), "type casting arg");
    if(!size) { return 0; }
    else { return (usize)__builtin_ctzl(size); }
}
static usize MafTwoPowOf(usize size) {
    return (usize)1 << size;
}
// WARNING: Overflows to 0
static usize MafRoundUpNearest2Pow(usize val) {
    if(val < 2) { return val; }
    else { return ((usize)1 << (sizeof(usize) * 8 - 1)) >> ((__builtin_clzl(val - 1)) - 1); }
}

#endif
