#include "../../lib/macros/macros.h"
#include "../../lib/macros/term.h"
#include "../../lib/conf/conf.h"
#include "../../lib/net/net.h"
#include "../../lib/workQ/workQ.h"
#include "../../lib/thread/thread.h"
#include "../../lib/sprot/sprot.h"
#include "../../lib/api/api.h"
#include "../../lib/file/file.h"
#include "../../lib/tunnel/tunnel.h"
#include "../../lib/table/table.h"
#include "../../lib/pipe/pipe.h"
#include "../../lib/vec/vec.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
#include "channel.h"
#include "table.h"
#include "client.h"
#include "lis.h"

static void ctrlcHandler(int signo) {
    int ign = signo;
    WorkQStop(que);
}

int main(void) {
    dataInit();
    confInit();
    lisInit();
    tableInit();

    if(WorkQPoll(que, &lisTask)) { exitFailure("Failed to poll on", "listener task", ""); }
    
    if(setSignalHandler(SIGINT, ctrlcHandler)) { exitFailure("Failed to set", "ctrl-c", "handler"); }    
    serverSetup = 1;

    WorkQLoop(que);

    WorkQDe(que);
    exit(0);
}
