// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef EDITOR_H
#define EDITOR_H 1

#include <lt/lt.h>
#include <lt/fwd.h>

#include "doc.h"

typedef struct highl highl_t;

typedef struct editor editor_t;

typedef
struct editor {
	doc_t* doc;

	isz tab_size;
	isz scroll_offs;
	isz vstep;
	u64 vstep_timeout_ms;

	isz width, height;
	isz hstart, vstart;

	u8 relative_linenums;

	u64 vstep_timeout_at_ms;
	u8 await_utf8;
	usz consec_cup;
	usz consec_cdn;

	lt_arena_t* hl_arena;
	void* hl_restore;
	highl_t** hl_lines;
} editor_t;


usz screen_x_to_cursor_x(editor_t* ed, lstr_t str, isz screen_x);
usz cursor_x_to_screen_x(editor_t* ed, lstr_t str, isz cursor_x);

void adjust_vbounds(editor_t* ed);

void move_to_selection_start(lt_texted_t* ed);
void move_to_selection_end(lt_texted_t* ed);

void center_line(editor_t* ed, usz line);

void delete_selection_if_present(editor_t* ed);

void page_up(editor_t* ed);
void page_down(editor_t* ed);

void ed_regenerate_hl(editor_t* ed);

void halfstep_left(editor_t* ed, b8 sync_selection);
void halfstep_right(editor_t* ed, b8 sync_selection);

#endif
