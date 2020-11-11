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

#include "workQ.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>

#include "../thread/thread.h"


SPRAI(WorkQ, q);

static void chand(int ign) { int ign2 = ign; WorkQStop(q); }
struct thing { QTask t; int val; };
static Ret cb1(PTask **task) {
    struct thing *myThing = (struct thing *)task;
    printf("woo %d\n", myThing->val--);
    if(myThing->val > 0) {
        sleep(1);
        WorkQPush(q, (QTask *)task);
        return PTASK_RET_SPARE;
    }
    else {
        return PTASK_RET_DONE;
    }
}
void dex(PTask **task) { free(task); }
static PTask face1 = {cb1,NULL,dex};

struct thing2 { QTask t; int *val; };
static Ret cb2(PTask **task) {
    struct thing2* theThingeningReturnsAgainForTheSecondToLastTime = (struct thing2 *)task;
    printf("GOT LINE FROM STDIN %d!!!\nECHO:", *theThingeningReturnsAgainForTheSecondToLastTime->val);
    char c;
    while((c = getchar()) != EOF) { printf("%c", c); }
    return PTASK_RET_REPOLL;
}
static PWhen * when(PTask **task) {
    struct thing2* theThingeningReturnsAgainForTheSecondToLastTime = (struct thing2 *)task;
    static PWhen whence[2] = {{0, PWHEN_IN}, {0}};
    return whence;
}
static void de(PTask **task) {
    struct thing2 *theThingeningReturnsAgainForTheSecondToLastTime = (struct thing2 *)task;
    free(theThingeningReturnsAgainForTheSecondToLastTime->val);
    free(theThingeningReturnsAgainForTheSecondToLastTime);
}
static PTask face2 = {cb2, when, de};

int main(void) {
    printf("Compiled\n");
    CHECK(fcntl(0, F_SETFL, O_NONBLOCK));
    CHECK(WorkQInit(q, 12500));
    CHECK(setSignalHandler(SIGINT, chand));
    
    for(int i = 0; i < 5; ++i) {
        struct thing *x = malloc(sizeof (struct thing));
        x->t.vptr = &face1;
        x->val = 3;
        CHECK(WorkQPush(q, (QTask *)x));
    }

    for(int i = 0; i < 1; ++i) {
        struct thing2 *x = malloc(sizeof (struct thing2));
        x->t.vptr = &face2;
        x->val = malloc(sizeof(int));
        *x->val = 7;
        CHECK(WorkQPoll(q, (QTask *)x));
    }

    WorkQLoop(q);

FAILED:
    WorkQDe(q);
    return 0;
}


