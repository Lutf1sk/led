// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/text.h>
#include <lt/ctype.h>
#include <lt/mem.h>
#include <lt/math.h>

#include "editor.h"
#include "highlight.h"

#include <string.h>

usz screen_x_to_cursor_x(editor_t* ed, lstr_t str, isz x) {
	isz screen_x = 0;

	for (char* it = str.str, *end = it + str.len; it < end;) {
		usz len;
		if (*it == '\t') {
			screen_x += ed->tab_size - screen_x % ed->tab_size;
			len = 1;
		}
		else {
			u32 c;
			len = lt_utf8_decode(it, &c);
			screen_x += lt_glyph_width(c);
		}

		if (screen_x > x) {
			return it - str.str;
		}
		it += len;
	}
	return str.len;
}

usz cursor_x_to_screen_x(editor_t* ed, lstr_t str, isz x) {
	isz screen_x = 0;

	for (char* it = str.str, *end = it + lt_min_isz(str.len, x); it < end;) {
		if (*it == '\t') {
			screen_x += ed->tab_size - screen_x % ed->tab_size;
			++it;
		}
		else {
			u32 c;
			it += lt_utf8_decode(it, &c);
			screen_x += lt_glyph_width(c);
		}
	}
	return screen_x;
}

void adjust_vbounds(editor_t* ed) {
	doc_t* doc = ed->doc;
	usz line = doc->ed.cursor_y;
	usz line_count = lt_texted_line_count(&doc->ed);

	// Move screen if cursor is above the upper boundary
	isz vbound_top = doc->line_top + ed->scroll_offs;
	if (line < vbound_top) {
		doc->line_top -= vbound_top - line;
		doc->line_top = lt_max_isz(doc->line_top, 0);
	}

	// Move screen if cursor is below the lower boundary
	isz vbound_bottom = (doc->line_top + ed->height) - ed->scroll_offs - 1;
	if (line > vbound_bottom) {
		doc->line_top += line - vbound_bottom;
		doc->line_top = lt_min_isz(doc->line_top, lt_max_isz(0, line_count - ed->height));
	}
}

void move_to_selection_start(lt_texted_t* ed) {
	isz start_y, start_x;
	lt_texted_get_selection(ed, &start_x, &start_y, NULL, NULL);
	lt_texted_gotoxy(ed, start_x, start_y, 1);
}

void move_to_selection_end(lt_texted_t* ed) {
	isz end_y, end_x;
	lt_texted_get_selection(ed, NULL, NULL, &end_x, &end_y);
	lt_texted_gotoxy(ed, end_x, end_y, 1);
}

void center_line(editor_t* ed, usz line) {
	doc_t* doc = ed->doc;
	lt_texted_t* txed = &doc->ed;
	usz line_count = lt_texted_line_count(txed);

	line = lt_clamp_isz(line, 0, line_count - 1);

	usz line_top = lt_max_isz(line - ed->height / 2, 0);
	if (line_top + ed->height >= line_count) {
		line_top = lt_max_isz(line_count - ed->height, 0);
	}

	doc->line_top = line_top;
}

void delete_selection_if_present(editor_t* ed) {
	lt_texted_t* txed = &ed->doc->ed;
	if (lt_texted_selection_present(txed)) {
		lt_texted_erase_selection(txed);
	}
}

void page_up(editor_t* ed) {
	doc_t* doc = ed->doc;
	lt_texted_t* txed = &doc->ed;
	usz line_max = lt_texted_line_count(txed) - 1;
	usz move_by = ed->height - 1;

	doc->line_top = lt_clamp_isz(doc->line_top - move_by, 0, line_max);
	lt_texted_gotoy(txed, lt_max_isz(txed->cursor_y - move_by, 0), 1);
}

void page_down(editor_t* ed) {
	doc_t* doc = ed->doc;
	lt_texted_t* txed = &doc->ed;
	usz line_max = lt_texted_line_count(txed) - 1;
	usz move_by = ed->height - 1;

	doc->line_top = lt_min_isz(doc->line_top + move_by, lt_max_isz(0, line_max - ed->height));
	lt_texted_gotoy(txed, lt_min_isz(txed->cursor_y + move_by, line_max), 1);
}

void ed_regenerate_hl(editor_t* ed) {
	lt_amrestore(ed->hl_arena, ed->hl_restore);
	ed->hl_lines = hl_generate(ed->doc, ed->doc->hl_mode, ed->hl_arena);
}

void halfstep_left(editor_t* ed, b8 sync_selection) {
	lt_texted_t* txed = &ed->doc->ed;
	usz len = lt_texted_line_len(txed, txed->cursor_y);
	usz indent = lt_texted_count_line_leading_indent(txed, txed->cursor_y);
	usz move_cols = (len - indent) / 2;
	usz move_to = txed->cursor_x - move_cols;
	if (indent == len) {
		move_to = 0;
	}
	lt_texted_gotox(txed, lt_clamp_isz(move_to, 0, len), sync_selection);
}

void halfstep_right(editor_t* ed, b8 sync_selection) {
	lt_texted_t* txed = &ed->doc->ed;
	usz len = lt_texted_line_len(txed, txed->cursor_y);
	usz indent = lt_texted_count_line_leading_indent(txed, txed->cursor_y);
	usz move_cols = (len - indent) / 2;
	usz move_to = txed->cursor_x + move_cols;
	if (txed->cursor_x < indent) {
		move_to = indent + move_cols;
	}
	lt_texted_gotox(txed, lt_clamp_isz(move_to, 0, len), sync_selection);
}

