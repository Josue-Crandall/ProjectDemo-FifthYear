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

#include "nacl.h"

#ifndef LIBSODIUM_IMPLEMENTATION

void randombytes(void * buf, ull bytes) {
    while(bytes > USIZE_MAX) {
        devURandom(buf, USIZE_MAX);
        buf += USIZE_MAX;
        bytes -= USIZE_MAX;
    }
    devURandom(buf, bytes);
}

#endif

Ret naclEq(u8 *plhs, u8 *prhs, usize len) {
    volatile u8 *lhs = plhs, *rhs = prhs;
    volatile u8 res = 0;
    volatile usize vlen = len;

    while(vlen--) { res |= lhs[vlen] ^ rhs[vlen]; }

    return 0 == res;
}
