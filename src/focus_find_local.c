// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/term.h>
#include <lt/mem.h>

#include "focus.h"
#include "editor.h"
#include "clr.h"
#include "algo.h"
#include "draw.h"

#include <stdio.h>
#include <stdlib.h>

focus_t focus_find_local = { draw_find_local, NULL, input_find_local };

#define BUFSZ 512

static char find_buf[BUFSZ];
static lstr_t find_str = LSTR(find_buf, 0);

static char replace_buf[BUFSZ];
static lstr_t replace_str = LSTR(replace_buf, 0);

static lstr_t* input = NULL;

static usz start_y, start_x;

static isz selected_index = -1;
static usz result_count = 0;
static doc_pos_t* results = NULL;

static u8 replace = 0;

void find_local(isz start_y_, isz start_x_) {
	input = &find_str;
	replace = 0;
	find_str.len = 0;
	replace_str.len = 0;
	focus = focus_find_local;
	start_y = start_y_;
	start_x = start_x_;
}

void draw_find_local(global_t* ed_global, void* args) {
	rec_goto(2, lt_term_height - 1);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_lstr(input->str, input->len);
	rec_str(" ");

	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_LIST_HIGHL]);
	char buf[128 + BUFSZ];
	if (!replace)
		sprintf(buf, "Result %zu/%zu (CTRL+R to replace)", selected_index + 1, result_count);
	else
		sprintf(buf, "Replace %zu occurences of '%.*s'", result_count, (int)find_str.len, find_str.str);
	rec_str(buf);
	rec_goto(2 + input->len, lt_term_height - 1);
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
		input = &replace_str;
	}
	else {
		replace = 0;
		input = &find_str;
	}
}

void input_find_local(global_t* ed_global, u32 c) {
	editor_t* ed = *ed_global->ed;

	b8 changed = 0;

	switch (c) {
	case LT_TERM_KEY_BSPACE:
		if (!input->len)
			edit_file(ed_global, ed);
		else {
			--input->len;
			changed = 1;
		}
		break;

	case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!input->len) case LT_TERM_KEY_ESC:
			edit_file(ed_global, ed);
		else {
			input->len = 0;
			changed = 1;
		}
		break;

	case 'R' | LT_TERM_MOD_CTRL:
		switch_repl();
		break;

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

	case '\n':
		edit_file(ed_global, ed);

		if (replace) {
			doc_replace_str(&ed->doc, find_str, replace_str);
			ed_regenerate_highl(ed);
			ed->cx = min(ed->doc.lines[ed->cy].len, ed->cx);
		}
		ed_sync_selection(ed);
		return;

	default:
		if (c >= 32 && c < 127 && input->len < BUFSZ) {
			input->str[input->len++] = c;
			changed = 1;
		}
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
		ed->sel_x = result.x + find_str.len;
	}
	else {
		ed_goto_line(ed, start_y);
		ed_center_line(ed, start_y);
		ed->cx = start_x;
		ed_sync_selection(ed);
	}
}

