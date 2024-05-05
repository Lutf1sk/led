#include "focus.h"
#include "editor.h"
#include "draw.h"
#include "clr.h"

#include <lt/io.h>
#include <lt/term.h>
#include <lt/shell.h>
#include <lt/str.h>
#define LT_ANSI_SHORTEN_NAMES 1
#include <lt/ansi.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

focus_t focus_shell = { draw_shell, NULL, input_shell };

void run_shell(void) {
	lt_term_restore();

	pid_t child = fork();
	if (child == 0) {
// 		lt_printf(FG_BYELLOW "led" RESET " > " FG_BYELLOW "running '/bin/bash'\n" RESET);
		setenv("LED", "1", true);
		execl("/bin/bash", "/bin/bash", NULL);
	}
	lt_printf("\n");

	wait(NULL);

	lt_term_restore();
	lt_term_init(LT_TERM_BPASTE | LT_TERM_ALTBUF | LT_TERM_MOUSE | LT_TERM_UTF8);
}

void draw_shell(editor_t* editor, void* args) {
	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_led(line_input, clr_strs[CLR_EDITOR_SEL], clr_strs[CLR_LIST_HEAD]);
	rec_str(" ");
	rec_crestore();
}

void input_shell(editor_t* ed, u32 c) {
	switch (c) {
	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!lt_texted_line_len(line_input, 0)) {
		case LT_TERM_KEY_ESC:
			edit_file(ed, ed->doc);
		}
	default:
		input_term_key(line_input, c);
		return;


	case '\n':
		edit_file(ed, ed->doc);
		break;
	}
}
