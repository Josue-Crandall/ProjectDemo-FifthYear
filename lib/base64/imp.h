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

#include <ctype.h>

static void toB64Imp(u8 *R() dst,  u8 *R() src,  u8 *R() end) {
    static u8 TABLE[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X','Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x','y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    while(src != end) {
        u8 temp;
        
        // Encode first 6 bits of byte
        *dst++ = TABLE[*src >> 2];
        // Move last 2 bits into front     
        temp = (*src & 3) << 4;
        // If there are no more bytes encode 2 bits by themselves             
        if(++src == end) {                       
            *dst++ = TABLE[temp];
            break;
        }
        
        // o.w. Encode next 6 bits (remaning two bits from byte) + next 4 bits
        *dst++ = TABLE[temp + (*src >> 4)];
        // Move left over 4 bits from byte into front
        temp = (*src & 15) << 2;
        // if there are no more bits encode 4 bits by themselves            
        if(++src == end) {                       
            *dst++ = TABLE[temp];
            break;
        }

        // o.w. encode the next 6 bits (remaining 4 bits + 2 from next byte)
        *dst++ = TABLE[temp + (*src >> 6)];
        // remaining 6 bits can be used all at once
        *dst++ = TABLE[*src++ & 63];        
                                                    
        // At this point there are no remaining bits and we can start over
    }
}

static Ret binToB64Size(usize *bytes) {
    usize temp = *bytes;
    ADD_CHECK(temp, 2, SIZE_MAX);
    temp /= 3;
    ADD_CHECK(*bytes, temp, SIZE_MAX);
    return 0;
FAILED:
    return -1;
}

////

static void toBinImp(u8 *R() dst, u8 *R() src, u8 *R() end) {
    static  u8 REVERSE_TABLE[] = {
        0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,62,0,0,0,63,52,53,54,
        55,56,57,58,59,60,61,0,0,0,
        0,0,0,0,0,1,2,3,4,5, 
        6,7,8,9,10,11,12,13,14,15, 
        16,17,18,19,20,21,22,23,24,25,
        0,0,0,0,0,0,26,27,28,29, 
        30,31,32,33,34,35,36,37,38,39,
        40,41,42,43,44,45,46,47,48,49,
        50,51,
    };

    while(src != end) {
        u8 temp;

        // fill in the first bytes first 6 bits
        *dst = REVERSE_TABLE[*src++] << 2;
        // fill in last 2 bits of the first byte
        temp = REVERSE_TABLE[*src++];
        *dst |= temp >> 4;                               
        if(src == end) { break; }

        // fill in first 4 bits of the second byte
        *++dst = temp << 4;                              
        temp = REVERSE_TABLE[*src++];
        // fill in the last 4 bits of the second byte            
        *dst |= temp >> 2;                               
        if(src == end) { break; }

        // fill in the top 2 bits of the third byte
        *++dst = temp << 6;
        // fill in the last 6 bits of the third byte                
        *dst++ |= REVERSE_TABLE[*src++];          

        // at this point there are no outstanding bits to worry about
    }
}

static usize b64ToBinSize(usize bytes) {
    // bytes does not include the null terminator
    return (bytes > 1) ? (bytes - 1 - ((bytes - 2) >> 2)) : 0;
}

static Ret validateChar(u8 c) {
    static  u8 VALIDATION_TABLE[] = {
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1, 0,+1,+1,+1, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0,+1,+1,+1,+1,+1,+1,
        +1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,+1,+1,+1,+1,+1,
        +1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,+1,+1,+1,+1,+1,
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,
        +1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,+1,
	};
    return VALIDATION_TABLE[c];
}

static Ret validateB64Size(usize len) {
    //// 0) valid lengths are (not including null terminator)
    // 1 -> 2 char
    // 2 -> 3 char
    // 3 -> 4 char      
    // 4 -> 6 char
    // 5 -> 7 char
    // 6 -> 8 char
    // 7 -> 10 char
    // 8 -> 11 char
    // 9 -> 12 char
    // 10 -> 14 char
    //// notice: skips 5, 9, 13, 17 ... e.g. (any multiple of 4) + 1
    // thus:
    if(0 == len) { return 0; } 
    else { return 0 == ((len-1) & 3); }
}

static Ret validateB64(u8 *R() it, u8 *R() end) {
    CHECK(validateB64Size(end - it));
    while(it != end) { CHECK(validateChar(*it++)) };
    return 0;
FAILED:
    return 1;
}

static void trim(u8 *R()*R() it, u8 *R()*R() end) {
    u8 *R() first = *it, *R() last = *end;

    while(first != last && isspace(*first)) { ++first; }
    while(last > first && isspace(last[-1])) { --last; }
    while(last > first && '=' == last[-1]) { --last; }

    *it = first; *end = last;
}

////

#define DEFB64(TYPE)                                            \
Ret TYPE##B64ToBin (TYPE *dst, u8 *src, usize len, Ret knownValid) { \
    u8 *it = src, *end = it + len;                              \
                                                                \
    if(!knownValid) {                                           \
        trim(&it, &end);                                        \
        if(validateB64(it, end)) {                              \
            DEBUG_LOG("Base64 to bin invalid input\n");         \
            return 1;                                           \
        }                                                       \
    }                                                           \
                                                                \
    len = b64ToBinSize(end - it);                               \
    CHECK( TYPE##Reserve(dst, len));                            \
    TYPE##SetSize(dst, len);                                    \
                                                                \
    u8 *dstIter = TYPE##Data(dst);                              \
                                                                \
    toBinImp(dstIter, it, end);                                 \
                                                                \
    return 0;                                                   \
FAILED:                                                         \
    return -1;                                                  \
}                                                               \
Ret TYPE##BinToB64(TYPE *dst, u8 *src, usize len, Ret skipPadding) {     \
    u8 *it = src,  *end = it + len;                             \
                                                                \
    CHECK(binToB64Size(&len));                                  \
    if(!skipPadding) {                                          \
        usize paddingAmt = len & 3;                             \
        if(paddingAmt) { paddingAmt = paddingAmt == 2 ? 2 : 1; }\
        ADD_CHECK(len, paddingAmt, SIZE_MAX);                   \
        CHECK(TYPE##Reserve(dst,len));                          \
        TYPE##SetSize(dst, len);                                \
        if(paddingAmt) {                                        \
            TYPE##Data(dst)[len - 1] = '=';                     \
            if(paddingAmt == 2) { TYPE ## Data(dst)[len - 2] = '='; } \
        }                                                       \
    }                                                           \
    else {                                                      \
        CHECK(TYPE##Reserve(dst, len));                         \
        TYPE##SetSize(dst, len);                                \
    }                                                           \
                                                                \
    u8 *dstIter = TYPE##Data(dst);                              \
                                                                \
    toB64Imp(dstIter, it, end);                                 \
                                                                \
    return 0;                                                   \
FAILED:                                                         \
    return -1;                                                  \
}                                                               \

#define DDB64(TYPE)     \
DECB64(TYPE);           \
DEFB64(TYPE);           \

