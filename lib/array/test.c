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

#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

DEC_ARRAY(Arr, int);
DEF_ARRAY(Arr, int, STD_ALLOC);

SPRAI(Arr, a);

int main(void) {
    // test0
    ArrDe(a);

    CHECK(ArrInit(a, 15));
    printf("%zu cap \n", ArrCap(a));
    CHECK(ArrReserve(a, 31));
    printf("%zu cap \n", ArrCap(a));

    int *data = ArrData(a);
    data[31] = 7;

    // Sanity check
    //data[35] = 7;
    
    ArrDe(a);
    CHECK(ArrInit(a, 16));
    data = ArrData(a);
    data[15] = 8;

    // Sanity check
    //data[16] = 7;

FAILED:
    ArrDe(a);
    return 0;
}
