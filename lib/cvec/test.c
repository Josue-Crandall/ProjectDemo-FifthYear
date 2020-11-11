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
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include "cvec.h"
DD_CVEC(CVec, char *, NOP, STD_ALLOC);

static CVec work;

char *BLURBS[] = {
    "Your orders. Templar?",
    "We feel your presence.",
    "We are vigilant.",
    "We need focus..",
    "Instructions.",
    "Your command?",
    "I stand ready.",
    "Let us attack!",
    "I'm wating...",
    "Yes?",
    "Awaiting instructions.",
    "Za Khaladas.",
    "Make use of me.",
    "I am needed?",
    "Your thoughts...?",
    "I heed thy call.",
    "Awaiting command.",
    "Standing by.",
    "Contact.",
    "Gee'hous!",
    "My life for Auir!",
    "I long for combat",
};

void * prod(void *arg) {
    void *ign = arg;
    //printf("prod has arrived\n");
    while(!CVecPush(&work, BLURBS[rand() % (sizeof(BLURBS) / sizeof(BLURBS[0]))])) { 
        sleep(rand() % 1 + 1); 
    }
    return 0;
}

void * cons(void *arg) {
    void *ign = arg;
    //printf("My life for Auir ... \n");

    const char *blurb;
    while( (blurb = CVecPop(&work))) {
        printf("%s\n", blurb);
        sleep(3);
    }

    return 0;
}

Ret test0() {
    for(int i = 0; i < 2; ++i) {
        PThreadInit(NULL, cons, NULL);
        PThreadInit(NULL, prod, NULL);
        PThreadInit(NULL, cons, NULL);
    }

    getchar();
    CVecStop(&work);
    sleep(1);
    // Note: not syncrhonized, will set off thread san at this point
    return 0;
}

int main(void) {
    srand(time(NULL));
    CHECK(CVecInit(&work, 1024));
    CHECK(test0());
    printf("passing!\n");

CLEAN:
    CVecDe(&work);
    return 0;

FAILED:
    printf("---   FAILED TESTS    --- \n");
    goto CLEAN;
}