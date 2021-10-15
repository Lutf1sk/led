// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef CHARTYPES_H
#define CHARTYPES_H 1

#include "common.h"

static inline INLINE
b8 is_digit(char c) {
	return c >= '0' && c <= '9';
}

static inline INLINE
b8 is_upper(char c) {
	return c >= 'A' && c <= 'Z';
}

static inline INLINE
b8 is_lower(char c) {
	return c >= 'a' && c <= 'z';
}

static inline INLINE
char to_upper(char c) {
	return c + (is_upper(c) ? c : c + ('a' - 'A'));
}

static inline INLINE
char to_lower(char c) {
	return c + (is_lower(c) ? c : c - ('a' - 'A'));
}

#endif
