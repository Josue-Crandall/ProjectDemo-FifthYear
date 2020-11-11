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

#include "alloc.h"


int main(void) {
    int *a = 0, *b = 0;

    CHECK(mlockRealloc((void **)&b, 0, 7, sizeof(int)));
    mlockFree(b, 7 * sizeof(int));
    NULL_CHECK(a = stdMalloc(5, sizeof(int)));
    NULL_CHECK(b = mlockMalloc(5, sizeof(int)));

    printf("test done\n");

FAILED:
    stdFree(a); mlockFree(b, 7 * sizeof(int));
    return 0;
}
