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

// WARNING: This is not actually a JSON implementation.
//          It is just JSON like. (e.g. no true,false,null)

#ifndef JC_JSON_H
#define JC_JSON_H
#include "../macros/macros.h"

#include "../string/string.h"
#include "../table/table.h"
#include "../vec/vec.h"

typedef struct JSON JSON;
DEC_VEC(JVec, JSON);
DEC_TABLE(JTbl, Str, JSON);
typedef struct JSON {
    #define JSON_OBJ 1
    #define JSON_ARR 2
    #define JSON_NUM 3
    #define JSON_STR 4 // Note: other values reserved internally
    Ret type;
    union {
        JTbl tbl;
        JVec vec;
        Str str;
        Str num;
    };
} JSON;

static Ret JSONInit(JSON *json, char *cstr);
// Note: out is assumed initialized and empty
static Ret JSONToStr(JSON *json, Str *out, Ret compact);
static void JSONDe(JSON *json);

#include "imp.h"

#endif