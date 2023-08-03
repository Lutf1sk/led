#include <lt/term.h>

#include "focus.h"
#include "draw.h"
#include "editor.h"

focus_t focus_terminal = { draw_terminal, NULL, input_terminal };

void terminal(void) {
	focus = focus_terminal;
}

void draw_terminal(editor_t* ed_global, void* args) {
	// !!
}

void input_terminal(editor_t* ed, u32 c) {
	if (c == LT_TERM_KEY_ESC)
		edit_file(ed, ed->doc);
}

