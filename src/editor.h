// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef EDITOR_H
#define EDITOR_H 1

#include "common.h"
#include "doc.h"
#include "allocators.h"

typedef struct highl highl_t;

typedef struct editor editor_t;

typedef
struct global {
	editor_t** ed;

	isz tab_size;
	isz scroll_offs;
	isz vstep;

	isz width, height;
	isz hstart, vstart;

	u8 predict_brackets;
	u8 await_utf8;
	u8 relative_linenums;
} global_t;

typedef
struct editor {
	global_t* global;

	isz line_top;

	isz cx, cy;
	isz target_cx, target_cy_offs;
	isz sel_x, sel_y;

	arestore_t restore;
	aframe_t* highl_arena;
	highl_t** highl_lines;

	doc_t doc;
} editor_t;

static inline INLINE
editor_t ed_make(void) {
	editor_t ed;
	ed.global = NULL;
	ed.line_top = 0;
	ed.cx = 0;
	ed.cy = 0;
	ed.target_cx = 0;
	ed.target_cy_offs = 0;
	ed.sel_x = 0;
	ed.sel_y = 0;
	ed.highl_arena = NULL;
	ed.highl_lines = NULL;
	return ed;
}

void ed_move_to_selection_start(editor_t* ed);
void ed_move_to_selection_end(editor_t* ed);

void ed_center_line(editor_t* ed, usz line);
void ed_goto_line(editor_t* ed, usz line);

void ed_delete_selection_prefix(editor_t* ed, lstr_t pfx);
void ed_prefix_selection(editor_t* ed, lstr_t pfx);

b8 ed_selection_available(editor_t* ed);
void ed_delete_selection(editor_t* ed);
void ed_delete_selection_if_available(editor_t* ed);

usz ed_selection_len(editor_t* ed);
void ed_write_selection_str(editor_t* ed, char* str);

b8 ed_get_selection(editor_t* ed, isz* out_start_y, isz* out_start_x, isz* out_end_y, isz* out_end_x);
void ed_sync_selection(editor_t* ed);

void ed_sync_target_cx(editor_t* ed);
void ed_sync_target_cy(editor_t* ed);

static inline INLINE
void ed_sync_target(editor_t* ed) {
	ed_sync_target_cx(ed);
	ed_sync_target_cy(ed);
}

usz ed_screen_x_to_cx(editor_t* ed, isz x, isz cy);
usz ed_cx_to_screen_x(editor_t* ed, isz x, isz cy);

void ed_cur_up(editor_t* ed, usz cx);
void ed_cur_down(editor_t* ed, usz cx);
void ed_cur_right(editor_t* ed);
void ed_cur_left(editor_t* ed);

void ed_page_up(editor_t* ed);
void ed_page_down(editor_t* ed);

usz ed_find_word_fwd(editor_t* ed);
usz ed_find_word_bwd(editor_t* ed);

void ed_delete_word_fwd(editor_t* ed);
void ed_delete_word_bwd(editor_t* ed);

void ed_paren_fwd(editor_t* ed);
void ed_paren_bwd(editor_t* ed);
void ed_paren_match(editor_t* ed);

isz ed_find_indent_pfx(editor_t* ed);
isz ed_find_indent(editor_t* ed);

#endif
