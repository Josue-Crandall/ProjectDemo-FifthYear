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

static isize getFileSize(char *path) {
    static_assert(sizeof(off_t) == sizeof(isize), "stat.st_size is off_t, returning isize");
    struct stat stats;
    CHECK(stat(path, &stats));
    return stats.st_size;
FAILED:
    return -1;
}
static int fileOpen(char *path) {
    int fd;
    EINTR_CHECK_IGN(fd = open(path, O_RDWR | O_CREAT, FILEH_PERMISSIONS));
    return fd;
}
static Ret fileRead(int fd, u8 *dst, isize size) {
    NEG_CHECK(size);

    while(size) {
        isize amt;
    READ:
        if( (amt = read(fd, dst, size)) < 1) {
            if(0 == amt) { goto FAILED; }
            else if(errno == EINTR) { goto READ; }
            else if(errno == EAGAIN || errno == EWOULDBLOCK) { goto READ; }
            else { goto FAILED; }
        }

        dst += amt;
        size -= amt;
    }

    return 0;

FAILED:
    DEBUG_LOG("fileRead has failed\n");
    return -1;
}
static Ret fileWrite(int fd,  u8 *src, isize size) {
    NEG_CHECK(size);

    while(size) {
        isize amt;
    WRITE:
        if( (amt = write(fd, src, size)) < 0) {
            if(errno == EINTR) { goto WRITE; }
            else if(errno == EAGAIN || errno == EWOULDBLOCK) { goto WRITE; }
            else { goto FAILED; }
        }

        src += amt;
        size -= amt;
    }

    return 0;

FAILED:
    DEBUG_LOG("fileWrite has failed\n");
    return -1;
}
static Ret fileLoad(char *path, u8 *dst, isize size) {
    int fd = -1;

    NEG_CHECK(fd = fileOpen(path));
    CHECK(fileRead(fd, dst, size));

    Ret err = 0;
CLEAN:
    FD_CLOSE(fd);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}
static Ret fileTrunc(int fd, isize size) {
    static_assert(sizeof(isize) == sizeof(off_t), "ftruncate takes off_t but we are passing isize");
    NEG_CHECK(size);
    EINTR_CHECK(ftruncate(fd, size));
    return 0;
FAILED:
    return -1;
}
static Ret filePTrunc(char *path, isize size) {
    static_assert(sizeof(isize) == sizeof(off_t), "truncate takes off_t but we are passing isize");
    NEG_CHECK(size);
    EINTR_CHECK(truncate(path, size));
    return 0;
FAILED:
    return -1;
}
static Ret fileSave(char *path, u8 *src, isize size) {
    int fd = -1;

    NEG_CHECK(fd = fileOpen(path));
    CHECK(fileTrunc(fd, 0));
    CHECK(fileWrite(fd, src, size));

    Ret err = 0;
CLEAN:
    FD_CLOSE(fd);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}
static Ret fileSeek(int fd, isize pos) {
    static_assert(sizeof(isize) == sizeof(off_t), "lseek takes off_t but we are passing isize");
    NEG_CHECK(pos);
    NEG_CHECK(lseek(fd, pos, SEEK_SET));
    return 0;
FAILED:
    return -1;
}
static isize fileSeekEnd(int fd) { return lseek(fd, 0, SEEK_END); }
static Ret fileRm(char *path) {
    // Note: Don't display debug warning on failure
    //       will be called on nonexistant files
    return remove(path);
}
static Ret fileMv(char *from, char *to) { return rename(from, to); }
static Ret fileAppend(char *path,  u8 *src, isize size) {
    int fd = -1;

    NEG_CHECK(fd = fileOpen(path));
    NEG_CHECK(fileSeekEnd(fd));
    CHECK(fileWrite(fd, src, size));

    Ret err = 0;
CLEAN:
    FD_CLOSE(fd);
    return err;
FAILED:
    err = -1;
    goto CLEAN;
}
static Ret fileLock(char *path, Ret nonblocking) {
    int fd = -1;

    NEG_CHECK(fd = fileOpen(path));

FLOCK:
    if(flock(fd, LOCK_EX | (nonblocking ? LOCK_NB : 0))) {
        if(errno == EWOULDBLOCK || errno == EAGAIN) { return 1; }
        else if (errno == EINTR) { goto FLOCK; }
        else { DEBUG_LOG("flock has failed\n"); goto FAILED; }
    }

    Ret ret = 0;
CLEAN:
    FD_CLOSE(fd);
    return ret;
FAILED:
    ret = -1;
    goto CLEAN;
}
static Ret fileUnlock(char *path) {
    int fd = -1;

    NEG_CHECK(fd = fileOpen(path));
FLOCK:
    if(flock(fd, LOCK_UN)) {
        if(errno == EWOULDBLOCK || errno == EAGAIN) { return 1; }
        else if (errno == EINTR) { goto FLOCK; }
        else { DEBUG_LOG("un - flock has failed\n"); goto FAILED; }
    }

    Ret ret = 0;
CLEAN:
    FD_CLOSE(fd);
    return ret;
FAILED:
    ret = -1;
    goto CLEAN;
}
#define DEF_B64FILE_FNS(TYPE)                                               \
static Ret TYPE##LoadFile(TYPE *R() buff, char *path) {                     \
    isize fileSize;                                                         \
    NEG_CHECK(fileSize = getFileSize(path));                                \
    CHECK(TYPE##Reserve(buff, fileSize));                                   \
    CHECK(fileLoad(path, TYPE##Data(buff), fileSize));                      \
    TYPE##SetSize(buff, fileSize);                                          \
    return 0;                                                               \
FAILED:                                                                     \
    return -1;                                                              \
}                                                                           \
static Ret TYPE##LoadB64File(TYPE *R() buff, char *path, TYPE *R() ws) {    \
    CHECK(TYPE##LoadFile(ws, path));                                        \
    CHECK(TYPE##B64ToBin(buff, TYPE##Data(ws), TYPE##Size(ws), 0));         \
    return 0;                                                               \
FAILED:                                                                     \
    return -1;                                                              \
}                                                                           \
static Ret TYPE##SaveB64File(TYPE *R() buff, char *path, TYPE *R() ws) {    \
    CHECK(TYPE##BinToB64(ws, TYPE##Data(buff), TYPE##Size(buff), 0))        \
    CHECK(TYPE##Size(ws) > ISIZE_MAX);                                      \
    CHECK(fileSave(path, TYPE##Data(ws), TYPE##Size(ws)));                  \
    return 0;                                                               \
FAILED:                                                                     \
    return -1;                                                              \
}                                                                           \

#define DD_B64FILE_FNS(TYPE)    \
DEC_B64FILE_FNS(TYPE)           \
DEF_B64FILE_FNS(TYPE)           \
