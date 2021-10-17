// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "editor.h"
#include "token_chars.h"
#include "algo.h"
#include "utf8.h"

#include <string.h>

void ed_move_to_selection_start(editor_t* ed) {
	isz start_y, start_x, end_y, end_x;
	ed_get_selection(ed, &start_y, &start_x, &end_y, &end_x);
	ed->cx = start_x;
	ed->cy = start_y;
}

void ed_move_to_selection_end(editor_t* ed) {
	isz start_y, start_x, end_y, end_x;
	ed_get_selection(ed, &start_y, &start_x, &end_y, &end_x);
	ed->cx = end_x;
	ed->cy = end_y;
}

void ed_goto_line(editor_t* ed, usz line) {
	line = clamp(line, 0, ed->doc.line_count - 1);

	usz line_top = max(line - ed->global->height / 2, 0);
	if (line_top + ed->global->height >= ed->doc.line_count)
		line_top = max(ed->doc.line_count - ed->global->height, 0);

	ed->cy = line;
	ed->line_top = line_top;
	ed->cx = min(ed_screen_x_to_cx(ed, ed->target_cx, ed->cy), ed->doc.lines[ed->cy].len);
}

void ed_delete_selection_prefix(editor_t* ed, lstr_t pfx) {
	isz start_y, start_x, end_y, end_x;
	ed_get_selection(ed, &start_y, &start_x, &end_y, &end_x);

	for (isz i = start_y; i <= end_y; ++i) {
		lstr_t* line = &ed->doc.lines[i];
		if (line->len < pfx.len || memcmp(pfx.str, line->str, pfx.len) != 0)
			continue;

		if (i == ed->cy)
			ed->cx = max(ed->cx - pfx.len, 0);
		if (i == ed->sel_y)
			ed->sel_x = max(ed->sel_x - pfx.len, 0);

		doc_erase_str(&ed->doc, i, 0, pfx.len);
	}
}

void ed_prefix_selection(editor_t* ed, lstr_t pfx) {
	isz start_y, start_x, end_y, end_x;
	ed_get_selection(ed, &start_y, &start_x, &end_y, &end_x);

	for (isz i = start_y; i <= end_y; ++i)
		doc_insert_str(&ed->doc, i, 0, pfx);
	ed->cx += pfx.len;
	ed->sel_x += pfx.len;
}

b8 ed_selection_available(editor_t* ed) {
	return ed->cx != ed->sel_x || ed->cy != ed->sel_y;
}

void ed_delete_selection(editor_t* ed) {
	usz sel_len = ed_selection_len(ed);

	isz start_y, start_x, end_y, end_x;
	ed_get_selection(ed, &start_y, &start_x, &end_y, &end_x);
	ed->cy = end_y;
	ed->cx = end_x;

	// TODO: This is an extremely slow and naive hack and needs to be replaced
	for (usz i = 0; i < sel_len; ++i) {
		if (ed->cx)
			doc_erase_char(&ed->doc, ed->cy, --ed->cx);
		else if (ed->cy) {
			ed_cur_up(ed, ISIZE_MAX);
			doc_merge_line(&ed->doc, ed->cy + 1);
		}
	}
}

void ed_delete_selection_if_available(editor_t* ed) {
	if (ed_selection_available(ed))
		ed_delete_selection(ed);
}

usz ed_selection_len(editor_t* ed) {
	isz start_y, start_x, end_y, end_x;
	ed_get_selection(ed, &start_y, &start_x, &end_y, &end_x);

	usz len = 0;

	for (isz i = start_y, j = start_x; i <= end_y; ++i) {
		lstr_t* line = &ed->doc.lines[i];

		if (i == end_y)
			len += end_x - j;
		else
			len += line->len - j + 1;

		j = 0;
	}

	return len;
}

void ed_write_selection_str(editor_t* ed, char* str) {
	isz start_y, start_x, end_y, end_x;
	ed_get_selection(ed, &start_y, &start_x, &end_y, &end_x);

	char* it = str;
	for (isz i = start_y, j = start_x; i <= end_y; ++i) {
		lstr_t* line = &ed->doc.lines[i];

		if (i == end_y) {
			usz bytes = end_x - j;
			memcpy(it, &line->str[j], bytes);
			it += bytes;
		}
		else {
			usz bytes = line->len - j;
			memcpy(it, &line->str[j], bytes);
			it += bytes;
			*(it++) = '\n';
		}

		j = 0;
	}
}

b8 ed_get_selection(editor_t* ed, isz* out_start_y, isz* out_start_x, isz* out_end_y, isz* out_end_x) {
	//if (ed->cy == ed->sel_y && ed->cx == ed->sel_x)
	//	return 0;

	if (ed->cy < ed->sel_y || (ed->cy == ed->sel_y && ed->cx < ed->sel_x)) {
		*out_start_y = ed->cy;
		*out_start_x = ed->cx;
		*out_end_y = ed->sel_y;
		*out_end_x = ed->sel_x;
		return 1;
	}
	else {
		*out_start_y = ed->sel_y;
		*out_start_x = ed->sel_x;
		*out_end_y = ed->cy;
		*out_end_x = ed->cx;
		return 1;
	}
}

void ed_sync_selection(editor_t* ed) {
	ed->sel_y = ed->cy;
	ed->sel_x = ed->cx;
}

void ed_sync_target_cx(editor_t* ed) {
	ed->target_cx = ed_cx_to_screen_x(ed, ed->cx, ed->cy);
}

void ed_sync_target_cy(editor_t* ed) {
	ed->target_cy_offs = ed->cy - ed->line_top;
}

usz ed_screen_x_to_cx(editor_t* ed, isz x, isz cy) {
	if (cy >= ed->doc.line_count || cy < 0)
		return 0;

	lstr_t* line = &ed->doc.lines[cy];
	isz screen_x = 0, tab_size = ed->global->tab_size;

	for (isz i = 0, last = 0; i < line->len;) {
		char c = line->str[i];
		if (c == '\t') {
			screen_x += tab_size - screen_x % tab_size;
			++i;
		}
		else {
			i += utf8_decode_len(c);
			++screen_x;
		}
		if (screen_x > x)
			return last;
		last = i;
	}
	return line->len;
}

usz ed_cx_to_screen_x(editor_t* ed, isz x, isz cy) {
	if (cy >= ed->doc.line_count || cy < 0)
		return 0;

	lstr_t* line = &ed->doc.lines[cy];
	isz screen_x = 0, tab_size = ed->global->tab_size;

	isz end = min(x, line->len);

	for (isz i = 0; i < end;) {
		char c = line->str[i];
		if (c == '\t') {
			screen_x += tab_size - screen_x % tab_size;
			++i;
		}
		else {
			i += utf8_decode_len(c);
			++screen_x;
		}
	}
	return screen_x;
}

void ed_cur_up(editor_t* ed, usz cx) {
	if (ed->cy) {
		--ed->cy;
		ed->cx = min(cx, ed->doc.lines[ed->cy].len);
	}
}

void ed_cur_down(editor_t* ed, usz cx) {
	if (ed->cy + 1 < ed->doc.line_count) {
		++ed->cy;
		ed->cx = min(cx, ed->doc.lines[ed->cy].len);
	}
}

void ed_cur_right(editor_t* ed) {
	lstr_t line = ed->doc.lines[ed->cy];

	if (ed->cx == line.len) {
		ed_cur_down(ed, 0);
		return;
	}

	usz utf8_len = utf8_decode_len(line.str[ed->cx]);

	if (ed->cx + utf8_len <= line.len)
		ed->cx += utf8_len;
}

void ed_cur_left(editor_t* ed) {
	lstr_t line = ed->doc.lines[ed->cy];

	if (!ed->cx) {
		ed_cur_up(ed, ISIZE_MAX);
		return;
	}

	--ed->cx;
	while ((line.str[ed->cx] & 0xC0) == 0x80)
		--ed->cx;
}

void ed_page_up(editor_t* ed) {
	if (ed->line_top + ed->global->height >= ed->doc.line_count && ed->cy + 1 == ed->doc.line_count) {
		if (ed->cy == ed->line_top + ed->target_cy_offs)
			ed->target_cy_offs = ed->global->height / 2;
		ed->cy = clamp(ed->line_top + ed->target_cy_offs, 0, ed->doc.line_count - 1);
	}
	else if (!ed->line_top)
		ed->cy = 0;
	else {
		usz offs = min(ed->line_top, ed->global->height);
		ed->cy -= offs;
		ed->line_top -= offs;
	}
	ed->cx = min(ed_screen_x_to_cx(ed, ed->target_cx, ed->cy), ed->doc.lines[ed->cy].len);
}

void ed_page_down(editor_t* ed) {
	if (!ed->line_top && !ed->cy) {
		if (ed->cy == ed->line_top + ed->target_cy_offs)
			ed->target_cy_offs = ed->global->height / 2;
		ed->cy = clamp(ed->line_top + ed->target_cy_offs, 0, ed->doc.line_count - 1);
	}
	else if (ed->line_top + ed->global->height >= ed->doc.line_count)
		ed->cy = ed->doc.line_count - 1;
	else {
		usz offs = clamp(ed->doc.line_count - (ed->line_top + ed->global->height), 0, ed->global->height);
		ed->cy += offs;
		ed->line_top += offs;
	}
	ed->cx = min(ed_screen_x_to_cx(ed, ed->target_cx, ed->cy), ed->doc.lines[ed->cy].len);
}

usz ed_find_word_fwd(editor_t* ed) {
	usz cx = ed->cx;
	lstr_t* line = &ed->doc.lines[ed->cy];
	char* str = line->str;

	// Skip whitespace
	while (cx < line->len && is_space(str[cx]))
		++cx;

	if (cx == line->len)
		return cx;
	char c = str[cx];

	if (is_ident_head(c) || is_numeric_head(c)) {
		// Skip underscores
		while (cx < line->len && str[cx] == '_')
			++cx;
		while (cx < line->len && is_numeric_body(str[cx]))
			cx += utf8_decode_len(str[cx]);
	}
	else {
		while (cx < line->len && !is_ident_body(str[cx]) && !is_space(str[cx]))
			++cx;
	}

	return cx;
}

usz ed_find_word_bwd(editor_t* ed) {
	usz cx = ed->cx;
	lstr_t* line = &ed->doc.lines[ed->cy];
	char* str = line->str;

	// Skip whitespace
	while (cx && is_space(str[cx - 1]))
		--cx;

	if (!cx)
		return 0;
	char c = str[cx - 1];

	if (is_ident_head(c) || is_numeric_head(c)) {
		// Skip underscores
		while (cx && str[cx - 1] == '_')
			--cx;
		while (cx && is_numeric_body(str[cx - 1])) {
			--cx;
			while ((str[cx - 1] & 0xC0) == 0x80)
				--cx;
		}
	}
	else {
		while (cx && !is_ident_body(str[cx - 1]) && !is_space(str[cx - 1]))
			--cx;
	}

	return cx;
}

void ed_delete_word_fwd(editor_t* ed) {
	if (ed->cx == ed->doc.lines[ed->cy].len) {
		if (ed->cy + 1 < ed->doc.line_count)
			doc_merge_line(&ed->doc, ed->cy + 1);
		return;
	}

	usz del_chars = ed_find_word_fwd(ed) - ed->cx;
	doc_erase_str(&ed->doc, ed->cy, ed->cx, del_chars);
}

void ed_delete_word_bwd(editor_t* ed) {
	if (!ed->cx) {
		if (ed->cy) {
			ed_cur_up(ed, ISIZE_MAX);
			doc_merge_line(&ed->doc, ed->cy + 1);
		}
		return;
	}

	usz word = ed_find_word_bwd(ed);
	usz del_chars = ed->cx - word;
	doc_erase_str(&ed->doc, ed->cy, word, del_chars);
	ed->cx -= del_chars;
}

void ed_paren_fwd(editor_t* ed) {
	for (isz l = ed->cy, r = ed->cx + 1; l < ed->doc.line_count; ++l) {
		lstr_t line = ed->doc.lines[l];

		for (; r < line.len; ++r) {
			if (line.str[r] == ')' || line.str[r] == ']' || line.str[r] == '}') {
				ed->cy = l;
				ed->cx = r;
				return;
			}
		}
		r = 0;
	}
}

void ed_paren_bwd(editor_t* ed) {
	for (isz l = ed->cy, r = ed->cx - 1; l >= 0; --l) {
		lstr_t line = ed->doc.lines[l];

		for (; r >= 0; --r) {
			if (line.str[r] == '(' || line.str[r] == '[' || line.str[r] == '{') {
				ed->cy = l;
				ed->cx = r;
				return;
			}
		}

		if (l > 0)
			r = ed->doc.lines[l - 1].len - 1;
	}
}

// TODO: This will not match properly if it has to iterate over a string/char
// containing unmatched parenthesis
static
void ed_paren_match_ch(editor_t* ed, int paren) {
	isz r = ed->cx, l = ed->cy, sc = 1;

	switch (paren) {
	case '[': case '{': case '(':
		goto fwd;

	default:
		break;
	}

	for (--r; l >= 0; --l) {
		lstr_t line = ed->doc.lines[l];

		for (; r >= 0; --r) {
			int c = line.str[r];
			switch (c) {
			case '[': case '{': case '(':
				--sc;
				break;
			case ']': case '}': case ')':
				++sc;
				break;

			default:
				continue;
			}

			if (sc <= 0) {
				ed->cy = l;
				ed->cx = r;
				return;
			}
		}

		if (l > 0)
			r = ed->doc.lines[l - 1].len - 1;
	}
	return;

fwd:
	for (++r; l < ed->doc.line_count; ++l) {
		lstr_t line = ed->doc.lines[l];

		for (; r < line.len; ++r) {
			int c = line.str[r];
			switch (c) {
			case '[': case '{': case '(':
				++sc;
				break;
			case ']': case '}': case ')':
				--sc;
				break;

			default:
				continue;
			}

			if (sc <= 0) {
				ed->cy = l;
				ed->cx = r;
				return;
			}
		}
		r = 0;
	}
}

void ed_paren_match(editor_t* ed) {
	int c = 0;
	if (ed->cx < ed->doc.lines[ed->cy].len)
		c = ed->doc.lines[ed->cy].str[ed->cx];
	ed_paren_match_ch(ed, c);
}

