// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "file_browser.h"
#include "editor.h"
#include "highlight.h"

#include <lt/str.h>
#include <lt/mem.h>
#include <lt/io.h>

#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <ctype.h>

static usz file_count = 0;
static editor_t* editors = NULL;

editor_t* fb_find_unsaved(void) {
	for (usz i = 0; i < file_count; ++i) {
		if (editors[i].doc.unsaved)
			return &editors[i];
	}
	return NULL;
}

editor_t* fb_first_file(void) {
	return file_count ? editors : NULL;
}

b8 streq_case_insensitive(char* str1, char* str2, usz len) {
	for (usz i = 0; i < len; ++i)
		if (toupper(*str1++) != toupper(*str2++))
			return 0;
	return 1;
}

usz fb_find_files(editor_t** out, usz out_count, lstr_t str) {
	usz buf_it = 0;

	for (usz i = 0; buf_it < out_count && i < file_count; ++i) {
		lstr_t name = editors[i].doc.name;

		// Search at every possible offset
		for (usz j = 0; j + str.len <= name.len; ++j) {
			if (name.len >= str.len && streq_case_insensitive(str.str, name.str + j, str.len)) {
				out[buf_it++] = &editors[i];
				break;
			}
		}
	}

	return buf_it;
}

editor_t* fb_find_file(lstr_t str) {
	for (usz i = 0; i < file_count; ++i) {
		lstr_t name = editors[i].doc.name;

		if (lt_lstr_eq(name, str))
			return &editors[i];
	}

	return NULL;
}

editor_t* fb_open(global_t* ed_global, lstr_t path) {
	// TODO: Return error if file is already open

	if (path.len >= PATH_MAX_LEN)
		ferrf("Path '%s' is too long\n", path);

	editors = realloc(editors, sizeof(editor_t) * (file_count + 1));
	LT_ASSERT(editors);

	lstr_t new_path = LSTR(lt_malloc(lt_libc_heap, path.len), path.len);
	LT_ASSERT(new_path.str);
	memcpy(new_path.str, path.str, path.len);

	editor_t* new = &editors[file_count];
	memset(new, 0, sizeof(editor_t));
	new->doc = doc_make(new_path, lt_lstr_split_bwd(new_path, '/'));
	doc_load(&new->doc);
	new->global = ed_global;
	new->hl_lines = NULL;
	new->hl_mode = hl_find_mode_by_extension(path);

	++file_count;
	return new;
}

void fb_close(editor_t* ed) {
	free(ed->doc.path.str);
	doc_free(&ed->doc);

	--file_count;
	memmove(ed, ed + 1, ((usz)editors + file_count * sizeof(editor_t)) - (usz)ed);
}

