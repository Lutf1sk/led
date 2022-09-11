#include <lt/term.h>

#include "focus.h"
#include "draw.h"
#include "editor.h"

focus_t focus_terminal = { draw_terminal, NULL, input_terminal };

void terminal(void) {
	focus = focus_terminal;
}

void draw_terminal(global_t* ed_global, void* args) {
	// !!
}

void input_terminal(global_t* ed_global, u32 c) {
	editor_t* ed = ed_global->ed;

	if (c == LT_TERM_KEY_ESC)
		edit_file(ed_global, ed);
}

