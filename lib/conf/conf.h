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

#ifndef JC_CONF_H
#define JC_CONF_H
#include "../macros/macros.h"

#include "../api/api.h"
#include "../table/table.h"
#include "../string/string.h"
DD_TABLE(ConfTable, char *, char *, res = strHash(key), res = !strcmp(lhs,rhs), NOP, NOP, STD_ALLOC);

typedef struct {
    Buff data;
    ConfTable table;
} Conf;

// Tokens: String, Eq, EOF
// Lexer:
// 1) ignores anything after a '#' until a '\n'
// 2) a string is anything inbetween " "
// 3) '=' not inbetween " " is an Eq
// Parser:
// conf : pair* EOF
// pair : String Eq String
// 
// Note: Duplicate keys not allowed
static Ret ConfInit(Conf *conf, char * filePath);
static char *ConfGet(Conf *conf, char *key);
static void ConfDe(Conf *conf);

#include "imp.h"

#endif
