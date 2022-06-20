// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "clr.h"
#include "common.h"
#include "conf.h"
#include "allocators.h"

#include <string.h>

char* clr_strs[CLR_COUNT];

char* conf_clr_default(aframe_t* arena, conf_t* conf, lstr_t key, char* default_) {
	conf = conf_find(conf, key, CONF_OBJECT);
	if (!conf)
		return default_;

	char* str = aframe_reserve(arena, 0);
	char* it = str;

	*it++ = 0x1B;
	*it++ = '[';
	it += sprintf(it, "%s", conf_find_bool_default(conf, CLSTR("bold"), 0) ? "1" : "22");
	*it++ = ';';
	it += sprintf(it, "%ld", conf_find_int_default(conf, CLSTR("fg"), 39));
	*it++ = ';';
	it += sprintf(it, "%ld", conf_find_int_default(conf, CLSTR("bg"), 49));
	*it++ = 'm';
	*it++ = 0;

	return aframe_reserve(arena, it - str);
}

void clr_load(aframe_t* arena, conf_t* conf) {
	// Generate color
	clr_strs[CLR_LINENUM] 		= conf_clr_default(arena, conf, CLSTR("linenum"),			"\x1B[22;37;100m");
	clr_strs[CLR_LINENUM_SEL]	= conf_clr_default(arena, conf, CLSTR("linenum_selected"),	"\x1B[22;30;47m");
	clr_strs[CLR_LINENUM_UFLOW]	= conf_clr_default(arena, conf, CLSTR("linenum_underflow"),	"\x1B[22;37;100m");
	clr_strs[CLR_HEADER_TAB]	= conf_clr_default(arena, conf, CLSTR("header_label"),		"\x1B[22;30;47m");
	clr_strs[CLR_HEADER_BG]		= conf_clr_default(arena, conf, CLSTR("header"),			"\x1B[22;30;100m");
	clr_strs[CLR_EDITOR]		= conf_clr_default(arena, conf, CLSTR("editor"),			"\x1B[22;37;40m");
	clr_strs[CLR_EDITOR_SEL]	= conf_clr_default(arena, conf, CLSTR("editor_selection"),	"\x1B[22;30;46m");

	// Syntax highlighting
	clr_strs[CLR_SYNTAX_UNKNOWN]		= conf_clr_default(arena, conf, CLSTR("syntax_unknown"),	"\x1B[22;37;40m");

	clr_strs[CLR_SYNTAX_STRING]			= conf_clr_default(arena, conf, CLSTR("syntax_string"),		"\x1B[22;33;40m");
	clr_strs[CLR_SYNTAX_CHAR]			= conf_clr_default(arena, conf, CLSTR("syntax_char"),		"\x1B[22;33;40m");
	clr_strs[CLR_SYNTAX_NUMBER]			= conf_clr_default(arena, conf, CLSTR("syntax_number"),		"\x1B[22;37;40m");

	clr_strs[CLR_SYNTAX_IDENTIFIER]		= conf_clr_default(arena, conf, CLSTR("syntax_identifier"),	"\x1B[22;97;40m");
	clr_strs[CLR_SYNTAX_KEYWORD]		= conf_clr_default(arena, conf, CLSTR("syntax_keyword"),	"\x1B[22;93;40m");
	clr_strs[CLR_SYNTAX_COMMENT]		= conf_clr_default(arena, conf, CLSTR("syntax_comment"),	"\x1B[22;32;40m");
	clr_strs[CLR_SYNTAX_DATATYPE]		= conf_clr_default(arena, conf, CLSTR("syntax_datatype"),	"\x1B[22;31;40m");

	clr_strs[CLR_SYNTAX_HASH]			= conf_clr_default(arena, conf, CLSTR("syntax_hash"),		"\x1B[22;37;40m");
	clr_strs[CLR_SYNTAX_OPERATOR]		= conf_clr_default(arena, conf, CLSTR("syntax_operator"),	"\x1B[22;36;40m");
	clr_strs[CLR_SYNTAX_PUNCTUATION]	= conf_clr_default(arena, conf, CLSTR("syntax_punctuation"),"\x1B[22;31;40m");

	clr_strs[CLR_SYNTAX_FUNCTION]		= conf_clr_default(arena, conf, CLSTR("syntax_function"),	"\x1B[1;97;40m");

	clr_strs[CLR_SYNTAX_TRAIL_INDENT]	= conf_clr_default(arena, conf, CLSTR("syntax_trail_indent"),"\x1B[22;30;41m");

	// Notification
	clr_strs[CLR_NOTIFY_ERROR]	= conf_clr_default(arena, conf, CLSTR("error"), "\x1B[1;30;41m");

	// File browser
	clr_strs[CLR_LIST_HEAD]		= conf_clr_default(arena, conf, CLSTR("list_head"),			"\x1B[22;30;47m");
	clr_strs[CLR_LIST_ENTRY]	= conf_clr_default(arena, conf, CLSTR("list_entry"),		"\x1B[22;37;40m");
	clr_strs[CLR_LIST_HIGHL]	= conf_clr_default(arena, conf, CLSTR("list_highlighted"),	"\x1B[22;37;100m");
}

