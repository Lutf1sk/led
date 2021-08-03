// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus_find_local.h"
#include "focus_editor.h"
#include "editor.h"
#include "clr.h"
#include "chartypes.h"
#include "algo.h"

#include <curses.h>
#include "curses_helpers.h"
#include "custom_keys.h"

focus_t focus_find_local = { draw_find_local, NULL, input_find_local };

static char input_buf[512];
static lstr_t input = LSTR(input_buf, 0);
static usz start_y, start_x;

void find_local(isz start_y_, isz start_x_) {
	input.len = 0;
	focus = focus_find_local;
	start_y = start_y_;
	start_x = start_x_;
}

void draw_find_local(global_t* ed_global, void* win_, void* args) {
	WINDOW* win = win_;
	
	isz width = ed_global->width;
	isz height = ed_global->height;
	
	wattr_set(win, 0, PAIR_BROWSE_FILES_INPUT, NULL);
	mvwprintw(win, height - 1, 0, " %.*s", (int)input.len, input.str);
	wcursyncup(win);
	waddnch(win, width - getcurx(win), ' ');
}

void input_find_local(global_t* ed_global, int c) {
	editor_t* ed = *ed_global->ed;
	
	switch (c) {
	case KEY_BACKSPACE:
		if (!input.len)
			focus = focus_editor;
		else
			--input.len;
		break;
		
	case KEY_CBACKSPACE:
		if (!input.len)
			focus = focus_editor;
		else
			input.len = 0;
		break;

	case KEY_ENTER: case '\n':
		focus = focus_editor;
		break;

	default:
		if (c >= 32 && c < 127 && input.len < sizeof(input_buf))
			input.str[input.len++] = c;
		break;
	}
	
	isz y, x;
	if (doc_find_str(&ed->doc, input, start_y, start_x, &y, &x)) {
		ed_goto_line(ed, y);
		ed->cx = x;
		ed_sync_selection(ed);
	}
}

