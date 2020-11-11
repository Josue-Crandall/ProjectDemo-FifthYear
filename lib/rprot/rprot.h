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

#ifndef JC_RPROT_H
#define JC_RPROT_H
#include "../macros/macros.h"

#include "../nacl/nacl.h"
#include "../api/api.h"

#define RPROT_BYTES     ((NACLA_PUB_BYTES) + (NACLS_KEY_BYTES + NACLA_BYTES) + 1 + NACLS_BYTES)

typedef Smem Rprot;

static Ret RprotMake(char *R() filePath1, char *R() filePath2);
static Ret RprotInit(Rprot *rprot, char *path);
// Moves keys forwards after encryption
static void RPEncryptFinish(Rprot *rprot, u8 sws[NACLH_BYTES]);
#define DEC_RPROT_FNS(TYPE)                                                             \
static Ret TYPE##RPEncrypt(TYPE *R() text, Rprot *rprot, TYPE *R() ws, Smem *R() sws);  \
static Ret TYPE##RPEncryptUnmut(TYPE *R() text, Rprot *rprot, TYPE *R() ws);            \
static Ret TYPE##RPDecrypt(TYPE *R() text, Rprot *rprot, TYPE *R() ws, Smem *R() sws);
// WARNING: Can fail
static Ret RprotDe(Rprot *rprot, char *path);

#include "imp.h"

DD_RPROT_FNS(Buff);
DD_RPROT_FNS(Smem);

#endif