// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/term.h>
#include <lt/mem.h>
#include <lt/texted.h>
#include <lt/io.h>

#include "focus.h"
#include "editor.h"
#include "clr.h"
#include "draw.h"

#include <stdio.h>
#include <stdlib.h>

focus_t focus_find_local = { draw_find_local, NULL, input_find_local };

static lt_texted_t repl_input;
static lt_texted_t* curr_input;

static usz start_y, start_x;
static lt_texted_iterator_t iterator;
static b8 found = 0;

void find_local_init(void) {
	LT_ASSERT(!lt_texted_create(&repl_input, lt_libc_heap));
}

void find_local(isz start_y_, isz start_x_) {
	focus = focus_find_local;
	start_y = start_y_;
	start_x = start_x_;

	iterator = lt_texted_iterator_create();
	iterator.col = start_x - 1;
	iterator.line = start_y;
	found = 0;

	lt_texted_clear(line_input);
	lt_texted_clear(&repl_input);
	curr_input = line_input;
}

void draw_find_local(editor_t* ed, void* args) {
	buf_set_pos(lt_term_height - 2, 0);
	buf_write_char(clr_attr[CLR_LIST_HEAD], ' ');
	buf_write_txed(clr_attr[CLR_EDITOR_SEL], clr_attr[CLR_LIST_HEAD], curr_input);
	buf_write_char(clr_attr[CLR_LIST_HEAD] | ATTR_FILL, ' ');
	buf_write_char(0, 0);

	usz sx = cursor_x_to_screen_x(ed, lt_texted_line_str(curr_input, 0), curr_input->cursor_x);
	buf_set_cursor(lt_term_height - 2, sx + 1);

	if (curr_input == line_input) {
		buf_writeln_utf8(lt_term_height - 1, clr_attr[CLR_LIST_HIGHL] | ATTR_FILL, CLSTR(" CTRL+R replace "));
	}
	else {
		buf_writeln_utf8(lt_term_height - 1, clr_attr[CLR_LIST_HIGHL] | ATTR_FILL, CLSTR(" CTRL+R find "));
	}
}

void find_next_result(lt_texted_t* ed) {
	found = lt_texted_iterate_occurences(ed, lt_texted_line_str(line_input, 0), &iterator);
	if (!found) {
		iterator.col = 0;
		iterator.line = 0;
		found = lt_texted_iterate_occurences(ed, lt_texted_line_str(line_input, 0), &iterator);
	}
}

void find_prev_result(lt_texted_t* ed) {
	found = lt_texted_iterate_occurences_bwd(ed, lt_texted_line_str(line_input, 0), &iterator);
	if (!found) {
		iterator.line = lt_texted_line_count(ed) -1;
		iterator.col = lt_texted_line_len(ed, iterator.line);
		found = lt_texted_iterate_occurences_bwd(ed, lt_texted_line_str(line_input, 0), &iterator);
	}
}

void input_find_local(editor_t* ed, u32 c) {
	doc_t* doc = ed->doc;
	lt_texted_t* txed = &doc->ed;

	b8 changed = 0;

	switch (c) {
	case '\n':
		if (curr_input == &repl_input && found) {
			lt_texted_input_str(txed, lt_texted_line_str(&repl_input, 0));
			ed_regenerate_hl(ed);
			find_next_result(txed);
		}

		if (curr_input == line_input || !found) {
			edit_file(ed, doc);
			return;
		}
		break;

	case LT_TERM_KEY_UP: case 'k' | LT_TERM_MOD_ALT:
		find_prev_result(txed);
		break;

	case LT_TERM_KEY_DOWN: case 'j' | LT_TERM_MOD_ALT:
		find_next_result(txed);
		break;

	case 'R' | LT_TERM_MOD_CTRL:
		if (curr_input == line_input) {
			curr_input = &repl_input;
		}
		else {
			curr_input = line_input;
		}
		break;

	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!lt_texted_line_len(curr_input, 0)) {
		case LT_TERM_KEY_ESC:
			edit_file(ed, doc);
		}
	default:
		changed = input_term_key(curr_input, c);
		break;
	}

	if (curr_input == line_input && changed) {
		iterator.col = start_x - 1;
		iterator.line = start_y;
		find_next_result(txed);
	}

	if (found) {
		lt_texted_gotoxy(txed, iterator.col + lt_texted_line_len(line_input, 0), iterator.line, 1);
		lt_texted_gotox(txed, iterator.col, 0);
		center_line(ed, iterator.line);
	}
	else {
		lt_texted_gotoxy(txed, start_x, start_y, 1);
		center_line(ed, start_y);
	}
}

