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

#include "conf.h"
#include <stdio.h>
#include "../macros/macros.h"
#include <string.h>
#include "../macros/term.h"

int main(void) {
    printf("--- Starting testing --- \n");

    Ret test0(); CHECK(test0());

    printf("--- Done testing --- \n");
    return 0;
FAILED:
    printf("----- %sFAILED TESTING%s ---\n", ANSI_COLOR_BOLD_RED, ANSI_COLOR_DEFAULT);
    return 0;
}

Ret test0() {
    Conf c;    
    CHECK(ConfInit(&c, "conf_test_cases.txt"));

    for(ConfTableGen gen = ConfTableBeg(&c.table); ConfTableNext(&gen); NOP) {
        printf("%s -> %s\n", gen.keys[gen.i], gen.vals[gen.i]);
    }

   Ret err = 0;
CLEAN:
    ConfDe(&c);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}