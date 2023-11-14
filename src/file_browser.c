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
static doc_t* docs = NULL;

doc_t* fb_find_unsaved(void) {
	for (usz i = 0; i < file_count; ++i) {
		if (docs[i].unsaved)
			return &docs[i];
	}
	return NULL;
}

doc_t* fb_first_file(void) {
	return file_count ? docs : NULL;
}

b8 streq_case_insensitive(char* str1, char* str2, usz len) {
	for (usz i = 0; i < len; ++i)
		if (toupper(*str1++) != toupper(*str2++))
			return 0;
	return 1;
}

usz fb_find_files(doc_t** out, usz out_count, lstr_t str) {
	usz buf_it = 0;

	for (usz i = 0; buf_it < out_count && i < file_count; ++i) {
		lstr_t name = docs[i].name;

		// Search at every possible offset
		for (usz j = 0; j + str.len <= name.len; ++j) {
			if (name.len >= str.len && streq_case_insensitive(str.str, name.str + j, str.len)) {
				out[buf_it++] = &docs[i];
				break;
			}
		}
	}

	return buf_it;
}

doc_t* fb_find_file(lstr_t str) {
	for (usz i = 0; i < file_count; ++i) {
		lstr_t name = docs[i].name;

		if (lt_lseq(name, str))
			return &docs[i];
	}

	return NULL;
}

doc_t* fb_open(editor_t* ed, lstr_t path) {
	// TODO: Return error if file is already open

	if (path.len >= PATH_MAX_LEN)
		ferrf("Path '%s' is too long\n", path);

	docs = realloc(docs, sizeof(doc_t) * (file_count + 1));
	LT_ASSERT(docs);

	lstr_t new_path = LSTR(lt_malloc(lt_libc_heap, path.len), path.len);
	LT_ASSERT(new_path.str);
	memcpy(new_path.str, path.str, path.len);

	doc_t* new = &docs[file_count];
	memset(new, 0, sizeof(doc_t));
	new->path = new_path;
	new->name = lt_lssplit_bwd(new_path, '/');
	new->hl_mode = hl_find_mode_by_extension(path);
	doc_load(new, ed);

	++file_count;
	return new;
}

void fb_close(doc_t* ed) {
	free(ed->path.str);
	doc_free(ed);

	--file_count;
	memmove(ed, ed + 1, ((usz)docs + file_count * sizeof(doc_t)) - (usz)ed);
}

