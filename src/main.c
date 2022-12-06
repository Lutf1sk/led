// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/io.h>
#include <lt/conf.h>
#include <lt/utf8.h>
#include <lt/mem.h>
#include <lt/term.h>
#include <lt/arg.h>

#include <stdio.h>
#include <stdlib.h>

#include "algo.h"
#include "clr.h"
#include "highlight.h"
#include "editor.h"
#include "file_browser.h"
#include "focus.h"

#include "draw.h"

global_t ed_globals;

char* get_highl(highl_t* node) {
	if (!node)
		return clr_strs[CLR_EDITOR];

	switch (node->mode) {
	case HLM_STRING: return clr_strs[CLR_SYNTAX_STRING];
	case HLM_CHAR: return clr_strs[CLR_SYNTAX_CHAR];
	case HLM_NUMBER: return clr_strs[CLR_SYNTAX_NUMBER];

	case HLM_IDENTIFIER: return clr_strs[CLR_SYNTAX_IDENTIFIER];
	case HLM_KEYWORD: return clr_strs[CLR_SYNTAX_KEYWORD];
	case HLM_COMMENT: return clr_strs[CLR_SYNTAX_COMMENT];
	case HLM_DATATYPE: return clr_strs[CLR_SYNTAX_DATATYPE];

	case HLM_FUNCTION: return clr_strs[CLR_SYNTAX_FUNCTION];

	case HLM_HASH: return clr_strs[CLR_SYNTAX_HASH];
	case HLM_OPERATOR: return clr_strs[CLR_SYNTAX_OPERATOR];
	case HLM_PUNCTUATION: return clr_strs[CLR_SYNTAX_PUNCTUATION];

	case HLM_INDENT:
		if (!node->next)
			return clr_strs[CLR_SYNTAX_TRAIL_INDENT];
		else
			return clr_strs[CLR_EDITOR];
		break;

	default:
		return clr_strs[CLR_SYNTAX_UNKNOWN];
	}
}

void draw_header(editor_t* ed) {
	rec_goto(0, 0);
	rec_clearline(clr_strs[CLR_HEADER_BG]);
	rec_str(clr_strs[CLR_HEADER_TAB]);
	rec_c(' ');
	if (ed) {
		rec_lstr(ed->doc.path.str, ed->doc.path.len);
	 	if (ed->doc.unsaved)
			rec_c('*');
		if (ed->doc.new)
			rec_str("[NEW]");
		if (ed->doc.read_only)
			rec_str("[RO]");
	}
	else
		rec_str("No file selected");
	rec_c(' ');
}

void draw_editor(editor_t* ed) {
	// Get selection coordinates and make y relative to line_top
	isz sel_start_y, sel_start_x, sel_end_y, sel_end_x;
	ed_get_selection(ed, &sel_start_y, &sel_start_x, &sel_end_y, &sel_end_x);
	sel_start_y -= ed->line_top;
	sel_end_y -= ed->line_top;

	isz line_top = ed->line_top;
	isz line_count = clamp(ed->doc.line_count - ed->line_top, 0, EDITOR_HEIGHT);

	usz tab_size = ed->global->tab_size;

	char line_num_buf[32];

	for (isz i = 0; i < line_count; ++i) {
		rec_goto(0, EDITOR_VSTART + i + 1);

		lstr_t line = ed->doc.lines[line_top + i];
		highl_t* hl = NULL;
		if (ed->hl_lines)
			hl = ed->hl_lines[line_top + i];

		isz linenum = i + line_top + 1;
		if (ed->global->relative_linenums)
			linenum -= ed->cy + 1;
		linenum %= 10000;
		if (i == ed->cy - line_top)
			linenum = (line_top + i + 1) % 10000;
		sprintf(line_num_buf, "%4zi ", linenum);

		u8 sel = (i >= sel_start_y) && (i <= sel_end_y);
		rec_str(clr_strs[sel ? CLR_LINENUM_SEL : CLR_LINENUM]);
		rec_str(line_num_buf);

		sel = (i > sel_start_y) && (i <= sel_end_y);

		if (sel)
			rec_str(clr_strs[CLR_EDITOR_SEL]);
		else
			rec_str(get_highl(hl));

		isz next_hl = -1;
		if (hl)
			next_hl = hl->len;

		for (isz j = 0, scr_x = 0; j < line.len && scr_x < EDITOR_WIDTH;) {
			if (!sel && i == sel_start_y && j == sel_start_x) {
				rec_str(clr_strs[CLR_EDITOR_SEL]);
				sel = 1;
			}
			if (sel && i == sel_end_y && j == sel_end_x) {
				rec_str(get_highl(hl));
				sel = 0;
			}

			while (hl && j >= next_hl) {
				hl = hl->next;
				if (!sel)
					rec_str(get_highl(hl));
				next_hl = j + hl->len;
			}

			char c = line.str[j];
			if (c == '\t') {
				isz tab_len = tab_size - (scr_x % tab_size);
				scr_x += tab_len;
				++j;
				rec_nc(tab_len, ' ');
			}
			else {
				++scr_x;
				usz end = j + lt_utf8_decode_len(c);
				while (j < end)
					rec_c(line.str[j++]);
			}
		}

		for (isz i = line_count; i < EDITOR_HEIGHT; ++i) {
			rec_goto(1, EDITOR_VSTART + i + 1);
			rec_clearline(clr_strs[CLR_EDITOR]);
			rec_str(clr_strs[CLR_LINENUM_UFLOW]);
			rec_str("     ");
		}
		rec_str(clr_strs[CLR_EDITOR]);
	}

	rec_goto(ed_cx_to_screen_x(ed, ed->cx, ed->cy) + EDITOR_HSTART + 1, ed->cy - line_top + EDITOR_VSTART + 1);
}

static
const lstr_t conf_subpath = CLSTR("/.config/led/led.conf");

lstr_t get_config_path(void) {
	const char* home_dir;
	if (!(home_dir = getenv("HOME")))
		return NLSTR();

	usz home_len = strlen(home_dir);
	if (home_len + conf_subpath.len + 1 >= PATH_MAX_LEN)
		ferr("Wtf, why is your home path that long?\n");

	static char path_buf[PATH_MAX_LEN] = "";
	memcpy(path_buf, home_dir, home_len);
	memcpy(path_buf + home_len, conf_subpath.str, conf_subpath.len);

	return LSTR(path_buf, home_len + conf_subpath.len);
}

void cleanup(int code, void* args) {
	editor_t* ed_it;
	while ((ed_it = fb_first_file()))
		fb_close(ed_it);

	lt_term_restore();
}

void on_exit(void*, void*);

int main(int argc, char** argv) {
// 	u32 c = 0;
// 	while (c != ('D' | LT_TERM_MOD_CTRL)) {
// 		c = lt_term_getkey();
// 		printf("Key 0x%X, Mod 0x%X\n", c & LT_TERM_KEY_MASK, c & LT_TERM_MOD_MASK);
// 	}
// 	lt_term_restore();
// 	return 0;

	lstr_t cpath = NLSTR();

	lt_arg_iterator_t arg_it = lt_arg_iterator_create(argc, argv);
	while (lt_arg_next(&arg_it)) {
		if (lt_arg_flag(&arg_it, 'h', CLSTR("help"))) {
			lt_printf(
				"usage: led [OPTIONS] FILE...\n"
				"options:\n"
				"  -h, --help           Display this information.\n"
				"  -c, --config=CONFIG  Use a custom config path located at CONFIG.\n"
			);
			return 0;
		}

		if (lt_arg_lstr(&arg_it, 'c', CLSTR("config"), &cpath))
			continue;

		fb_open(&ed_globals, LSTR(*arg_it.it, arg_it.arg_len));
	}

	if (!cpath.str)
		cpath = get_config_path();

	lt_arena_t* arena = lt_amcreate(NULL, LT_GB(1), 0);
	lt_alloc_t* alloc = (lt_alloc_t*)arena;

	memset(&ed_globals, 0, sizeof(ed_globals));

	lstr_t conf_file;
	if (!lt_file_read_entire(cpath, &conf_file, alloc))
		ferr("Failed to read config file\n");
	lt_conf_t config;
	if (!lt_conf_parse(&config, conf_file))
		ferr("Failed to parse config file\n");

	ed_globals.scroll_offs = lt_conf_find_int_default(&config, CLSTR("editor.scroll_offset"), 2);
	ed_globals.tab_size = lt_conf_find_int_default(&config, CLSTR("editor.tab_size"), 4);
	ed_globals.vstep = lt_conf_find_int_default(&config, CLSTR("editor.vstep"), 2);
	ed_globals.vstep_timeout_ms = lt_conf_find_int_default(&config, CLSTR("editor.vstep_timeout_ms"), 250);
	ed_globals.predict_brackets = lt_conf_find_bool_default(&config, CLSTR("editor.predict_brackets"), 0);
	ed_globals.relative_linenums = lt_conf_find_bool_default(&config, CLSTR("editor.relative_linenums"), 0);

	clr_load(&config);

	lt_conf_free(&config);

	write_buf = lt_malloc(alloc, LT_MB(4));
	ed_globals.hl_arena = arena;
	ed_globals.hl_restore = lt_amsave(arena);

	lt_term_init(LT_TERM_BPASTE | LT_TERM_ALTBUF | LT_TERM_MOUSE);
	on_exit(cleanup, NULL);

	edit_file(&ed_globals, fb_first_file());

	for (;;) {
		ed_globals.width = EDITOR_WIDTH;
		ed_globals.height = EDITOR_HEIGHT;
		ed_globals.hstart = EDITOR_HSTART;
		ed_globals.vstart = EDITOR_VSTART;

		// Update highlighting and redraw all windows.
		if (!ed_globals.await_utf8) {
			write_it = write_buf;

			rec_clear(clr_strs[CLR_EDITOR]);
			draw_header(ed_globals.ed);

			if (ed_globals.ed)
				draw_editor(ed_globals.ed);
			if (focus.draw)
				focus.draw(&ed_globals, focus.draw_args);
			lt_term_write_direct(write_buf, write_it - write_buf);
		}

		// Get and handle input.
		u32 c = lt_term_getkey();

		switch (c) {
		case 'E' | LT_TERM_MOD_CTRL:
			notify_exit();
			break;

		case 'O' | LT_TERM_MOD_CTRL:
			browse_filesystem();
			break;

		default:
			if (focus.input)
				focus.input(&ed_globals, c);
			break;
		}
	}
	return 0;
}

