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
#include "../../lib/conf/conf.h"
#include "../../lib/net/net.h"
#include "../../lib/fwatch/fwatch.h"
#include "../../lib/file/file.h"
#include "../../lib/table/table.h"
#include "../../lib/string/string.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

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
#include "tracker.h"
#include "task.h"

static void ctrlcHandler(int signo) {
    int ign = signo;
    exit(0);
}

int main(void) {
    if(setSignalHandler(SIGINT, ctrlcHandler)) { exitFailure("Failed to set", "ctrl-c handler", ""); }
    if(setSignalHandler(SIGTERM, ctrlcHandler)) { exitFailure("Failed to set", "sig term handler", ""); }

    dataInit();
    confInit();
    
    printf("Launching proxy ...\n");
    char *argsToProxy[] = {PROXY_EXEC_PATH, NULL};
    if(forkedExec(PROXY_EXEC_PATH, argsToProxy, NULL) < 0) { exitFailure("Failed to", "launch proxy", ""); }
    
    printf(" Dork sync (sleep 1) waiting for proxy\n");
    sleep(1);

    printf("Connecting to proxy ...\n");
    if(TCPSockInit(toProxy, proxyIP, proxyPort)) { exitFailure("Failed to connect", "to proxy", ""); }
    printf(" %sdone%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_DEFAULT);

    taskInit();

    PollerLoop(mpoll);

    exit(0);
}
