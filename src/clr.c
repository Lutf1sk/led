// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "clr.h"

#include <lt/conf.h>
#include <lt/io.h>

char clr_strs[CLR_COUNT][128] = {
	[CLR_LINENUM] = "\x1B[22;37;100m",
	[CLR_LINENUM_UFLOW] = "\x1B[22;37;100m",
	[CLR_LINENUM_SEL] = "\x1B[22;30;47m",

	[CLR_HEADER_TAB] = "\x1B[22;30;47m",
	[CLR_HEADER_BG] = "\x1B[22;30;100m",

	[CLR_EDITOR] = "\x1B[22;37;40m",
	[CLR_EDITOR_SEL] = "\x1B[22;30;46m",

	[CLR_SYNTAX_UNKNOWN] = "\x1B[22;37;40m",

	[CLR_SYNTAX_STRING] = "\x1B[22;33;40m",
	[CLR_SYNTAX_CHAR] = "\x1B[22;33;40m",
	[CLR_SYNTAX_NUMBER] = "\x1B[22;37;40m",

	[CLR_SYNTAX_IDENTIFIER] = "\x1B[22;97;40m",
	[CLR_SYNTAX_KEYWORD] = "\x1B[22;93;40m",
	[CLR_SYNTAX_COMMENT] = "\x1B[22;32;40m",
	[CLR_SYNTAX_DATATYPE] = "\x1B[22;31;40m",

	[CLR_SYNTAX_HASH] = "\x1B[22;37;40m",
	[CLR_SYNTAX_OPERATOR] = "\x1B[22;36;40m",
	[CLR_SYNTAX_PUNCTUATION] = "\x1B[22;31;40m",

	[CLR_SYNTAX_FUNCTION] = "\x1B[1;97;40m",

	[CLR_NOTIFY_ERROR] = "\x1B[1;30;41m",

	[CLR_LIST_HEAD] = "\x1B[22;30;47m",
	[CLR_LIST_ENTRY] = "\x1B[22;37;40m",
	[CLR_LIST_HIGHL] = "\x1B[22;37;100m",

	[CLR_SYNTAX_TRAIL_INDENT] = "\x1B[22;30;41m",
};

static
void load_clr(u32 clr, lt_conf_t* conf, lstr_t key) {
	conf = lt_conf_find(conf, key);
	if (!conf)
		return;

	lt_str_printf(clr_strs[clr], "\x1B[%s;%iq;%iqm%c",
			lt_conf_bool(lt_conf_find(conf, CLSTR("bold")), 0) ? "1" : "22",
			lt_conf_int(lt_conf_find(conf, CLSTR("fg")), 39),
			lt_conf_int(lt_conf_find(conf, CLSTR("bg")), 49), 0);
}

void clr_load(lt_conf_t* conf) {
	load_clr(CLR_LINENUM, conf, CLSTR("linenum"));
	load_clr(CLR_LINENUM_UFLOW, conf, CLSTR("linenum_underflow"));
	load_clr(CLR_LINENUM_SEL, conf, CLSTR("linenum_selected"));

	load_clr(CLR_HEADER_TAB, conf, CLSTR("header_label"));
	load_clr(CLR_HEADER_BG, conf, CLSTR("header"));

	load_clr(CLR_EDITOR, conf, CLSTR("editor"));
	load_clr(CLR_EDITOR_SEL, conf, CLSTR("editor_selection"));

	load_clr(CLR_SYNTAX_UNKNOWN, conf, CLSTR("syntax_unknown"));

	load_clr(CLR_SYNTAX_STRING, conf, CLSTR("syntax_string"));
	load_clr(CLR_SYNTAX_CHAR, conf, CLSTR("syntax_char"));
	load_clr(CLR_SYNTAX_NUMBER, conf, CLSTR("syntax_number"));

	load_clr(CLR_SYNTAX_IDENTIFIER, conf, CLSTR("syntax_identifier"));
	load_clr(CLR_SYNTAX_KEYWORD, conf, CLSTR("syntax_keyword"));
	load_clr(CLR_SYNTAX_COMMENT, conf, CLSTR("syntax_comment"));
	load_clr(CLR_SYNTAX_DATATYPE, conf, CLSTR("syntax_datatype"));

	load_clr(CLR_SYNTAX_HASH, conf, CLSTR("syntax_hash"));
	load_clr(CLR_SYNTAX_OPERATOR, conf, CLSTR("syntax_operator"));
	load_clr(CLR_SYNTAX_PUNCTUATION, conf, CLSTR("syntax_punctuation"));

	load_clr(CLR_SYNTAX_FUNCTION, conf, CLSTR("syntax_function"));

	load_clr(CLR_NOTIFY_ERROR, conf, CLSTR("error"));

	load_clr(CLR_LIST_HEAD, conf, CLSTR("list_head"));
	load_clr(CLR_LIST_ENTRY, conf, CLSTR("list_entry"));
	load_clr(CLR_LIST_HIGHL, conf, CLSTR("list_highlighted"));

	load_clr(CLR_SYNTAX_TRAIL_INDENT, conf, CLSTR("syntax_trail_indent"));
}

