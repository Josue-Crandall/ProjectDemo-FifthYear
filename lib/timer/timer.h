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

#ifndef JC_TIMER_H
#define JC_TIMER_H
#include "../macros/macros.h"

#include <time.h>
#include <errno.h>

typedef struct Timer {
    struct timespec ts;
} Timer;

static Ret TimerStart(Timer *timer);
static Ret TimerStop(Timer *timer);
static void TimerDisplay(Timer *timer);

static Ret SleepMilisec(usize ms);

#include "imp.h"


#endif