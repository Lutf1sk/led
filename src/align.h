// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef ALIGNMENT_H
#define ALIGNMENT_H 1

#include "common.h"

#define WORD_SIZE (sizeof(usz)<<1)
#define WORD_MASK (WORD_SIZE-1)

static inline INLINE
b8 is_pow2(usz n) {
	return (n & (n - 1)) == 0;
}

static inline INLINE
usz pad(usz size, usz align) {
	usz align_mask = align - 1;
	usz n = size & align_mask;
	n ^= align_mask;
	return n + 1;
}

static inline INLINE
usz word_align_fwd(usz val) {
	return (val + (WORD_MASK)) & ~(WORD_MASK);
}

static inline INLINE
usz word_align_bwd(usz val) {
	return val & ~WORD_MASK;
}

static inline INLINE
usz align_fwd(usz val, usz align) {
	usz mask = align - 1;
	return (val + mask) & ~mask;
}

static inline INLINE
usz align_bwd(usz val, usz align) {
	return val & ~(align - 1);
}

#endif
