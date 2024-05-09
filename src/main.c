// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/io.h>
#include <lt/conf.h>
#include <lt/text.h>
#include <lt/mem.h>
#include <lt/term.h>
#include <lt/arg.h>
#include <lt/math.h>
#include <lt/str.h>

#define LT_ANSI_SHORTEN_NAMES 1
#include <lt/ansi.h>

#include <stdio.h>
#include <stdlib.h>

#include "clr.h"
#include "highlight.h"
#include "editor.h"
#include "file_browser.h"
#include "focus.h"
#include "clipboard.h"
#include "keybinds.h"
#include "notify.h"

#include "draw.h"

static u32 mode_clr_tab[HLM_COUNT];

u32 get_highl(highl_t* node) {
	if (!node) {
		return mode_clr_tab[HLM_UNKNOWN];
	}
	if (node->mode == HLM_INDENT) {
		return clr_attr[(!node->next ? CLR_SYNTAX_TRAIL_INDENT : CLR_EDITOR)];
	}
	return mode_clr_tab[node->mode];
}

void draw_header(editor_t* ed) {
	if (ed->doc) {
		char* unsaved   = ed->doc->unsaved   ? "*"     : "";
		char* new       = ed->doc->new       ? "[NEW]" : "";
		char* read_only = ed->doc->read_only ? "[RO]"  : "";

		lstr_t line = lt_lsbuild(lt_libc_heap, " %S%s%s%s ", ed->doc->path, unsaved, new, read_only);
		buf_writeln_utf8(0, clr_attr[CLR_HEADER_TAB], line);
		lt_hmfree(line.str);
	}

	else {
		buf_writeln_utf8(0, clr_attr[CLR_HEADER_TAB], CLSTR(" no file selected "));

	}
}

void draw_editor(editor_t* ed) {
	doc_t* doc = ed->doc;
	lt_texted_t* txed = &doc->ed;

	// Get selection coordinates and make y relative to line_top
	isz sel_start_y, sel_start_x, sel_end_y, sel_end_x;
	lt_texted_get_selection(txed, &sel_start_x, &sel_start_y, &sel_end_x, &sel_end_y);
	sel_start_y -= doc->line_top;
	sel_end_y -= doc->line_top;

	isz line_top = doc->line_top;
	isz line_count = lt_isz_clamp(lt_texted_line_count(txed) - doc->line_top, 0, EDITOR_HEIGHT);

	usz tab_size = ed->tab_size;

	char linenum_buf[32];

	u32 linenum_attr = ATTR_FG_WHT | ATTR_BG_BLK | ATTR_BG_BRIGHT;
	u32 linenum_uflow_attr = ATTR_FG_WHT | ATTR_BG_BLK | ATTR_BG_BRIGHT;
	u32 linenum_sel_attr = ATTR_BG_WHT | ATTR_BG_BLK;

	usz cy = txed->cursor_y;
	usz cx = txed->cursor_x;

	for (isz i = 0; i < line_count; ++i) {
		lstr_t line = lt_texted_line_str(txed, line_top + i);
		highl_t* hl = (ed->hl_lines ? ed->hl_lines[line_top + i] : NULL);

		buf_set_pos(i + EDITOR_VSTART, 0);

		isz linenum = i + line_top + 1;
		if (ed->relative_linenums) {
			linenum -= cy + 1;
		}
		linenum %= 10000;
		if (i == cy - line_top) {
			linenum = (line_top + i + 1) % 10000;
		}
		sprintf(linenum_buf, "%4zi ", lt_abs(linenum));

		u8 sel = (i >= sel_start_y) && (i <= sel_end_y);
		buf_write_utf8(sel ? linenum_sel_attr : linenum_attr, LSTR(linenum_buf, 5));

		sel = (i > sel_start_y) && (i <= sel_end_y);

		u32 attr = (sel ? clr_attr[CLR_EDITOR_SEL] : get_highl(hl));

		isz next_hl = (hl ? hl->len : -1);

		for (isz j = 0, scr_x = 0; j < line.len && scr_x < EDITOR_WIDTH;) {
			if (!sel && i == sel_start_y && j == sel_start_x) {
				attr = clr_attr[CLR_EDITOR_SEL];
				sel = 1;
			}
			if (sel && i == sel_end_y && j == sel_end_x) {
				attr = get_highl(hl);
				sel = 0;
			}

			while (hl && hl->next && j >= next_hl) {
				hl = hl->next;
				if (!sel) {
					attr = get_highl(hl);
				}
				next_hl = j + hl->len;
			}

			char c = line.str[j];
			if (c == '\t') {
				isz tab_len = tab_size - (scr_x % tab_size);
				scr_x += tab_len;
				++j;
				for (isz i = 0; i < tab_len; ++i) {
					buf_write_char(attr, ' ');
				}
			}
			else {
				++scr_x;

				u32 utf8_c;
				j += lt_utf8_decode(line.str + j, &utf8_c);
				buf_write_char(attr, utf8_c);
			}
		}
		buf_write_char(0, 0);
	}

	for (isz i = line_count; i < EDITOR_HEIGHT; ++i) {
		buf_writeln_utf8(i + EDITOR_VSTART, linenum_uflow_attr, CLSTR("     "));
	}

	usz sx = cursor_x_to_screen_x(ed, lt_texted_line_str(txed, txed->cursor_y), txed->cursor_x);
	buf_set_cursor(txed->cursor_y - ed->doc->line_top + EDITOR_VSTART, sx + EDITOR_HSTART);
}

#ifdef LT_LINUX
#	define HOME_ENV "HOME"
#	define CONF_SUBPATH ".config/led/led.conf"
#elif defined(LT_WINDOWS)
#	define HOME_ENV "USERPROFILE"
#	define CONF_SUBPATH "led.conf"
#endif

lstr_t get_config_path(void) {
	const char* home_dir;
	if (!(home_dir = getenv(HOME_ENV))) {
		return NLSTR();
	}

	lstr_t path;
	if (lt_aprintf(&path, lt_libc_heap, "%s/%s", home_dir, CONF_SUBPATH) < 0) {
		return NLSTR();
	}
	return path;
}

#define LT_ANSI_SHORTEN_NAMES 1
#include <lt/ansi.h>

#include <lt/rle.h>

int main(int argc, char** argv) {
	LT_DEBUG_INIT();

	lt_err_t err;

	lstr_t cpath = NLSTR();

	lt_darr(lstr_t) open_paths = lt_darr_create(lstr_t, 128, lt_libc_heap);

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

		if (lt_arg_lstr(&arg_it, 'c', CLSTR("config"), &cpath)) {
			continue;
		}

		lt_darr_push(open_paths, LSTR(*arg_it.it, arg_it.arg_len));
	}

	if (!cpath.str) {
		cpath = get_config_path();
	}

	editor_t editor;
	memset(&editor, 0, sizeof(editor_t));

	lt_arena_t* arena = lt_amcreate(NULL, LT_GB(1), 0);
	lt_alloc_t* alloc = (lt_alloc_t*)arena;
	void* restore = lt_amsave(arena);

	lstr_t conf_file;
	if ((err = lt_freadallp_utf8(cpath, &conf_file, alloc))) {
		lt_ferrf("failed to read config file '%S': %S\n", cpath, lt_err_str(err));
	}
	lt_conf_t config, *found;
	lt_conf_err_info_t conf_err;
	if ((lt_conf_parse(&config, conf_file.str, conf_file.len, &conf_err, alloc))) {
		lt_ferrf("failed to parse config file '%S': %S\n", cpath, conf_err.err_str);
	}

	editor.scroll_offs = lt_conf_find_int_default(&config, CLSTR("editor.scroll_offset"), 2);
	editor.tab_size = lt_max(1, lt_conf_find_int_default(&config, CLSTR("editor.tab_size"), 4));
	update_tab_size(editor.tab_size);
	editor.remove_trailing_indent = lt_conf_find_bool_default(&config, CLSTR("editor.remove_trailing_indent"), 1);
	editor.vstep = lt_conf_find_int_default(&config, CLSTR("editor.vstep"), 2);
	editor.vstep_timeout_ms = lt_conf_find_int_default(&config, CLSTR("editor.vstep_timeout_ms"), 250);
	editor.relative_linenums = lt_conf_find_bool_default(&config, CLSTR("editor.relative_linenums"), 0);
	editor.tabs_to_spaces = lt_conf_find_bool_default(&config, CLSTR("editor.tabs_to_spaces"), 0);

	shell_load(&config);

	clr_load(&config);
	mode_clr_tab[HLM_STRING]      = clr_attr[CLR_SYNTAX_STRING];
	mode_clr_tab[HLM_CHAR]        = clr_attr[CLR_SYNTAX_CHAR];
	mode_clr_tab[HLM_NUMBER]      = clr_attr[CLR_SYNTAX_NUMBER];
	mode_clr_tab[HLM_IDENTIFIER]  = clr_attr[CLR_SYNTAX_IDENTIFIER];
	mode_clr_tab[HLM_KEYWORD]     = clr_attr[CLR_SYNTAX_KEYWORD];
	mode_clr_tab[HLM_COMMENT]     = clr_attr[CLR_SYNTAX_COMMENT];
	mode_clr_tab[HLM_DATATYPE]    = clr_attr[CLR_SYNTAX_DATATYPE];
	mode_clr_tab[HLM_FUNCTION]    = clr_attr[CLR_SYNTAX_FUNCTION];
	mode_clr_tab[HLM_HASH]        = clr_attr[CLR_SYNTAX_HASH];
	mode_clr_tab[HLM_OPERATOR]    = clr_attr[CLR_SYNTAX_OPERATOR];
	mode_clr_tab[HLM_PUNCTUATION] = clr_attr[CLR_SYNTAX_PUNCTUATION];
	mode_clr_tab[HLM_UNKNOWN]     = clr_attr[CLR_SYNTAX_UNKNOWN];

	keybind_init();
	keybinds_load(lt_conf_find_array(&config, CLSTR("keybinds"), &found));
	hl_load(lt_conf_find_array(&config, CLSTR("highlight"), &found));

	lt_amrestore(arena, restore);

	editor.hl_arena = arena;
	editor.hl_restore = lt_amsave(arena);

	for (usz i = 0; i < lt_darr_count(open_paths); ++i) {
		fb_open(&editor, open_paths[i]);
	}

	lt_term_init(LT_TERM_BPASTE | LT_TERM_ALTBUF | LT_TERM_MOUSE | LT_TERM_UTF8);

	buf_resize(lt_term_width, lt_term_height);

	clipboard_init();
	focus_init();
	find_local_init();
	notify_init();

	edit_file(&editor, fb_first_file());

	lstr_t notification = NLSTR();

	for (;;) {
		editor.width = EDITOR_WIDTH;
		editor.height = EDITOR_HEIGHT;
		editor.hstart = EDITOR_HSTART;
		editor.vstart = EDITOR_VSTART;

		// Update highlighting and redraw all windows.
		if (!editor.await_utf8) {
			draw_header(&editor);

			if (editor.doc) {
				draw_editor(&editor);
			}
			else {
				buf_set_cursor(0, 0);
			}
			if (focus.draw) {
				focus.draw(&editor, focus.draw_args);
			}

			if (notification.str || pop_notification(&notification)) {
				lstr_t str = lt_lsbuild(lt_libc_heap, " %S ", notification);
				buf_writeln_utf8(lt_term_height - 1, ATTR_FG_BLK | ATTR_BG_RED, str);
				lt_hmfree(str.str);
			}

			buf_present();

			if (notification.str) {
				while ((lt_term_getkey() & LT_TERM_KEY_MASK) == LT_TERM_KEY_MPOS) {}
				lt_hmfree(notification.str);
				notification = NLSTR();
				continue;
			}
		}

		// Get and handle input.
		u32 c = lt_term_getkey();

		switch (c) {
		case LT_TERM_KEY_RESIZE:
			buf_resize(lt_term_width, lt_term_height);
			break;

		case 'E' | LT_TERM_MOD_CTRL:
			notify_exit();
			break;

		case 'O' | LT_TERM_MOD_CTRL:
			browse_filesystem();
			break;

		case 'D' | LT_TERM_MOD_CTRL:
			run_shell();
			break;

		case 'K' | LT_TERM_MOD_CTRL:
			browse_files();
			break;

		default:
			if (focus.input)
				focus.input(&editor, c);
			break;
		}
	}
	return 0;
}

