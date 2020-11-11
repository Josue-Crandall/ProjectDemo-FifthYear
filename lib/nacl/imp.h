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

#define NACLA_N_BYTES      (crypto_box_NONCEBYTES)
#define NACLS_N_BYTES      (crypto_secretbox_NONCEBYTES)
#define NACLA_Z_BYTES      (crypto_box_ZEROBYTES)
#define NACLA_BZ_BYTES     (crypto_box_BOXZEROBYTES)
#define NACLS_Z_BYTES      (crypto_secretbox_ZEROBYTES)
#define NACLS_BZ_BYTES     (crypto_secretbox_BOXZEROBYTES)
static_assert(sizeof(ull) >= sizeof(usize), "Nacl uses llu on platform, were going to use usize in its place.");
static_assert(NACLA_Z_BYTES >= NACLA_BZ_BYTES, "MAC should exist in this space. Req for NACLA_BYTES");
static_assert(NACLS_Z_BYTES >= NACLS_BZ_BYTES, "MAC should exist in this space. Req for NACLS_BYTES");
static_assert(NACLH_BYTES >= NACLS_KEY_BYTES, "For ratcheting keys");

static void naclKeyPair(u8 pk[NACLA_PUB_BYTES], u8 sk[NACLA_SEC_BYTES]) {
    naclInit();
    crypto_box_keypair(pk, sk);
}
static void naclHash(u8 dst[NACLH_BYTES], void *const R() src, usize srcLen) {
    naclInit();
    crypto_hash(dst, src, srcLen);
}
static void naclRatchet(u8 key[NACLS_KEY_BYTES], u8 sws[NACLH_BYTES]) {
    naclInit();
    crypto_hash(sws, key, NACLS_KEY_BYTES);
    memcpy(key, sws, NACLS_KEY_BYTES);
    explicit_bzero(sws, NACLH_BYTES);
}
static void naclMix(u8 key[NACLS_KEY_BYTES], u8 secret[NACLS_KEY_BYTES], u8 sws[NACLH_BYTES]) {
    naclInit();
    for(usize i = 0; i < NACLS_KEY_BYTES; ++i) { key[i] ^= secret[i]; }
    explicit_bzero(secret, NACLS_KEY_BYTES);
    naclRatchet(key, sws);
}

#define NACL_ENCRYPT_START(PRE, BUFF)                                   \
    naclInit();                                                         \
                                                                        \
    usize len = BUFF##Size(text);                                       \
    usize ctLen = len;                                                  \
    ADD_CHECK(ctLen, PRE##_BYTES, SIZE_MAX);                            \
    CHECK(BUFF##Reserve(text, ctLen));                                  \
    u8 *R() plainText = BUFF##Data(text);                               \
                                                                        \
    u8 *R() nonce = plainText + len + (PRE##_BYTES - PRE##_N_BYTES);    \
    devURandom(nonce, PRE##_N_BYTES);                                   \
                                                                        \
    usize wsLen = PRE##_Z_BYTES;                                        \
    ADD_CHECK(wsLen, len, SIZE_MAX);                                    \
    ADD_CHECK(wsLen, wsLen, SIZE_MAX);                                  \
    CHECK(BUFF##Reserve(ws, wsLen));                                    \
    wsLen >>= 1;                                                        \
    u8 *R() wsData = BUFF##Data(ws);                                    \
    memset(wsData, 0, PRE##_Z_BYTES);                                   \
    memcpy(wsData + PRE##_Z_BYTES, plainText, len);                     \
                                                                        \

//  Encryption function call goes here

#define NACL_ENCRYPT_END(PRE, BUFF)                                     \
    wsData += wsLen + PRE##_BZ_BYTES;                                   \
    wsLen -= PRE##_BZ_BYTES;                                            \
    memcpy(plainText, wsData, wsLen);                                   \
    BUFF##SetSize(text, ctLen);                                         \
    explicit_bzero(BUFF##Data(ws) + PRE##_Z_BYTES, len);                \
    return 0;                                                           \
                                                                        \
FAILED:                                                                 \
    explicit_bzero(BUFF##Data(ws), BUFF##Size(ws));                     \
    return -1;                                                          \

#define NACL_DECRYPT_START(PRE, BUFF)           \
    naclInit();                                 \
                                                \
    usize len = BUFF##Size(text);               \
    u8 *R() ct = BUFF##Data(text);              \
                                                \
    CHECK(len < PRE##_BYTES);                   \
                                                \
    u8 *R() nonce = ct + len - PRE##_N_BYTES;   \
    len -= PRE##_N_BYTES;                       \
                                                \
    usize wsLen = PRE##_BZ_BYTES;               \
    ADD_CHECK(wsLen, len, SIZE_MAX);            \
    ADD_CHECK(wsLen, wsLen, SIZE_MAX);          \
    CHECK(BUFF##Reserve(ws, wsLen));            \
    wsLen >>= 1;                                \
    u8 *R() wsData = BUFF##Data(ws);            \
    memset(wsData, 0, PRE##_BZ_BYTES);          \
    memcpy(wsData + PRE##_BZ_BYTES, ct, len);   \

// Decrypt function call goes here

#define NACL_DECRYPT_END(PRE, BUFF)                             \
    wsData += wsLen + PRE##_Z_BYTES;                            \
    wsLen -= PRE##_Z_BYTES;                                     \
    memcpy(ct, wsData, wsLen);                                  \
    explicit_bzero(wsData, wsLen);                              \
    BUFF##SetSize(text, wsLen);                                 \
    return 0;                                                   \
                                                                \
AUTH_FAILED:                                                    \
    explicit_bzero(wsData + wsLen, wsLen);                      \
    return 1;                                                   \
FAILED:                                                         \
    explicit_bzero(BUFF##Data(ws), BUFF##Size(ws));             \
    return -1;                                                  \

#define A_ENCRYPT_FN()                  \
    crypto_box(                         \
        wsData + wsLen,  /* dst */      \
        wsData,          /* src */      \
        wsLen,           /* len */      \
        nonce,           /* non */      \
        pk,              /* pk */       \
        sk               /* sk */       \
    );                                  \

#define S_ENCRYPT_FN()                  \
    crypto_secretbox(                   \
        wsData + wsLen,  /* dst */      \
        wsData,          /* src */      \
        wsLen,           /* len */      \
        nonce,           /* non */      \
        key              /* key */      \
    );                                  \

#define A_DECRYPT_FN()                  \
    if(crypto_box_open(                 \
        wsData + wsLen,  /* dst */      \
        wsData,          /* src */      \
        wsLen,           /* len */      \
        nonce,           /* non */      \
        pk,              /* pk  */      \
        sk               /* sk  */      \
    )) {                                \
        DEBUG_LOG("NACLA Decryption Auth failure\n");    \
        goto AUTH_FAILED;               \
    }                                   \

#define S_DECRYPT_FN()                  \
    if(crypto_secretbox_open(           \
        wsData + wsLen,  /* dst */      \
        wsData,          /* src */      \
        wsLen,           /* len */      \
        nonce,           /* non */      \
        key              /* key */      \
    )) {                                \
        DEBUG_LOG("NACLS Decryption Auth failure\n");    \
        goto AUTH_FAILED;               \
    }                                   \

#define NACL_DEF_FNS(TYPE)                                                                                      \
static Ret TYPE##AEncrypt(TYPE *R() text, u8 pk[NACLA_PUB_BYTES], u8 sk[NACLA_SEC_BYTES], TYPE *R() ws) {       \
    NACL_ENCRYPT_START(NACLA, TYPE); A_ENCRYPT_FN(); NACL_ENCRYPT_END(NACLA, TYPE);                             \
}                                                                                                               \
static Ret TYPE##ADecrypt(TYPE *R() text, u8 pk[NACLA_PUB_BYTES], u8 sk[NACLA_SEC_BYTES], TYPE *R() ws) {       \
    NACL_DECRYPT_START(NACLA, TYPE); A_DECRYPT_FN(); NACL_DECRYPT_END(NACLA, TYPE);                             \
}                                                                                                               \
static Ret TYPE##SEncrypt(TYPE *R() text, u8 key[NACLS_KEY_BYTES], TYPE *R() ws) {                              \
    NACL_ENCRYPT_START(NACLS, TYPE); S_ENCRYPT_FN(); NACL_ENCRYPT_END(NACLS, TYPE);                             \
}                                                                                                               \
static Ret TYPE##SDecrypt(TYPE *R() text, u8 key[NACLS_KEY_BYTES], TYPE *R() ws) {                              \
    NACL_DECRYPT_START(NACLS, TYPE); S_DECRYPT_FN(); NACL_DECRYPT_END(NACLS, TYPE);                             \
}                                                                                                               \

#define NACL_DD_FNS(TYPE) NACL_DEC_FNS(TYPE); NACL_DEF_FNS(TYPE);
