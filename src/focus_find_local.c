// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/term.h>
#include <lt/mem.h>
#include <lt/texted.h>
#include <lt/io.h>

#include "focus.h"
#include "editor.h"
#include "clr.h"
#include "algo.h"
#include "draw.h"

#include <stdio.h>
#include <stdlib.h>

focus_t focus_find_local = { draw_find_local, NULL, input_find_local };

static lt_led_t repl_input;
static lt_led_t* curr_input;

static usz start_y, start_x;

static isz selected_index = -1;
static usz result_count = 0;
static doc_pos_t* results = NULL;

static u8 replace = 0;

void find_local_init(void) {
	LT_ASSERT(!lt_led_create(&repl_input, lt_libc_heap));
}

void find_local(isz start_y_, isz start_x_) {
	replace = 0;
	focus = focus_find_local;
	start_y = start_y_;
	start_x = start_x_;

	lt_led_clear(line_input);
	lt_led_clear(&repl_input);
	curr_input = line_input;
}

void draw_find_local(global_t* ed_global, void* args) {
	rec_goto(2, lt_term_height - 1);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_led(curr_input, clr_strs[CLR_EDITOR_SEL], clr_strs[CLR_LIST_HEAD]);
	rec_str(" ");

	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_LIST_HIGHL]);
	char buf[128 + 512]; // !!
	if (!replace)
		lt_sprintf(buf, "Result %uz/%uz (CTRL+R to replace)", selected_index + 1, result_count);
	else
		lt_sprintf(buf, "Replace %uz occurences of '%S'", result_count, lt_led_get_str(line_input));
	rec_str(buf);
	rec_goto(2 + input_cursor_pos(curr_input), lt_term_height - 1);
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
	lstr_t find_str = lt_led_get_str(line_input);
	usz new_count = doc_find_str(&ed->doc, find_str, NULL);
	if (new_count) {
		if (new_count > result_count) {
			results = realloc(results, new_count * sizeof(doc_pos_t));
			if (!results)
				ferrf("Memory allocation failed: %s\n", os_err_str());
		}
		doc_find_str(&ed->doc, find_str, results);
	}
	result_count = new_count;

	selected_index = find_index();
}

void switch_repl(void) {
	if (!replace) {
		replace = 1;
		curr_input = &repl_input;
	}
	else {
		replace = 0;
		curr_input = line_input;
	}
}

void input_find_local(global_t* ed_global, u32 c) {
	editor_t* ed = ed_global->ed;

	b8 changed = 0;

	switch (c) {
	case '\n':
		edit_file(ed_global, ed);

		if (replace) {
			doc_replace_str(&ed->doc, lt_led_get_str(line_input), lt_led_get_str(&repl_input));
			ed_regenerate_hl(ed);
			ed->cx = min(ed->doc.lines[ed->cy].len, ed->cx);
		}
		ed_sync_selection(ed);
		return;

	case LT_TERM_KEY_UP:
		if (selected_index > 0)
			--selected_index;
		else if (selected_index == 0)
			selected_index = max(result_count - 1, 0);
		break;

	case LT_TERM_KEY_DOWN:
		if (selected_index != -1 && ++selected_index >= result_count)
			selected_index = 0;
		break;

	case 'R' | LT_TERM_MOD_CTRL:
		switch_repl();
		break;

	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!lt_darr_count(curr_input->str))
	case LT_TERM_KEY_ESC:
			edit_file(ed_global, ed);
	default:
		changed = input_term_key(curr_input, c);
		break;
	}

	if (!replace && changed)
		update_results(ed);

	if (selected_index >= 0) {
		doc_pos_t result = results[selected_index];
		ed_goto_line(ed, result.y);
		ed_center_line(ed, result.y);
		ed->cx = result.x;
		ed->sel_y = result.y;
		ed->sel_x = result.x + lt_darr_count(line_input->str);
	}
	else {
		ed_goto_line(ed, start_y);
		ed_center_line(ed, start_y);
		ed->cx = start_x;
		ed_sync_selection(ed);
	}
}

