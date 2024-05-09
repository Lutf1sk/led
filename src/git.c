#include "git.h"
#include "file_browser.h"

#include <lt/strstream.h>
#include <lt/mem.h>
#include <lt/str.h>
#include <lt/io.h>

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

void git_open_tracked(editor_t* ed, char* git_path) {
	lt_strstream_t ss;
	lt_strstream_create(&ss, lt_libc_heap);

	int pfd[2];
	if (pipe(pfd) < 0) {
		return;
	}

	pid_t child = fork();
	if (child == 0) {
		close(pfd[0]);
		dup2(pfd[1], STDOUT_FILENO);
		int nullfd = open("/dev/null", O_WRONLY);
		dup2(nullfd, STDERR_FILENO);
		close(nullfd);
		close(pfd[1]);
		execl(git_path, git_path, "ls-files", NULL);
		exit(2);
	}

	close(pfd[1]);

	isz bytes = 0;
	char buf[4096];
	while ((bytes = read(pfd[0], buf, sizeof(buf)))) {
		if (bytes < 0) {
			break;
		}
		lt_strstream_write(&ss, buf, bytes);
	}

	wait(NULL);
	close(pfd[0]);

	lstr_t str = ss.str;
	while ((isz)str.len > 0) {
		lstr_t path = lt_lssplit(str, '\n');

		fb_open(ed, path);
		str = lt_lsdrop(str, path.len + 1);
	}

	lt_strstream_destroy(&ss);
}
