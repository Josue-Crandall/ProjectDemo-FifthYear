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

#ifndef JC_PIPE_H
#define JC_PIPE_H
#include "../macros/macros.h"

#include <unistd.h>
#include <errno.h>

typedef struct {
    int fds[2];
    Ret alive;
} Pipe;

// WARNING: pipe, pipe2 are functions in global namespace.

// Note: if pipOther is NULL then the pipe talks to itself
// Note: if pipOther is not NULL then the pipes talk to one another 
// Note: set not to close on exec
// Note: set to packet rather than stream transmission
static Ret PipeInit(Pipe *pip, Pipe *pipOther);

// len max of PIPE_BUF
// returns 1 on EWOULDBLOCK, -1 on failure, and 0 on success
static Ret PipeWrite(Pipe *pip, void *data, usize len);
// len max of PIPE_BUF
// returns 1 on EWOULDBLOCK, -1 on failure (or pipe closed), and 0 on success
// on sucess: *len is set to packet size (and must be < PIPE_BUF)
static Ret PipeRead(Pipe *pip, void *data, usize *len);

static int PipeGetReadEnd(Pipe *pip) { return pip->fds[0]; }
static int PipeGetWriteEnd(Pipe *pip) { return pip->fds[1]; }
static void PipeCloseReadEnd(Pipe *pip) { FD_CLOSE(pip->fds[0]); pip->fds[0] = -1; }
static void PipeCloseWriteEnd(Pipe *pip) { FD_CLOSE(pip->fds[1]); pip->fds[1] = -1; }
static void PipeDe(Pipe *pip) { if(pip->alive) { FD_CLOSE(pip->fds[0]); FD_CLOSE(pip->fds[1]); } }

#include "imp.h"

#endif
