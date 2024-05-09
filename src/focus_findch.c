#include <lt/texted.h>
#include <lt/term.h>
#include <lt/ctype.h>
#include <lt/str.h>
#include <lt/math.h>

#include "editor.h"
#include "focus.h"

focus_t focus_findch = { draw_findch, NULL, input_findch };

static b8 bwd = 0;

void findch(b8 bwd_) {
	bwd = bwd_;
	focus = focus_findch;
}

void draw_findch(editor_t* editor, void* args) {

}

void input_findch(editor_t* ed, u32 c) {
	lt_texted_t* txed = &ed->doc->ed;
	switch (c) {
	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
	case LT_TERM_KEY_ESC:
		edit_file(ed, ed->doc);
		break;

	default:
		b8 res = 0;
		usz x, y;
		char cc = c;
		if (bwd) {
			res = lt_texted_find_last_occurence(txed, LSTR(&cc, 1), &x, &y);
		}
		else {
			res = lt_texted_find_next_occurence(txed, LSTR(&cc, 1), &x, &y);
		}
		if (res) {
			lt_texted_gotoxy(txed, x, y, 1);
		}
		edit_file(ed, ed->doc);
		break;
	}
}
