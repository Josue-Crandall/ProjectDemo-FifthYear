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

#include "../rng/rng.h"
#include "../alloc/alloc.h"

// Note: Don't invalidate generator on removal.
// If required add explicit shrink functions.
// Note: alloc.h now reliant on zero value return to out
// on not found NAME##Rm()

#define TBL_DEFAULT_CAPACITY    16
static_assert(TBL_DEFAULT_CAPACITY > 0, "Required for grow logic.");

static_assert(sizeof(umax) >= 2, "Using pointers after NULL");
#define TBL_FLG_SPARE_ENTRY     ((void *)(1))

#define TABLE_STATE_EMPTY 0 // Must be 0
#define TABLE_STATE_FULL  1
#define TABLE_STATE_HOLD  2
#define TABLE_CAP() (table->mask + 1)
static_assert(TBL_DEFAULT_CAPACITY != 0, "Doesn't work with grow logic");
static void TableRandomize(usize *const R() a, usize *const R() b) {
    devURandom(a, sizeof(*a)); *a |= 1; devURandom(b, sizeof(*b));
}

#define DEF_TABLE(NAME, KEY_T, VAL_T, HASH, KEY_EQ, KEY_DE, VAL_DE, ALLOC)  \
static usize NAME##Hash(NAME *R() table, KEY_T key) {                       \
    usize res; { HASH; }                                                    \
    for(usize shift = sizeof(usize) * 4; shift > 1; shift >>= 1) {          \
        res = res * table->a + table->b;                                    \
        res ^= ((res >> shift) | (res << shift));                           \
    }                                                                       \
    return res & table->mask;                                               \
}                                                                           \
static Ret NAME##KeyEq(KEY_T lhs, KEY_T rhs) {                              \
    Ret res; { KEY_EQ; } return res;                                        \
}                                                                           \
static void NAME##KeyDe(KEY_T key) {                                        \
    const KEY_T ignWarnings = key; { KEY_DE; }                              \
}                                                                           \
static void NAME##ValDe(VAL_T val) {                                        \
    const VAL_T ignWarnings = val; { VAL_DE; }                              \
}                                                                           \
static Ret NAME##Init(NAME *table, usize cap) {                             \
    table->states = 0; table->keys = 0; table->vals = 0;                    \
                                                                            \
    if(0 == cap) { cap = TBL_DEFAULT_CAPACITY; }                            \
    POW2_ROUND(cap, usize);                                                 \
    table->mask = cap - 1;                                                  \
    table->count = 0;                                                       \
    TableRandomize(&table->a, &table->b);                                   \
                                                                            \
    NULL_CHECK(table->states = ALLOC##Calloc(cap, sizeof(Ret)));            \
    NULL_CHECK(table->keys = ALLOC##Malloc(cap, sizeof(KEY_T)));            \
    NULL_CHECK(table->vals = ALLOC##Malloc(cap, sizeof(VAL_T)));            \
                                                                            \
    return 0;                                                               \
FAILED:                                                                     \
    ALLOC##Free(table->states, cap * sizeof(Ret));                          \
    ALLOC##Free(table->keys, cap * sizeof(KEY_T));                          \
    ALLOC##Free(table->vals, cap * sizeof(VAL_T));                          \
    table->states = 0;                                                      \
    return -1;                                                              \
}                                                                           \
static NAME##Gen NAME##Beg(NAME *table) {                                   \
    NAME##Gen gen = { ((usize)0) - 1, TABLE_CAP(), table->states,           \
        table->keys, table->vals };                                         \
    return gen;                                                             \
}                                                                           \
static Ret NAME##Next(NAME##Gen *gen) {                                     \
    if(++gen->i == gen->cap) { return 0; }                                  \
    else if(gen->states[gen->i] == TABLE_STATE_FULL) { return 1; }          \
    else { return NAME##Next(gen); }                                        \
}                                                                           \
static void NAME##Clear(NAME *table) {                                      \
    for(usize i = 0, end = TABLE_CAP(); i < end; ++i) {                     \
        if(table->states[i] == TABLE_STATE_FULL) {                          \
            NAME##KeyDe(table->keys[i]);                                    \
            NAME##ValDe(table->vals[i]);                                    \
        }                                                                   \
        table->states[i] = TABLE_STATE_EMPTY;                               \
    }                                                                       \
    table->count = 0;                                                       \
}                                                                           \
static void NAME##De(NAME *table) {                                         \
    if(table->states) {                                                     \
        NAME##Clear(table);                                                 \
        usize tableCap = TABLE_CAP();                                       \
        ALLOC##Free(table->states, tableCap * sizeof(Ret));                 \
        ALLOC##Free(table->keys, tableCap * sizeof(KEY_T));                 \
        ALLOC##Free(table->vals, tableCap * sizeof(VAL_T));                 \
    }                                                                       \
}                                                                           \
static Ret NAME##PushImp(NAME *table, KEY_T key, VAL_T val, Ret replace) {  \
    usize i = NAME##Hash(table, key);                                       \
    usize cap = TABLE_CAP();                                                \
                                                                            \
    while(table->states[i] == TABLE_STATE_FULL) {                           \
        if(NAME##KeyEq(key, table->keys[i])) {                              \
            if(replace) {                                                   \
                NAME##KeyDe(table->keys[i]);                                \
                NAME##ValDe(table->vals[i]);                                \
                table->keys[i] = key;                                       \
                table->vals[i]= val;                                        \
                --table->count;                                             \
                return 0;                                                   \
            }                                                               \
            else {  return 1; }                                             \
        }                                                                   \
        else if(++i == cap) { i = 0; }                                      \
    }                                                                       \
                                                                            \
    table->states[i] = TABLE_STATE_FULL;                                    \
    table->keys[i] = key;                                                   \
    table->vals[i] = val;                                                   \
    return 0;                                                               \
}                                                                           \
static Ret NAME##Grow(NAME *table) {                                        \
    NAME growTable = {0};                                                   \
    usize newCap = TABLE_CAP();                                             \
                                                                            \
    if((table->count << 1) + 1 > newCap) {                                  \
        UNSIGNED_LSHIFT_CHECK(newCap, usize);                               \
        CHECK(NAME##Init(&growTable, newCap));                              \
                                                                            \
        for(NAME##Gen gen = NAME##Beg(table); NAME##Next(&gen); NOP) {      \
            NAME##PushImp(&growTable, gen.keys[gen.i], gen.vals[gen.i], 0); \
        }                                                                   \
                                                                            \
        usize tableCap = TABLE_CAP();                                       \
        ALLOC##Free(table->states, tableCap * sizeof(Ret));                 \
        ALLOC##Free(table->keys, tableCap * sizeof(KEY_T));                 \
        ALLOC##Free(table->vals, tableCap * sizeof(VAL_T));                 \
        *table = growTable;                                                 \
    }                                                                       \
                                                                            \
    ++table->count;                                                         \
                                                                            \
    return 0;                                                               \
FAILED:                                                                     \
    return -1;                                                              \
}                                                                           \
static Ret NAME##Push(NAME *table, KEY_T key, VAL_T val, Ret replace) {     \
    CHECK(NAME##Grow(table));                                               \
    return NAME##PushImp(table, key, val, replace);                         \
FAILED:                                                                     \
    return -1;                                                              \
}                                                                           \
static VAL_T *NAME##At(NAME *table, KEY_T key, Ret *insert) {               \
    usize i = NAME##Hash(table, key), start = i, cap = TABLE_CAP();         \
    while(table->states[i] != TABLE_STATE_EMPTY) {                          \
        if(table->states[i] == TABLE_STATE_FULL && NAME##KeyEq(key, table->keys[i])) { \
            if(insert) { *insert = 0; }                                     \
            return table->vals + i;                                         \
        }                                                                   \
        else if(++i == cap) { i = 0; }                                      \
        if(i == start) { break; }                                           \
    }                                                                       \
                                                                            \
    if(insert) {                                                            \
        CHECK(NAME##Grow(table));                                           \
        table->states[i] = TABLE_STATE_FULL;                                \
        table->keys[i] = key;                                               \
        memset(table->vals + i, 0, sizeof(VAL_T));                          \
        *insert = 1;                                                        \
        return table->vals + i;                                             \
    }                                                                       \
    else { return 0; }                                                      \
FAILED:                                                                     \
    *insert = -1;                                                           \
    return 0;                                                               \
}                                                                           \
static void NAME##Rm(NAME *table, KEY_T key, VAL_T *R() out) {              \
    usize i = NAME##Hash(table, key), start = i, cap = TABLE_CAP();         \
    while(table->states[i] != TABLE_STATE_EMPTY) {                          \
        if(table->states[i] == TABLE_STATE_FULL && NAME##KeyEq(key, table->keys[i])) { \
            if(0 == out) {                                                  \
                NAME##KeyDe(table->keys[i]);                                \
                NAME##ValDe(table->vals[i]);                                \
            }                                                               \
            else if(TBL_FLG_SPARE_ENTRY != out) {                           \
                NAME##KeyDe(table->keys[i]);                                \
                *out = table->vals[i];                                      \
            }                                                               \
            --table->count;                                                 \
                                                                            \
            {                                                               \
                usize next = i + 1; if(next == cap) { next = 0; }           \
                if(table->states[next] == TABLE_STATE_EMPTY) {              \
                    table->states[i] = TABLE_STATE_EMPTY;                   \
                    while(1) {                                              \
                        i = i ? i - 1 : cap - 1;                            \
                        if(table->states[i] == TABLE_STATE_HOLD) {          \
                            table->states[i] = TABLE_STATE_EMPTY;           \
                        }                                                   \
                        else { return; }                                    \
                    }                                                       \
                }                                                           \
                else {                                                      \
                    table->states[i] = TABLE_STATE_HOLD;                    \
                    return;                                                 \
                }                                                           \
            }                                                               \
        }                                                                   \
        else if(++i == cap) { i = 0; }                                      \
        if(i == start) { break; }                                           \
    }                                                                       \
                                                                            \
    if((void *)out > TBL_FLG_SPARE_ENTRY) { memset(out, 0, sizeof(VAL_T)); }\
}                                                                           \
