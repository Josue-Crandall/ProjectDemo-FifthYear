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

#define VEC_DEFAULT_CAPACITY    16
static_assert(VEC_DEFAULT_CAPACITY > 0, "Required for grow logic.");


#define DEF_VEC(NAME, VAL_T, VAL_DE, ALLOC)                                     \
static void NAME##ValDe(VAL_T val) { val = val; { VAL_DE; } }                   \
static VAL_T* NAME##Data(NAME *vec) { return vec->data; }                       \
static void NAME##SetSize(NAME *vec, usize newLen) { vec->len = newLen; }       \
static usize NAME##Size(NAME *vec) { return vec->len; }                         \
static Ret NAME##Alive(NAME *vec) { return vec->data ? 1 : 0; }                 \
static Ret NAME##Init(NAME *vec, usize cap) {                                   \
    if(0 == cap) { cap = VEC_DEFAULT_CAPACITY; }                                \
    POW2_ROUND(cap, usize);                                                     \
                                                                                \
    NULL_CHECK(vec->data = ALLOC##Malloc(cap, sizeof(VAL_T)));                  \
    vec->len = 0;                                                               \
    vec->cap = cap;                                                             \
                                                                                \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static NAME##Gen NAME##Beg(NAME *vec) {                                         \
    NAME##Gen val = { vec->data - 1, vec->data + vec->len };                    \
    return val;                                                                 \
}                                                                               \
static Ret NAME##Next(NAME##Gen *gen) { return ++gen->val != gen->end; }        \
static void NAME##De(NAME *vec) {                                               \
    if(vec->data) {                                                             \
        for(NAME##Gen gen = NAME##Beg(vec); NAME##Next(&gen); NOP) {            \
            NAME##ValDe(*gen.val);                                              \
        }                                                                       \
        ALLOC##Free(vec->data, vec->cap * sizeof(VAL_T));                       \
    }                                                                           \
}                                                                               \
static void NAME##Clear(NAME *vec) {                                            \
    for(NAME##Gen gen = NAME##Beg(vec); NAME##Next(&gen); NOP) {                \
        NAME##ValDe(*gen.val);                                                  \
    }                                                                           \
    vec->len = 0;                                                               \
}                                                                               \
static Ret NAME##Reserve(NAME *vec, usize amt) {                                \
    if(amt > vec->cap) {                                                        \
        usize newCap = vec->cap;                                                \
        do { UNSIGNED_LSHIFT_CHECK(newCap, usize); } while(newCap < amt);       \
                                                                                \
        CHECK(ALLOC##Realloc((void **)&vec->data, vec->cap * sizeof(VAL_T), newCap, sizeof(VAL_T))); \
        vec->cap = newCap;                                                      \
    }                                                                           \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static Ret NAME##Grow(NAME *vec, usize amt) {                                   \
    usize newLen = vec->len;                                                    \
    ADD_CHECK(newLen, amt, SIZE_MAX);                                           \
    if(newLen > vec->cap) { CHECK(NAME##Reserve(vec, newLen)); }                \
    vec->len = newLen;                                                          \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static Ret NAME##PushArr(NAME *vec, VAL_T *data, usize amt) {                   \
    CHECK(NAME##Grow(vec, amt));                                                \
    memcpy(vec->data + vec->len - amt, data, sizeof(VAL_T) * amt);              \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static Ret NAME##Push(NAME *vec, VAL_T val) {                                   \
    CHECK(NAME##Grow(vec, 1));                                                  \
    vec->data[vec->len - 1] = val;                                              \
    return 0;                                                                   \
FAILED:                                                                         \
    return -1;                                                                  \
}                                                                               \
static VAL_T NAME##Pop(NAME *vec) {                                             \
    if(0 == vec->len) { RAI(VAL_T, res); return res; }                          \
    return vec->data[--vec->len];                                               \
}                                                                               \
static usize NAME##Rm(NAME *vec, usize start, usize finish, void *ppred) {      \
    VAL_T *iter = vec->data + start;                                            \
    VAL_T *end = vec->data + finish;                                            \
    NAME##Pred *pred = ppred;                                                   \
                                                                                \
    for(NOP; iter != end; ++iter) {                                             \
        if(pred->cb(pred, iter)) { NAME##ValDe(*iter); break; }                 \
    }                                                                           \
    VAL_T * newEnd = iter;                                                      \
    if(iter != end) {                                                           \
        while(++iter != end) {                                                  \
            if(pred->cb(pred, iter)) { NAME##ValDe(*iter); }                    \
            else { *newEnd++ = *iter; }                                         \
        }                                                                       \
    }                                                                           \
                                                                                \
    usize count = end - newEnd;                                                 \
    vec->len -= count;                                                          \
    return count;                                                               \
}                                                                               \
