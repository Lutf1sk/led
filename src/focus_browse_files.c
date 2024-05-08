// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/term.h>
#include <lt/texted.h>
#include <lt/math.h>
#include <lt/str.h>

#include "focus.h"
#include "file_browser.h"
#include "clr.h"
#include "editor.h"
#include "common.h"

#include "draw.h"

focus_t focus_browse_files = { draw_browse_files, NULL, input_browse_files };

#define MAX_ENTRY_COUNT 1024

#define MAX_VISIBLE_ENTRIES 15

static doc_t* selected = NULL;
static usz selected_index = 0;
static usz visible_index = 0;
static usz max_index = 0;

void browse_files(void) {
	focus = focus_browse_files;
	selected_index = 0;
	visible_index = 0;
	lt_texted_clear(line_input);
}

void draw_browse_files(editor_t* ed, void* args) {
	(void)args;

	isz start_height = lt_term_height - MAX_VISIBLE_ENTRIES - 1;

	buf_set_pos(start_height, 0);
	buf_write_char(clr_attr[CLR_LIST_HEAD], ' ');
	buf_write_txed(clr_attr[CLR_EDITOR_SEL], clr_attr[CLR_LIST_HEAD], line_input);
	buf_write_char(clr_attr[CLR_LIST_HEAD] | ATTR_FILL, ' ');
	buf_write_char(0, 0);

	usz sx = cursor_x_to_screen_x(ed, lt_texted_line_str(line_input, 0), line_input->cursor_x);
	buf_set_cursor(start_height, sx + 1);

	doc_t* found[MAX_ENTRY_COUNT];
	usz found_count = fb_find_files(found, MAX_ENTRY_COUNT, lt_texted_line_str(line_input, 0));
	usz visible_count = lt_usz_min(found_count, MAX_VISIBLE_ENTRIES);

	selected = NULL;

	selected_index = lt_isz_clamp(selected_index, 0, found_count - 1);
	visible_index = lt_isz_clamp(visible_index, selected_index - visible_count + 1, selected_index);
	visible_index = lt_isz_clamp(visible_index, 0, found_count - visible_count);

	// Draw available files
	for (usz i = 0; i < visible_count; ++i) {
		usz index = visible_index + i;

		u32 attr0 = clr_attr[CLR_LIST_DIR];
		u32 attr1 = clr_attr[CLR_LIST_FILE];

		if (index == selected_index) {
			selected = found[selected_index];
			attr0 = (attr0 & ~ATTR_BG_MASK) | (clr_attr[CLR_LIST_HIGHL] & ATTR_BG_MASK);
			attr1 = (attr1 & ~ATTR_BG_MASK) | (clr_attr[CLR_LIST_HIGHL] & ATTR_BG_MASK);
		}

		lstr_t basename = lt_lsbasename(found[index]->path);
		lstr_t dirname = lt_lsdirname(found[index]->path);

		buf_set_pos(start_height + i + 1, 0);
		buf_write_utf8(attr0, CLSTR(" "));
		buf_write_utf8(attr0, dirname);
		buf_write_utf8(attr0, CLSTR("/ "));
		buf_write_utf8(attr1 | ATTR_FILL, basename);
		buf_write_char(0, 0);
	}

	// Fill underflowed slots
	for (usz i = visible_count; i < MAX_VISIBLE_ENTRIES; ++i) {
		buf_writeln_utf8(start_height + i + 1, clr_attr[CLR_LIST_ENTRY] | ATTR_FILL, CLSTR(" "));
	}

	max_index = found_count;
}

void input_browse_files(editor_t* ed, u32 c) {
	doc_t* doc = ed->doc;

	if (c != (LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL) && c != (LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL| LT_TERM_MOD_SHIFT)) {
		ed->consec_cdn = 0;
	}
	if (c != (LT_TERM_KEY_UP | LT_TERM_MOD_CTRL) && c != (LT_TERM_KEY_UP | LT_TERM_MOD_CTRL| LT_TERM_MOD_SHIFT)) {
		ed->consec_cup = 0;
	}

	switch (c) {
	case '\n':
		edit_file(ed, selected ? selected : doc);
		break;

	case LT_TERM_KEY_UP: case 'k' | LT_TERM_MOD_ALT:
		if (selected_index) {
			--selected_index;
		}
		break;

	case LT_TERM_KEY_DOWN: case 'j' | LT_TERM_MOD_ALT:
		if (selected_index + 1 < max_index) {
			++selected_index;
		}
		break;

	case LT_TERM_KEY_UP | LT_TERM_MOD_CTRL: {
		usz vstep = ++ed->consec_cup * ed->vstep;
		for (usz i = 0; i < vstep; ++i) {
			if (selected_index && --selected_index < visible_index) {
				--visible_index;
			}
		}
	}	break;

	case LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL: {
		usz vstep = ++ed->consec_cdn * ed->vstep;
		for (usz i = 0; i < vstep; ++i) {
			if (selected_index + 1 < max_index && ++selected_index >= visible_index + MAX_ENTRY_COUNT) {
				++visible_index;
			}
		}
	}	break;

	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!lt_texted_line_len(line_input, 0)) {
		case LT_TERM_KEY_ESC:
			edit_file(ed, doc);
		}
	default:
		input_term_key(line_input, c);
		break;
	}
}


