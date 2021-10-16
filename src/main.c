// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <stdlib.h>
#include <string.h>

#include <locale.h>

#include <curses.h>
#include "curses_helpers.h"

#include "fhl.h"
#include "algo.h"
#include "conf.h"
#include "allocators.h"
#include "clr.h"
#include "highlight.h"
#include "editor.h"
#include "custom_keys.h"
#include "file_browser.h"
#include "focus.h"

WINDOW* header_w = NULL;
WINDOW* linenum_w = NULL;
WINDOW* editor_w = NULL;

#define HEADER_HEIGHT 1
#define LINENUM_WIDTH 5

#define EDITOR_HSTART (LINENUM_WIDTH)
#define EDITOR_VSTART (HEADER_HEIGHT)

#define EDITOR_HEIGHT (LINES - EDITOR_VSTART)
#define EDITOR_WIDTH (COLS - EDITOR_HSTART)

editor_t* ed = NULL;

global_t ed_globals;

void set_highl(highl_t* node) {
	if (!node) {
		wattr_set(editor_w, 0, PAIR_EDITOR, NULL);
		return;
	}

	switch (node->mode) {
	case HLM_UNKNOWN: wattr_set(editor_w, 0, PAIR_SYNTAX_UNKNOWN, NULL); break;

	case HLM_STRING: wattr_set(editor_w, 0, PAIR_SYNTAX_STRING, NULL); break;
	case HLM_CHAR: wattr_set(editor_w, 0, PAIR_SYNTAX_CHAR, NULL); break;
	case HLM_NUMBER: wattr_set(editor_w, 0, PAIR_SYNTAX_NUMBER, NULL); break;

	case HLM_IDENTIFIER: wattr_set(editor_w, 0, PAIR_SYNTAX_IDENTIFIER, NULL); break;
	case HLM_KEYWORD: wattr_set(editor_w, 0, PAIR_SYNTAX_KEYWORD, NULL); break;
	case HLM_COMMENT: wattr_set(editor_w, 0, PAIR_SYNTAX_COMMENT, NULL); break;
	case HLM_DATATYPE: wattr_set(editor_w, 0, PAIR_SYNTAX_DATATYPE, NULL); break;

	case HLM_FUNCTION: wattr_set(editor_w, A_BOLD, PAIR_SYNTAX_FUNCTION, NULL); break;

	case HLM_HASH: wattr_set(editor_w, 0, PAIR_SYNTAX_HASH, NULL); break;
	case HLM_BRACKET: wattr_set(editor_w, 0, PAIR_SYNTAX_BRACKET, NULL); break;
	case HLM_OPERATOR: wattr_set(editor_w, 0, PAIR_SYNTAX_OPERATOR, NULL); break;
	case HLM_PUNCTUATION: wattr_set(editor_w, 0, PAIR_SYNTAX_PUNCTUATION, NULL); break;

	case HLM_INDENT:
		if (!node->next)
			wattr_set(editor_w, 0, PAIR_SYNTAX_TRAIL_INDENT, NULL);
		else
			wattr_set(editor_w, 0, PAIR_EDITOR, NULL);
		break;

	default:
		break;
	}
}

void add_doc_name(doc_t* doc) {
	waddstr(header_w, doc->path);
 	if (doc->unsaved)
		waddch(header_w, '*');
	if (doc->new)
		waddstr(header_w, "[NEW]");
	if (doc->read_only)
		waddstr(header_w, "[RO]");
}

void draw_header(editor_t* ed) {
	wmove(header_w, 0, 0);
	wattr_set(header_w, 0, PAIR_HEADER_TAB, NULL);

	waddch(header_w, ' ');
	if (ed)
		add_doc_name(&ed->doc);
	else
		waddstr(header_w, "No file selected");
	waddch(header_w, ' ');

	wattr_set(header_w, 0, PAIR_HEADER_BG, NULL);
	waddnch(header_w, COLS - getcurx(header_w), ' ');
}

static inline INLINE
void draw_linenum(usz y, usz line_num) {
	mvwprintw(linenum_w, y, 0, "%4zu ", line_num % 10000);
}

static
void draw_line(editor_t* ed, usz y, usz line_index) {
	lstr_t* line = &ed->doc.lines[line_index];
	wmove(editor_w, y, 0);
	highl_t* hl_node = ed->highl_lines[line_index];

	isz j = 0;
	while (hl_node) {
		set_highl(hl_node);
		isz len = hl_node->len;
		wprintw(editor_w, "%.*s", (int)len, &line->str[j]);

		j += len;
		hl_node = hl_node->next;
	}

	wattr_set(editor_w, 0, PAIR_EDITOR, NULL);
	wprintw(editor_w, "%.*s", (int)(line->len - j), &line->str[j]);
}

static
void draw_line_hbound(editor_t* ed, usz y, usz line_index, usz max_x) {
	lstr_t* line = &ed->doc.lines[line_index];
	wmove(editor_w, y, 0);
	highl_t* hl_node = ed->highl_lines[line_index];

	isz j = 0;
	while (hl_node && j < max_x) {
		set_highl(hl_node);
		isz len = clamp(hl_node->len, 0, max_x - j);
		wprintw(editor_w, "%.*s", (int)len, &line->str[j]);

		j += len;
		hl_node = hl_node->next;
	}

	wattr_set(editor_w, 0, PAIR_EDITOR, NULL);
	wprintw(editor_w, "%.*s", (int)clamp(line->len - j, 0, max_x - j), &line->str[j]);
}

static
void draw_line_hoffs(editor_t* ed, usz y, usz line_index, usz start_x, usz start_col) {
	lstr_t* line = &ed->doc.lines[line_index];
	highl_t* hl_node = ed->highl_lines[line_index];

	wmove(editor_w, y, start_col);

	isz j = 0;
	while (hl_node && j + hl_node->len <= start_x) {
		j += hl_node->len;
		hl_node = hl_node->next;
	}

	if (hl_node) {
		set_highl(hl_node);
		const isz len = (j + hl_node->len) - start_x;
		wprintw(editor_w, "%.*s", (int)len, &line->str[start_x]);
		j += hl_node->len;
		hl_node = hl_node->next;

		while (hl_node) {
			set_highl(hl_node);
			const isz len = hl_node->len;
			wprintw(editor_w, "%.*s", (int)len, &line->str[j]);

			j += len;
			hl_node = hl_node->next;
		}
	}

	isz remaining = (line->len - start_x) - j;
	if (remaining > 0) {
		wattr_set(editor_w, 0, PAIR_EDITOR, NULL);
		wprintw(editor_w, "%.*s", (int)remaining, &line->str[start_x + j]);
	}
}

void draw_editor(editor_t* ed) {
	// Get correct cursor position
	isz cx = ed_cx_to_screen_x(ed, ed->cx, ed->cy);
	wmove(editor_w, ed->cy - ed->line_top, cx);
	wcursyncup(editor_w);

	// Get selection coordinates and make y relative to line_top
	isz sel_start_y, sel_start_x, sel_end_y, sel_end_x;
	ed_get_selection(ed, &sel_start_y, &sel_start_x, &sel_end_y, &sel_end_x);
	sel_start_y -= ed->line_top;
	sel_end_y -= ed->line_top;

	wattr_set(editor_w, 0, PAIR_EDITOR, NULL);
	const isz line_top = ed->line_top;
	const isz line_count = clamp(ed->doc.line_count - ed->line_top, 0, EDITOR_HEIGHT);

	// Draw line numbers preceding the selection
	wattr_set(linenum_w, 0, PAIR_LINENUM, NULL);
	const isz pre_selection_lines = sel_start_y;
	for (isz i = 0; i < pre_selection_lines; ++i)
		draw_linenum(i, line_top + i + 1);

	const isz sel_min = max(sel_start_y, 0);
	const isz sel_max = min(sel_end_y, EDITOR_HEIGHT - 1);
	// Draw line numbers contained in the selection
	wattr_set(linenum_w, 0, PAIR_LINENUM_SEL, NULL);
	for (isz i = sel_min; i <= sel_max; ++i)
		mvwprintw(linenum_w, i, 0, "%4zu ", (line_top + i + 1) % 10000);

	// Draw remaining line numbers
	wattr_set(linenum_w, 0, PAIR_LINENUM, NULL);
	for (isz i = sel_end_y + 1; i < line_count; ++i)
		draw_linenum(i, line_top + i + 1);

	// Fill any remaining line numbers
	const isz underflow_count = EDITOR_HEIGHT - line_count;
	wattr_set(linenum_w, 0, PAIR_LINENUM_UFLOW, NULL);
	for (isz i = 0; i < underflow_count; ++i)
		waddstr(linenum_w, "  .. ");

	// Draw text preceding the selection
	for (isz i = 0; i < pre_selection_lines; ++i)
		draw_line(ed, i, line_top + i);

	// Draw text on selected lines
	isz col = 0;
	if (sel_start_y >= 0) {
		draw_line_hbound(ed, sel_start_y, line_top + sel_start_y, sel_start_x);
		col = getcurx(editor_w);
	}

	wattr_set(editor_w, A_STANDOUT, PAIR_EDITOR, NULL);
	for (isz i = sel_min, j = sel_start_y >= 0 ? sel_start_x : 0; i <= sel_max; ++i) {
		lstr_t* line = &ed->doc.lines[line_top + i];
		isz len = (i == sel_end_y ? sel_end_x : line->len) - j;
		wmove(editor_w, i, col);
		wprintw(editor_w, "%.*s", (int)len, &line->str[j]);
		j = 0;
		col = 0;
	}
	wattr_off(editor_w, A_STANDOUT, NULL);

	if (sel_end_y < EDITOR_HEIGHT)
		draw_line_hoffs(ed, sel_end_y, line_top + sel_end_y, sel_end_x, getcurx(editor_w));

	// Draw remaining text
	for (isz i = sel_end_y + 1; i < line_count; ++i)
		draw_line(ed, i, line_top + i);
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

	delwin(header_w);
	delwin(linenum_w);
	delwin(editor_w);

	clr_pop(); // Pop previous colors
	refresh();
	endwin();

	// Force disable mouse events again
	printf("\x1B[?1003l");
	fflush(stdout);
}

int main(int argc, char** argv) {
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

	// Initialize ncurses
	setlocale(LC_ALL, "");
	if (!initscr())
		ferr("Failed to initialize ncurses\n");

	ed_globals.scroll_offs = conf_find_int_default(editor_cf, CLSTR("scroll_offset"), 2);
	ed_globals.tab_size = conf_find_int_default(editor_cf, CLSTR("tab_size"), 4);
	ed_globals.vstep = conf_find_int_default(editor_cf, CLSTR("vstep"), 4);
	ed_globals.predict_brackets = conf_find_bool_default(editor_cf, CLSTR("predict_brackets"), 0);

	ed_globals.ed = &ed;

	set_tabsize(ed_globals.tab_size);

	// Set input modes
	noecho();
	raw();
	keypad(stdscr, 1);
	mouseinterval(0);
	mmask_t mmask_old;
	mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, &mmask_old);

	// This hack is here to make the terminal report mouse
	// positions without setting the $TERM env variable,
	// there is probably a better way to accomplish this.
	printf("\x1B[?1003h");
	// Enable 'bracketed paste' mode
	printf("\x1b[?2004h");
	fflush(stdout);

	clear();

	// Register custom keycodes
	register_custom_keys();

	// Initialize colors
	start_color();
	clr_push(); // Push current colors until exit

	conf_t* colors_cf = conf_find(&config, CLSTR("colors"), CONF_OBJECT);
	if (!colors_cf)
		ferr("No 'colors' object found in config file\n");
	clr_load(colors_cf);

	// Free config
	conf_free(&config);
	free(conf_data);

	on_exit(cleanup, NULL);

	// Load documents
	for (usz i = 1; i < argc; ++i)
		fb_open(&ed_globals, CLSTR(argv[i]));

	ed = fb_first_file();

	edit_file(&ed_globals, ed);

	for (;;) {
		// (Re)create windows when first started or when the
		// size of the terminal changed.
		int max_x, max_y;
		getmaxyx(editor_w, max_y, max_x);

		ed_globals.width = EDITOR_WIDTH;
		ed_globals.height = EDITOR_HEIGHT;
		ed_globals.vstart = EDITOR_VSTART;
		ed_globals.hstart = EDITOR_HSTART;

		if (!editor_w || max_x != EDITOR_WIDTH || max_y != EDITOR_HEIGHT) {
			delwin(header_w);
			delwin(linenum_w);
			delwin(editor_w);

			header_w = subwin(stdscr, HEADER_HEIGHT, COLS, 0, 0);
			linenum_w = subwin(stdscr, EDITOR_HEIGHT, LINENUM_WIDTH, HEADER_HEIGHT, 0);
			editor_w = subwin(stdscr, EDITOR_HEIGHT, EDITOR_WIDTH, HEADER_HEIGHT, LINENUM_WIDTH);
		}

		// Update highlighting and redraw all windows.
		erase();
		draw_header(ed);
		if (ed) {
			aframe_restore(ed->highl_arena, &ed->restore);
			ed->highl_lines = highl_generate(ed->highl_arena, &ed->doc);
			draw_editor(ed);

		}
		if (focus.draw)
			focus.draw(&ed_globals, editor_w, focus.draw_args);
		refresh();

		// Get and handle input.
		int c = wgetch(stdscr);

		switch (c) {
		case 'E' - CTRL_MOD_DIFF:
			notify_exit();
			break;

		case 'O' - CTRL_MOD_DIFF:
			browse_filesystem();
			break;

		case KEY_RESIZE:
			// For some reason, the tab size is reset to default when a terminal is resized.
			set_tabsize(ed_globals.tab_size);
			// Fall through to propagate event
		default:
			if (focus.input)
				focus.input(&ed_globals, c);
			break;
		}
	}
	return 0;
}
