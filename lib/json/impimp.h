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

#define JTOKEN_NUL 0 // reserved
#define JTOKEN_ERR 5
#define JTOKEN_EOF 6
#define JTOKEN_LCURLY 7
#define JTOKEN_RCURLY 8
#define JTOKEN_LBRACK 9
#define JTOKEN_RBRACK 10
#define JTOKEN_STRING 11
#define JTOKEN_NUMBER 12
#define JTOKEN_COMMA 13
#define JTOKEN_COLON 14
#define JTOKEN_ID 15

//// Lexer
static void JSONString(JSON *out, char **json) {
    char *iter = *json;

    CHECK(StrInit(&out->str, ""));

    while(1) {
        CHECK(!*++iter);
        if(*iter == '"') { break; }
        else { CHECK(StrCPush(&out->str, *iter)); }
    }

    *json = ++iter;
    out->type = JTOKEN_STRING;
    return;

FAILED:
    StrDe(&out->str);
    out->type = JTOKEN_ERR;
    return;
}
static void JSONNumber(JSON *out, char **json) {
    char *iter = *json;

    CHECK(StrInit(&out->str, ""));

    while(1) {
        CHECK(StrCPush(&out->str, *iter++));
        if(!(*iter >= '0' && *iter <= '9')) { break; }
    }

    *json = iter;
    out->type = JTOKEN_NUMBER;
    return;

FAILED:
    StrDe(&out->str);
    out->type = JTOKEN_ERR;
    return;
}
static Ret JSONIDWhitelist(char c) {
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') { return 0; }
    else if(c >= '0' && c <= '9') { return 1; }
    else { return -1; }
}
static void JSONIdentifier(JSON *out, char **json) {
    char *iter = *json;

    CHECK(StrInit(&out->str, ""));

    CHECK(JSONIDWhitelist(*iter));
    CHECK(StrCPush(&out->str, *iter));

    while(1) {
        Ret res = JSONIDWhitelist(*++iter);
        if(res < 0) { break; }
        CHECK(StrCPush(&out->str, *iter));
    }

    *json = iter;
    out->type = JTOKEN_ID;
    return;

FAILED:
    StrDe(&out->str);
    out->type = JTOKEN_ERR;
    return;
}
static void JSONNextToken(JSON *out, char **json) {
    char *iter = *json;

SWITCH:
    switch(*iter) {
        case ' ': case '\n': case '\t': case '\r': { ++iter; goto SWITCH; } break;
        case '\0': { out->type = JTOKEN_EOF; } break;
        case '{': { ++iter; out->type = JTOKEN_LCURLY; } break;
        case '}': { ++iter; out->type = JTOKEN_RCURLY; } break;
        case '[': { ++iter; out->type = JTOKEN_LBRACK; } break;
        case ']': { ++iter; out->type = JTOKEN_RBRACK; } break;
        case ',': { ++iter; out->type = JTOKEN_COMMA; } break;
        case ':': { ++iter; out->type = JTOKEN_COLON; } break;
        case '"': { JSONString(out, &iter); } break;
        case '1': case '2': case '3': case '4': case '5': case '6': // not case '0'
        case '7': case '8': case '9': { JSONNumber(out, &iter); } break;
        default: { JSONIdentifier(out, &iter); } break;
    }

    if(JTOKEN_ERR != out->type) { *json = iter; }
}
//// ~Lexer

//// Parser
static Ret JSONParseObject(JSON *out, char **json, Ret lcurlyEaten);
static Ret JSONParseArray(JSON *out, char **json, Ret lbrackEaten);
static Ret JSONParseNumber(JSON *json);
static Ret JSONParseString(JSON *json);
static Ret JSONParseValue(JSON *json);

static Ret JSONParseNumber(JSON *json) {
    json->type = JSON_NUM;
    Str temp = json->str;
    json->num = temp;
    return 0;
}
static Ret JSONParseString(JSON *json) {
    json->type = JSON_STR;
    Str temp = json->str;
    json->str = temp;
    return 0;
}
static Ret JSONParseArray(JSON *out, char **json, Ret lbrackEaten) {
    JSON val = {0};

    out->type = JSON_ARR;
    CHECK(JVecInit(&out->vec, 0));

    if(!lbrackEaten) { 
        JSONNextToken(&val, json);
        CHECK(JTOKEN_LBRACK != val.type);
    }

    while(1) {
        JSONNextToken(&val, json);
        switch(val.type) {
            case JTOKEN_RBRACK: { return 0; } break;
            case JTOKEN_STRING: { CHECK(JSONParseString(&val)); } break; 
            case JTOKEN_NUMBER: { CHECK(JSONParseNumber(&val)); } break;
            case JTOKEN_LCURLY: { CHECK(JSONParseObject(&val, json, 1)); } break;
            case JTOKEN_LBRACK: { CHECK(JSONParseArray(&val, json, 1)); } break;
            default: { 
                DEBUG_LOG("Failed JSONParseArray, expected value\n"); goto FAILED; 
            }
        }

        CHECK(JVecPush(&out->vec, val));
        val.type = JTOKEN_NUL;

        JSONNextToken(&val, json);
        switch(val.type) {
            // Note: departure from JSON, allowing extraneous end comma
            case JTOKEN_COMMA: { NOP; } break;
            case JTOKEN_RBRACK: { return 0; } break;
            default: { 
                DEBUG_LOG("Failed JSONParseObject, expected , or ]\n"); goto FAILED; 
            }
        }
    }

    return 0;

FAILED:
    JSONDe(out);
    JSONDe(&val);
    out->type = JTOKEN_ERR;
    return -1;
}
static Ret JSONParseObject(JSON *out, char **json, Ret lcurlyEaten) {
    JSON key = {0}, val = {0};

    out->type = JSON_OBJ;
    CHECK(JTblInit(&out->tbl, 0));

    if(!lcurlyEaten) { 
        JSONNextToken(&key, json);
        CHECK(JTOKEN_LCURLY != key.type);
    }

    while(1) {
        JSONNextToken(&key, json);
        switch(key.type) {
            case JTOKEN_RCURLY: { return 0; } break;
            case JTOKEN_ID: {
                JSONNextToken(&val, json);
                CHECK(JTOKEN_COLON != val.type);

                JSONNextToken(&val, json);
                switch(val.type) {
                    case JTOKEN_STRING: { CHECK(JSONParseString(&val)); } break;
                    case JTOKEN_NUMBER: { CHECK(JSONParseNumber(&val)); } break;
                    case JTOKEN_LCURLY: { CHECK(JSONParseObject(&val, json, 1));  } break;
                    case JTOKEN_LBRACK: { CHECK(JSONParseArray(&val, json, 1)); } break;
                    default: { 
                        DEBUG_LOG("Failed JSONParseObject, expected object value\n"); goto FAILED; 
                    }
                }

                CHECK(JTblPush(&out->tbl, key.str, val, 0));
                key.type = val.type = JTOKEN_NUL;

                JSONNextToken(&key, json);
                switch(key.type) {
                    // Note: departure from JSON, allowing extraneous end comma
                    case JTOKEN_COMMA: { NOP; } break;
                    case JTOKEN_RCURLY: { return 0; } break;
                    default: { DEBUG_LOG("Failed JSONParseObject, expected , or }\n"); goto FAILED; }
                }
            } break;
            default: { DEBUG_LOG("Failed JSONParseObject, expected ID or }\n"); goto FAILED; }
        }
    }

FAILED:
    JSONDe(out);
    JSONDe(&key);
    JSONDe(&val);
    out->type = JTOKEN_ERR;
    return -1;
}
//// ~Parser
