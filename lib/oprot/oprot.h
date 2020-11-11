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

#ifndef JC_OPROT_H
#define JC_OPROT_H
#include "../macros/macros.h"

#include "../nacl/nacl.h"
#include "../api/api.h"

#define OPROT_BYTES     (sizeof(u64) + 1 + NACLS_BYTES)

typedef Smem Oprot;

// rngFilePath: may be NULL
static Ret OprotMake(char *R() filePath1, char *R() filePath2, usize len, char *R() rngPath);
static Ret OprotInit(Oprot *prot,  char *path);

// Note: Encrypt moves keys forwards immediately.
#define DEC_OPROT_FNS(TYPE)                                                                    \
static Ret TYPE##OPEncrypt(TYPE *R() text, Oprot *oprot, TYPE *R() ws, u8 sws[NACLH_BYTES]);   \
static Ret TYPE##OPEncryptUnmut(TYPE *R() text, Oprot *oprot, TYPE *R() ws);                   \
static Ret TYPE##OPDecrypt(TYPE *R() text, Oprot *oprot, TYPE *R() ws, u8 sws[NACLH_BYTES]);
// WARNING: Can fail
static Ret OprotDe(Oprot * R() prot);

#include "imp.h"

DD_OPROT_FNS(Buff);
DD_OPROT_FNS(Smem);

#endif