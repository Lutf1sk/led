// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/texted.h>
#include <lt/term.h>
#include <lt/str.h>
#include <lt/ctype.h>

#include "focus.h"
#include "draw.h"
#include "clr.h"
#include "editor.h"
#include "algo.h"
#include "clipboard.h"

focus_t focus_command = { draw_command, NULL, input_command };

void command(void) {
	focus = focus_command;
	lt_led_clear(line_input);
}

void draw_command(global_t* ed_global, void* args) {
	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_led(line_input, clr_strs[CLR_EDITOR_SEL], clr_strs[CLR_LIST_HEAD]);
	rec_str(" ");
	rec_crestore();
}

typedef
struct ctx {
	char* it;
	char* end;
} ctx_t;

typedef
struct pos {
	isz line;
	isz col;
} pos_t;

static global_t* ed_global = NULL;
static editor_t* ed = NULL;

static
usz parse_uint(ctx_t* cx) {
	char* start = cx->it;
	while (cx->it < cx->end && lt_is_digit(*cx->it))
		++cx->it;
	return lt_lstr_uint(LSTR(start, cx->it - start));
}

static
lstr_t parse_block(ctx_t* cx) {
	if (cx->it >= cx->end || *cx->it != '[')
		return NLSTR();
	++cx->it;

	char* start = cx->it;
	while (cx->it < cx->end && *++cx->it != ']')
		;
	return LSTR(start, cx->it - start);
}

static
pos_t parse_pos(ctx_t* cx) {
	pos_t pos;
	pos.line = ed->cy;
	pos.col = ed->cx;

	if (cx->it >= cx->end)
		return pos;
	switch (*cx->it++) {
	case 'f':
		pos.col += parse_uint(cx);
		return pos;

	case 'b':
		pos.col -= parse_uint(cx);
		return pos;

	case 'u':
		pos.line -= parse_uint(cx);
		return pos;

	case 'd':
		pos.line += parse_uint(cx);
		return pos;

	case 'w':
		if (cx->it >= cx->end)
			return pos;
		switch (*cx->it++) {
		case 'f':
			pos.col = ed_find_word_fwd(ed);
			return pos;
		case 'b':
			pos.col = ed_find_word_bwd(ed);
			return pos;
		}

	case 't':
		pos.line = 0;
		pos.col = 0;
		return pos;

	case 'e':
		pos.line = ed->doc.line_count - 1;
		pos.col = ed->doc.lines[pos.line].len;
		return pos;

	case 's':
		if (cx->it >= cx->end)
			return pos;

		switch (*cx->it++) {
		case 's':
			ed_get_selection(ed, &pos.line, &pos.col, NULL, NULL);
			return pos;

		case 'e':
			ed_get_selection(ed, NULL, NULL, &pos.line, &pos.col);
			return pos;
		}
		break;

	case 'l':
		if (cx->it >= cx->end)
			return pos;

		switch (*cx->it++) {
		case 's':
			pos.col = 0;
			return pos;

		case 'e':
			pos.col = ed->doc.lines[pos.line].len;
			return pos;

		default:
			pos.line = parse_uint(cx);
			return pos;
		}
		break;

	case 'i':
		pos.col = ed_find_indent(ed);
		return pos;

	case 'c':
		pos.col = parse_uint(cx);
		return pos;
	}

	return pos;
}

static
void execute(lstr_t cmd) {
	ctx_t cx;
	cx.it = cmd.str;
	cx.end = cmd.str + cmd.len;

	while (cx.it < cx.end) {
		b8 sync_selection = 0;

		switch (*cx.it++) {
		case 'j': sync_selection = 1;
		case 's': {
			pos_t pos = parse_pos(&cx);
			ed->cy = clamp(pos.line, 0, ed->doc.line_count - 1);
			ed->cx = clamp(pos.col, 0, ed->doc.lines[ed->cy].len);
			if (sync_selection)
				ed_sync_selection(ed);
		}	break;

		case 'c': {
			usz clipboard = parse_uint(&cx);
			if (clipboard >= CLIPBOARD_COUNT)
				break;
			clipboard_clear(clipboard);
			ed_write_selection_str(ed, (lt_io_callback_t)lt_strstream_write, &clipboards[clipboard]);
		}	break;

		case 'p': {
			usz clipboard = parse_uint(&cx);
			if (clipboard >= CLIPBOARD_COUNT)
				break;
			ed_insert_string(ed, clipboards[clipboard].str);
			ed_sync_selection(ed);
		}	break;

		case 'd':
			ed_delete_selection_if_available(ed);
			ed_sync_selection(ed);
			break;

		case 'l': {
			usz iterations = parse_uint(&cx);
			lstr_t block = parse_block(&cx);
			for (usz i = 0; i < iterations; ++i)
				execute(block);
		}	break;

		case 'i':
			ed_insert_string(ed, parse_block(&cx));
			ed_sync_selection(ed);
			break;
		}
	}
}

void input_command(global_t* ed_global_, u32 c) {
	ed_global = ed_global_;
	ed = ed_global_->ed;

	switch (c) {
	case '\n':
		execute(lt_led_get_str(line_input));
		lt_led_clear(line_input);
		edit_file(ed_global, ed);
		break;

	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!lt_darr_count(line_input->str))
	case LT_TERM_KEY_ESC:
			edit_file(ed_global, ed);
	default:
		input_term_key(line_input, c);
		break;
	}
}

