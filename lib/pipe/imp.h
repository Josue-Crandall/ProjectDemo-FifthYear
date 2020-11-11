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

#include "../thread/thread.h"
#include <fcntl.h>

static_assert(ISIZE_MAX >= PIPE_BUF, "Handling len with isize/usize");

static Ret initSinglePipe(Pipe *pip) {
    pip->fds[0] = pip->fds[1] = -1;
    CHECK(pipe2(pip->fds, O_DIRECT | O_NONBLOCK));
    pip->alive = 1;
    return 0;
FAILED:
    FD_CLOSE(pip->fds[0]);
    FD_CLOSE(pip->fds[1]);
    pip->alive = 0;
    return -1;
}
static Ret initDoublePipe(Pipe *ppip, Pipe *ppipOther) {
    Pipe *R() pip = ppip, *R() pipOther = ppipOther;
    pipOther->fds[0] = pipOther->fds[1] = pip->fds[0] = pip->fds[1] = -1;
    CHECK(pipe2(pip->fds, O_DIRECT | O_NONBLOCK));
    CHECK(pipe2(pipOther->fds, O_DIRECT | O_NONBLOCK));
    SWAP(pip->fds[0], pipOther->fds[0], int);
    pip->alive = 1;
    pipOther->alive = 1;
    return 0;
FAILED:
    FD_CLOSE(pip->fds[0]);
    FD_CLOSE(pip->fds[1]);
    FD_CLOSE(pipOther->fds[0]);
    FD_CLOSE(pipOther->fds[1]);
    pip->alive = 0;
    pipOther->alive = 0;
    return -1;
}
static Ret handleSigPipe() {
    static Ret SET_SIG_PIPE_HANDLER = 1;
    if(SET_SIG_PIPE_HANDLER) {
        CHECK(setSignalHandler(SIGPIPE, SIG_IGN));
        SET_SIG_PIPE_HANDLER = 0;
    }

    return 0;
FAILED:
    return -1;
}
static Ret PipeInit(Pipe *pip, Pipe *pipOther) {
    CHECK(handleSigPipe());
    if(pipOther) { return initDoublePipe(pip, pipOther); }
    else { return initSinglePipe(pip); }
FAILED:
    return -1;
}
static Ret PipeWrite( Pipe *pip,  void *data, usize len) {
    ssize_t amt;

WRITE:
    amt = write(pip->fds[1], data, len);
    if(amt < 0) {
        if(errno == EINTR) { goto WRITE; }
        else if(errno == EWOULDBLOCK || errno == EAGAIN) { return 1; }
        else {
            DEBUG_LOG("PipeWrite has failed\n");
            return -1;
        }
    }

    return 0;
}
static Ret PipeRead(Pipe *pip, void *data, usize *len) {
    ssize_t amt;

READ:
    amt = read(pip->fds[0], data, PIPE_BUF);
    if(amt < 1) {
        if(0 == amt) { goto FAILED; }
        if(errno == EINTR) { goto READ; }
        else if(errno == EWOULDBLOCK || errno == EAGAIN) {
            *len = 0;
            return 1;
        }
        else {
            DEBUG_LOG("PipeRead has failed\n");
            goto FAILED;
        }
    }

    // asserted to fit
    *len = amt;
    return 0;

FAILED:
    *len = 0;
    return -1;
}
