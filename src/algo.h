// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef ALGO_H
#define ALGO_H 1

static inline INLINE
isz min(isz v1, isz v2) {
	return v1 < v2 ? v1 : v2;
}

static inline INLINE
isz max(isz v1, isz v2) {
	return v1 > v2 ? v1 : v2;
}

static inline INLINE
isz clamp(isz v, isz min, isz max) {
	return v < min ? min : v > max ? max : v;
}

#endif
