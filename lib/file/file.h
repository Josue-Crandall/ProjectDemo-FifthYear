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

#ifndef JC_FILE_H
#define JC_FILE_H
#include "../macros/macros.h"

#include "../alloc/alloc.h"
#include "../base64/base64.h"
#include "../api/api.h"

#include <sys/stat.h> // mode_t
#include <sys/types.h> // open
#include <unistd.h> // close
#include <errno.h> // errno
#include <fcntl.h> // O_RDWR, O_CREAT
#include <stdio.h> // remove
#include <sys/file.h>

#define FILEH_PERMISSIONS 0600

// Note: reading file functions spin on EAGAIN || EWOULDBLOCK.

// Note: close file descriptors, safe to call on negative values
// static void FD_CLOSE(int fd);

// returns < 0 on failure
// on success returns files size
static isize getFileSize(char *path);
// returns < 0 on failure
// returns fd on success
static int fileOpen(char *path);
// nonzero on failure
static Ret fileRead(int fd, u8 *dst, isize size);
// nonzero on failure
static Ret fileWrite(int fd,  u8 *src, isize size);
// returns non zero on failure
static Ret fileLoad(char *path, u8 *dst, isize size);
// nonzero on failure
static Ret fileTrunc(int fd, isize size);
// nonzero on failure
static Ret filePTrunc(char *path, isize size);
// WARNING: truncates file @ path before writing to the path
// returns nonzero on failure
static Ret fileSave(char *path, u8 *src, isize size);
// returns non zero on failure
static Ret fileSeek(int fd, isize pos);
// returns current location on success
// returns < 0 on failure
static isize fileSeekEnd(int fd);
// nonzero on failure
static Ret fileRm(char *path);
// nonzero on failure
static Ret fileMv(char *from, char *to);
// returns nonzero on faillure
static Ret fileAppend(char *path,  u8 *src, isize size);
// WARNING: saveB64File truncates file @ path before writing to the path
#define DEC_B64FILE_FNS(TYPE)                                               \
static Ret TYPE##LoadFile(TYPE *R() buff, char *path);                      \
static Ret TYPE##LoadB64File(TYPE *R() buff, char *path, TYPE *R() ws);     \
static Ret TYPE##SaveB64File(TYPE *R() buff, char *path, TYPE *R() ws);
// Instantiated for Buff and Smem

// Note: Locks created by flock() are preserved across an execve(2).
static Ret fileLock(char *path, Ret nonblocking);
static Ret fileUnlock(char *path);

#include "imp.h"

DD_B64FILE_FNS(Buff);
DD_B64FILE_FNS(Smem);

#endif