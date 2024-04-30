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

#define FOR_EACH_HLMODE() \
	HLMODE_OP(C,          "c",          "// ") \
	HLMODE_OP(CPP,        "c++",        "// ") \
	HLMODE_OP(CS,         "c#",         "// ") \
	HLMODE_OP(ONYX,       "onyx",       "// ") \
	HLMODE_OP(LPC,        "lpc",        "// ") \
	HLMODE_OP(L,          "l",          "> " ) \
	HLMODE_OP(RUST,       "rust",       "// ") \
	HLMODE_OP(JS,         "javascript", "// ") \
	HLMODE_OP(GIT_COMMIT, "git_commit", "# " ) \
	HLMODE_OP(UNKNOWN,    "none",       "# " ) \
	HLMODE_OP(MAKEFILE,   "makefile",   "# " ) \
	HLMODE_OP(BASH,       "bash",       "# " ) \

typedef
enum hl_mode {
#define HLMODE_OP(ename, sname, comment) HL_##ename,
	FOR_EACH_HLMODE()
#undef HLMODE_OP
} hl_mode_t;

hl_mode_t hl_find_mode_by_name(lstr_t name);
hl_mode_t hl_find_mode_by_extension(lstr_t path);
void hl_register_extension(lstr_t extension, lstr_t mode_str);

lstr_t comment_style_by_hlmode(hl_mode_t mode);

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
