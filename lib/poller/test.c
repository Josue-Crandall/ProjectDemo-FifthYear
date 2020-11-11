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

#include "poller.h"
#include "../macros/term.h"
#include "../thread/thread.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

SPRAI(Poller, p);

struct myTask {
    PTask *t;
    int x, y;
};
static Ret cb(PTask **task) {
    struct myTask *t = (struct myTask *)task;
    printf("%d: ", t->x++);
    int c;
    while((c = getchar()) != EOF) { printf("%c", c); }
    printf("\n");
    if(t->x < 5) { return PTASK_RET_REPOLL; }
    else { 
        return PTASK_RET_ERROR;
        return PTASK_RET_DONE; 
    }
}
static PWhen *when(PTask **task) {
    struct myTask *t = (struct myTask *)task;
    static PWhen whenArr[2] = { {0, PWHEN_IN}, {0} };
    return whenArr;
}
static void de(PTask **task) {
    struct myTask *t = (struct myTask *)task;
    NOP;
}
static struct PTask face = {cb, when, de};
static struct myTask task = {&face, 0, 1};

void cHand(int ign) {
    int ign2 = ign;
    PollerStop(p);
}

int main(void) {
    CHECK(fcntl(0, F_SETFL, O_NONBLOCK));
    CHECK(setSignalHandler(SIGINT, cHand));
    CHECK(PollerInit(p, 10000));
    CHECK(PollerPush(p, &task.t));

    PollerLoop(p);

FAILED:
    PollerDe(p);
    return 0;
}

