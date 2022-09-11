// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/term.h>

#include "focus.h"
#include "file_browser.h"
#include "clr.h"
#include "editor.h"
#include "common.h"
#include "algo.h"

#include "draw.h"

focus_t focus_browse_files = { draw_browse_files, NULL, input_browse_files };

#define MAX_ENTRY_COUNT 15

static char input_buf[PATH_MAX_LEN];
static lstr_t input = LSTR(input_buf, 0);
static editor_t* selected = NULL;
static usz selected_index = 0;
static usz max_index = 0;

void browse_files(void) {
	focus = focus_browse_files;
	selected_index = 0;
	input.len = 0;
}

void draw_browse_files(global_t* ed_globals, void* args) {
	(void)args;

	usz start_height = lt_term_height - MAX_ENTRY_COUNT;

	rec_goto(2, start_height);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_lstr(input.str, input.len);

	editor_t* found[MAX_ENTRY_COUNT];
	usz found_count = fb_find_files(found, MAX_ENTRY_COUNT, input);

	selected_index = clamp(selected_index, 0, found_count - 1);

	selected = NULL;

	// Draw available files
	rec_str(clr_strs[CLR_LIST_ENTRY]);
	for (usz i = 0; i < found_count; ++i) {
		if (i == selected_index) {
			rec_goto(2, start_height + i + 1);
			rec_clearline(clr_strs[CLR_LIST_HIGHL]);
			rec_str(found[i]->doc.path);
			rec_str(clr_strs[CLR_LIST_ENTRY]);

			selected = found[selected_index];
		}
		else {
			rec_goto(2, start_height + i + 1);
			rec_clearline("");
			rec_str(found[i]->doc.path);
		}
	}

	// Fill underflowed slots
	for (usz i = found_count; i < MAX_ENTRY_COUNT; ++i) {
		rec_goto(0, start_height + i + 1);
		rec_clearline("");
	}

	max_index = found_count;
}

void input_browse_files(global_t* ed_global, u32 c) {
	editor_t* ed = ed_global->ed;

	switch (c) {
	case '\n':
		edit_file(ed_global, selected ? selected : ed);
		break;

	case LT_TERM_KEY_BSPACE:
		if (!input.len)
			edit_file(ed_global, ed);
		else
			--input.len;
		break;

	case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!input.len) case LT_TERM_KEY_ESC:
			edit_file(ed_global, ed);
		while (input.len && input.str[--input.len] != '.')
			;
		break;

	case LT_TERM_KEY_UP:
		if (selected_index)
			--selected_index;
		break;

	case LT_TERM_KEY_DOWN:
		if (selected_index + 1 < max_index)
			++selected_index;
		break;

	default:
		if (c >= 32 && c < 127 && input.len < PATH_MAX_LEN)
			input.str[input.len++] = c;
		break;
	}
}


