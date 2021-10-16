// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H 1

#include "common.h"

typedef
enum highl_mode {
	HLM_UNKNOWN,

	HLM_INDENT,

	HLM_STRING,
	HLM_CHAR,
	HLM_NUMBER,

	HLM_IDENTIFIER,
	HLM_KEYWORD,
	HLM_COMMENT,
	HLM_DATATYPE,
	HLM_FUNCTION,

	HLM_HASH,
	HLM_BRACKET,
	HLM_OPERATOR,
	HLM_PUNCTUATION,
} highl_mode_t;

typedef
struct highl {
	highl_mode_t mode;
	usz len;
	struct highl* next;
} highl_t;

typedef struct aframe aframe_t;
typedef struct doc doc_t;

highl_t** highl_generate(aframe_t* arena, doc_t* doc);

#endif
