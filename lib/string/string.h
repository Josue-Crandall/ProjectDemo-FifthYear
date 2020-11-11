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

#ifndef JC_STR_H
#define JC_STR_H
#include "../macros/macros.h"

#include "../api/api.h"

#include <string.h>

#define STR_EXTRA_BASE_CAPACITY 3

typedef Buff Str;

static Ret StrInit(Str *str, const char *const base);
static char * StrIng(Str *str);
static usize StrLen(Str *str);
static Ret StrPush(Str *str, const char *const cstr);
static Ret StrCPush(Str *str, char ch);
static void StrPop(Str *str, usize amt);
static void StrClear(Str *str);
static usize StrHash(Str *str);
static Ret StrEq(Str *R() lhs, Str *R() rhs);
static void StrDe(Str *str);

#include "imp.h"

#endif