// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "clr.h"
#include "editor.h"
#include "file_browser.h"
#include "common.h"
#include "draw.h"

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

void draw_browse_filesystem(global_t* ed_globals, void* args) {
	(void)args;

	usz start_height = lt_term_height - MAX_ENTRY_COUNT;

	rec_goto(2, start_height);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_lstr(input.str, input.len);

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

			rec_goto(2, start_height + ++found_count);
			if (entry->d_type == DT_DIR || entry->d_type == DT_LNK)
				rec_clearline(clr_strs[CLR_LIST_HIGHL]);
			else
				rec_clearline(clr_strs[CLR_LIST_ENTRY]);
			rec_str(entry->d_name);
		}
	}

	rec_str(clr_strs[CLR_LIST_ENTRY]);
	for (usz i = found_count; i < MAX_ENTRY_COUNT; ++i) {
		rec_goto(0, start_height + i + 1);
		rec_clearline("");
	}
}

void input_browse_filesystem(global_t* ed_global, u32 c) {
	editor_t* ed = *ed_global->ed;

	switch (c) {
	case '\n': {
		editor_t* new_ed = fb_open(ed_global, input);
		edit_file(ed_global, new_ed ? new_ed : ed);
	}	break;

	case LT_TERM_KEY_BSPACE:
		if (!input.len)
			edit_file(ed_global, ed);
		else
			input.len--;
		break;

	case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!input.len)
			edit_file(ed_global, ed);
		while (input.len && input.str[--input.len] != '/')
			;
		break;

	default:
		if (c >= 32 && c < 127 && input.len < PATH_MAX_LEN)
			input.str[input.len++] = c;
		break;
	}
}


