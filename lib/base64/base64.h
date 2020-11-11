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

#ifndef JC_BASE64_H
#define JC_BASE64_H
#include "../macros/macros.h"

#include "../api/api.h"

#define DECB64(TYPE)                                                    \
Ret TYPE##BinToB64(TYPE *dst, u8 *src, usize len, Ret skipPadding);     \
Ret TYPE##B64ToBin(TYPE *dst, u8 *src, usize len, Ret validAndPadless); \

#include "imp.h"

//// Instantiations
DDB64(Buff);
DDB64(Smem);

#endif
