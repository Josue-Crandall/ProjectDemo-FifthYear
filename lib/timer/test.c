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

#include "timer.h"

#include <unistd.h>

int main(void) {
    Timer t1 ,t2;    

    TimerStart(&t1);
    sleep(1);
    TimerStop(&t1);

    TimerStart(&t2);
    sleep(2);
    TimerStop(&t2);

    TimerDisplay(&t1); printf("\n");
    TimerDisplay(&t2); printf("\n");

    return 0;
}
