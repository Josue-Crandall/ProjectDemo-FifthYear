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

#ifndef JC_RNG_H
#define JC_RNG_H
#include "../macros/macros.h"

#include <sys/random.h>
#define DEV_URANDOM_MAXYBYTES 256

static void devURandom(void *pbuf, usize amt) {
    u8 *buf = pbuf;
    while(amt > DEV_URANDOM_MAXYBYTES) {
        getrandom(buf, DEV_URANDOM_MAXYBYTES, 0);
        buf += DEV_URANDOM_MAXYBYTES;
        amt -= DEV_URANDOM_MAXYBYTES;
    }
    getrandom(buf, amt, 0);
}

static usize rejSample(usize lim) {
    static_assert(sizeof(usize) < DEV_URANDOM_MAXYBYTES, "No check on getrandom calls");
    if(0 == lim) { return 0; }

    size_t mask = lim;
    POW2_ROUND(mask, usize);
    mask -= 1;

    while(1) {
        usize res;
        getrandom(&res, sizeof(usize), 0);
        res &= mask;
        if(res < lim) { return res; }
    }
}

#endif

// Note: want arc4random on openbsd


/*
    apt i rng-tools
    rngtools < key
*/
