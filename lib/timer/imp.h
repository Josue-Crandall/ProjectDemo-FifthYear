/*
    CC v.01 a language project.
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

#include <stdio.h>

static Ret TimerStart(Timer *timer) {
    CHECK(clock_gettime(CLOCK_MONOTONIC, &timer->ts));
    return 0;
FAILED:
    return -1;
}
static Ret TimerStop(Timer *timer) {
    struct timespec temp;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &temp));

    ull total = (ull)temp.tv_nsec - (ull)timer->ts.tv_nsec;
    if(temp.tv_nsec < timer->ts.tv_nsec) {
        temp.tv_sec -= 1;
        total += 1000000000llu;
    }
    timer->ts.tv_nsec = total;
    timer->ts.tv_sec = temp.tv_sec - timer->ts.tv_sec;
    
    return 0;
FAILED:
    return -1;
}
static void TimerDisplay(Timer *timer) {
    ull ms = (ull)timer->ts.tv_nsec / 1000000;
    ull ns = (ull)timer->ts.tv_nsec % 1000000;
    fprintf(stderr, "(%llus, %llums, %lluns)", (ull)timer->ts.tv_sec, ms, ns);
}
static Ret SleepMilisec(usize ms) {
    time_t s; 
    long ns;

    usize s_temp = ms / 1000;
    ns = (ms % 1000) * 1000000;
    time_t t_temp = s_temp;
    CHECK((ull)t_temp < (ull)s_temp);
    s = t_temp;
    
    struct timespec time_spec = { s, ns };
    struct timespec remainder;

    for(int res = 1; (res = nanosleep(&time_spec, &remainder)); NOP) {
        if(EINTR != errno) { goto FAILED; }
    }

    return 0;
FAILED:
    DEBUG_LOG("SleepMilisec has failed\n");
    return -1;
}


