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

#include "../../lib/macros/macros.h"
#include "../../lib/macros/term.h"
#include "../../lib/thread/thread.h"
#include "../../lib/poller/poller.h"
#include "../../lib/conf/conf.h"
#include "../../lib/net/net.h"
#include "../../lib/sprot/sprot.h"
#include "../../lib/file/file.h"
#include "../../lib/tunnel/tunnel.h"
#include "../../lib/poller/poller.h"
#include "../../lib/rprot/rprot.h"
#include "../../lib/oprot/oprot.h"

#include <stdlib.h>
#include <endian.h>
#include <stdio.h>

static void exitFailure(const char *R() s, const char *R() h, const char *R() e) {
    printf("%sEXIT FAILURE:%s %s%s%s %s%s%s %s%s%s\n",
        ANSI_COLOR_RED, ANSI_COLOR_DEFAULT,
        ANSI_COLOR_DEFAULT, s, ANSI_COLOR_DEFAULT,
        ANSI_COLOR_BOLD_CYAN, h, ANSI_COLOR_DEFAULT,
        ANSI_COLOR_DEFAULT, e, ANSI_COLOR_DEFAULT
    );
    exit(0);
}

#include "data.h"
#include "conf.h"
#include "server.h"
#include "client.h"
#include "task.h"

static void ctrlcHandler(int signo) {
    int ign = signo;
    // Ignore SIGINT, client will terminate socket ending loop
}

int main(void) {
    if(setSignalHandler(SIGINT, ctrlcHandler)) { exitFailure("Failed to set", "ctrl-c handler", ""); }

    dataInit();
    confInit();
    printf("Proxy connecting to client ...\n"); clientInit(); printf(" %sdone%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_DEFAULT);
    printf("Proxy connecting to server ...\n"); serverInit(); printf(" %sdone%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_DEFAULT);
    taskInit();

    PollerLoop(mpoll);

    exit(0);
}