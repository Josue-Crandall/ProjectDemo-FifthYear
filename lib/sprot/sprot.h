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

#ifndef JC_SPROT_H
#define JC_SPROT_H
#include "../macros/macros.h"

#include "../nacl/nacl.h"
#include "../api/api.h"
#include "../net/net.h" // Network chunk bytes

#define SPROT_BYTES (1 + NACLS_BYTES)

typedef Smem Sprot;
static Ret SprotInit(Sprot *sprot);
static u8 *SprotData(Sprot *sprot);
static usize SprotSize(Sprot *sprot);
//  Returns: 
#define SPROT_HS_FAIL     (-1)
#define SPROT_HS_DONE     (0)
#define SPROT_HS_SEND     (1)
#define SPROT_HS_RECV     (2)
//  SPROT_HS_DONE for done,
//  SPROT_HS_FAIL if failed,
//  SPROT_HS_SEND on requires send'ing SprotSize from SprotData,
//  SPROT_HS_RECV on requires recv'ing SprotSize into SprotData,
static Ret SprotClientHandshake(Sprot *sprot, u8 pk[NACLA_PUB_BYTES], u8 sk[NACLA_SEC_BYTES], Smem *ws);
static Ret SprotServerHandshake(Sprot *sprot, u8 pk[NACLA_PUB_BYTES], u8 sk[NACLA_SEC_BYTES], Smem *ws);
static void SPEncryptFinish(Sprot *sprot, u8 sws[NACLH_BYTES]);
#define DEC_SPROT_FNS(TYPE)                                                                         \
static Ret TYPE##SPEncrypt(TYPE *R() text, Sprot *sprot, TYPE *R() ws);                             \
static Ret TYPE##SPEncryptUnmut(TYPE *R() text, Sprot *sprot, TYPE *R() ws);                        \
static Ret TYPE##SPDecrypt(TYPE *R() text, Sprot *sprot, TYPE *R() ws, u8 sws[NACLH_BYTES]);
static void SprotDe(Sprot *sprot);

#include "imp.h"

DD_SPROT_FNS(Buff);
DD_SPROT_FNS(Smem);

#endif