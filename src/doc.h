// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef DOC_H
#define DOC_H 1

#include "common.h"

typedef
struct doc {
	lstr_t path;
	lstr_t name;

	usz line_count;
	usz line_alloc_count;
	lstr_t* lines;

	b8 unsaved;
	b8 read_only;
	b8 new;
	b8 leading_bom;
	b8 crlf;
} doc_t;

typedef
struct doc_pos {
	usz y;
	usz x;
} doc_pos_t;

static LT_INLINE
doc_t doc_make(lstr_t path, lstr_t name) {
	doc_t doc;
	doc.path = path;
	doc.name = name;
	doc.line_count = 0;
	doc.line_alloc_count = 0;
	doc.lines = NULL;
	doc.new = 0;
	doc.unsaved = 0;
	doc.read_only = 0;
	doc.leading_bom = 0;
	doc.crlf = 0;
	return doc;
}

usz doc_find_str(doc_t* doc, lstr_t str, doc_pos_t* out_pos);
usz doc_replace_str(doc_t* doc, lstr_t str, lstr_t repl);

void doc_insert_char(doc_t* doc, usz line_index, usz index, char ch);
void doc_erase_char(doc_t* doc, usz line_index, usz index);

void doc_insert_str(doc_t* doc, usz line_index, usz index, lstr_t str);
void doc_erase_str(doc_t* doc, usz line_index, usz index, usz len);

void doc_split_line(doc_t* doc, usz line_index, usz index);
void doc_merge_line(doc_t* doc, usz line_index);

void doc_load(doc_t* doc);
b8 doc_save(doc_t* doc);

void doc_free(doc_t* doc);

#endif
