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

static Ret lisCB(PTask **task) {
    void *ign = task;

    while(1) {
        Ret res;
        TCPSock newClient;

        NEG_CHECK(res = TCPLisAccept(lis, &newClient, 0));
        if(res > 0) { return PTASK_RET_REPOLL; }
        else { clientDonate(&newClient); }
    }
FAILED:
    return PTASK_RET_ERROR;
}
static PWhen lisTrigs[2] = {{0, PWHEN_IN},{0,0}};
static PWhen *lisTrig(PTask **task) { void *ign = task; return lisTrigs; }
static PTask lisFace = { lisCB, lisTrig, NULL };
static QTask lisTask = {&lisFace, 0, 0};

static void lisInit() {
    if(TCPLisInit(lis, serverIP, serverPort)) { exitFailure("Failed to setup listening socket at port", serverPort, ""); }
    else {
        int tcpLisFD = TCPLisGetFD(lis);
        lisTrigs[0].fd = tcpLisFD;
        printf("Server bound to ip: %s, port: %d\n",
            serverIP ? serverIP : "INADDR_ANY",
            serverPort ? atoi(serverPort) : getSockBindPort(tcpLisFD));
    }
}