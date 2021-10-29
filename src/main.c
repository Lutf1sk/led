// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <stdlib.h>
#include <string.h>

#include "fhl.h"
#include "algo.h"
#include "conf.h"
#include "allocators.h"
#include "clr.h"
#include "highlight.h"
#include "editor.h"
#include "file_browser.h"
#include "focus.h"
#include "utf8.h"

#include "draw.h"

editor_t* ed = NULL;

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
		rec_str(ed->doc.path);
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
		highl_t* hl = ed->highl_lines[line_top + i];

		sprintf(line_num_buf, "%4zu ", (line_top + i + 1) % 10000);
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

			while (j >= next_hl) {
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
				usz end = j + utf8_decode_len(c);
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

FILE* fopen_config(void) {
	const char* home_dir;
	if (!(home_dir = getenv("HOME")))
		return NULL;

	usz home_len = strlen(home_dir);
	if (home_len + conf_subpath.len + 1 >= PATH_MAX_LEN)
		ferr("Wtf, why is your home path that long?\n");

	char path_buf[PATH_MAX_LEN] = "";
	memcpy(path_buf, home_dir, home_len);
	memcpy(path_buf + home_len, conf_subpath.str, conf_subpath.len);
	path_buf[home_len + conf_subpath.len] = 0;

	FILE* fp = fhl_fopen_r(path_buf);
	if (fp)
		return fp;

	return NULL;
}

void cleanup(int code, void* args) {
	editor_t* ed_it;
	while ((ed_it = fb_first_file()))
		fb_close(ed_it);

	lt_term_restore();
}

int main(int argc, char** argv) {
	lt_term_init(LT_TERM_BPASTE | LT_TERM_ALTBUF | LT_TERM_MOUSE);

// 	u32 c = 0;
// 	while (c != ('D' | LT_TERM_MOD_CTRL)) {
// 		c = lt_term_getkey();
// 		printf("Key 0x%X, Mod 0x%X\n", c & LT_TERM_KEY_MASK, c & LT_TERM_MOD_MASK);
// 	}
// 	lt_term_restore();
// 	return 0;

	allocators_initialize();

	// Read config
	FILE* conf_fp = fopen_config();
	if (!conf_fp)
		ferr("No config file available in default locations\n");

	usz conf_len = fhl_fsize(conf_fp);
	char* conf_data = malloc(conf_len);
	if (!conf_data)
		ferrf("Memory allocation failed: %s\n", os_err_str());
	fhl_fread(conf_fp, conf_data, conf_len);

	conf_t config = conf_parse(LSTR(conf_data, conf_len));

	conf_t* editor_cf = conf_find(&config, CLSTR("editor"), CONF_OBJECT);
	if (!editor_cf)
		ferr("Missing required option 'editor'\n");

	ed_globals.scroll_offs = conf_find_int_default(editor_cf, CLSTR("scroll_offset"), 2);
	ed_globals.tab_size = conf_find_int_default(editor_cf, CLSTR("tab_size"), 4);
	ed_globals.vstep = conf_find_int_default(editor_cf, CLSTR("vstep"), 4);
	ed_globals.predict_brackets = conf_find_bool_default(editor_cf, CLSTR("predict_brackets"), 0);
	ed_globals.await_utf8 = 0;

	ed_globals.ed = &ed;

	conf_t* colors_cf = conf_find(&config, CLSTR("colors"), CONF_OBJECT);
	if (!colors_cf)
		ferr("No 'colors' object found in config file\n");
	clr_load(colors_cf);
	free(conf_data);

	// Load documents
	for (usz i = 1; i < argc; ++i)
		fb_open(&ed_globals, CLSTR(argv[i]));

	ed = fb_first_file();

	on_exit(cleanup, NULL);

	edit_file(&ed_globals, ed);

	aframe_t* write_arena = aframe_alloc(GB(1));
	write_buf = aframe_reserve(write_arena, 0);

	for (;;) {
		ed_globals.width = EDITOR_WIDTH;
		ed_globals.height = EDITOR_HEIGHT;
		ed_globals.hstart = EDITOR_HSTART;
		ed_globals.vstart = EDITOR_VSTART;

		// Update highlighting and redraw all windows.
		if (!ed_globals.await_utf8) {
			write_it = write_buf;

			rec_clear(clr_strs[CLR_EDITOR]);
			draw_header(ed);

			if (ed)
				draw_editor(ed);
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

