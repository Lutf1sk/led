// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef ALIGNMENT_H
#define ALIGNMENT_H 1

#include "common.h"

static inline INLINE
b8 is_pow2(usz n) {
	return (n & (n - 1)) == 0;
}

static inline INLINE
usz pad(usz size, usz align) {
	usz rem = size % align;
	if (!rem)
		return 0;
	return align - rem;
}

static inline INLINE
usz align_fwd(usz val, usz align) {
	return val + pad(val, align);
}

static inline INLINE
usz align_bwd(usz val, usz align) {
	return val - val % align;
}

#endif
