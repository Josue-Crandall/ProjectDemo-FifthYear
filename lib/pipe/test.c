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

#include "pipe.h"

#include <string.h>

int main(void) {
    printf("Starting test ... \n");

    Ret test0(); CHECK(test0());

    printf("Passed test!\n");
    return 0;
FAILED:
    printf("Failed test!\n");
    return 0;
}

Ret test0() {
    Ret err= 0;
    Pipe pip = {0}, pip2 = {0}, pip3 = {0};

    CHECK(PipeInit(&pip, 0));
    CHECK(PipeInit(&pip2, &pip3));

    u8 msg[] =  {1,2,3,4,5};
    u8 dup[] =  {1,2,3,4,5};
    u8 echo[] = {0,0,0,0,0};
    usize echoLen;

    CHECK(PipeWrite(&pip, msg, 5));
    CHECK(PipeRead(&pip, echo, &echoLen));
    CHECK(echoLen != 5);
    CHECK(memcmp(echo, msg, 5));
    explicit_bzero(echo, 5);

    CHECK(PipeWrite(&pip2, msg, 5));
    CHECK(PipeRead(&pip3, echo, &echoLen));
    CHECK(echoLen != 5);
    CHECK(memcmp(echo, msg, 5));
    explicit_bzero(echo, 5);

    CHECK(PipeWrite(&pip3, msg, 5));
    CHECK(PipeRead(&pip2, echo, &echoLen));
    CHECK(echoLen != 5);
    CHECK(memcmp(echo, msg, 5));
    explicit_bzero(echo, 5);

    u8 unchanged[] = {1,2,3,4,5};
    CHECK(memcmp(unchanged, msg, 5));

CLEAN:
    PipeDe(&pip);
    PipeDe(&pip2);
    PipeDe(&pip3);
    return 0;
FAILED:
    err = -1;
    goto CLEAN;
}

