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
	HLM_COUNT
} hl_tk_t;

typedef
struct highl {
	u32 mode;
	u32 len;
	struct highl* next;
} highl_t;

typedef struct doc doc_t;

#define FOR_EACH_HLMODE() \
	HLMODE_OP(C,          "c",          "// ", hl_generate_c          ) \
	HLMODE_OP(CPP,        "c++",        "// ", hl_generate_cpp        ) \
	HLMODE_OP(CS,         "c#",         "// ", hl_generate_cs         ) \
	HLMODE_OP(ONYX,       "onyx",       "// ", hl_generate_onyx       ) \
	HLMODE_OP(LPC,        "lpc",        "// ", hl_generate_lpc        ) \
	HLMODE_OP(L,          "l",          "> " , hl_generate_l          ) \
	HLMODE_OP(RUST,       "rust",       "// ", hl_generate_rust       ) \
	HLMODE_OP(JS,         "javascript", "// ", hl_generate_js         ) \
	HLMODE_OP(GIT_COMMIT, "git_commit", "# " , hl_generate_git_commit ) \
	HLMODE_OP(UNKNOWN,    "none",       "# " , hl_generate_unknown    ) \
	HLMODE_OP(MAKEFILE,   "makefile",   "# ",  hl_generate_makefile   ) \
	HLMODE_OP(BASH,       "bash",       "# ",  hl_generate_bash       )

typedef
enum modeid {
#define HLMODE_OP(ename, sname, comment, func) HL_##ename,
	FOR_EACH_HLMODE()
#undef HLMODE_OP
	HL_COUNT,
} modeid_t;

typedef
struct mode {
	lstr_t name;
	lstr_t comment_style;
	highl_t** (*generate_func)(doc_t*, lt_arena_t*);
} mode_t;

extern mode_t modes[HL_COUNT];

modeid_t hl_find_mode_by_name(lstr_t name);
modeid_t hl_find_mode_by_extension(lstr_t path);
void hl_register_extension(lstr_t extension, lstr_t mode_str);

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
