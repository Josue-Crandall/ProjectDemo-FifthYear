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

#include "../macros/macros.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "arc.h"


int main(void) {
    printf("Testing: arc.h ... ");

    srand(time(0));

    for(int i = 0; i < 10; ++i) { Ret test0(); CHECK(test0()); }

    printf(" passed\n");
    return 0;
    
FAILED:
    printf(" FAILED\n");
    return 0;
}

typedef struct { int a; Arc count; } Thing;

Ret test0() {
    Ret res = -1;

    Thing *a, *b, *c;
    {
        a = (Thing *) malloc(sizeof(Thing));
        a->a = 12;
        ArcInit(&a->count, 1);
    }
    {
        b = a;
        ArcInc(&b->count);
        c = b;
        ArcInc(&c->count);
    }

    if(ArcDec(&a->count)) free(a);
    if(ArcDec(&b->count)) free(b);

    CHECK(c->a != 12);
    
    res = 0;
    
FAILED:
    if(ArcDec(&c->count)) free(c);
    return res;
}
