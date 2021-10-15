// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "editor.h"
#include "clr.h"
#include "chartypes.h"
#include "algo.h"

#include <curses.h>
#include "curses_helpers.h"
#include "custom_keys.h"

#include <stdlib.h>

focus_t focus_find_local = { draw_find_local, NULL, input_find_local };

static char input_buf[512];
static lstr_t input = LSTR(input_buf, 0);

static usz start_y, start_x;

static isz selected_index = -1;
static usz result_count = 0;
static doc_pos_t* results = NULL;

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
	mvwprintw(win, height - 2, 0, " %.*s", (int)input.len, input.str);
	wcursyncup(win);
	waddnch(win, width - getcurx(win), ' ');

	wattr_set(win, 0, PAIR_BROWSE_FILES_SEL, NULL);
	mvwprintw(win, height - 1, 0, " Result %zu/%zu", selected_index + 1, result_count);
	waddnch(win, width - getcurx(win), ' ');
}

static
isz find_index(void) {
	for (usz i = 0; i < result_count; ++i) {
		doc_pos_t result = results[i];
		if (result.y > start_y || (result.y == start_y && result.x >= start_x))
			return i;
	}

	for (usz i = 0; i < result_count; ++i) {
		doc_pos_t result = results[i];
		if (result.y <= start_y || (result.y == start_y && result.x < start_x))
			return i;
	}
	return -1;
}

static
void update_results(editor_t* ed) {
	usz new_count = doc_find_str(&ed->doc, input, NULL);
	if (new_count) {
		if (new_count > result_count) {
			results = realloc(results, new_count * sizeof(doc_pos_t));
			if (!results)
				ferrf("Memory allocation failed: %s\n", os_err_str());
		}
		doc_find_str(&ed->doc, input, results);
	}
	result_count = new_count;

	selected_index = find_index();
}

void input_find_local(global_t* ed_global, int c) {
	editor_t* ed = *ed_global->ed;

	switch (c) {
	case KEY_BACKSPACE:
		if (!input.len)
			edit_file(ed_global, ed);
		else {
			--input.len;
			update_results(ed);
		}
		break;

	case KEY_CBACKSPACE:
		if (!input.len)
			edit_file(ed_global, ed);
		else {
			input.len = 0;
			update_results(ed);
		}
		break;

	case KEY_UP:
		if (selected_index > 0)
			--selected_index;
		else if (selected_index == 0)
			selected_index = imax(result_count - 1, 0);
		break;

	case KEY_DOWN:
		if (selected_index != -1 && ++selected_index >= result_count)
			selected_index = 0;
		break;

	case KEY_ENTER: case '\n':
		edit_file(ed_global, ed);
		ed_sync_selection(ed);
		return;

	default:
		if (c >= 32 && c < 127 && input.len < sizeof(input_buf)) {
			input.str[input.len++] = c;
			update_results(ed);
		}
		break;
	}

	if (selected_index >= 0) {
		doc_pos_t result = results[selected_index];
		ed_goto_line(ed, result.y);
		ed->cx = result.x;
		ed->sel_y = result.y;
		ed->sel_x = result.x + input.len;
	}
	else {
		ed_goto_line(ed, start_y);
		ed->cx = start_x;
		ed_sync_selection(ed);
	}
}

