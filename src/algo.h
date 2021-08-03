// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef ALGO_H
#define ALGO_H 1

static inline INLINE
usz min(usz v1, usz v2) {
	return v1 < v2 ? v1 : v2;
}

static inline INLINE
usz max(usz v1, usz v2) {
	return v1 > v2 ? v1 : v2;
}

static inline INLINE
usz clamp(usz v, usz min, usz max) {
	return v < min ? min : v > max ? max : v;
}

static inline INLINE
isz imin(isz v1, isz v2) {
	return v1 < v2 ? v1 : v2;
}

static inline INLINE
isz imax(isz v1, isz v2) {
	return v1 > v2 ? v1 : v2;
}

static inline INLINE
isz iclamp(isz v, isz min, isz max) {
	return v < min ? min : v > max ? max : v;
}

#endif
