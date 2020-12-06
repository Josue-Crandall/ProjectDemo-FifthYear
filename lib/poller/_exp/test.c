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

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

SPRAI(Poller, poller);

void init(void) {
    CHECK(fcntl(0, F_SETFL, O_NONBLOCK));
    CHECK(PollerInit(poller));
    return;
FAILED:
    exit(-1);
}
void cleanup(void) {
    PollerDe(poller);
}

void test0() {
    CHECK(PollerReg(poller, 0, 1));
    
    while(1) {
        int num;
        NEG_CHECK(num = PollerPoll(poller, 1000));
        if(!num) { printf("Timeout !\n"); continue; }
        CHECK(num != 1);
        CHECK(PollerWhy(poller, 0) != POLLER_IN);
        CHECK(PollerWhat(poller, 0) != 0);
        printf("Echo: ");
        while(1) {
            int c = getchar();
            if(c == EOF) { break; }
            printf("%c", c);
        }
    }

FAILED:
    exit(-1);
}

int main(void) {
    CHECK(atexit(cleanup)); init();

    test0();

    exit(0);
FAILED:
    exit(-1);
}
