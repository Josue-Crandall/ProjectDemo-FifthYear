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

#ifndef JC_NACL_H
#define JC_NACL_H
#include "../macros/macros.h"

#include <string.h>

#include "../api/api.h"
#include "../rng/rng.h"

// WARNING!!: Assuming nacl functions don't have 
// alignment restrictions in calls. (e.g. rprotMake, rpot enc imp)

//#define LIBSODIUM_IMPLEMENTATION
#ifdef LIBSODIUM_IMPLEMENTATION
    #include <sodium.h>     // Libsodium -lsodium

    #include <unistd.h>
    static void naclInit() {
        while(sodium_init() < 0) {
            DEBUG_LOG("Libsodium is failing to initialize\n");
            sleep(1);
        };
    }

#else
    #include <crypto_box.h> // NaCl -lnacl
    #include <crypto_secretbox.h>
    #include <crypto_hash.h>

    static void naclInit() {}
    extern void randombytes(void *buf, ull bytes);

#endif

#define NACL_MAX_VALIDATED_LEN 4096

#define NACLA_BYTES        (crypto_box_NONCEBYTES + crypto_box_ZEROBYTES - crypto_box_BOXZEROBYTES)
#define NACLS_BYTES        (crypto_secretbox_NONCEBYTES + crypto_secretbox_ZEROBYTES - crypto_secretbox_BOXZEROBYTES)
#define NACLA_PUB_BYTES    (crypto_box_PUBLICKEYBYTES)
#define NACLA_SEC_BYTES    (crypto_box_SECRETKEYBYTES)
#define NACLS_KEY_BYTES    (crypto_secretbox_KEYBYTES)
#define NACLH_BYTES        (crypto_hash_BYTES)

static void naclKeyPair(u8 pk[NACLA_PUB_BYTES], u8 sk[NACLA_SEC_BYTES]);
// extern void randombytes(void *buf, ull bytes);
#define NACL_DEC_FNS(TYPE)                                                                                      \
static Ret TYPE##AEncrypt(TYPE *R() plainText, u8 pk[NACLA_PUB_BYTES], u8 sk[NACLA_SEC_BYTES], TYPE *R() ws);   \
static Ret TYPE##ADecrypt(TYPE *R() ciherText, u8 pk[NACLA_PUB_BYTES], u8 sk[NACLA_SEC_BYTES], TYPE *R() ws);   \
static Ret TYPE##SEncrypt(TYPE *R() plainText, u8 key[NACLS_KEY_BYTES], TYPE *R() ws);                          \
static Ret TYPE##SDecrypt(TYPE *R() ciherText, u8 key[NACLS_KEY_BYTES], TYPE *R() ws);
static void naclHash(u8 dst[NACLH_BYTES], void *const R() src, usize srcLen);
static void naclRatchet(u8 key[NACLS_KEY_BYTES], u8 sws[NACLH_BYTES]);
static void naclMix(u8 key[NACLS_KEY_BYTES], u8 secret[NACLS_KEY_BYTES], u8 sws[NACLH_BYTES]);
// WARNING: Not validated.
Ret naclEq(u8 *lhs, u8 *rhs, usize len);

#include "imp.h"

NACL_DD_FNS(Buff);
NACL_DD_FNS(Smem);

#endif
