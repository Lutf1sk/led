// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus_browse_files.h"
#include "focus_editor.h"
#include "file_browser.h"
#include "clr.h"
#include "editor.h"
#include "common.h"

#include <curses.h>
#include "curses_helpers.h"
#include "custom_keys.h"

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

void draw_browse_files(global_t* ed_globals, void* win_, void* args) {
	WINDOW* win = win_;
	(void)args;
	
	int height = ed_globals->height;
	int width = ed_globals->width;
	
	usz start_height = height - 1 - MAX_ENTRY_COUNT;
	
	wattr_set(win, 0, PAIR_BROWSE_FILES_INPUT, NULL);
	mvwprintw(win, start_height, 0, " %.*s", (int)input.len, input.str);
	wcursyncup(win);
	
	waddnch(win, width - getcurx(win), ' ');

	editor_t* found[MAX_ENTRY_COUNT];
	usz found_count = fb_find_files(found, MAX_ENTRY_COUNT, input);

	// Draw available files
	wattr_set(win, 0, PAIR_BROWSE_FILES_ENTRY, NULL);
	for (usz i = 0; i < found_count; ++i) {
		mvwprintw(win, start_height + 1 + i, 0, " %s", found[i]->doc.path);
		waddnch(win, width - getcurx(win), ' ');
	}
	
	// Draw selected file
	usz sel_index = selected_index;
	if (sel_index < found_count) {
		selected = found[sel_index];
		wattr_set(win, 0, PAIR_BROWSE_FILES_SEL, NULL);
		mvwprintw(win, start_height + 1 + sel_index, 0, " %s", found[sel_index]->doc.path);	
		waddnch(win, width - getcurx(win), ' ');
	}
	else
		selected = NULL;

	// Fill underflowed slots
	wattr_set(win, 0, PAIR_BROWSE_FILES_ENTRY, NULL);
	for (usz i = found_count; i < MAX_ENTRY_COUNT; ++i)
		mvwaddnch(win, start_height + 1 + i, 0, width, ' ');
	
	max_index = found_count;
}

void input_browse_files(global_t* ed_global, int c) {
	switch (c) {
	case '\n': case KEY_ENTER:
		if (selected)
			*ed_global->ed = selected;
		focus = focus_editor;
		break;
		
	case KEY_BACKSPACE:
		if (input.len)
			--input.len;
		else
			focus = focus_editor;
		break;
	
	case KEY_CBACKSPACE:
		if (!input.len)
			focus = focus_editor;
		while (input.len && input.str[--input.len] != '.')
			;
		break;
	
	case KEY_UP:
		if (selected_index)
			--selected_index;
		break;
		
	case KEY_DOWN:
		if (selected_index + 1 < max_index)
			++selected_index;
		break;
		
	default:
		if (c >= 32 && c < 127 && input.len < PATH_MAX_LEN)
			input.str[input.len++] = c;
		break;
	}
}


