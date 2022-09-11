// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef CLR_H
#define CLR_H

#include <lt/fwd.h>

#include "common.h"

enum {
	CLR_LINENUM,
	CLR_LINENUM_UFLOW,
	CLR_LINENUM_SEL,

	CLR_HEADER_TAB,
	CLR_HEADER_BG,

	CLR_EDITOR,
	CLR_EDITOR_SEL,

	CLR_SYNTAX_UNKNOWN,

	CLR_SYNTAX_STRING,
	CLR_SYNTAX_CHAR,
	CLR_SYNTAX_NUMBER,

	CLR_SYNTAX_IDENTIFIER,
	CLR_SYNTAX_KEYWORD,
	CLR_SYNTAX_COMMENT,
	CLR_SYNTAX_DATATYPE,

	CLR_SYNTAX_HASH,
	CLR_SYNTAX_OPERATOR,
	CLR_SYNTAX_PUNCTUATION,

	CLR_SYNTAX_FUNCTION,

	CLR_NOTIFY_ERROR,

	CLR_LIST_HEAD,
	CLR_LIST_ENTRY,
	CLR_LIST_HIGHL,

	CLR_SYNTAX_TRAIL_INDENT,

	CLR_COUNT
};
extern char clr_strs[CLR_COUNT][128];

void clr_load(lt_conf_t* clr_conf);

#endif
