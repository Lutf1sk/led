// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef TOKEN_CHARS_H
#define TOKEN_CHARS_H 1

#include "chartypes.h"

static inline INLINE
b8 is_numeric_head(char c) {
	return is_digit(c);
}

static inline INLINE
b8 is_numeric_body(char c) {
	return is_numeric_head(c) || c == '.' || is_alpha(c);
}

static inline INLINE
b8 is_ident_head(char c) {
	return is_alpha(c) || c == '_';
}

static inline INLINE
b8 is_ident_body(char c) {
	return is_ident_head(c) || is_digit(c);
}

#endif