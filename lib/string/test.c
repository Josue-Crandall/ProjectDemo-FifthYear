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

#include "string.h"

static Str s;

Ret t0() {
    NOP; printf("%s<--\n", StrIng(&s));
    CHECK(StrPush(&s, "testing")); printf("%s<--\n", StrIng(&s));
    CHECK(StrPush(&s, " 123")); printf("%s<--\n", StrIng(&s));
    StrPop(&s, 4); printf("%s<--\n", StrIng(&s));
    CHECK(StrPush(&s, "d3094873209rjriehsjb;hg;jlkoithjovigtjhgm-85tu85mt9-2u548t94hthm")); printf("%s<--\n", StrIng(&s));
    StrClear(&s); printf("%s<--\n", StrIng(&s));
    CHECK(StrPush(&s, "testing")); printf("%s<--\n", StrIng(&s));
    Ret ret = 0;
CLEAN:
    // De()
    return ret;
FAILED:
    ret = -1;
    goto CLEAN;
}

int main(void) {
    CHECK(StrInit(&s, " xxx: "));

    CHECK(t0());

    Ret ret = 0;
CLEAN:
    StrDe(&s);
    return ret;
FAILED:
    ret = -1;
    goto CLEAN;
}