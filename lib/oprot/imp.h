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

#include "../file/file.h"
#include "../string/string.h"
#define OPROT_KEY_CHUNK_SIZE 4096

static const u8 OPROT_ZEROS[MAX(OPROT_KEY_CHUNK_SIZE,sizeof(u64))];

#define OPROT_OPEN_TYPE(NAME,NUM)                                   \
    CHECK(StrPush(path##NUM, #NAME));                               \
    NEG_CHECK(NAME##File##NUM = fileOpen(StrIng(path##NUM))); \
    CHECK(fileTrunc(NAME##File##NUM, 0));                           \
    StrPop(path##NUM, strlen(#NAME));
#define OPROT_OPEN_FILES()                                          \
    OPROT_OPEN_TYPE(key, 1); OPROT_OPEN_TYPE(out, 1);               \
    OPROT_OPEN_TYPE(in, 1); OPROT_OPEN_TYPE(key, 2);                \
    OPROT_OPEN_TYPE(out, 2); OPROT_OPEN_TYPE(in, 2);
#define OPROT_WRITE_KEY(WRITE_BLOCK)                                                              \
    for(usize writeLen = len; writeLen;                                                     \
        writeLen = writeLen > OPROT_KEY_CHUNK_SIZE ? writeLen - OPROT_KEY_CHUNK_SIZE : 0)   \
    {                                                                                       \
        u8 *R() data1 = SmemData(buff1);                                                    \
        u8 *R() data2 = SmemData(buff2);                                                    \
        u8 *R() src = data2 + OPROT_KEY_CHUNK_SIZE;                                         \
        u8 *R() srcEnd = src;                                                               \
        u8 *R() iter = data1;                                                               \
        u8 *R() iterEnd = iter + OPROT_KEY_CHUNK_SIZE;                                      \
        for(NOP; iter != iterEnd; ++iter) {                                                 \
            u8 mask = 1;                                                                    \
            while(mask) {                                                                   \
                if(src == srcEnd) { CHECK(fileRead(rngFile, data2, OPROT_KEY_CHUNK_SIZE)); src = data2; }   \
                switch(*src & 3) {                                                          \
                    case 1: *iter |= mask;                                                  \
                    case 2: mask <<= 1;                                                     \
                }                                                                           \
                if(! (*src >>= 2)) { ++src; }                                               \
            }                                                                               \
        }                                                                                   \
        randombytes(data2, OPROT_KEY_CHUNK_SIZE);                                           \
        for(usize i = 0; i < OPROT_KEY_CHUNK_SIZE; ++i) { data1[i] ^= data2[i]; }           \
        { WRITE_BLOCK; }                                                                    \
    }

Ret OprotMake(char *R() filePath1, char *R() filePath2, usize len, char *R() rngPath) {
    PRAI(Smem, buff1); PRAI(Smem, buff2);
    PRAI(Str, path1); PRAI(Str, path2);
    int rngFile, keyFile1, keyFile2, outFile1, outFile2, inFile1, inFile2;
    rngFile = keyFile1 = keyFile2 = outFile1 = outFile2 = inFile1 = inFile2 = -1;

    rngPath = rngPath ? rngPath : "/dev/urandom";
    CHECK(SmemInit(buff1, MAX(NACLS_KEY_BYTES * 2, OPROT_KEY_CHUNK_SIZE)));
    CHECK(SmemInit(buff2, OPROT_KEY_CHUNK_SIZE));
    CHECK(StrInit(path1, filePath1)); CHECK(StrInit(path2, filePath2));
    NEG_CHECK(rngFile = fileOpen(rngPath));
    OPROT_OPEN_FILES();

    randombytes(SmemData(buff1), NACLS_KEY_BYTES * 2);
    CHECK(fileWrite(keyFile1, SmemData(buff1), NACLS_KEY_BYTES * 2));
    CHECK(fileWrite(keyFile2, SmemData(buff1) + NACLS_KEY_BYTES, NACLS_KEY_BYTES));
    CHECK(fileWrite(keyFile2, SmemData(buff1), NACLS_KEY_BYTES));

    OPROT_WRITE_KEY(
        CHECK(fileWrite(outFile1, data1, OPROT_KEY_CHUNK_SIZE));
        CHECK(fileWrite(inFile2, data1, OPROT_KEY_CHUNK_SIZE)));
    OPROT_WRITE_KEY(
        CHECK(fileWrite(inFile1, data1, OPROT_KEY_CHUNK_SIZE));
        CHECK(fileWrite(outFile2, data1, OPROT_KEY_CHUNK_SIZE)));

    Ret ret = 0;
CLEAN:
    SmemDe(buff1);SmemDe(buff2);
    StrDe(path1); StrDe(path2);
    FD_CLOSE(rngFile);FD_CLOSE(keyFile1);FD_CLOSE(keyFile2);FD_CLOSE(outFile1);
    FD_CLOSE(outFile2);FD_CLOSE(inFile1);FD_CLOSE(inFile2);
    return ret;
FAILED:
    ret = -1;
    goto CLEAN;
}

typedef struct OprotState {
    u8 outKey[NACLS_KEY_BYTES], inKey[NACLS_KEY_BYTES];
    isize inPlace, outPlace;
    u64 mac;
    int keyFD, outFD, inFD;
} OprotState;

static Ret OprotInit(Oprot *prot,  char *ppath) {
    PRAI(Str, path);
    int keyFD = -1, outFD = -1, inFD = -1;
    memset(prot, 0, sizeof(Oprot));

    CHECK(SmemInit(prot, sizeof(OprotState)));
    CHECK(StrInit(path, ppath));
    CHECK(StrPush(path, "key")); NEG_CHECK(keyFD = fileOpen(StrIng(path))); StrPop(path, 3);
    CHECK(StrPush(path, "in")); NEG_CHECK(inFD = fileOpen(StrIng(path))); StrPop(path, 2);
    CHECK(StrPush(path, "out")); NEG_CHECK(outFD = fileOpen(StrIng(path)));

    OprotState *state = (OprotState *)SmemData(prot);
    CHECK(fileRead(outFD, state->outKey, NACLS_KEY_BYTES));
    CHECK(fileRead(inFD, state->inKey, NACLS_KEY_BYTES));
    CHECK(fileSeek(keyFD, 0));
    NEG_CHECK(state->inPlace = fileSeekEnd(inFD));
    NEG_CHECK(state->outPlace = fileSeekEnd(outFD));
    state->outFD = outFD; state->inFD = inFD; state->keyFD = keyFD;

    Ret ret = 0;
CLEAN:
    StrDe(path);
    return ret;
FAILED:
    FD_CLOSE(keyFD); FD_CLOSE(outFD); FD_CLOSE(inFD);
    SmemDe(prot);

    memset(prot, 0, sizeof(Oprot));
    ret = -1;
    goto CLEAN;
}
static Ret OprotDe(Oprot * R() prot) {
    if(SmemAlive(prot)) {
        OprotState *state = (OprotState *)SmemData(prot);
        CHECK(fileWrite(state->keyFD, state->outKey, NACLS_KEY_BYTES));
        CHECK(fileWrite(state->keyFD, state->inKey, NACLS_KEY_BYTES));

        Ret ret = 0;
    CLEAN:
        FD_CLOSE(state->keyFD);
        FD_CLOSE(state->outFD);
        FD_CLOSE(state->inFD);
        SmemDe(prot);
        return ret;
    FAILED:
        ret = -1;
        goto CLEAN;
    }

    return 0;
}


#define OPROT_PAD_TEXT_FN(TYPE)                                                 \
static Ret padText##TYPE(TYPE *R() text, Oprot *oprot, TYPE *R() ws) {          \
    OprotState *state = (OprotState *)SmemData(oprot);                          \
                                                                                \
    isize place = state->outPlace;                                              \
    usize len = TYPE##Size(text);                                               \
    CHECK(len > ISIZE_MAX);                                                     \
    CHECK((usize)place < len + sizeof(u64));                                    \
    place -= len + sizeof(u64);                                                 \
    CHECK(TYPE##Reserve(ws, len + sizeof(u64)));                                \
    u8 *R() wsData = TYPE##Data(ws);                                            \
                                                                                \
    CHECK(fileSeek(state->outFD, place));                                       \
    CHECK(fileRead(state->outFD, wsData, len + sizeof(u64)));                   \
    CHECK(fileSeek(state->outFD, place));                                       \
    usize zeroLen = len;                                                        \
    while(zeroLen > OPROT_KEY_CHUNK_SIZE) {                                     \
        CHECK(fileWrite(state->outFD, (u8 *R())OPROT_ZEROS, OPROT_KEY_CHUNK_SIZE));   \
        zeroLen -= OPROT_KEY_CHUNK_SIZE;                                        \
    }                                                                           \
    CHECK(fileWrite(state->outFD, (u8 *R())OPROT_ZEROS, zeroLen));                    \
    CHECK(fileSeek(state->outFD, place));                                       \
    CHECK(fileTrunc(state->outFD, place));                                      \
    state->outPlace = place;                                                    \
                                                                                \
    CHECK(TYPE##PushArr(text, (u8 *R())OPROT_ZEROS, sizeof(u64)));              \
    u8 *R() pt = TYPE##Data(text);                                              \
    u8 *R() macPtr = pt + len;                                                  \
                                                                                \
    state->mac = 0;                                                             \
    for(usize i = 0; i < len; ++i) {                                            \
        state->mac = (state->mac << 1) | (state->mac >> (sizeof(u64) * 8 - 1)); \
        state->mac += pt[i];                                                    \
        pt[i] ^= wsData[i];                                                     \
    }                                                                           \
                                                                                \
    memcpy(macPtr, &state->mac, sizeof(u64));                                   \
    for(usize i = 0; i < sizeof(u64); ++i) { pt[len + i] ^= wsData[len + i]; }  \
                                                                                \
    explicit_bzero(wsData, len + sizeof(u64));                                  \
    explicit_bzero(&state->mac, sizeof(u64));                                   \
                                                                                \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}

#define OPROT_UNPAD_TEXT_FN(TYPE)                                               \
static Ret unpadText##TYPE(TYPE *R() text, Oprot *oprot, TYPE *R() ws) {        \
    OprotState *state = (OprotState *)SmemData(oprot);                          \
                                                                                \
    isize place = state->inPlace;                                               \
    usize len = TYPE##Size(text) - sizeof(u64);                                 \
    CHECK(len > ISIZE_MAX);                                                     \
    CHECK((usize)place < len + sizeof(u64));                                    \
    place -= len + sizeof(u64);                                                 \
    CHECK(TYPE##Reserve(ws, len + sizeof(u64)));                                \
    u8 *R() wsData = TYPE##Data(ws);                                            \
                                                                                \
    CHECK(fileSeek(state->inFD, place));                                        \
    CHECK(fileRead(state->inFD, wsData, len + sizeof(u64)));                    \
    CHECK(fileSeek(state->inFD, place));                                        \
    usize zeroLen = len;                                                        \
    while(zeroLen > OPROT_KEY_CHUNK_SIZE) {                                     \
        CHECK(fileWrite(state->inFD, (u8 *R())OPROT_ZEROS, OPROT_KEY_CHUNK_SIZE));    \
        zeroLen -= OPROT_KEY_CHUNK_SIZE;                                        \
    }                                                                           \
    CHECK(fileWrite(state->inFD, (u8 *R())OPROT_ZEROS, zeroLen));                     \
    CHECK(fileSeek(state->inFD, place));                                        \
    CHECK(fileTrunc(state->inFD, place));                                       \
    state->inPlace = place;                                                     \
                                                                                \
    u8 *R() pt = TYPE##Data(text);                                              \
    u8 *R() macPtr = pt + len;                                                  \
                                                                                \
    state->mac = 0;                                                             \
    for(usize i = 0; i < len; ++i) {                                            \
        pt[i] ^= wsData[i];                                                     \
        state->mac = (state->mac << 1) | (state->mac >> (sizeof(u64) * 8 - 1)); \
        state->mac += pt[i];                                                    \
    }                                                                           \
    for(usize i = 0; i < sizeof(u64); ++i) { pt[len + i] ^= wsData[len + i]; }  \
                                                                                \
    Ret otpAuthed = naclEq((u8 *R())&state->mac, macPtr, sizeof(u64));          \
    explicit_bzero(wsData, len + sizeof(u64));                                  \
    explicit_bzero(&state->mac, sizeof(u64));                                   \
                                                                                \
    CHECK(!otpAuthed)                                                           \
    TYPE##SetSize(text, len);                                                   \
                                                                                \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}

#define DEF_OPROT_FNS(TYPE)                                                                    \
OPROT_PAD_TEXT_FN(TYPE);                                                                       \
OPROT_UNPAD_TEXT_FN(TYPE);                                                                     \
static Ret TYPE##OPEncrypt(TYPE *R() text, Oprot *oprot, TYPE *R() ws, u8 sws[NACLH_BYTES]) {  \
    OprotState *state = (OprotState *) SmemData(oprot);                                        \
    usize ptLen = TYPE##Size(text);                                                            \
    CHECK(padText##TYPE(text, oprot, ws));                                                     \
    TYPE##Push(text, 0);                                                                       \
    CHECK(TYPE##SEncrypt(text, state->outKey, ws));                                            \
    naclRatchet(state->outKey, sws);                                                           \
    return 0;                                                                                  \
FAILED:                                                                                        \
    return -1;                                                                                 \
}                                                                                              \
static Ret TYPE##OPEncryptUnmut(TYPE *R() text,Oprot *oprot, TYPE *R() ws) {                   \
    OprotState *state = (OprotState *)SmemData(oprot);                                         \
    TYPE##Push(text, 1);                                                                       \
    CHECK(TYPE##SEncrypt(text, state->outKey, ws));                                            \
    return 0;                                                                                  \
FAILED:                                                                                        \
    return -1;                                                                                 \
}                                                                                              \
static Ret TYPE##OPDecrypt(TYPE *R() text, Oprot *oprot, TYPE *R() ws, u8 sws[NACLH_BYTES]) {  \
    OprotState *state = (OprotState *)SmemData(oprot);                                         \
    CHECK(TYPE##Size(text) < OPROT_BYTES);                                                     \
    CHECK(TYPE##SDecrypt(text, state->inKey, ws));                                             \
    if(0 == TYPE##Pop(text)) {                                                                 \
        naclRatchet(state->inKey, sws);                                                        \
        CHECK(unpadText##TYPE(text, oprot, ws));                                               \
    }                                                                                          \
    return 0;                                                                                  \
FAILED:                                                                                        \
    return -1;                                                                                 \
}                                                                                              \

#define DD_OPROT_FNS(TYPE)  \
DEC_OPROT_FNS(TYPE);        \
DEF_OPROT_FNS(TYPE);        \
