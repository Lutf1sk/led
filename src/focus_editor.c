// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/text.h>
#include <lt/term.h>
#include <lt/mem.h>
#include <lt/ctype.h>
#include <lt/str.h>
#include <lt/io.h>
#include <lt/math.h>

#include "focus.h"
#include "file_browser.h"
#include "editor.h"
#include "highlight.h"
#include "clipboard.h"
#include "command.h"
#include "keybinds.h"
#include "notify.h"

#include <string.h>
#include <stdlib.h>

focus_t focus_editor = { NULL, NULL, input_editor };

void edit_file(editor_t* ed, doc_t* doc) {
	focus = focus_editor;
	ed->doc = doc;
	if (doc)
		ed_regenerate_hl(ed);
}

void input_editor(editor_t* ed, u32 c) {
	doc_t* doc = ed->doc;
	lt_texted_t* txed = &doc->ed;

	if (!doc)
		return;

	u8 modified = 1;

	static b8 m1_pressed = 0;

	if (c != (LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL) && c != (LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL| LT_TERM_MOD_SHIFT)) {
		ed->consec_cdn = 0;
	}
	if (c != (LT_TERM_KEY_UP | LT_TERM_MOD_CTRL) && c != (LT_TERM_KEY_UP | LT_TERM_MOD_CTRL| LT_TERM_MOD_SHIFT)) {
		ed->consec_cup = 0;
	}

	switch (c) {
	case 'Q' | LT_TERM_MOD_CTRL: modified = 0;
		fb_close(doc);
		edit_file(ed, fb_first_file());
		return;

	case 'F' | LT_TERM_MOD_CTRL: modified = 0;
		find_local(txed->cursor_y, txed->cursor_x);
		break;

	case 'f' | LT_TERM_MOD_ALT: modified = 0;
		findch(0);
		break;

	case 'F' | LT_TERM_MOD_ALT: modified = 0;
		findch(1);
		break;

	case 'S' | LT_TERM_MOD_CTRL: modified = 0;
		if (ed->remove_trailing_indent) {
			remove_trailing_indent(ed);
		}

		if (!doc_save(doc)) {
			notify("failed to save document");
		}
		break;

	case '/' | LT_TERM_MOD_CTRL:
		// This is a pretty hackish way of toggling a comment,
		// might want to improve this later
		lstr_t comment_style = modes[ed->doc->hl_mode].comment_style;
		char stylex2[32];
		memcpy(stylex2, comment_style.str, comment_style.len);
		memcpy(stylex2 + comment_style.len, comment_style.str, comment_style.len);
		lt_texted_prefix_nonempty_selection(txed, comment_style);
		lt_texted_delete_selection_prefix(txed, LSTR(stylex2, comment_style.len * 2));
		break;

	case '\\' | LT_TERM_MOD_CTRL: modified = 0; case 'x' | LT_TERM_MOD_ALT:
		command();
		break;

	case 'u' | LT_TERM_MOD_ALT: modified = 0;
		reljump(1);
		break;

	case 'd' | LT_TERM_MOD_ALT: modified = 0;
		reljump(0);
		break;

	case 'R' | LT_TERM_MOD_CTRL: modified = 0;
		doc_free(ed->doc);
		doc_load(ed->doc, ed);
		break;

	// ----- MOUSE
	case LT_TERM_KEY_MB1_DN: modified = 0; {
		usz line = lt_isz_clamp(doc->line_top + (lt_term_mouse_y - ed->vstart), 0, lt_texted_line_count(txed) - 1);
		usz col = screen_x_to_cursor_x(ed, lt_texted_line_str(txed, line), lt_term_mouse_x - ed->hstart);
		lt_texted_gotoxy(txed, col, line, 1);
		m1_pressed = 1;
	}	break;

	case LT_TERM_KEY_MB1_UP: modified = 0;
		if (m1_pressed) {
			usz line = lt_isz_clamp(doc->line_top + (lt_term_mouse_y - ed->vstart), 0, lt_texted_line_count(txed) - 1);
			usz col = screen_x_to_cursor_x(ed, lt_texted_line_str(txed, line), lt_term_mouse_x - ed->hstart);
			lt_texted_gotoxy(txed, col, line, 0);
		}
		m1_pressed = 0;
		break;

	case LT_TERM_KEY_MPOS: modified = 0;
		if (m1_pressed) {
			usz line = lt_isz_clamp(doc->line_top + (lt_term_mouse_y - ed->vstart), 0, lt_texted_line_count(txed) - 1);
			usz col = screen_x_to_cursor_x(ed, lt_texted_line_str(txed, line), lt_term_mouse_x - ed->hstart);
			lt_texted_gotoxy(txed, col, line, 0);
		}
		break;

	// ----- UP

	case LT_TERM_KEY_UP | LT_TERM_MOD_SHIFT: modified = 0;
		lt_texted_cursor_up(txed, 0);
		break;

	case LT_TERM_KEY_UP: modified = 0;
		if (lt_texted_selection_present(txed) && txed->select_y != txed->cursor_y) {
			move_to_selection_start(txed);
			break;
		}
		lt_texted_cursor_up(txed, 1);
		break;

	case LT_TERM_KEY_UP | LT_TERM_MOD_CTRL: modified = 0; {
		if (lt_texted_selection_present(txed) && txed->select_y != txed->cursor_y) {
			move_to_selection_start(txed);
			break;
		}

		usz vstep = ++ed->consec_cup * ed->vstep;
		for (usz i = 0; i < vstep; ++i) {
			lt_texted_cursor_up(txed, 1);
		}
	}	break;

	case LT_TERM_KEY_UP | LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT: modified = 0; {
		usz vstep = ++ed->consec_cup * ed->vstep;
		for (usz i = 0; i < vstep; ++i) {
			lt_texted_cursor_up(txed, 0);
		}
	}	break;

	case 'P' | LT_TERM_MOD_ALT:
	case LT_TERM_KEY_UP | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT: modified = 0; {
		isz move_h = ed->height / 2;
		if (txed->cursor_y >= doc->line_top + move_h || doc->line_top == 0) {
			lt_texted_gotoy(txed, lt_isz_max(txed->cursor_y - move_h + ed->scroll_offs, 0), 0);
		}
		center_line(ed, txed->cursor_y);
	}	break;

	case 'p' | LT_TERM_MOD_ALT:
	case LT_TERM_KEY_UP | LT_TERM_MOD_ALT: modified = 0; {
		isz move_h = ed->height / 2;
		if (txed->cursor_y >= doc->line_top + move_h || doc->line_top == 0) {
			lt_texted_gotoy(txed, lt_isz_max(txed->cursor_y - move_h + ed->scroll_offs, 0), 1);
		}
		center_line(ed, txed->cursor_y);
	}	break;

	// ----- DOWN

	case LT_TERM_KEY_DOWN | LT_TERM_MOD_SHIFT: modified = 0;
		lt_texted_cursor_down(txed, 0);
		break;

	case LT_TERM_KEY_DOWN: modified = 0;
		if (lt_texted_selection_present(txed) && txed->select_y != txed->cursor_y) {
			move_to_selection_end(txed);
			break;
		}
		lt_texted_cursor_down(txed, 1);
		break;

	case LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL: modified = 0; {
		if (lt_texted_selection_present(txed) && txed->select_y != txed->cursor_y) {
			move_to_selection_end(txed);
			break;
		}

		usz vstep = ++ed->consec_cdn * ed->vstep;
		for (usz i = 0; i < vstep; ++i) {
			lt_texted_cursor_down(txed, 1);
		}
	}	break;

	case LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT: modified = 0; {
		usz vstep = ++ed->consec_cdn * ed->vstep;
		for (usz i = 0; i < vstep; ++i) {
			lt_texted_cursor_down(txed, 0);
		}
	}	break;

	case 'M' | LT_TERM_MOD_ALT:
	case LT_TERM_KEY_DOWN | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT: modified = 0; {
		isz move_h = ed->height / 2;
		usz line_count = lt_texted_line_count(txed);

		if (txed->cursor_y <= doc->line_top + move_h || doc->line_top + ed->height >= line_count) {
			lt_texted_gotoy(txed, txed->cursor_y + move_h - ed->scroll_offs, 0);
		}
		center_line(ed, txed->cursor_y);
	}	break;

	case 'm' | LT_TERM_MOD_ALT:
	case LT_TERM_KEY_DOWN | LT_TERM_MOD_ALT: modified = 0; {
		isz move_h = ed->height / 2;
		usz line_count = lt_texted_line_count(txed);

		if (txed->cursor_y <= doc->line_top + move_h || doc->line_top + ed->height >= line_count) {
			lt_texted_gotoy(txed, txed->cursor_y + move_h - ed->scroll_offs, 1);
		}
		center_line(ed, txed->cursor_y);
	}	break;

	// ----- RIGHT

	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_SHIFT: modified = 0; lt_texted_cursor_right(txed, 0); break;
	case LT_TERM_KEY_RIGHT: modified = 0; lt_texted_cursor_right(txed, 1); break;
	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT: modified = 0; lt_texted_step_right(txed, 0); break;
	case 'e' | LT_TERM_MOD_ALT:
	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_CTRL: modified = 0; lt_texted_step_right(txed, 1); break;
	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT: modified = 0; halfstep_right(ed, 0); break;
	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_ALT: modified = 0; halfstep_right(ed, 1); break;

	// ----- LEFT

	case LT_TERM_KEY_LEFT | LT_TERM_MOD_SHIFT: modified = 0; lt_texted_cursor_left(txed, 0); break;
	case LT_TERM_KEY_LEFT: modified = 0; lt_texted_cursor_left(txed, 1); break;
	case LT_TERM_KEY_LEFT | LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT: modified = 0; lt_texted_step_left(txed, 0); break;
	case 'b' | LT_TERM_MOD_ALT:
	case LT_TERM_KEY_LEFT | LT_TERM_MOD_CTRL: modified = 0; lt_texted_step_left(txed, 1); break;
	case LT_TERM_KEY_LEFT | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT: modified = 0; halfstep_left(ed, 0); break;
	case LT_TERM_KEY_LEFT | LT_TERM_MOD_ALT: modified = 0; halfstep_left(ed, 1); break;

	// ----- PAGE UP/DOWN

	case LT_TERM_KEY_PAGEUP: modified = 0;
		page_up(ed);
		break;

	case LT_TERM_KEY_PAGEDN: modified = 0;
		page_down(ed);
		break;

	// ----- DELETE

	case LT_TERM_KEY_DELETE | LT_TERM_MOD_ALT:
	case LT_TERM_KEY_DELETE | LT_TERM_MOD_CTRL: lt_texted_delete_word_fwd(txed); break;
	case LT_TERM_KEY_DELETE: lt_texted_delete_fwd(txed); break;

	// ----- BACKSPACE

	case LT_TERM_KEY_BSPACE | LT_TERM_MOD_ALT:
	case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL: lt_texted_delete_word_bwd(txed); break;
	case LT_TERM_KEY_BSPACE: lt_texted_delete_bwd(txed); break;

	// ----- TAB

	case LT_TERM_KEY_TAB | LT_TERM_MOD_SHIFT:
		unindent_selection(ed);
		break;

	case LT_TERM_KEY_TAB: {
		char space_buf[256];
		usz tab_size = lt_min(ed->tab_size, sizeof(space_buf));
		lt_mset8(space_buf, ' ', tab_size);

		if (lt_texted_selection_present(txed)) {
			if (ed->tabs_to_spaces) {
				lt_texted_prefix_nonempty_selection(txed, LSTR(space_buf, tab_size));
			}
			else {
				lt_texted_prefix_nonempty_selection(txed, CLSTR("\t"));
			}
		}
		else if (ed->tabs_to_spaces) {
			usz screen_pos = cursor_x_to_screen_x(ed, lt_texted_line_str(txed, txed->cursor_y), txed->cursor_x);
			usz space_count = ed->tab_size - screen_pos % ed->tab_size;
			lt_texted_input_str(txed, LSTR(space_buf, space_count));
		}
		else {
			lt_texted_input_str(txed, CLSTR("\t"));
		}
	}	break;

	// ----- MISC.

	case '\n':
		delete_selection_if_present(ed);
		lt_texted_break_line(txed);
		auto_indent(ed, txed->cursor_y);
		break;

	case ']' | LT_TERM_MOD_ALT: modified = 0; {
		doc_pos_t block_start = find_enclosing_block_end(ed, txed->cursor_x, txed->cursor_y);
		lt_texted_gotoxy(txed, block_start.x, block_start.y, 1);
	}	break;

	case '[' | LT_TERM_MOD_ALT: modified = 0; {
		doc_pos_t block_start = find_enclosing_block(ed, txed->cursor_x, txed->cursor_y);
		lt_texted_gotoxy(txed, block_start.x, block_start.y, 1);
	}	break;

	case LT_TERM_KEY_F4: modified = 0; {
		lstr_t ext = lt_lssplit_bwd(doc->name, '.');
		lstr_t name = doc->name;
		name.len -= ext.len;

		isz res = 0;
		lstr_t new_name = NLSTR();
		if (lt_lseq(ext, CLSTR("c"))) {
			res = lt_aprintf(&new_name, lt_libc_heap, "%Sh", name);
		}
		else if (lt_lseq(ext, CLSTR("h"))) {
			res = lt_aprintf(&new_name, lt_libc_heap, "%Sc", name);
		}
		else {
			break;
		}

		if (res < 0) {
			break;
		}

		doc_t* file = fb_find_file(new_name);
		if (file) {
			edit_file(ed, file);
		}

		lt_mfree(lt_libc_heap, new_name.str);
	}	break;

	case LT_TERM_KEY_BPASTE: {
		delete_selection_if_present(ed);
		while ((c = lt_term_getkey()) != LT_TERM_KEY_NBPASTE) {
			char utf8_buf[4];
			usz utf8_len = lt_utf8_encode(c, utf8_buf);
			lt_texted_input_str(txed, LSTR(utf8_buf, utf8_len));
		}
	}	break;

	default: {
		lstr_t cmd = lookup_keybind(c);
		if (cmd.len) {
			modified = execute_string(ed, cmd);
			break;
		}

		if (lt_is_unicode_control_char(c) || (c & (LT_TERM_KEY_SPECIAL_BIT | LT_TERM_MOD_MASK))) {
			modified = 0;
			break;
		}

		delete_selection_if_present(ed);

		char utf8_buf[4];
		usz utf8_len = lt_utf8_encode(c, utf8_buf);
		modified = lt_texted_input_str(txed, LSTR(utf8_buf, utf8_len));
	}	break;
	}

	adjust_vbounds(ed);

	if (modified) {
		doc->unsaved = 1;
		ed_regenerate_hl(ed);
	}
}

