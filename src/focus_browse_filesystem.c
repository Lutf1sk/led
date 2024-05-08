// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#define _GNU_SOURCE

#include <lt/term.h>
#include <lt/texted.h>
#include <lt/mem.h>
#include <lt/darr.h>
#include <lt/str.h>
#include <lt/math.h>
#include <lt/io.h>

#include "focus.h"
#include "clr.h"
#include "editor.h"
#include "file_browser.h"
#include "common.h"
#include "draw.h"

#include <stdlib.h>

focus_t focus_browse_filesystem = { draw_browse_filesystem, NULL, input_browse_filesystem };

#define MAX_ENTRY_COUNT 15

isz selected_index = 0;
isz visible_index = 0;

typedef
struct file {
	lstr_t name;
	lt_dirent_type_t type;
} file_t;

lt_darr(file_t) files = NULL;

static
void update_file_list(void) {
	for (usz i = 0; i < lt_darr_count(files); ++i) {
		lt_mfree(lt_libc_heap, files[i].name.str);
	}
	lt_darr_clear(files);

	lstr_t input = lt_texted_line_str(line_input, 0);

	b8 free_input = 0;
	if (lt_lsprefix(input, CLSTR("~/"))) {
		char* home_path = getenv("HOME");
		input = lt_lsbuild(lt_libc_heap, "%s/%S", (home_path) ? home_path : "/home", lt_lsdrop(input, 2));
		free_input = 1;
	}

	lstr_t dirname = lt_lsdirname(input);
	lstr_t basename = input.len ? lt_lsbasename(input) : CLSTR("");
	if (lt_lssuffix(input, CLSTR("/"))) {
		dirname = input;
		basename = CLSTR("");
	}

	lt_dir_t* dir = lt_dopenp(dirname, lt_libc_heap);

	if (dir) {
		lt_dirent_t* entry = NULL;
		while ((entry = lt_dread(dir))) {
			if (lt_lseq(entry->name, CLSTR(".")) || lt_lseq(entry->name, CLSTR(".."))) {
				continue;
			}
			if (!lt_lsprefix(entry->name, basename)) {
				continue;
			}

			lstr_t name = lt_strdup(lt_libc_heap, entry->name);
			lt_darr_push(files, (file_t){ name, entry->type });
		}

		lt_dclose(dir, lt_libc_heap);
	}

	selected_index = lt_isz_max(lt_isz_min(selected_index, lt_darr_count(files) - 1), 0);
	visible_index = lt_isz_max(lt_isz_min(visible_index, lt_darr_count(files) - MAX_ENTRY_COUNT), 0);

	if (free_input) {
		lt_hmfree(input.str);
	}
}

void browse_filesystem(void) {
	focus = focus_browse_filesystem;
	lt_texted_clear(line_input);
	selected_index = 0;

	if (!files) {
		files = lt_darr_create(file_t, 256, lt_libc_heap);
	}
	update_file_list();
}

void draw_browse_filesystem(editor_t* ed, void* args) {
	isz start_height = lt_term_height - MAX_ENTRY_COUNT - 1;

	buf_set_pos(start_height, 0);
	buf_write_char(clr_attr[CLR_LIST_HEAD], ' ');
	buf_write_txed(clr_attr[CLR_EDITOR_SEL], clr_attr[CLR_LIST_HEAD], line_input);
	buf_write_char(clr_attr[CLR_LIST_HEAD] | ATTR_FILL, ' ');
	buf_write_char(0, 0);

	usz sx = cursor_x_to_screen_x(ed, lt_texted_line_str(line_input, 0), line_input->cursor_x);
	buf_set_cursor(start_height, sx + 1);

	usz visible_count = lt_usz_min(MAX_ENTRY_COUNT, lt_darr_count(files));

	for (usz i = 0; i < visible_count; ++i) {
		usz index = visible_index + i;

		u32 attr = ATTR_FG_WHT;

		buf_set_pos(start_height + i + 1, 0);
		if (files[index].type == LT_DIRENT_DIR) {
			attr = clr_attr[CLR_LIST_DIR];
		}
		else if (files[index].type == LT_DIRENT_SYMLINK) {
			attr = clr_attr[CLR_LIST_SYMLINK];
		}
		else if (files[index].type == LT_DIRENT_FILE) {
			attr = clr_attr[CLR_LIST_FILE];
		}

		if (index == selected_index) {
			attr = (attr & ~ATTR_BG_MASK) | (clr_attr[CLR_LIST_HIGHL] & ATTR_BG_MASK);
		}

		buf_write_char(attr, ' ');
		buf_write_utf8(attr | ATTR_FILL, files[index].name);
		if (files[index].type == LT_DIRENT_DIR) {
			buf_write_char(attr | ATTR_FILL, '/');
		}
		buf_write_char(0, 0);
	}

	for (usz i = visible_count; i < MAX_ENTRY_COUNT; ++i) {
		usz term_y = start_height + i + 1;
		buf_writeln_utf8(term_y, clr_attr[CLR_LIST_ENTRY] | ATTR_FILL, CLSTR("  "));
	}
}

void input_browse_filesystem(editor_t* ed, u32 c) {
	doc_t* doc = ed->doc;

	if (c != (LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL) && c != (LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL| LT_TERM_MOD_SHIFT)) {
		ed->consec_cdn = 0;
	}
	if (c != (LT_TERM_KEY_UP | LT_TERM_MOD_CTRL) && c != (LT_TERM_KEY_UP | LT_TERM_MOD_CTRL| LT_TERM_MOD_SHIFT)) {
		ed->consec_cup = 0;
	}

	switch (c) {
	case '\n': {
		lstr_t name = lt_texted_line_str(line_input, 0);
		if (name.len) {
			doc_t* new_doc = fb_open(ed, name);
			if (new_doc) {
				edit_file(ed, new_doc);
			}
			else {
				edit_file(ed, fb_first_file());
			}
		}
		else {
			edit_file(ed, doc);
		}
	}	break;

	case LT_TERM_KEY_TAB: {
		if (!lt_darr_count(files)) {
			return;
		}

		lstr_t name = files[selected_index].name;
		usz name_offs = lt_lssplit_bwd(lt_texted_line_str(line_input, 0), '/').len;
		lt_texted_input_str(line_input, LSTR(name.str + name_offs, name.len - name_offs));
		if (files[selected_index].type != LT_DIRENT_FILE) {
			input_term_key(line_input, '/');
		}
		update_file_list();
	}	break;

	case LT_TERM_KEY_UP: case 'k' | LT_TERM_MOD_ALT:
		if (selected_index && --selected_index < visible_index) {
			--visible_index;
		}
		break;
	case LT_TERM_KEY_DOWN: case 'j' | LT_TERM_MOD_ALT:
		if (selected_index < lt_darr_count(files) - 1 && ++selected_index >= visible_index + MAX_ENTRY_COUNT) {
			++visible_index;
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
			if (selected_index < lt_darr_count(files) - 1 && ++selected_index >= visible_index + MAX_ENTRY_COUNT) {
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
		if (input_term_key(line_input, c))
			update_file_list();
		break;
	}
}


