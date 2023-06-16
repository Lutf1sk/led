// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#define _GNU_SOURCE

#include <lt/term.h>
#include <lt/texted.h>
#include <lt/mem.h>
#include <lt/darr.h>
#include <lt/str.h>
#include <lt/math.h>

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

void browse_filesystem(void) {
	focus = focus_browse_filesystem;
	lt_led_clear(line_input);
}

typedef
struct file {
	lstr_t name;
	b8 highlight;
} file_t;

void draw_browse_filesystem(global_t* ed_globals, void* args) {
	(void)args;

	usz start_height = lt_term_height - MAX_ENTRY_COUNT;

	rec_goto(2, start_height);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_led(line_input, clr_strs[CLR_EDITOR_SEL], clr_strs[CLR_LIST_HEAD]);
	rec_str(" ");

	lstr_t input = lt_led_get_str(line_input);

	char dir_path[PATH_MAX_LEN + 1];
	memcpy(dir_path, input.str, input.len);
	dir_path[input.len] = 0;

	char* inp_name = "";

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

	lt_darr(file_t) files = lt_darr_create(file_t, 256, lt_libc_heap);

	if (dir) {
		usz inp_name_len = strlen(inp_name);

		struct dirent* entry = NULL;
		while ((entry = readdir(dir)) && lt_darr_count(files) < MAX_ENTRY_COUNT) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			if (strncmp(entry->d_name, inp_name, inp_name_len) != 0)
				continue;

			b8 highlight = entry->d_type == DT_DIR || entry->d_type == DT_LNK;
			lstr_t name = lt_strdup(lt_libc_heap, lt_lstr_from_cstr(entry->d_name));
			lt_darr_push(files, (file_t){ name, highlight });
		}
	}

	usz visible_index = 0;
	usz visible_count = lt_min_usz(MAX_ENTRY_COUNT, lt_darr_count(files));

	for (usz i = 0; i < visible_count; ++i) {
		usz index = visible_index + i;

		rec_goto(2, start_height + i + 1);

		if (files[index].highlight)
			rec_clearline(clr_strs[CLR_LIST_HIGHL]);
		else
			rec_clearline(clr_strs[CLR_LIST_ENTRY]);
		rec_lstr(files[index].name.str, files[index].name.len);
	}

	rec_str(clr_strs[CLR_LIST_ENTRY]);
	for (usz i = visible_count; i < MAX_ENTRY_COUNT; ++i) {
		rec_goto(0, start_height + i + 1);
		rec_clearline("");
	}

	for (usz i = 0; i < lt_darr_count(files); ++i)
		lt_mfree(lt_libc_heap, files[i].name.str);
	lt_darr_destroy(files);

	rec_crestore();
}

void input_browse_filesystem(global_t* ed_global, u32 c) {
	editor_t* ed = ed_global->ed;

	switch (c) {
	case '\n': {
		lstr_t name = lt_led_get_str(line_input);
		if (name.len) {
			editor_t* new_ed = fb_open(ed_global, name);
			edit_file(ed_global, new_ed ? new_ed : ed);
		}
		else
			edit_file(ed_global, ed);
	}	break;

	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!lt_darr_count(line_input->str))
	case LT_TERM_KEY_ESC:
			edit_file(ed_global, ed);
	default:
		input_term_key(line_input, c);
		break;
	}
}


