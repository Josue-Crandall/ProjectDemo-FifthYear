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

static Ret repaintPresent() {
    CHECK(fileLock(presentPath, 0));
    CHECK(filePTrunc(presentPath, 0));
    for(TrackerGen gen = TrackerBeg(ptrack); TrackerNext(&gen); NOP) {
        CHECK(fileAppend(presentPath, (u8 *R())gen.keys[gen.i], strlen(gen.keys[gen.i])));
    }
    Ret ret = 0;
CLEAN:
    if(fileUnlock(presentPath)) { DEBUG_LOG("Failed to unlock presentPath\n"); }
    return ret;
FAILED:
    ret = -1;
    goto CLEAN;
}
static Ret repaintTyping() {
    CHECK(fileLock(typingPath, 0));
    CHECK(filePTrunc(typingPath, 0));
    for(TrackerGen gen = TrackerBeg(ttrack); TrackerNext(&gen); NOP) {
        CHECK(fileAppend(typingPath, (u8 *R())gen.keys[gen.i], strlen(gen.keys[gen.i])));
        CHECK(fileAppend(typingPath, TYPING_MSG, TYPING_MSG_LEN));
    }
    Ret ret = 0;
CLEAN:
    if(fileUnlock(typingPath)) { DEBUG_LOG("Failed to unlock typingPath\n"); }
    return ret;
FAILED:
    ret = -1;
    goto CLEAN;
}
static Ret addPresent(Buff *name) {
    char *newStr = 0;
    CHECK(BuffPush(name, '\n')); CHECK(BuffPush(name, 0));

    time_t *key = TrackerAt(ptrack, (char *R())BuffData(name), 0);
    if(key) { *key = PRESENT_DURATION_SECONDS; }
    else {
        NULL_CHECK(newStr = malloc(BuffSize(name)));
        memcpy(newStr, BuffData(name), BuffSize(name));
        CHECK(TrackerPush(ptrack, newStr, PRESENT_DURATION_SECONDS, 0));
    }

    CHECK(repaintPresent());
    
    return 0;
FAILED:
    free(newStr);
    return -1;
}
static Ret rmPresent(Buff *name) {
    CHECK(BuffPush(name, '\n')); CHECK(BuffPush(name, 0));
    TrackerRm(ptrack, (char *R())BuffData(name), 0);
    CHECK(repaintPresent());
    return 0;
FAILED:
    return -1;
}
static Ret addTyping(Buff *name) {
    char *newStr = 0;
    CHECK(BuffPush(name, 0));

    time_t *key = TrackerAt(ttrack, (char *R())BuffData(name), 0);
    if(key) { *key = TYPING_DURATION_SECONDS; }
    else {
        NULL_CHECK(newStr = malloc(BuffSize(name)));
        memcpy(newStr, BuffData(name), BuffSize(name));
        CHECK(TrackerPush(ttrack, newStr, TYPING_DURATION_SECONDS, 0));
    }

    CHECK(repaintTyping());

    return 0;
FAILED:
    free(newStr);
    return -1;
}
static Ret rmTyping(Buff *name) {
    CHECK(BuffPush(name, 0));
    TrackerRm(ttrack, (char *R())BuffData(name), 0);
    CHECK(repaintTyping());
    return 0;
FAILED:
    return -1;
}
static Ret trackerPassTime() {
    time_t newTime = time(0);
    time_t delta = newTime - currentTime;
    
    currentTime = newTime;

    if(delta) {
        Ret changed = 0;
        for(TrackerGen gen = TrackerBeg(ptrack); TrackerNext(&gen); NOP) {
            if(gen.vals[gen.i] <= delta) { TrackerRm(ptrack, gen.keys[gen.i], 0); changed = 1; }
            else { gen.vals[gen.i] -= delta; }
        }
        if(changed) { repaintPresent(); }

        changed = 0;
        for(TrackerGen gen = TrackerBeg(ttrack); TrackerNext(&gen); NOP) {
            printf("TYPING REMAINING %zu\n", gen.vals[gen.i]);
            
            if(gen.vals[gen.i] <= delta) { TrackerRm(ttrack, gen.keys[gen.i], 0); changed = 1; }
            else { gen.vals[gen.i] -= delta; }
        }
        if(changed) { printf("TYPING CHANGED\n"); repaintTyping(); }
    }

    return 0;
FAILED:
    return -1;
}