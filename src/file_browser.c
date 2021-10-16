// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "file_browser.h"
#include "editor.h"
#include "highlight.h"
#include "allocators.h"

#include <string.h>
#include <stdlib.h>
#include <libgen.h>

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

usz fb_find_files(editor_t** out, usz out_count, lstr_t str) {
	usz buf_it = 0;

	for (usz i = 0; buf_it < out_count && i < file_count; ++i) {
		char* name = editors[i].doc.name;
		usz len = strlen(name);

		// Search at every possible offset
		for (usz j = 0; j + str.len <= len; ++j) {
			if (len >= str.len && memcmp(str.str, name + j, str.len) == 0) {
				out[buf_it++] = &editors[i];
				break;
			}
		}
	}

	return buf_it;
}

editor_t* fb_find_file(lstr_t str) {
	for (usz i = 0; i < file_count; ++i) {
		char* name = editors[i].doc.name;
		usz len = strlen(name);

		if (len == str.len && memcmp(str.str, name, str.len) == 0)
			return &editors[i];
	}

	return NULL;
}

editor_t* fb_open(global_t* ed_global, lstr_t path) {
	// TODO: Return error if file is already open

	if (path.len >= PATH_MAX_LEN)
		ferrf("Path '%s' is too long\n", path);

	editors = realloc(editors, sizeof(editor_t) * (file_count + 1));
	if (!editors)
		ferrf("Memory allocation failed: %s\n", os_err_str());

	// Allocate null terminated copy of path
	char* path_nt = malloc(path.len + 1);
	if (!path_nt)
		ferrf("Memory allocation failed: %s\n", os_err_str());
	memcpy(path_nt, path.str, path.len);
	path_nt[path.len] = 0;

	editor_t* new = &editors[file_count];
	*new = ed_make();
	new->doc = doc_make(path_nt, basename(path_nt));
	doc_load(&new->doc);
	new->highl_arena = aframe_alloc(GB(1));
	new->global = ed_global;
	new->restore = arestore_make(new->highl_arena);

	++file_count;
	return new;
}

void fb_close(editor_t* ed) {
	free(ed->doc.path);
	aframe_free(ed->highl_arena);
	doc_free(&ed->doc);

	--file_count;
	memmove(ed, ed + 1, ((usz)editors + file_count * sizeof(editor_t)) - (usz)ed);
}

