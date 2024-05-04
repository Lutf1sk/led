// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/text.h>
#include <lt/ctype.h>
#include <lt/mem.h>
#include <lt/math.h>
#include <lt/str.h>

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

	for (char* it = str.str, *end = it + lt_isz_min(str.len, x); it < end;) {
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
		doc->line_top = lt_isz_max(doc->line_top, 0);
	}

	// Move screen if cursor is below the lower boundary
	isz vbound_bottom = (doc->line_top + ed->height) - ed->scroll_offs - 1;
	if (line > vbound_bottom) {
		doc->line_top += line - vbound_bottom;
		doc->line_top = lt_isz_min(doc->line_top, lt_isz_max(0, line_count - ed->height));
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

	line = lt_isz_clamp(line, 0, line_count - 1);

	usz line_top = lt_isz_max(line - ed->height / 2, 0);
	if (line_top + ed->height >= line_count) {
		line_top = lt_isz_max(line_count - ed->height, 0);
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

	doc->line_top = lt_isz_clamp(doc->line_top - move_by, 0, line_max);
	lt_texted_gotoy(txed, lt_isz_max(txed->cursor_y - move_by, 0), 1);
}

void page_down(editor_t* ed) {
	doc_t* doc = ed->doc;
	lt_texted_t* txed = &doc->ed;
	usz line_max = lt_texted_line_count(txed) - 1;
	usz move_by = ed->height - 1;

	doc->line_top = lt_isz_min(doc->line_top + move_by, lt_isz_max(0, line_max - ed->height));
	lt_texted_gotoy(txed, lt_isz_min(txed->cursor_y + move_by, line_max), 1);
}

void ed_regenerate_hl(editor_t* ed) {
	lt_amrestore(ed->hl_arena, ed->hl_restore);
	ed->hl_lines = modes[ed->doc->hl_mode].generate_func(ed->doc, ed->hl_arena);
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
	lt_texted_gotox(txed, lt_isz_clamp(move_to, 0, len), sync_selection);
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
	lt_texted_gotox(txed, lt_isz_clamp(move_to, 0, len), sync_selection);
}

void unindent_selection(editor_t* ed) {
	lt_texted_t* txed = &ed->doc->ed;

	usz start_y, end_y;
	lt_texted_get_selection(txed, NULL, &start_y, NULL, &end_y);

	for (usz i = start_y; i <= end_y; ++i) {
		lstr_t line = lt_texted_line_str(txed, i);
		if (!line.len) {
			continue;
		}

		usz erase = 1;
		if (line.str[0] == ' ') {
			while (erase < ed->tab_size && erase < line.len && line.str[erase] == ' ') {
				++erase;
			}
		}
		else if (line.str[0] != '\t') {
			continue;
		}

		lt_darr_erase(txed->lines[i], 0, erase);

		if (i == txed->cursor_y) {
			txed->cursor_x = lt_isz_clamp(txed->cursor_x - erase, 0, lt_darr_count(txed->lines[txed->cursor_y]));
			lt_texted_sync_tx(txed);
		}
		if (i == txed->select_y) {
			txed->select_x = lt_isz_clamp(txed->select_x - erase, 0, lt_darr_count(txed->lines[txed->select_y]));
		}
	}
}

i8 bidir_tab[256] = {
	['('] = 1,  ['{'] = 1,  ['['] = 1,
	[')'] = -1, ['}'] = -1, [']'] = -1,
};

// returns new balance, if returned balance is 0; writes last tested index to `out_x`
isz bidir_match_balance_fwd(lstr_t str, isz bal, usz out_x[static 1]) {
	for (char* it = str.str, *end = it + str.len; it < end; ++it) {
		bal += bidir_tab[(u8)*it];
		if (!bal) {
			*out_x = it - str.str;
			return bal;
		}
	}
	return bal;
}

// returns new balance, if returned balance is 0; writes last tested index to `out_x`
isz bidir_match_balance_bwd(lstr_t str, isz bal, usz out_x[static 1]) {
	for (char* start = str.str, *it = start + str.len - 1; it >= start; --it) {
		bal += bidir_tab[(u8)*it];
		if (!bal) {
			*out_x = it - start;
			return bal;
		}
	}
	return bal;
}

isz bidir_str_balance(lstr_t str) {
	isz bal = 0;
	for (char* it = str.str, *end = it + str.len; it < end; ++it) {
		bal += bidir_tab[(u8)*it];
	}
	return bal;
}

doc_pos_t find_enclosing_block(editor_t* ed, isz x, isz y) {
	lt_texted_t* txed = &ed->doc->ed;
	isz bal = -1;
	for (;;) {
		if (y < 0) {
			return (doc_pos_t) { 0 };
		}
		lstr_t line_str = lt_texted_line_str(txed, y);
		if (x >= 0) {
			line_str.len = x;
			x = -1;
		}
		bal = bidir_match_balance_bwd(line_str, bal, &x);
		if (!bal) {
			return (doc_pos_t) { .x = x + 1, .y = y };
		}
		--y;
	}
}

doc_pos_t find_enclosing_block_end(editor_t* ed, isz x, isz y) {
	lt_texted_t* txed = &ed->doc->ed;
	isz bal = 1;
	isz line_count = lt_texted_line_count(txed);
	for (;;) {
		if (y >= line_count) {
			return (doc_pos_t) {
				.x = lt_texted_line_len(txed, line_count - 1),
				.y = line_count };
		}
		lstr_t line_str = lt_texted_line_str(txed, y);
		isz xoffs = 0;
		if (x >= 0) {
			line_str = lt_lsdrop(line_str, x);
			xoffs = x;
			x = -1;
		}
		bal = bidir_match_balance_fwd(line_str, bal, &x);
		if (!bal) {
			return (doc_pos_t) { .x = x + xoffs, .y = y };
		}
		++y;
	}
}

u8 indent_size_tab[256] = {
	[' '] = 1,
	['\t'] = 1,
	['\v'] = 1,
	['\r'] = 1,
	['\n'] = 1,
};

void update_tab_size(usz tab_size) {
	indent_size_tab['\t'] = tab_size;
}

isz indent_in_spaces(lstr_t str, usz tab_size) {
	isz indent = 0;
	for (char* it = str.str, *end = it + str.len; it < end; ++it) {
		indent += indent_size_tab[(u8)*it];
	}
	return indent;
}

lstr_t leading_indent(lstr_t str) {
	usz i = 0;
	while (i < str.len && indent_size_tab[(u8)str.str[i]]) {
		++i;
	}
	return lt_lstake(str, i);
}

lstr_t trailing_indent(lstr_t str) {
	for (char* start = str.str, *it = start + str.len - 1; it >= start; --it) {
		if (!indent_size_tab[(u8)*it]) {
			return lt_lsfrom_range(it + 1, str.str + str.len);
		}
	}
	return str;
}

usz find_nonempty_bwd(lt_texted_t* txed, usz line_idx) {
	for (isz i = line_idx - 1; i >= 0; --i) {
		if (lt_texted_line_len(txed, i)) {
			return i;
		}
	}
	return 0;
}

void auto_indent(editor_t* ed, usz line_idx) {
	lt_texted_t* txed = &ed->doc->ed;

	lstr_t line = lt_texted_line_str(txed, line_idx);

	isz erase_chars = leading_indent(line).len;
	lt_darr_erase(txed->lines[line_idx], 0, erase_chars);

	if (line_idx == txed->cursor_y) {
		txed->cursor_x = lt_isz_max(txed->cursor_x - erase_chars, 0);
		lt_texted_sync_selection(txed);
		lt_texted_sync_tx(txed);
	}

	if (!line_idx) {
		return;
	}

	usz first_nonempty_idx = find_nonempty_bwd(txed, line_idx);
	lstr_t first_nonempty = lt_texted_line_str(txed, first_nonempty_idx);

	lstr_t indent_str = leading_indent(first_nonempty);
	isz indent = indent_in_spaces(indent_str, ed->tab_size);
	isz dir = lt_isz_max(bidir_str_balance(first_nonempty), 0);

	if (first_nonempty_idx != line_idx && first_nonempty.len) {
		if (line.len && line.str[line.len - 1] == ':') {
			--dir;
		}

		char last_char = first_nonempty.str[first_nonempty.len - 1];
		if (last_char == ':' || last_char == ')') {
			++dir;
		}
		else {
			usz prev_line_idx = find_nonempty_bwd(txed, first_nonempty_idx);
			if (prev_line_idx != first_nonempty_idx) {
				lstr_t prev_line = lt_texted_line_str(txed, prev_line_idx);
				dir -= (prev_line.len && prev_line.str[prev_line.len - 1] == ')');
			}
		}
	}

	for (char* it = line.str, *end = it + line.len; it < end && bidir_tab[(u8)*it] < 0; ++it) {
		dir--;
	}

	indent -= (dir < 0) * ed->tab_size;
	indent += (dir > 0) * ed->tab_size;

	if (indent < 0) {
		indent = 0;
	}

	char indent_with = '\t';
	isz indent_chars = indent / ed->tab_size;
	if (ed->tabs_to_spaces) {
		indent_with = ' ';
		indent_chars *= ed->tab_size;
	}

	for (isz i = 0; i < indent_chars; ++i) {
		lt_darr_insert(txed->lines[line_idx], 0, &indent_with, 1);
	}

	if (line_idx == txed->cursor_y) {
		txed->cursor_x += indent_chars;
		lt_texted_sync_selection(txed);
		lt_texted_sync_tx(txed);
	}
}

void auto_indent_selection(editor_t* ed) {
	usz start_y, end_y;
	lt_texted_get_selection(&ed->doc->ed, NULL, &start_y, NULL, &end_y);

	for (usz i = start_y; i <= end_y; ++i) {
		auto_indent(ed, i);
	}
}

void remove_trailing_indent(editor_t* ed) {
	lt_texted_t* txed = &ed->doc->ed;
	usz line_count = lt_texted_line_count(txed);
	for (usz i = 0; i < line_count; ++i) {
		lstr_t line = lt_texted_line_str(txed, i);
		lstr_t trailing = trailing_indent(line);
		if (!(i == txed->cursor_y && txed->cursor_x + line.str > trailing.str)) {
			lt_darr_erase(txed->lines[i], trailing.str - line.str, trailing.len);
		}
	}
}
