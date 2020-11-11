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

DEF_TABLE(JTbl, Str, JSON, res = StrHash(&key), res = StrEq(&lhs, &rhs), StrDe(&key), JSONDe(&val), STD_ALLOC);
DEF_VEC(JVec, JSON, JSONDe(&val), STD_ALLOC);

#include "impimp.h"

static Ret JSONInit(JSON *json, char *cstr) {
    JSON temp = {0};
    CHECK(JSONParseObject(json, &cstr, 0));
    JSONNextToken(&temp, &cstr);
    CHECK(JTOKEN_EOF != temp.type);
    return 0;
FAILED:
    JSONDe(json);
    JSONDe(&temp);
    json->type = JTOKEN_NUL;
    return -1;
}

static void JSONDe(JSON *json) {
    if(json) {
        switch(json->type) {
            case JSON_OBJ: { JTblDe(&json->tbl); } break;
            case JSON_ARR: { JVecDe(&json->vec); } break;
            case JSON_NUM: { StrDe(&json->num); } break;
            case JTOKEN_STRING: case JTOKEN_NUMBER: case JTOKEN_ID:
            case JSON_STR: { StrDe(&json->str); } break;
        }
    }
}

static Ret JSONToStrImp(JSON *json, Str *out, Ret compact, usize indentLevel) {
    switch(json->type) {
        case JSON_OBJ: {
            if(!compact) {
                CHECK(StrCPush(out, '\n'));
                for(usize i = 0; i < indentLevel; ++i) { CHECK(StrCPush(out, ' ')); } 
            }
            CHECK(StrCPush(out, '{'));
            Ret newLined = 0;
            for(JTblGen gen = JTblBeg(&json->tbl); JTblNext(&gen); NOP) {
                newLined = 1;
                if(!compact) { 
                    CHECK(StrCPush(out, '\n')); 
                    for(usize i = 0; i < indentLevel + 2; ++i) { CHECK(StrCPush(out, ' ')); } 
                }
                CHECK(StrPush(out, StrIng(&gen.keys[gen.i])));
                if(!compact) { CHECK(StrCPush(out, ' ')); } CHECK(StrCPush(out, ':')); if(!compact) { CHECK(StrCPush(out, ' ')); }
                CHECK(JSONToStrImp(&gen.vals[gen.i], out, compact, indentLevel + 2));
                CHECK(StrCPush(out, ','));
                if(!compact) { CHECK(StrCPush(out, ' ')); }
            }
            if(!compact && newLined) { 
                CHECK(StrCPush(out, '\n')); 
                if(!compact) { for(usize i = 0; i < indentLevel; ++i) { CHECK(StrCPush(out, ' ')); } }
            }
            CHECK(StrCPush(out, '}'));
        } break;
        case JSON_ARR: {
            CHECK(StrCPush(out, '['));
            for(JVecGen gen = JVecBeg(&json->vec); JVecNext(&gen); NOP) {
                CHECK(JSONToStrImp(gen.val, out, compact, indentLevel + 2));
                CHECK(StrCPush(out, ','));
                if(!compact) { CHECK(StrCPush(out, ' ')); }
            }
            CHECK(StrCPush(out, ']'));
        } break;
        case JSON_NUM: { CHECK(StrPush(out, StrIng(&json->num))); } break;
        case JSON_STR: { 
            CHECK(StrCPush(out, '"'));
            CHECK(StrPush(out, StrIng(&json->str)));
            CHECK(StrCPush(out, '"'));
        } break;
        default: { goto FAILED; }
    }
    return 0;
FAILED:
    return -1;
}

static Ret JSONToStr(JSON *json, Str *out, Ret compact) {
    CHECK(JSONToStrImp(json, out, compact, 0));
    if(!compact) { CHECK(StrCPush(out, '\n')); }
    return 0;
FAILED:
    return -1;
}
