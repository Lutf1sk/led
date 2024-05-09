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
#include <lt/shell.h>
#include <lt/conf.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

char* shell_path = NULL;
lt_darr(char*) shell_args = NULL;

void shell_load(lt_conf_t* cf) {
	if (shell_path) {
		lt_hmfree(shell_path);
	}
	if (shell_args) {
		for (usz i = 0; i < lt_darr_count(shell_args) && shell_args[i]; ++i) {
			lt_hmfree(shell_args[i]);
		}
		lt_darr_destroy(shell_args);
		shell_args = NULL;
	}
	shell_path = lt_lstos(lt_conf_find_str_default(cf, CLSTR("shell.path"), CLSTR("/bin/bash")), lt_libc_heap);

	lt_conf_t* args = lt_conf_find_array(cf, CLSTR("shell.args"), NULL);
	shell_args = lt_darr_create(char*, 16, lt_libc_heap);
	lt_darr_push(shell_args, shell_path);
	if (args) {
		for (usz i = 0; i < args->child_count; ++i) {
			lt_conf_t* arg = &args->children[i];
			if (arg->stype != LT_CONF_STRING) {
				lt_werrf("shell argument must be of type 'string'\n");
				continue;
			}
			lt_darr_push(shell_args, lt_lstos(arg->str_val, lt_libc_heap));
		}
	}
	lt_darr_push(shell_args, NULL);

}

void run_shell(void) {
	lt_term_restore();

	pid_t child = fork();
	if (child == 0) {

		if (strcmp(getenv("TERM"), "linux") == 0) {
			lstr_t final_write = CLSTR(RESET CSET(9999, 0) "\n");
			lt_term_write_direct(final_write.str, final_write.len);
		}

		setenv("LED", "1", true);
		execv(shell_path, shell_args);
	}
	lt_printf("\n");

	wait(NULL);

	lt_term_restore();
	lt_term_init(LT_TERM_BPASTE | LT_TERM_ALTBUF | LT_TERM_MOUSE | LT_TERM_UTF8);
}
