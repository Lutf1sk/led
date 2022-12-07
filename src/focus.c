// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"

#include <lt/textedit.h>
#include <lt/mem.h>
#include <lt/term.h>
#include <lt/utf8.h>

focus_t focus = { NULL, NULL, NULL };

static lt_lineedit_t line_input_;
lt_lineedit_t* line_input = &line_input_;

void focus_none(void) {
	focus = (focus_t){ NULL, NULL, NULL };
}

void focus_init(void) {
	LT_ASSERT(lt_lineedit_create(line_input, lt_libc_heap));
}

b8 input_term_key(lt_lineedit_t* ed, u32 key) {
	switch (key) {
	case LT_TERM_KEY_LEFT: lt_lineedit_cursor_left(ed); return 0;
	case LT_TERM_KEY_RIGHT: lt_lineedit_cursor_right(ed); return 0;
	case LT_TERM_KEY_LEFT|LT_TERM_MOD_CTRL: lt_lineedit_step_left(ed); return 0;
	case LT_TERM_KEY_RIGHT|LT_TERM_MOD_CTRL: lt_lineedit_step_right(ed); return 0;

	case LT_TERM_KEY_BSPACE: lt_lineedit_delete_bwd(ed); return 1;
	case LT_TERM_KEY_DELETE: lt_lineedit_delete_fwd(ed); return 1;
	case LT_TERM_KEY_BSPACE|LT_TERM_MOD_CTRL: lt_lineedit_delete_word_bwd(ed); return 1;
	case LT_TERM_KEY_DELETE|LT_TERM_MOD_CTRL: lt_lineedit_delete_word_fwd(ed); return 1;

	default: return lt_lineedit_input_str(ed, LSTR((char*)&key, 1));
	}
}

usz input_cursor_pos(lt_lineedit_t* ed) {
	return lt_utf8_glyph_count(LSTR(ed->str, ed->cursor_pos));
}

