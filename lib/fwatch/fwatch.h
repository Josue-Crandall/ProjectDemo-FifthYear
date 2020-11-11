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

#ifndef JC_FWATCH_H
#define JC_FWATCH_H
#include "../macros/macros.h"

#include "../poller/poller.h"
#include "../file/file.h"

#include <sys/inotify.h>
#include <errno.h>
#include <unistd.h>

typedef struct FWatch {
    int fd, wfd;
    Ret alive;
    PWhen trigs[2];
} FWatch;

// Warning: Will create file if it doesn't exist rather than fail
// returns < 0 on failure
static Ret FWatchInit(FWatch *fwatch, char *filePath);
// Note: clears read buffer, letting fwatch know we have seen its events
static Ret FWatchEmpty(FWatch *fwatch);
static PWhen *FWatchPTrig(FWatch *fwatch);
static void FWatchDe(FWatch *fwatch);

#include "imp.h"

#endif
