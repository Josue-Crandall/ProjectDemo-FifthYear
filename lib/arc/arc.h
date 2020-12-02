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

#ifndef JC_ARC_H
#define JC_ARC_H
#include "../macros/macros.h"

#include <stdatomic.h>

// Note: Can't count on roll over due to least types being the only ones defined.
typedef atomic_size_t 	      Arc;
typedef atomic_uint_least8_t  Arc8;
typedef atomic_uint_least32_t Arc32;
typedef atomic_uint_least64_t Arc64;

//// Static initializer:
#define ARCX_INIT(VAL) ATOMIC_VAR_INIT(VAL)

static void ArcInit(Arc *arc, usize val)  { atomic_init(arc, val); }
static void Arc8Init(Arc *arc, u8 val) 	 { atomic_init(arc, val); }
static void Arc32Init(Arc *arc, u32 val)  { atomic_init(arc, val); }
static void Arc64Init(Arc *arc, u64 val)  { atomic_init(arc, val); }

static void ArcAdd(Arc *arc, usize val) { atomic_fetch_add_explicit(arc, val, memory_order_release); }
static void Arc8Add(Arc *arc, u8 val)   { atomic_fetch_add_explicit(arc, val, memory_order_release); }
static void Arc32Add(Arc *arc, u32 val) { atomic_fetch_add_explicit(arc, val, memory_order_release); }
static void Arc64Add(Arc *arc, u64 val) { atomic_fetch_add_explicit(arc, val, memory_order_release); }

static void ArcSub(Arc *arc, usize val) { atomic_fetch_sub_explicit(arc, val, memory_order_release); }
static void Arc8Sub(Arc *arc, u8 val)   { atomic_fetch_sub_explicit(arc, val, memory_order_release); }
static void Arc32Sub(Arc *arc, u32 val) { atomic_fetch_sub_explicit(arc, val, memory_order_release); }
static void Arc64Sub(Arc *arc, u64 val) { atomic_fetch_sub_explicit(arc, val, memory_order_release); }

// Returns value before | 'ing
static usize ArcOr(Arc *arc, usize val) { return atomic_fetch_or_explicit(arc, val, memory_order_acq_rel); }
static u8 Arc8Or(Arc *arc, u8 val) { return atomic_fetch_or_explicit(arc, val, memory_order_acq_rel); }
static u32 Arc32Or(Arc *arc, u32 val) { return atomic_fetch_or_explicit(arc, val, memory_order_acq_rel); }
static u64 Arc64Or(Arc *arc, u64 val) { return atomic_fetch_or_explicit(arc, val, memory_order_acq_rel); }

// Returns value before  & 'ing
static usize ArcAnd(Arc *arc, usize val) { return atomic_fetch_and_explicit(arc, val, memory_order_acq_rel); }
static u8 Arc8And(Arc *arc, u8 val) { return atomic_fetch_and_explicit(arc, val, memory_order_acq_rel); }
static u32 Arc32And(Arc *arc, u32 val) { return atomic_fetch_and_explicit(arc, val, memory_order_acq_rel); }
static u64 Arc64And(Arc *arc, u64 val) { return atomic_fetch_and_explicit(arc, val, memory_order_acq_rel); }

// Returns value before ^ ' ing
static usize ArcXor(Arc *arc, usize val) { return atomic_fetch_xor_explicit(arc, val, memory_order_acq_rel); }
static u8 Arc8Xor(Arc *arc, u8 val) { return atomic_fetch_xor_explicit(arc, val, memory_order_acq_rel); }
static u32 Arc32Xor(Arc *arc, u32 val) { return atomic_fetch_xor_explicit(arc, val, memory_order_acq_rel); }
static u64 Arc64Xor(Arc *arc, u64 val) { return atomic_fetch_xor_explicit(arc, val, memory_order_acq_rel); }

// Returns 1 when last out
static Ret ArcDec(Arc *arc) { return atomic_fetch_sub_explicit(arc, 1, memory_order_acq_rel) == 1; }
// Returns 1 on first dibs
static Ret ArcInc(Arc *arc) { return atomic_fetch_add_explicit(arc, 1, memory_order_acq_rel) == 0; }

static usize ArcRead(Arc *arc) { return atomic_load_explicit(arc, memory_order_acquire); }
static u8 Arc8Read(Arc *arc)    { return atomic_load_explicit(arc, memory_order_acquire); }
static u32 Arc32Read(Arc *arc)  { return atomic_load_explicit(arc, memory_order_acquire); }
static u64 Arc64Read(Arc *arc)  { return atomic_load_explicit(arc, memory_order_acquire); }

static void ArcWrite(Arc *arc, usize val) { atomic_store_explicit(arc, val, memory_order_release); }
static void Arc8Write(Arc *arc, u8 val)   { atomic_store_explicit(arc, val, memory_order_release); }
static void Arc32Write(Arc *arc, u32 val) { atomic_store_explicit(arc, val, memory_order_release); }
static void Arc64Write(Arc *arc, u64 val) { atomic_store_explicit(arc, val, memory_order_release); }

static usize ArcSwap(Arc *arc, usize val) { return atomic_exchange_explicit(arc, val, memory_order_acq_rel); }
static u8 Arc8Swap(Arc8 *arc, usize val) { return atomic_exchange_explicit(arc, val, memory_order_acq_rel); }
static u32 Arc32Swap(Arc32 *arc, usize val) { return atomic_exchange_explicit(arc, val, memory_order_acq_rel); }
static u64 Arc64Swap(Arc64 *arc, usize val) { return atomic_exchange_explicit(arc, val, memory_order_acq_rel); }

#endif