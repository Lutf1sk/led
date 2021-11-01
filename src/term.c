// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "term.h"
#include "utf8.h"

#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <ctype.h>

static lt_term_flags_t term_flags;
static struct termios old_term;

u32 lt_term_width = 0, lt_term_height = 0;
i32 lt_term_mouse_x = 0, lt_term_mouse_y = 0;

static volatile u8 resized = 0;

static
void lt_update_term_dimensions(void) {
	struct winsize wsz;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsz);

	lt_term_width = wsz.ws_col;
	lt_term_height = wsz.ws_row;
}

static
void lt_handle_winch(int sig) {
	resized = 1;
}

void lt_term_init(lt_term_flags_t flags) {
	term_flags = flags;

	struct termios term;
	tcgetattr(STDOUT_FILENO, &term);
	old_term = term;

	if (!(flags & LT_TERM_ECHO))
		term.c_lflag &= ~ECHO;
	if (!(flags & LT_TERM_CANON))
		term.c_lflag &= ~ICANON;
	if (!(flags & LT_TERM_SIGNAL))
		term.c_lflag &= ~ISIG;

	tcsetattr(STDOUT_FILENO, TCSANOW, &term);
	tcflush(STDOUT_FILENO, TCIOFLUSH);

	if (flags & LT_TERM_MOUSE)
		write(STDOUT_FILENO, "\x1B[?1003h", 8);
	if (flags & LT_TERM_BPASTE)
		write(STDOUT_FILENO, "\x1b[?2004h", 8);
	if (flags & LT_TERM_ALTBUF)
		write(STDOUT_FILENO, "\x1b[?1049h", 8);

	fsync(STDOUT_FILENO);

	struct sigaction sact = {0};
	sact.sa_handler = lt_handle_winch;
	sigaction(SIGWINCH, &sact, NULL);

	lt_update_term_dimensions();
}

void lt_term_restore(void) {
	tcsetattr(STDOUT_FILENO, TCSANOW, &old_term);
	tcflush(STDOUT_FILENO, TCIOFLUSH);

	if (term_flags & LT_TERM_MOUSE)
		write(STDOUT_FILENO, "\x1B[?1003l", 8);
	if (term_flags & LT_TERM_BPASTE)
		write(STDOUT_FILENO, "\x1b[?2004l", 8);
	if (term_flags & LT_TERM_ALTBUF)
		write(STDOUT_FILENO, "\x1b[?1049l", 8);

	fsync(STDOUT_FILENO);
}

static
int lt_getc(void) {
	unsigned char c;
	if (read(STDIN_FILENO, &c, 1) < 0)
		return 0;
	return c;
}

static
u8 is_param_byte(u8 c) {
	return (c >= 0x30) && (c <= 0x3F);
}

static
u8 is_intermediate_byte(u8 c) {
	return (c >= 0x20) && (c <= 0x2F);
}

// static
// u8 is_final_byte(u8 c) {
// 	return (c >= 0x40) && (c <= 0x7E);
// }

u32 lt_term_getkey(void) {
	if (resized) {
		resized = 0;
		lt_update_term_dimensions();
		return LT_TERM_KEY_RESIZE;
	}

	u32 c = lt_getc();
	u32 mod = 0;

	if (c == 0x1B) {
		struct pollfd pfd;
		pfd.fd = STDIN_FILENO;
		pfd.events = POLLIN;

		if (!poll(&pfd, 1, 1))
			return LT_TERM_KEY_ESC;

		c = lt_getc();
		if (c == 'O') { // Parse F1 - F4
			if (!poll(&pfd, 1, 1))
				return 'O' | LT_TERM_MOD_ALT;
			c = lt_getc();
			switch (c) {
			case 'P': return LT_TERM_KEY_F1;
			case 'Q': return LT_TERM_KEY_F2;
			case 'R': return LT_TERM_KEY_F3;
			case 'S': return LT_TERM_KEY_F4;
			default: return 0;
			}
		}

		if (c != '[' || !poll(&pfd, 1, 1)) {
			mod = LT_TERM_MOD_ALT;
			goto unescaped;
		}

		u32 param[16];
		u16 param_count = 0;
		u8 param_len = 0;

		c = lt_getc();
		// Read all parameter bytes
		while (is_param_byte(c)) {
			if (isdigit(c)) {
				if (!param_len++)
					param[param_count++] = 0;
				param[param_count - 1] *= 10;
				param[param_count - 1] += c - '0';
			}
			else if (c == ';') {
				if (!param_len)
					param[param_count++] = 1;
				param_len = 0;
			}
			c = lt_getc();
		}

		// Skip all intermediate bytes (if there are any)
		while (is_intermediate_byte(c))
			c = lt_getc();

		// Convert modifiers if possible
		if (param_count >= 2) {
			switch (param[1]) {
			case 1: mod = 0; break;
			case 2: mod = LT_TERM_MOD_SHIFT; break;
			case 3: mod = LT_TERM_MOD_ALT; break;
			case 4: mod = LT_TERM_MOD_SHIFT | LT_TERM_MOD_ALT; break;
			case 5: mod = LT_TERM_MOD_CTRL; break;
			case 6: mod = LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT; break;
			case 7: mod = LT_TERM_MOD_CTRL | LT_TERM_MOD_ALT; break;
			case 8: mod = LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT | LT_TERM_MOD_ALT; break;
			}
		}

		// Convert special characters
		if (param_count < 1 || param[0] == 1) {
			switch (c) {
			case 'A': return LT_TERM_KEY_UP | mod;
			case 'B': return LT_TERM_KEY_DOWN | mod;
			case 'C': return LT_TERM_KEY_RIGHT | mod;
			case 'D': return LT_TERM_KEY_LEFT | mod;
			case 'H': return LT_TERM_KEY_HOME | mod;
			case 'F': return LT_TERM_KEY_END | mod;
			case 'P': return LT_TERM_KEY_F1 | mod;
			case 'Q': return LT_TERM_KEY_F2 | mod;
			case 'R': return LT_TERM_KEY_F3 | mod;
			case 'S': return LT_TERM_KEY_F4 | mod;
			case 'Z': return LT_TERM_KEY_TAB | LT_TERM_MOD_SHIFT | mod;

			case 'M': { // Mouse event
				u8 b = lt_getc() - 32;
				u8 x = lt_getc() - 32;
				u8 y = lt_getc() - 32;
				lt_term_mouse_x = x - 1;
				lt_term_mouse_y = y - 1;

				switch (b & 3) {
				case 0: c = LT_TERM_KEY_MB1_DN; break;
				case 1: c = LT_TERM_KEY_MB2_DN; break;
				case 2: c = LT_TERM_KEY_MB3_DN; break;
				case 3: c = LT_TERM_KEY_MB1_UP; break;
				}

				if (b >= 32) {
					b -= 32;
					c = LT_TERM_KEY_MPOS;
				}

				if (b & 4) mod |= LT_TERM_MOD_SHIFT;
				if (b & 8) mod |= LT_TERM_MOD_ALT;
				if (b & 16) mod |= LT_TERM_MOD_CTRL;
				return c | mod;
			}	break;

			default: return 0;
			}
		}

		if (param_count >= 1 || c == '~') {
			switch (param[0]) {
			case 2: return LT_TERM_KEY_INSERT | mod;
			case 3: return LT_TERM_KEY_DELETE | mod;
			case 5: return LT_TERM_KEY_PAGEUP | mod;
			case 6: return LT_TERM_KEY_PAGEDN | mod;
			case 15: return LT_TERM_KEY_F5 | mod;
			case 17: return LT_TERM_KEY_F6 | mod;
			case 18: return LT_TERM_KEY_F7 | mod;
			case 19: return LT_TERM_KEY_F8 | mod;
			case 20: return LT_TERM_KEY_F9 | mod;
			case 21: return LT_TERM_KEY_F10 | mod;
			case 23: return LT_TERM_KEY_F11 | mod;
			case 24: return LT_TERM_KEY_F12 | mod;
			case 200: return LT_TERM_KEY_BPASTE;
			case 201: return LT_TERM_KEY_NBPASTE;
			default: return 0;
			}
		}

		return 0;
	}
unescaped:

	// Sort out CTRL+alpha
	if (c < 32 && c) {
		switch (c) {
		case 0: case '\t': case '\n': case LT_TERM_KEY_ESC: return c | mod;
		case '\b': return LT_TERM_KEY_BSPACE | mod | LT_TERM_MOD_CTRL;
		case 0x1F: return '/' | mod | LT_TERM_MOD_CTRL;

		default:
			c += 64;
			mod |= LT_TERM_MOD_CTRL;
			break;
		}
	}

	if (term_flags & LT_TERM_UTF8) {
		u32 utf8_len = utf8_decode_len(c);

		char recv_buf[4] = { c, 0, 0, 0 };
		char* it = recv_buf;
		switch (utf8_len) {
		case 4: *++it = lt_getc();
		case 3: *++it = lt_getc();
		case 2: *++it = lt_getc();
		case 1:
			utf8_decode(recv_buf, &c);
			return c | mod;
		}
	}
	return c | mod;
}

void lt_term_write_direct(char* str, usz len) {
	write(STDOUT_FILENO, str, len);
	fsync(STDOUT_FILENO);
}

