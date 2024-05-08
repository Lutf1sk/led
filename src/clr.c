// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "clr.h"
#include "draw.h"

#include <lt/conf.h>
#include <lt/io.h>

u32 clr_attr[CLR_COUNT] = {
	[CLR_LINENUM]       = ATTR_BG_BRIGHT | ATTR_BG_BLK | ATTR_FG_WHT,
	[CLR_LINENUM_UFLOW] = ATTR_BG_BRIGHT | ATTR_BG_BLK | ATTR_FG_WHT,
	[CLR_LINENUM_SEL]   = ATTR_BG_WHT | ATTR_FG_BLK,

	[CLR_HEADER_TAB] = ATTR_BG_WHT | ATTR_FG_BLK,
	[CLR_HEADER_BG]  = ATTR_BG_BLK | ATTR_BG_BRIGHT,

	[CLR_EDITOR]     = ATTR_FG_WHT | ATTR_BG_BLK,
	[CLR_EDITOR_SEL] = ATTR_FG_BLK | ATTR_BG_CYN,

	[CLR_SYNTAX_UNKNOWN] = ATTR_FG_WHT | ATTR_BG_BLK,

	[CLR_SYNTAX_STRING] = ATTR_FG_YLW | ATTR_BG_BLK,
	[CLR_SYNTAX_CHAR]   = ATTR_FG_YLW | ATTR_BG_BLK,
	[CLR_SYNTAX_NUMBER] = ATTR_FG_WHT | ATTR_BG_BLK,

	[CLR_SYNTAX_IDENTIFIER] = ATTR_FG_WHT | ATTR_BG_BLK | ATTR_FG_BRIGHT,
	[CLR_SYNTAX_KEYWORD]    = ATTR_FG_YLW | ATTR_BG_BLK | ATTR_FG_BRIGHT,
	[CLR_SYNTAX_COMMENT]    = ATTR_FG_GRN | ATTR_BG_BLK,
	[CLR_SYNTAX_DATATYPE]   = ATTR_FG_RED | ATTR_BG_BLK,

	[CLR_SYNTAX_HASH]        = ATTR_FG_WHT | ATTR_BG_BLK,
	[CLR_SYNTAX_OPERATOR]    = ATTR_FG_CYN | ATTR_BG_BLK,
	[CLR_SYNTAX_PUNCTUATION] = ATTR_FG_RED | ATTR_BG_BLK,

	[CLR_SYNTAX_FUNCTION] = ATTR_BOLD | ATTR_FG_WHT | ATTR_BG_BLK | ATTR_FG_BRIGHT,

	[CLR_NOTIFY_ERROR] = ATTR_BOLD | ATTR_FG_BLK | ATTR_BG_RED,

	[CLR_LIST_HEAD]    = ATTR_FG_BLK | ATTR_BG_WHT,
	[CLR_LIST_ENTRY]   = ATTR_FG_WHT | ATTR_BG_BLK,
	[CLR_LIST_HIGHL]   = ATTR_FG_WHT | ATTR_BG_BLK | ATTR_BG_BRIGHT,
	[CLR_LIST_DIR]     = ATTR_FG_YLW | ATTR_BG_BLK | ATTR_FG_BRIGHT,
	[CLR_LIST_FILE]    = ATTR_FG_WHT | ATTR_BG_BLK | ATTR_FG_BRIGHT,
	[CLR_LIST_SYMLINK] = ATTR_FG_CYN | ATTR_BG_BLK,

	[CLR_SYNTAX_TRAIL_INDENT] = ATTR_BG_RED,
};

static
void load_clr(u32 clr, lt_conf_t* conf, lstr_t key) {
	conf = lt_conf_find(conf, key);
	if (!conf) {
		return;
	}

	b8 bold = lt_conf_find_bool_default(conf, CLSTR("bold"), 0);
	i64 fg = lt_conf_find_int_default(conf, CLSTR("fg"), 7);
	i64 bg = lt_conf_find_int_default(conf, CLSTR("bg"), 0);

	u32 attr = 0;
	if (bold) attr |= ATTR_BOLD;
	attr |= (fg & 0xF) << 16;
	attr |= (bg & 0xF) << 20;

	clr_attr[clr] = attr;
}

void clr_load(lt_conf_t* conf) {
	load_clr(CLR_LINENUM, conf, CLSTR("colors.linenum"));
	load_clr(CLR_LINENUM_UFLOW, conf, CLSTR("colors.linenum_underflow"));
	load_clr(CLR_LINENUM_SEL, conf, CLSTR("colors.linenum_selected"));

	load_clr(CLR_HEADER_TAB, conf, CLSTR("colors.header_label"));
	load_clr(CLR_HEADER_BG, conf, CLSTR("colors.header"));

	load_clr(CLR_EDITOR, conf, CLSTR("colors.editor"));
	load_clr(CLR_EDITOR_SEL, conf, CLSTR("colors.editor_selection"));

	load_clr(CLR_SYNTAX_UNKNOWN, conf, CLSTR("colors.syntax_unknown"));

	load_clr(CLR_SYNTAX_STRING, conf, CLSTR("colors.syntax_string"));
	load_clr(CLR_SYNTAX_CHAR, conf, CLSTR("colors.syntax_char"));
	load_clr(CLR_SYNTAX_NUMBER, conf, CLSTR("colors.syntax_number"));

	load_clr(CLR_SYNTAX_IDENTIFIER, conf, CLSTR("colors.syntax_identifier"));
	load_clr(CLR_SYNTAX_KEYWORD, conf, CLSTR("colors.syntax_keyword"));
	load_clr(CLR_SYNTAX_COMMENT, conf, CLSTR("colors.syntax_comment"));
	load_clr(CLR_SYNTAX_DATATYPE, conf, CLSTR("colors.syntax_datatype"));

	load_clr(CLR_SYNTAX_HASH, conf, CLSTR("colors.syntax_hash"));
	load_clr(CLR_SYNTAX_OPERATOR, conf, CLSTR("colors.syntax_operator"));
	load_clr(CLR_SYNTAX_PUNCTUATION, conf, CLSTR("colors.syntax_punctuation"));

	load_clr(CLR_SYNTAX_FUNCTION, conf, CLSTR("colors.syntax_function"));

	load_clr(CLR_NOTIFY_ERROR, conf, CLSTR("colors.error"));

	load_clr(CLR_LIST_HEAD, conf, CLSTR("colors.list_head"));
	load_clr(CLR_LIST_ENTRY, conf, CLSTR("colors.list_entry"));
	load_clr(CLR_LIST_HIGHL, conf, CLSTR("colors.list_highlight"));
	load_clr(CLR_LIST_DIR, conf, CLSTR("colors.list_directory"));
	load_clr(CLR_LIST_FILE, conf, CLSTR("colors.list_file"));
	load_clr(CLR_LIST_SYMLINK, conf, CLSTR("colors.list_symlink"));

	load_clr(CLR_SYNTAX_TRAIL_INDENT, conf, CLSTR("colors.syntax_trail_indent"));
}
