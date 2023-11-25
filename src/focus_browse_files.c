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
#include "algo.h"

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

	usz start_height = lt_term_height - MAX_VISIBLE_ENTRIES;

	rec_goto(2, start_height);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_led(line_input, clr_strs[CLR_EDITOR_SEL], clr_strs[CLR_LIST_HEAD]);
	rec_str(" ");

	doc_t* found[MAX_ENTRY_COUNT];
	usz found_count = fb_find_files(found, MAX_ENTRY_COUNT, lt_texted_line_str(line_input, 0));
	usz visible_count = lt_min_usz(found_count, MAX_VISIBLE_ENTRIES);

	selected = NULL;

	selected_index = clamp(selected_index, 0, found_count - 1);
	visible_index = clamp(visible_index, selected_index - visible_count + 1, selected_index);
	visible_index = clamp(visible_index, 0, found_count - visible_count);

	// Draw available files
	rec_str(clr_strs[CLR_LIST_ENTRY]);
	for (usz i = 0; i < visible_count; ++i) {
		usz index = visible_index + i;

		rec_goto(2, start_height + i + 1);

		if (index == selected_index) {
			rec_clearline(clr_strs[CLR_LIST_HIGHL]);
			selected = found[selected_index];
			rec_str(clr_strs[CLR_LIST_HIGHL]);
		}
		else {
			rec_clearline(clr_strs[CLR_LIST_ENTRY]);
			rec_str(clr_strs[CLR_LIST_ENTRY]);
		}

		lstr_t basename = lt_lsbasename(found[index]->path);
		lstr_t dirname = lt_lsdirname(found[index]->path);

		rec_str(clr_strs[CLR_LIST_DIR]);
		rec_lstr(dirname.str, dirname.len);

		rec_str("/ ");

		rec_str(clr_strs[CLR_LIST_FILE]);
		rec_lstr(basename.str, basename.len);
	}

	// Fill underflowed slots
	for (usz i = visible_count; i < MAX_VISIBLE_ENTRIES; ++i) {
		rec_goto(0, start_height + i + 1);
		rec_clearline("");
	}
	rec_crestore();

	max_index = found_count;
}

void input_browse_files(editor_t* ed, u32 c) {
	doc_t* doc = ed->doc;

	switch (c) {
	case '\n':
		edit_file(ed, selected ? selected : doc);
		break;

	case LT_TERM_KEY_UP: case 'k' | LT_TERM_MOD_ALT:
		if (selected_index)
			--selected_index;
		break;

	case LT_TERM_KEY_DOWN: case 'j' | LT_TERM_MOD_ALT:
		if (selected_index + 1 < max_index)
			++selected_index;
		break;

	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!lt_texted_line_len(line_input, 0))
	case LT_TERM_KEY_ESC:
			edit_file(ed, doc);
	default:
		input_term_key(line_input, c);
		break;
	}
}


