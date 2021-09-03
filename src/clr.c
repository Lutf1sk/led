// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "clr.h"
#include "common.h"
#include "conf.h"

#include <curses.h>

#include <string.h>

#define CLR_256_TO_1000(c) ((short)(((f64)c / 256.0f) * 1000.0f))
#define RGB_256_TO_1000(r, g, b) CLR_256_TO_1000(r), CLR_256_TO_1000(g), CLR_256_TO_1000(b)

short saved_colors[256][3];

void clr_push() {
	for (int i = 0; i < COLORS; ++i)
		color_content(i, &saved_colors[i][0], &saved_colors[i][1], &saved_colors[i][2]);
}

void clr_pop() {
	for (int i = 0; i < COLORS; ++i)
		init_color(i, saved_colors[i][0], saved_colors[i][1], saved_colors[i][2]);
}

void init_color_from_conf(int color, lstr_t key, conf_t* parent_conf) {
	conf_t* conf = conf_find(parent_conf, key, CONF_ARRAY);
	if (!conf)
		ferrf("Missing compatible setting for '%.*s'\n", (int)key.len, key.str);
	if (conf->child_count != 3)
		ferrf("Unknown color format in setting '%.*s'\n", (int)key.len, key.str);

	const char* clr_str[3] = {"Red", "Green", "Blue"};
	for (int i = 0; i < 3; i++) {
		if (conf->children[i].stype != CONF_INT)
			ferrf("Expected integer for option '%s' in '%.*s'\n", clr_str[i], (int)key.len, key.str);
	}

	init_color(color, RGB_256_TO_1000(conf->children[0].int_val, conf->children[1].int_val, conf->children[2].int_val));
}

void clr_load(conf_t* clr_conf) {
	ASSERT(clr_conf);

	if (!can_change_color() || COLOR_PAIRS < 256) {
		// Generate color pairs
		init_pair(PAIR_LINENUM, 0, 7);
		init_pair(PAIR_LINENUM_UFLOW, 0, 6);
		init_pair(PAIR_LINENUM_SEL, 0, 3);
		init_pair(PAIR_HEADER_TAB, 0, 7);
		init_pair(PAIR_HEADER_BG, 0, 7);
		init_pair(PAIR_EDITOR, 7, 0);

		// Syntax highlighting pairs
		init_pair(PAIR_SYNTAX_UNKNOWN, 7, 0);

		init_pair(PAIR_SYNTAX_STRING, 1, 0);
		init_pair(PAIR_SYNTAX_CHAR, 1, 0);
		init_pair(PAIR_SYNTAX_NUMBER, 7, 0);

		init_pair(PAIR_SYNTAX_IDENTIFIER, 7, 0);
		init_pair(PAIR_SYNTAX_KEYWORD, 3, 0);
		init_pair(PAIR_SYNTAX_COMMENT, 2, 0);
		init_pair(PAIR_SYNTAX_DATATYPE, 1, 0);

		init_pair(PAIR_SYNTAX_HASH, 7, 0);
		init_pair(PAIR_SYNTAX_BRACKET, 6, 0);
		init_pair(PAIR_SYNTAX_OPERATOR, 6, 0);
		init_pair(PAIR_SYNTAX_PUNCTUATION, 6, 0);

		init_pair(PAIR_SYNTAX_FUNCTION, 7, 0);

		init_pair(PAIR_SYNTAX_TRAIL_INDENT, 0, 1);


		// File browser pairs
		init_pair(PAIR_BROWSE_FILES_INPUT, 0, 3);
		init_pair(PAIR_BROWSE_FILES_ENTRY, 0, 3);
		init_pair(PAIR_BROWSE_FILES_SEL, 0, 7);
		return;
	}

	// Line numbers
	conf_t* linenum_cf = conf_find(clr_conf, CLSTR("linenum"), CONF_OBJECT);
	if (!linenum_cf)
		ferr("Missing required option 'linenum'\n");
	init_color_from_conf(CLR_LINENUM_BG, CLSTR("background"), linenum_cf);
	init_color_from_conf(CLR_LINENUM_FG, CLSTR("text"), linenum_cf);

	// Line numbers underflowed
	conf_t* linenum_uflow_cf = conf_find(clr_conf, CLSTR("linenum_underflow"), CONF_OBJECT);
	if (!linenum_uflow_cf)
		ferr("Missing required option 'linenum_underflow'\n");
	init_color_from_conf(CLR_LINENUM_UFLOW_BG, CLSTR("background"), linenum_uflow_cf);
	init_color_from_conf(CLR_LINENUM_UFLOW_FG, CLSTR("text"), linenum_uflow_cf);

	// Line numbers selected
	conf_t* linenum_sel_cf = conf_find(clr_conf, CLSTR("linenum_selected"), CONF_OBJECT);
	if (!linenum_sel_cf)
		ferr("Missing required option 'linenum_selected'\n");
	init_color_from_conf(CLR_LINENUM_SEL_BG, CLSTR("background"), linenum_sel_cf);
	init_color_from_conf(CLR_LINENUM_SEL_FG, CLSTR("text"), linenum_sel_cf);

	// Header
	conf_t* header_cf = conf_find(clr_conf, CLSTR("header"), CONF_OBJECT);
	if (!header_cf)
		ferr("Missing required option 'header'\n");
	init_color_from_conf(CLR_HEADER_BG, CLSTR("background"), header_cf);
	init_color_from_conf(CLR_HEADER_TAB_BG, CLSTR("tab_background"), header_cf);
	init_color_from_conf(CLR_HEADER_TAB_FG, CLSTR("text"), header_cf);

	// Editor
	conf_t* editor_cf = conf_find(clr_conf, CLSTR("editor"), CONF_OBJECT);
	if (!editor_cf)
		ferr("Missing required option 'editor'\n");
	init_color_from_conf(CLR_EDITOR_BG, CLSTR("background"), editor_cf);
	init_color_from_conf(CLR_EDITOR_FG, CLSTR("text"), editor_cf);

	// Generate color pairs
	init_pair(PAIR_LINENUM, CLR_LINENUM_FG, CLR_LINENUM_BG);
	init_pair(PAIR_LINENUM_UFLOW, CLR_LINENUM_UFLOW_FG, CLR_LINENUM_UFLOW_BG);
	init_pair(PAIR_LINENUM_SEL, CLR_LINENUM_SEL_FG, CLR_LINENUM_SEL_BG);
	init_pair(PAIR_HEADER_TAB, CLR_HEADER_TAB_FG, CLR_HEADER_TAB_BG);
	init_pair(PAIR_HEADER_BG, CLR_HEADER_TAB_FG, CLR_HEADER_BG);
	init_pair(PAIR_EDITOR, CLR_EDITOR_FG, CLR_EDITOR_BG);

	// Syntax highlighting colors
	conf_t* highl_cf = conf_find(clr_conf, CLSTR("highlight"), CONF_OBJECT);
	if (!highl_cf)
		ferr("Missing required option 'highl_cf'\n");
	init_color_from_conf(CLR_SYNTAX_UNKNOWN, CLSTR("unknown"), highl_cf);

	init_color_from_conf(CLR_SYNTAX_STRING, CLSTR("string"), highl_cf);
	init_color_from_conf(CLR_SYNTAX_CHAR, CLSTR("char"), highl_cf);
	init_color_from_conf(CLR_SYNTAX_NUMBER, CLSTR("number"), highl_cf);

	init_color_from_conf(CLR_SYNTAX_IDENTIFIER, CLSTR("identifier"), highl_cf);
	init_color_from_conf(CLR_SYNTAX_KEYWORD, CLSTR("keyword"), highl_cf);
	init_color_from_conf(CLR_SYNTAX_COMMENT, CLSTR("comment"), highl_cf);
	init_color_from_conf(CLR_SYNTAX_DATATYPE, CLSTR("datatype"), highl_cf);

	init_color_from_conf(CLR_SYNTAX_HASH, CLSTR("hash"), highl_cf);
	init_color_from_conf(CLR_SYNTAX_BRACKET, CLSTR("bracket"), highl_cf);
	init_color_from_conf(CLR_SYNTAX_OPERATOR, CLSTR("operator"), highl_cf);
	init_color_from_conf(CLR_SYNTAX_PUNCTUATION, CLSTR("punctuation"), highl_cf);

	init_color_from_conf(CLR_SYNTAX_FUNCTION, CLSTR("function"), highl_cf);

	init_color_from_conf(CLR_SYNTAX_TRAIL_INDENT, CLSTR("trailing_indent"), highl_cf);

	// Syntax highlighting pairs
	init_pair(PAIR_SYNTAX_UNKNOWN, CLR_SYNTAX_UNKNOWN, CLR_EDITOR_BG);

	init_pair(PAIR_SYNTAX_STRING, CLR_SYNTAX_STRING, CLR_EDITOR_BG);
	init_pair(PAIR_SYNTAX_CHAR, CLR_SYNTAX_CHAR, CLR_EDITOR_BG);
	init_pair(PAIR_SYNTAX_NUMBER, CLR_SYNTAX_NUMBER, CLR_EDITOR_BG);

	init_pair(PAIR_SYNTAX_IDENTIFIER, CLR_SYNTAX_IDENTIFIER, CLR_EDITOR_BG);
	init_pair(PAIR_SYNTAX_KEYWORD, CLR_SYNTAX_KEYWORD, CLR_EDITOR_BG);
	init_pair(PAIR_SYNTAX_COMMENT, CLR_SYNTAX_COMMENT, CLR_EDITOR_BG);
	init_pair(PAIR_SYNTAX_DATATYPE, CLR_SYNTAX_DATATYPE, CLR_EDITOR_BG);

	init_pair(PAIR_SYNTAX_HASH, CLR_SYNTAX_HASH, CLR_EDITOR_BG);
	init_pair(PAIR_SYNTAX_BRACKET, CLR_SYNTAX_BRACKET, CLR_EDITOR_BG);
	init_pair(PAIR_SYNTAX_OPERATOR, CLR_SYNTAX_OPERATOR, CLR_EDITOR_BG);
	init_pair(PAIR_SYNTAX_PUNCTUATION, CLR_SYNTAX_PUNCTUATION, CLR_EDITOR_BG);

	init_pair(PAIR_SYNTAX_FUNCTION, CLR_SYNTAX_FUNCTION, CLR_EDITOR_BG);

	init_pair(PAIR_SYNTAX_TRAIL_INDENT, CLR_EDITOR_FG, CLR_SYNTAX_TRAIL_INDENT);

	// Notification colors
	conf_t* notify_err_cf = conf_find(clr_conf, CLSTR("notify_error"), CONF_OBJECT);
	if (!notify_err_cf)
		ferr("Missing required option 'notify_error'\n");
	init_color_from_conf(CLR_NOTIFY_ERROR_BG, CLSTR("background"), notify_err_cf);
	init_color_from_conf(CLR_NOTIFY_ERROR_FG, CLSTR("text"), notify_err_cf);

	// Notification pairs
	init_pair(PAIR_NOTIFY_ERROR, CLR_NOTIFY_ERROR_FG, CLR_NOTIFY_ERROR_BG);

	// File browser colors
	conf_t* file_browser_cf = conf_find(clr_conf, CLSTR("file_browser"), CONF_OBJECT);
	if (!file_browser_cf)
		ferr("Missing required option 'file_browser'\n");
	init_color_from_conf(CLR_BROWSE_FILES_INPUT_BG, CLSTR("input_background"), file_browser_cf);
	init_color_from_conf(CLR_BROWSE_FILES_INPUT_FG, CLSTR("input_text"), file_browser_cf);
	init_color_from_conf(CLR_BROWSE_FILES_ENTRY_BG, CLSTR("entry_background"), file_browser_cf);
	init_color_from_conf(CLR_BROWSE_FILES_ENTRY_FG, CLSTR("entry_text"), file_browser_cf);
	init_color_from_conf(CLR_BROWSE_FILES_SEL_BG, CLSTR("selected_background"), file_browser_cf);
	init_color_from_conf(CLR_BROWSE_FILES_SEL_FG, CLSTR("selected_text"), file_browser_cf);

	// File browser pairs
	init_pair(PAIR_BROWSE_FILES_INPUT, CLR_BROWSE_FILES_INPUT_FG, CLR_BROWSE_FILES_INPUT_BG);
	init_pair(PAIR_BROWSE_FILES_ENTRY, CLR_BROWSE_FILES_ENTRY_FG, CLR_BROWSE_FILES_ENTRY_BG);
	init_pair(PAIR_BROWSE_FILES_SEL, CLR_BROWSE_FILES_SEL_FG, CLR_BROWSE_FILES_SEL_BG);
}

