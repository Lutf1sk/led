// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "clipboard.h"

#include <lt/texted.h>
#include <lt/mem.h>
#include <lt/term.h>
#include <lt/utf8.h>
#include <lt/ctype.h>

focus_t focus = { NULL, NULL, NULL };

static lt_led_t line_input_;
lt_led_t* line_input = &line_input_;

void focus_none(void) {
	focus = (focus_t){ NULL, NULL, NULL };
}

void focus_init(void) {
	LT_ASSERT(!lt_led_create(line_input, lt_libc_heap));
}

b8 input_term_key(lt_led_t* ed, u32 key) {
	switch (key) {
	case LT_TERM_KEY_LEFT: lt_led_cursor_left(ed, 1); return 0;
	case LT_TERM_KEY_RIGHT: lt_led_cursor_right(ed, 1); return 0;
	case LT_TERM_KEY_LEFT|LT_TERM_MOD_CTRL: lt_led_step_left(ed, 1); return 0;
	case LT_TERM_KEY_RIGHT|LT_TERM_MOD_CTRL: lt_led_step_right(ed, 1); return 0;

	case LT_TERM_KEY_LEFT|LT_TERM_MOD_SHIFT: lt_led_cursor_left(ed, 0); return 0;
	case LT_TERM_KEY_RIGHT|LT_TERM_MOD_SHIFT: lt_led_cursor_right(ed, 0); return 0;
	case LT_TERM_KEY_LEFT|LT_TERM_MOD_CTRL|LT_TERM_MOD_SHIFT: lt_led_step_left(ed, 0); return 0;
	case LT_TERM_KEY_RIGHT|LT_TERM_MOD_CTRL|LT_TERM_MOD_SHIFT: lt_led_step_right(ed, 0); return 0;

	case LT_TERM_KEY_HOME: lt_led_gotox(ed, 0, 1); return 0;
	case LT_TERM_KEY_HOME|LT_TERM_MOD_SHIFT: lt_led_gotox(ed,0, 0); return 0;
	case LT_TERM_KEY_END: lt_led_gotox(ed, -1, 1); return 0;
	case LT_TERM_KEY_END|LT_TERM_MOD_SHIFT: lt_led_gotox(ed, -1, 0); return 0;
	case LT_TERM_KEY_HOME|LT_TERM_MOD_CTRL: lt_led_gotox(ed, 0, 1); return 0;
	case LT_TERM_KEY_HOME|LT_TERM_MOD_CTRL|LT_TERM_MOD_SHIFT: lt_led_gotox(ed,0, 0); return 0;
	case LT_TERM_KEY_END|LT_TERM_MOD_CTRL: lt_led_gotox(ed, -1, 1); return 0;
	case LT_TERM_KEY_END|LT_TERM_MOD_CTRL|LT_TERM_MOD_SHIFT: lt_led_gotox(ed, -1, 0); return 0;

	case LT_TERM_KEY_BSPACE: lt_led_delete_bwd(ed); return 1;
	case LT_TERM_KEY_DELETE: lt_led_delete_fwd(ed); return 1;
	case LT_TERM_KEY_BSPACE|LT_TERM_MOD_CTRL: lt_led_delete_word_bwd(ed); return 1;
	case LT_TERM_KEY_DELETE|LT_TERM_MOD_CTRL: lt_led_delete_word_fwd(ed); return 1;

	case 'X'|LT_TERM_MOD_CTRL: {
		if (!lt_led_selection_present(ed))
			return 0;
		clipboard_clear();
		lstr_t sel = lt_led_get_selection_str(ed);
		lt_strstream_write(&clipboard, sel.str, sel.len);
		lt_led_erase_selection(ed);
	}	return 1;

	case 'C'|LT_TERM_MOD_CTRL: {
		if (!lt_led_selection_present(ed))
			return 0;
		clipboard_clear();
		lstr_t sel = lt_led_get_selection_str(ed);
		lt_strstream_write(&clipboard, sel.str, sel.len);
	}	return 1;

	case 'V'|LT_TERM_MOD_CTRL:
		lt_led_input_str(ed, clipboard.str);
		return 1;

	case 'D'|LT_TERM_MOD_CTRL:
		lt_led_gotox(ed, 0, 1);
		lt_led_gotox(ed, -1, 0);
		return 0;

	default: {
		if (lt_is_unicode_control_char(key) || (key & (LT_TERM_KEY_SPECIAL_BIT | LT_TERM_MOD_MASK)))
			return 0;

		char utf8_buf[4];
		lstr_t utf8_str = LSTR(utf8_buf, lt_utf8_encode(utf8_buf, key));
		return lt_led_input_str(ed, utf8_str);
	}
	}
}

b8 texted_input_term_key(lt_texted_t* ed, u32 key) {
	switch (key) {
	case LT_TERM_KEY_LEFT: lt_texted_cursor_left(ed, 1); return 0;
	case LT_TERM_KEY_RIGHT: lt_texted_cursor_right(ed, 1); return 0;
	case LT_TERM_KEY_LEFT|LT_TERM_MOD_CTRL: lt_texted_step_left(ed, 1); return 0;
	case LT_TERM_KEY_RIGHT|LT_TERM_MOD_CTRL: lt_texted_step_right(ed, 1); return 0;
	case LT_TERM_KEY_UP: lt_texted_cursor_up(ed, 1); return 0;
	case LT_TERM_KEY_DOWN: lt_texted_cursor_down(ed, 1); return 0;
	case LT_TERM_KEY_UP|LT_TERM_MOD_CTRL: lt_texted_step_up(ed, 1); return 0;
	case LT_TERM_KEY_DOWN|LT_TERM_MOD_CTRL: lt_texted_step_down(ed, 1); return 0;

	case LT_TERM_KEY_LEFT|LT_TERM_MOD_SHIFT: lt_texted_cursor_left(ed, 0); return 0;
	case LT_TERM_KEY_RIGHT|LT_TERM_MOD_SHIFT: lt_texted_cursor_right(ed, 0); return 0;
	case LT_TERM_KEY_LEFT|LT_TERM_MOD_CTRL|LT_TERM_MOD_SHIFT: lt_texted_step_left(ed, 0); return 0;
	case LT_TERM_KEY_RIGHT|LT_TERM_MOD_CTRL|LT_TERM_MOD_SHIFT: lt_texted_step_right(ed, 0); return 0;
	case LT_TERM_KEY_UP|LT_TERM_MOD_SHIFT: lt_texted_cursor_up(ed, 0); return 0;
	case LT_TERM_KEY_DOWN|LT_TERM_MOD_SHIFT: lt_texted_cursor_down(ed, 0); return 0;
	case LT_TERM_KEY_UP|LT_TERM_MOD_CTRL|LT_TERM_MOD_SHIFT: lt_texted_step_up(ed, 0); return 0;
	case LT_TERM_KEY_DOWN|LT_TERM_MOD_CTRL|LT_TERM_MOD_SHIFT: lt_texted_step_down(ed, 0); return 0;

	case LT_TERM_KEY_HOME: lt_texted_gotox(ed, 0, 1); return 0;
	case LT_TERM_KEY_HOME|LT_TERM_MOD_SHIFT: lt_texted_gotox(ed, 0, 0); return 0;
	case LT_TERM_KEY_END: lt_texted_gotox(ed, -1, 1); return 0;
	case LT_TERM_KEY_END|LT_TERM_MOD_SHIFT: lt_texted_gotox(ed, -1, 0); return 0;
	case LT_TERM_KEY_HOME|LT_TERM_MOD_CTRL: lt_texted_gotoxy(ed, 0, 0, 1); return 0;
	case LT_TERM_KEY_HOME|LT_TERM_MOD_CTRL|LT_TERM_MOD_SHIFT: lt_texted_gotoxy(ed, 0, 0, 0); return 0;
	case LT_TERM_KEY_END|LT_TERM_MOD_CTRL: lt_texted_gotoxy(ed, -1, -1, 1); return 0;
	case LT_TERM_KEY_END|LT_TERM_MOD_CTRL|LT_TERM_MOD_SHIFT: lt_texted_gotoxy(ed, -1, -1, 0); return 0;

	case LT_TERM_KEY_BSPACE: lt_texted_delete_bwd(ed); return 1;
	case LT_TERM_KEY_DELETE: lt_texted_delete_fwd(ed); return 1;
	case LT_TERM_KEY_BSPACE|LT_TERM_MOD_CTRL: lt_texted_delete_word_bwd(ed); return 1;
	case LT_TERM_KEY_DELETE|LT_TERM_MOD_CTRL: lt_texted_delete_word_fwd(ed); return 1;

	case 'X'|LT_TERM_MOD_CTRL:
		if (!lt_texted_selection_present(ed))
			return 0;
		clipboard_clear();
		lt_texted_write_selection(ed, &clipboard, (lt_io_callback_t)lt_strstream_write);
		lt_texted_erase_selection(ed);
		return 1;

	case 'C'|LT_TERM_MOD_CTRL:
		if (!lt_texted_selection_present(ed))
			return 0;
		clipboard_clear();
		lt_texted_write_selection(ed, &clipboard, (lt_io_callback_t)lt_strstream_write);
		return 1;

	case 'V'|LT_TERM_MOD_CTRL:
		lt_texted_input_str(ed, clipboard.str);
		return 1;

	case 'D'|LT_TERM_MOD_CTRL:
		lt_texted_gotox(ed, 0, 1);
		lt_texted_gotox(ed, -1, 0);
		return 0;

	case '\n':
		lt_texted_break_line(ed);
		return 1;

	default: {
		if (lt_is_unicode_control_char(key) || (key & (LT_TERM_KEY_SPECIAL_BIT | LT_TERM_MOD_MASK)))
			return 0;

		char utf8_buf[4];
		lstr_t utf8_str = LSTR(utf8_buf, lt_utf8_encode(utf8_buf, key));
		return lt_texted_input_str(ed, utf8_str);
	}
	}
}

usz input_cursor_pos(lt_led_t* ed) {
	return lt_utf8_glyph_count(LSTR(ed->str, ed->cursor_pos));
}

