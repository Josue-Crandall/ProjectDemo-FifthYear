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

#include "net.h"
#include "../macros/term.h"

#include <unistd.h>

// socks test
Ret t2() {
    Ret err = 0;
    TCPSock s = {0};

    CHECK(TCPSockProxyInit(&s, "3g2upl4pq6kufc4m.onion", "80", "127.0.0.1", "9050"));
    
    //sleep(1);
    //u8 data;
    //Ret res;
    //while(0 == (res = TCPSockRecv(&s, &data, 1))) { printf("%c", (char)data); }
    //printf("%u\n", res);

CLEAN:
    TCPSockDe(&s);
    if(!err) { printf("Passed socksproxy testing\n"); }
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}
// udp test
Ret t0() {
    Ret err = 0;

    UDPLis lis = {0};
    UDPSock s = {0};

    CHECK(UDPLisInit(&lis, "127.0.0.1", "54327"));
    CHECK(UDPSockInit(&s, "127.0.0.1", "54327"));
    
    u8 msg[] = {1,2,3,4,5,6,7};

    for(int i = 0; i < 52; ++i) {
        CHECK(UDPSockSend(&s, msg, sizeof(msg)));
    }

    u8 echo[] = {0,0,0,0,0,0,0};
    UDPAddr addr;

    for(int i = 0; i < 51; ++i) {
        CHECK(sizeof(msg) != UDPLisRecv(&lis, echo, NULL, 1));
    }

    CHECK(sizeof(msg) != UDPLisRecv(&lis, echo, &addr, 1))

    for(int i = 0; i < 23; ++i) {
        CHECK(UDPLisSend(&lis, echo, sizeof(echo), &addr));
    }

    for(int i = 0; i < 23; ++i) {
        CHECK(sizeof(msg) != UDPSockRecv(&s, echo, 1));
        CHECK(memcmp(msg, echo, sizeof(msg)));
    }

CLEAN:
    UDPLisDe(&lis);
    UDPSockDe(&s);
    if(!err) { printf("Passed udp testing\n"); }
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}
// tcp test
Ret t1() {
    Ret err = 0;
    TCPLis lis = {0};
    TCPSock s = {0};
    TCPSock c = {0};

    CHECK(TCPLisInit(&lis, "127.0.0.1", "54327"));
    CHECK(TCPSockInit(&s, "127.0.0.1", "54327"));
    CHECK(TCPLisAccept(&lis, &c, 1));

    u8 msg[] = {1,2,3,4,5,6,7};

    for(int i = 0; i < 3; ++i) {
        CHECK(TCPSockSend(&s, msg, sizeof(msg), NET_BLOCK_FULL));
    }

    u8 echo[sizeof(msg) * 3] = {0};
    CHECK(TCPSockRecv(&c, echo, sizeof(echo), NET_BLOCK_FULL));
    
    for(int i = 0; i > 3; ++i) {
        CHECK(memcmp(echo + i * 7, msg, 7));
    }

    int thing = 0;
    while(1) {
        int res = TCPSockSend(&s, echo, sizeof(echo), NET_BLOCK_NONE);
        CHECK(res == -1);
        if(res == 1) { break; }
        thing += 3;
    }
    while(thing) {
        int res = TCPSockRecv(&c, msg, sizeof(msg), NET_BLOCK_NONE);
        CHECK(res == -1);
        if(res == 1) { sleep(1); }
        thing -= 1;
        CHECK(memcmp(msg, echo, 7));
    }

    // Note: how many of these can be placed before the next send
    // depends on the size of the sending buffer, this was just
    // to test async sends and recovery on my specific system
    //CHECK(TCPSockRecv(&c, msg, sizeof(msg))); 
    CHECK(TCPSockSend(&s, echo, sizeof(echo), NET_BLOCK_NONE));
    CHECK(TCPSockRecv(&c, msg, sizeof(msg), NET_BLOCK_NONE));
    CHECK(TCPSockRecv(&c, msg, sizeof(msg), NET_BLOCK_NONE));
    CHECK(TCPSockRecv(&c, msg, sizeof(msg), NET_BLOCK_NONE)); 

CLEAN:
    TCPLisDe(&lis);
    TCPSockDe(&s);
    TCPSockDe(&c);
    if(!err) { printf("Passed tcp testing\n"); }
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}

// tcp test
Ret t3() {
    Ret err = 0;
    TCPLis lis = {0};
    TCPSock s = {0};
    TCPSock c = {0};
    Buff buff = {0};
    Smem smem = {0};

    CHECK(TCPLisInit(&lis, "127.0.0.1", "54327"));
    CHECK(TCPSockInit(&s, "127.0.0.1", "54327"));
    CHECK(TCPLisAccept(&lis, &c, 1));
    CHECK(BuffInit(&buff, 0));
    CHECK(SmemInit(&smem, 0));

    u8 msg[] = {1,2,3,4,5,6,7};

    for(int i = 0; i < 3; ++i) {
        CHECK(TCPSockSendP(&s, msg, sizeof(msg), NET_BLOCK_FULL));
        CHECK(TCPSockSendP(&s, msg, sizeof(msg), NET_BLOCK_FULL));
    }
    for(int i = 0; i < 3; ++i) {
        CHECK(BuffTCPRecvP(&buff, &c, NET_BLOCK_FULL));
        CHECK(memcmp(msg, BuffData(&buff), BuffSize(&buff)));
        CHECK(SmemTCPRecvP(&smem, &c, NET_BLOCK_FULL));
        CHECK(memcmp(msg, SmemData(&smem), SmemSize(&smem)));   
    }
    
CLEAN:
    TCPLisDe(&lis);
    TCPSockDe(&s);
    TCPSockDe(&c);
    BuffDe(&buff);
    SmemDe(&smem);
    if(!err) { printf("Passed tcp packet testing\n"); }
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}


int main(void) {
    CHECK(t0());
    CHECK(t1());
    CHECK(t2());
    CHECK(t3());
    return 0;
FAILED:
    printf("--- %sFAILED TESTING%s --- \n", ANSI_COLOR_RED, ANSI_COLOR_DEFAULT);
}
