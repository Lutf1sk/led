// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "file_browser.h"
#include "editor.h"
#include "highlight.h"
#include "focus.h"

#include <lt/str.h>
#include <lt/mem.h>
#include <lt/io.h>
#include <lt/math.h>
#include <lt/ctype.h>

static usz file_count = 0;
static doc_t* docs = NULL;

doc_t* fb_find_unsaved(void) {
	for (usz i = 0; i < file_count; ++i) {
		if (docs[i].unsaved) {
			return &docs[i];
		}
	}
	return NULL;
}

doc_t* fb_first_file(void) {
	return file_count ? docs : NULL;
}

typedef
struct scored_match {
	usz score;
	doc_t* doc;
} scored_match_t;

usz fb_find_files(doc_t** out, usz out_count, lstr_t str) {
	usz max_files = lt_min(out_count, file_count);

	if (!str.len) {
		for (usz i = 0; i < max_files; ++i) {
			out[i] = &docs[i];
		}
		return max_files;
	}

	lt_darr(scored_match_t) matches = lt_darr_create(scored_match_t, out_count, lt_libc_heap);

	for (usz i = 0; i < file_count; ++i) {
		lstr_t path = docs[i].path;

		char* it = str.str, *end = it + str.len;
		for (usz j = 0; j < path.len && it < end; ++j) {
			if (lt_to_lower(path.str[j]) == lt_to_lower(*it)) {
				++it;
			}
			else if (path.str[j] == '.') {
				; // !!
			}
			else {
				if (it == end) {
					lt_darr_push(matches, (scored_match_t){ .score = 0, .doc = &docs[i] });
					break;
				}
				it = str.str;
			}
		}
		if (it == end) {
			lt_darr_push(matches, (scored_match_t){ .score = 0, .doc = &docs[i] });
		}
	}

	usz found_count = lt_min(out_count, lt_darr_count(matches));
	for (usz i = 0; i < found_count; ++i) {
		out[i] = matches[i].doc;
	}

	lt_darr_destroy(matches);

	return found_count;
}

doc_t* fb_find_file(lstr_t str) {
	for (usz i = 0; i < file_count; ++i) {
		lstr_t name = docs[i].name;

		if (lt_lseq(name, str)) {
			return &docs[i];
		}
	}

	return NULL;
}

doc_t* fb_open(editor_t* ed, lstr_t path) {
	lt_stat_t stat;
	if (!lt_statp(path, &stat) && stat.type == LT_DIRENT_DIR) {
		lt_dir_t* dir = lt_dopenp(path, lt_libc_heap);
		if (!dir) {
			return NULL;
		}
		lt_foreach_dirent(ent, dir) {
			if (lt_lsprefix(ent->name, CLSTR("."))) {
				continue;
			}
			lstr_t ent_path = lt_lsbuild(lt_libc_heap, "%S/%S", path, ent->name);
			fb_open(ed, ent_path);
			lt_hmfree(ent_path.str);
		}
		return NULL;
	}

	// TODO: Return error if file is already open

	if (path.len >= PATH_MAX_LEN) {
		return NULL;
	}

	lt_dir_t* dir = lt_dopenp(path, lt_libc_heap);
	if (dir) {
		lt_dclose(dir, lt_libc_heap);
		return ed->doc;
	}

	docs = realloc(docs, sizeof(doc_t) * (file_count + 1));
	LT_ASSERT(docs);

	lstr_t new_path = lt_strdup(lt_libc_heap, path);
	LT_ASSERT(new_path.str);

	doc_t* new = &docs[file_count];
	memset(new, 0, sizeof(doc_t));
	new->path = new_path;
	new->name = lt_lsbasename(new_path);
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

