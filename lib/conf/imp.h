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

// Returns 0 if key not found
static char *ConfGet(Conf *conf, char *key) {
    char **temp = ConfTableAt(&conf->table, key, 0);
    return temp ? *temp : NULL;
}
static void ConfDe(Conf *conf) {
    ConfTableDe(&conf->table);
    BuffDe(&conf->data);
}

#include <string.h>
#include <ctype.h>
#include "../file/file.h"

static_assert(sizeof(umax) >= 2, "Using pointers after NULL");

#define CONF_TOK_EOF ((char *)0)
#define CONF_TOK_EQ  ((char *)1)
//      CONF_TOK_STR (o.w.)
struct ConfLex { u8 *R() it, *R() end; char *R() token; };
static void lexConsume(struct ConfLex *lex) { ++lex->it; }
static void lexSpace(struct ConfLex *lex) {
    while(1) {
        lexConsume(lex);
        if(lex->it == lex->end || !isspace(*lex->it)) { break; }
    }
}
static void lexComment(struct ConfLex *lex) {
    while(1) {
        lexConsume(lex);
        if(lex->it == lex->end) { break; }
        else if('\n' == *lex->it) { lexConsume(lex); break; }
    }
}
static Ret lexString(struct ConfLex *lex) {
    lexConsume(lex);
    lex->token = (char *)lex->it;
    while(1) {
        if(lex->it == lex->end) { return -1; }
        else if('"' == *lex->it) { *lex->it = 0; lexConsume(lex); return 0; }
        else { lexConsume(lex); }
    }
}
static Ret nextToken(struct ConfLex *lex) {
    while(1) {
        if(lex->it == lex->end) { lex->token = CONF_TOK_EOF; return 0; }
        else if(isspace(*lex->it)) { lexSpace(lex); }
        else if('=' == *lex->it) { lexConsume(lex); lex->token = CONF_TOK_EQ; return 0; }
        else if('#' == *lex->it) { lexComment(lex); }
        else if('"' == *lex->it) { return lexString(lex); }
        else { return -1; }
    }
}
static Ret parsePair(Conf *conf, struct ConfLex *lex) {
    char *R() key, *R() val;
    CHECK(lex->token <= CONF_TOK_EQ); key = lex->token; CHECK(nextToken(lex));
    CHECK(lex->token != CONF_TOK_EQ); CHECK(nextToken(lex));
    CHECK(lex->token <= CONF_TOK_EQ); val = lex->token; CHECK(nextToken(lex));
    CHECK(ConfTablePush(&conf->table, key, val, 0));
    return 0;
FAILED:
    return -1;
}
static Ret parseConf(Conf *conf) {
    struct ConfLex lex = { BuffData(&conf->data), BuffData(&conf->data) + BuffSize(&conf->data), 0 };
    CHECK(nextToken(&lex));
    while(CONF_TOK_EOF != lex.token) { CHECK(parsePair(conf, &lex)); }
    return 0;
FAILED:
    return -1;
}

static Ret ConfInit(Conf *conf, char * filePath) {
    memset(conf, 0, sizeof(Conf));
    CHECK(BuffInit(&conf->data, 0));
    CHECK(ConfTableInit(&conf->table, 0));
    CHECK(BuffLoadFile(&conf->data, filePath));
    CHECK(parseConf(conf));
    return 0;
FAILED:
    ConfTableDe(&conf->table);
    BuffDe(&conf->data);
    memset(conf, 0, sizeof(Conf));
    return -1;
}