// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef DOC_H
#define DOC_H 1

#include "common.h"

#include <lt/mem.h>
#include <lt/texted.h>

typedef
struct doc {
	lstr_t path;
	lstr_t name;

	b8 unsaved;
	b8 read_only;
	b8 new;
	b8 leading_bom;
	b8 crlf;

	// ---
	isz line_top;
	lt_texted_t ed;

	u32 hl_mode;
} doc_t;

typedef
struct doc_pos {
	usz y;
	usz x;
} doc_pos_t;

usz find_str(doc_t* doc, lstr_t str, doc_pos_t* out_pos);
usz replace_str(doc_t* doc, lstr_t str, lstr_t repl);

void insert_char(doc_t* doc, usz line_index, usz index, char ch);
void erase_char(doc_t* doc, usz line_index, usz index);

void insert_str(doc_t* doc, usz line_index, usz index, lstr_t str);
void erase_str(doc_t* doc, usz line_index, usz index, usz len);

void split_line(doc_t* doc, usz line_index, usz index);
void merge_line(doc_t* doc, usz line_index);

void doc_load(doc_t* doc, editor_t* ed);
b8 doc_save(doc_t* doc);

void doc_free(doc_t* doc);

#endif
