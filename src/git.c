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

void git_open_tracked(editor_t* ed, char* git_path, lstr_t repo_path) {
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
		if (repo_path.str) {
			char* repo_cpath = lt_lstos(repo_path, lt_libc_heap);
			execl(git_path, git_path, "-C", repo_cpath, "ls-files", NULL);
			lt_hmfree(repo_cpath);
		}
		else {
			execl(git_path, git_path, "ls-files", NULL);
		}
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
		lstr_t entry = lt_lssplit(str, '\n');
		lstr_t path = entry;

		if (repo_path.str) {
			path = lt_lsbuild(lt_libc_heap, "%S/%S", repo_path, path);
		}

		lt_stat_t st;
		if (lt_statp(path, &st) == LT_SUCCESS && st.type == LT_DIRENT_DIR) {
			git_open_tracked(ed, git_path, path);
		}
		else {
			fb_open(ed, path);
		}
		str = lt_lsdrop(str, entry.len + 1);

		if (repo_path.str) {
			lt_hmfree(path.str);
		}
	}

	lt_strstream_destroy(&ss);
}
