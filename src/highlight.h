// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H 1

#include <lt/lt.h>
#include <lt/fwd.h>

typedef
enum highl_tk {
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
} highl_tk_t;

typedef
struct highl {
	highl_tk_t mode;
	usz len;
	struct highl* next;
} highl_t;

typedef struct doc doc_t;

typedef
enum highl_mode {
	HL_C,
	HL_GIT_COMMIT,
	HL_UNKNOWN,
} highl_mode_t;

highl_mode_t hl_find_mode(lstr_t path);
highl_t** highl_generate(doc_t* doc, highl_mode_t mode, lt_alloc_t* alloc);
highl_t** highl_generate_c(doc_t* doc, lt_alloc_t* alloc);

#endif
