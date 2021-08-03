// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "clr.h"
#include "editor.h"
#include "file_browser.h"
#include "common.h"

#include <curses.h>
#include "curses_helpers.h"
#include "custom_keys.h"

#include <dirent.h>
#include <libgen.h>
#include <string.h>

focus_t focus_browse_filesystem = { draw_browse_filesystem, NULL, input_browse_filesystem };

#define MAX_ENTRY_COUNT 15

static char input_buf[PATH_MAX_LEN + 1];
static lstr_t input = LSTR(input_buf, 0);

void browse_filesystem(void) {
	focus = focus_browse_filesystem;
	input.len = 0;
}

void draw_browse_filesystem(global_t* ed_globals, void* win_, void* args) {
	WINDOW* win = win_;
	(void)args;
	
	int height = ed_globals->height;
	int width = ed_globals->width;
	
	usz start_height = height - 1 - MAX_ENTRY_COUNT;
	
	wattr_set(win, 0, PAIR_BROWSE_FILES_INPUT, NULL);
	mvwprintw(win, start_height, 0, " %.*s", (int)input.len, input.str);
	wcursyncup(win);	
	waddnch(win, width - getcurx(win), ' ');
	
	char dir_path[PATH_MAX_LEN + 1];
	memcpy(dir_path, input.str, input.len);
	dir_path[input.len] = 0;
	
	char* inp_name = "";
	
	usz found_count = 0;
	DIR* dir = opendir(dir_path);
	if (!dir) {
		usz slash = 0;
		for (usz i = 0; i < input.len; ++i)
			if (input.str[i] == '/')
				slash = i;
		if (slash) {
			dir_path[slash++] = 0;
			dir = opendir(dir_path);
		}
		else
			dir = opendir(".");
		inp_name = &dir_path[slash];
	}
	
	if (dir) {
		usz inp_name_len = strlen(inp_name);
		
		struct dirent* entry = NULL;
		while ((entry = readdir(dir)) && found_count < MAX_ENTRY_COUNT) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			if (strncmp(entry->d_name, inp_name, inp_name_len) != 0)
				continue;
			
			if (entry->d_type == DT_DIR || entry->d_type == DT_LNK)
				wattr_set(win, 0, PAIR_BROWSE_FILES_SEL, NULL);
			else
				wattr_set(win, 0, PAIR_BROWSE_FILES_ENTRY, NULL);
			
			mvwprintw(win, start_height + found_count++ + 1, 0, " %s", entry->d_name);
			waddnch(win, width - getcurx(win), ' ');
		}
	}
	
	wattr_set(win, 0, PAIR_BROWSE_FILES_ENTRY, NULL);
	for (usz i = found_count; i < MAX_ENTRY_COUNT; ++i)
		waddnch(win, width, ' ');
}

void input_browse_filesystem(global_t* ed_global, int c) {
	switch (c) {
	case KEY_ENTER: case '\n':
		focus = focus_editor;
		editor_t* new_ed = fb_open(ed_global, input);
		if (new_ed)
			*ed_global->ed = new_ed;
		break;
		
	case KEY_BACKSPACE:
		if (input.len)
			input.len--;
		else
			focus = focus_editor;
		break;
		
	case KEY_CBACKSPACE:
		if (!input.len)
			focus = focus_editor;
		while (input.len && input.str[--input.len] != '/')
			;
		break;
		
	default:
		if (c >= 32 && c < 127 && input.len < PATH_MAX_LEN)
			input.str[input.len++] = c;
		break;
	}	
}


