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
struct global {
	editor_t* ed;

	isz tab_size;
	isz scroll_offs;
	isz vstep;
	u64 vstep_timeout_ms;

	isz width, height;
	isz hstart, vstart;

	u8 predict_brackets;
	u8 relative_linenums;

	lt_arena_t* hl_arena;
	void* hl_restore;
	u64 vstep_timeout_at_ms;
	u8 await_utf8;
	usz consec_cup;
	usz consec_cdn;
} global_t;

typedef
struct editor {
	global_t* global;

	isz line_top;

	isz cx, cy;
	isz target_cx, target_cy_offs;
	isz sel_x, sel_y;

	highl_t** hl_lines;
	u32 hl_mode;

	doc_t doc;
} editor_t;

void ed_move_to_selection_start(editor_t* ed);
void ed_move_to_selection_end(editor_t* ed);

void ed_center_line(editor_t* ed, usz line);
void ed_goto_line(editor_t* ed, usz line);

void ed_delete_selection_prefix(editor_t* ed, lstr_t pfx);
void ed_prefix_selection(editor_t* ed, lstr_t pfx);
void ed_prefix_nonempty_selection(editor_t* ed, lstr_t pfx);

b8 ed_selection_available(editor_t* ed);
void ed_delete_selection(editor_t* ed);
void ed_delete_selection_if_available(editor_t* ed);

usz ed_selection_len(editor_t* ed);
void ed_write_selection_str(editor_t* ed, lt_io_callback_t callb, void* usr);

b8 ed_get_selection(editor_t* ed, isz* out_start_y, isz* out_start_x, isz* out_end_y, isz* out_end_x);
void ed_sync_selection(editor_t* ed);

void ed_sync_target_cx(editor_t* ed);
void ed_sync_target_cy(editor_t* ed);

static LT_INLINE
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

void ed_expand_selection(editor_t* ed);

void ed_regenerate_hl(editor_t* ed);

#endif
