// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H 1

#include <lt/lt.h>
#include <lt/fwd.h>

typedef
enum hl_tk {
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
} hl_tk_t;

typedef
struct highl {
	u32 mode;
	u32 len;
	struct highl* next;
} highl_t;

typedef struct doc doc_t;

typedef
enum hl_mode {
	HL_C,
	HL_CPP,
	HL_CS,
	HL_ONYX,
	HL_LPC,
	HL_L,
	HL_RUST,
	HL_JS,
	HL_GIT_COMMIT,
	HL_UNKNOWN,
	HL_MAKEFILE,
	HL_BASH,
} hl_mode_t;

hl_mode_t hl_find_mode_by_name(lstr_t name);
hl_mode_t hl_find_mode_by_extension(lstr_t path);
void hl_register_extension(lstr_t extension, lstr_t mode_str);

highl_t** hl_generate(doc_t* doc, hl_mode_t mode, lt_arena_t* alloc);

highl_t** hl_generate_c(doc_t* doc, lt_arena_t* alloc);
highl_t** hl_generate_makefile(doc_t* doc, lt_arena_t* alloc);
highl_t** hl_generate_bash(doc_t* doc, lt_arena_t* alloc);
highl_t** hl_generate_cpp(doc_t* doc, lt_arena_t* alloc);
highl_t** hl_generate_rust(doc_t* doc, lt_arena_t* alloc);
highl_t** hl_generate_cs(doc_t* doc, lt_arena_t* alloc);
highl_t** hl_generate_onyx(doc_t* doc, lt_arena_t* alloc);
highl_t** hl_generate_lpc(doc_t* doc, lt_arena_t* alloc);
highl_t** hl_generate_l(doc_t* doc, lt_arena_t* alloc);
highl_t** hl_generate_js(doc_t* doc, lt_arena_t* alloc);

void hl_load(lt_conf_t* hl);

#endif
