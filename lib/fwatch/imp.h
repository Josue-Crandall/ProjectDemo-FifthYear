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

static Ret FWatchInit(FWatch *fwatch, char *filePath) {
    fwatch->fd = fwatch->wfd = -1;
    
    NEG_CHECK(fwatch->fd = fileOpen(filePath));
    FD_CLOSE(fwatch->fd);
    fwatch->fd = -1;

    NEG_CHECK(fwatch->fd = inotify_init1(IN_CLOEXEC | IN_NONBLOCK));
    NEG_CHECK(fwatch->wfd = inotify_add_watch(fwatch->fd, filePath, IN_MODIFY));
    fwatch->alive = 1;

    fwatch->trigs[0].fd = fwatch->fd; fwatch->trigs[0].when = PWHEN_IN;
    fwatch->trigs[1].when = 0;

    return 0;
FAILED:
    FD_CLOSE(fwatch->fd);
    FD_CLOSE(fwatch->wfd);
    fwatch->alive = 0;
    return -1;
}
static void FWatchDe(FWatch *fwatch) {
    if(fwatch->alive) { FD_CLOSE(fwatch->fd); }
}
static Ret FWatchEmpty(FWatch *fwatch) {
    struct inotify_event ign;
    u8 ignName[NAME_MAX]; static_assert(NAME_MAX < ISIZE_MAX, "This should not happen");
    isize bytesRead;

    Ret res = 0;

    while(1) {
        bytesRead = read(fwatch->fd, &ign, sizeof(ign));
        if(bytesRead < 0) {
            if(errno == EINTR) { continue; }
            else if (errno == EAGAIN || errno == EWOULDBLOCK) { goto CLEAN; }
            else { DEBUG_LOG("FWatchEmpty failed\n"); goto FAILED; }
        }
        CHECK(bytesRead != sizeof(struct inotify_event));
        
        if(ign.len) {
        NAME_READ:
            bytesRead = read(fwatch->fd, ignName, ign.len);
            if(bytesRead < 0) {
                if(errno == EINTR) { goto NAME_READ; }
                else { DEBUG_LOG("FWatchEmpty failed\n"); goto FAILED; }
            }
            CHECK(bytesRead != ign.len);
        }

        res = 0;
    }

CLEAN:
    return res;
FAILED:
    return -1;
}
static PWhen *FWatchPWhen(FWatch *fwatch) { return fwatch->trigs; }
