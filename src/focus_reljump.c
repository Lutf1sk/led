#include <lt/texted.h>
#include <lt/term.h>
#include <lt/ctype.h>
#include <lt/str.h>
#include <lt/math.h>

#include "editor.h"
#include "focus.h"

focus_t focus_reljump = { draw_reljump, NULL, input_reljump };

static b8 bwd = 0;

void reljump(b8 bwd_) {
	bwd = bwd_;
	lt_texted_clear(line_input);
	focus = focus_reljump;
}

void draw_reljump(editor_t* editor, void* args) {

}

void input_reljump(editor_t* ed, u32 c) {
	lt_texted_t* txed = &ed->doc->ed;
	switch (c) {
	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!lt_texted_line_len(line_input, 0)) {
		case LT_TERM_KEY_ESC:
			edit_file(ed, ed->doc);
		}
	default:
		if (lt_is_digit(c)) {
			input_term_key(line_input, c);
		}
		if (lt_texted_line_len(line_input, 0) != 2) {
			break;
		}
	case '\n': case ';':
		u64 line_offs = 0;
		lt_lstou(lt_texted_line_str(line_input, 0), &line_offs);
		isz new_y = txed->cursor_y;
		isz new_x = 0;
		if (bwd) {
			new_y -= line_offs;
			new_x = 0;
		}
		else {
			new_y += line_offs;
			new_x = -1;
		}

		lt_texted_gotoxy(txed, new_x, lt_isz_max(new_y, 0), 1);
		edit_file(ed, ed->doc);
		break;
	}
}
