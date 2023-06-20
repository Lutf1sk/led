#include <lt/mem.h>
#include <lt/darr.h>
#include <lt/str.h>
#include <lt/term.h>
#include <lt/ctype.h>

#include "keybinds.h"

typedef
struct keybind {
	u32 key;
	lstr_t command;
} keybind_t;

static lt_darr(keybind_t) keybinds;

void keybind_init(void) {
	keybinds = lt_darr_create(keybind_t, 128, lt_libc_heap);
}

u32 modstr_to_key(lstr_t mod) {
	if (mod.len <= 0) {
		lt_werrf("modifier string is empty\n");
		return 0;
	}

	if (lt_lstr_eq(mod, CLSTR("alt")))
		return LT_TERM_MOD_ALT;
	if (lt_lstr_eq(mod, CLSTR("ctrl")))
		return LT_TERM_MOD_CTRL;
	if (lt_lstr_eq(mod, CLSTR("shift")))
		return LT_TERM_MOD_SHIFT;

	lt_werrf("unknown modifier string '%S'\n", mod);
	return 0;
}

u32 keystr_to_key(lstr_t key) {
	if (key.len <= 0) {
		lt_werrf("key string is empty\n");
		return 0;
	}

	if (key.len == 1)
		return key.str[0];

	if (lt_lstr_eq(key, CLSTR("backspace"))) return LT_TERM_KEY_BSPACE;
	if (lt_lstr_eq(key, CLSTR("escape"))) return LT_TERM_KEY_ESC;
	if (lt_lstr_eq(key, CLSTR("tab"))) return LT_TERM_KEY_TAB;

	if (lt_lstr_eq(key, CLSTR("up"))) return LT_TERM_KEY_UP;
	if (lt_lstr_eq(key, CLSTR("down"))) return LT_TERM_KEY_DOWN;
	if (lt_lstr_eq(key, CLSTR("right"))) return LT_TERM_KEY_RIGHT;
	if (lt_lstr_eq(key, CLSTR("left"))) return LT_TERM_KEY_LEFT;

	if (lt_lstr_eq(key, CLSTR("pageup"))) return LT_TERM_KEY_PAGEUP;
	if (lt_lstr_eq(key, CLSTR("pagedown"))) return LT_TERM_KEY_PAGEDN;
	if (lt_lstr_eq(key, CLSTR("home"))) return LT_TERM_KEY_HOME;
	if (lt_lstr_eq(key, CLSTR("end"))) return LT_TERM_KEY_END;
	if (lt_lstr_eq(key, CLSTR("insert"))) return LT_TERM_KEY_INSERT;
	if (lt_lstr_eq(key, CLSTR("delete"))) return LT_TERM_KEY_DELETE;

	if (lt_lstr_eq(key, CLSTR("f0"))) return LT_TERM_KEY_F0;
	if (lt_lstr_eq(key, CLSTR("f1"))) return LT_TERM_KEY_F1;
	if (lt_lstr_eq(key, CLSTR("f2"))) return LT_TERM_KEY_F2;
	if (lt_lstr_eq(key, CLSTR("f3"))) return LT_TERM_KEY_F3;
	if (lt_lstr_eq(key, CLSTR("f4"))) return LT_TERM_KEY_F4;
	if (lt_lstr_eq(key, CLSTR("f5"))) return LT_TERM_KEY_F5;
	if (lt_lstr_eq(key, CLSTR("f6"))) return LT_TERM_KEY_F6;
	if (lt_lstr_eq(key, CLSTR("f7"))) return LT_TERM_KEY_F7;
	if (lt_lstr_eq(key, CLSTR("f8"))) return LT_TERM_KEY_F8;
	if (lt_lstr_eq(key, CLSTR("f9"))) return LT_TERM_KEY_F9;
	if (lt_lstr_eq(key, CLSTR("f10"))) return LT_TERM_KEY_F10;
	if (lt_lstr_eq(key, CLSTR("f11"))) return LT_TERM_KEY_F11;
	if (lt_lstr_eq(key, CLSTR("f12"))) return LT_TERM_KEY_F12;
	if (lt_lstr_eq(key, CLSTR("f13"))) return LT_TERM_KEY_F13;
	if (lt_lstr_eq(key, CLSTR("f14"))) return LT_TERM_KEY_F14;
	if (lt_lstr_eq(key, CLSTR("f15"))) return LT_TERM_KEY_F15;
	if (lt_lstr_eq(key, CLSTR("f16"))) return LT_TERM_KEY_F16;
	if (lt_lstr_eq(key, CLSTR("f17"))) return LT_TERM_KEY_F17;
	if (lt_lstr_eq(key, CLSTR("f18"))) return LT_TERM_KEY_F18;
	if (lt_lstr_eq(key, CLSTR("f19"))) return LT_TERM_KEY_F19;
	if (lt_lstr_eq(key, CLSTR("f20"))) return LT_TERM_KEY_F20;

	lt_werrf("unknown key string '%S'\n", key);
	return 0;
}

void reg_keybind_command(u32 key, lstr_t cmd) {
	u32 mod = key & LT_TERM_MOD_MASK;
	u32 k = key & LT_TERM_KEY_MASK;
	if (!(key & LT_TERM_KEY_SPECIAL_BIT) && mod == LT_TERM_MOD_CTRL && k < 0x80)
		key = lt_to_upper(k) | mod;

	keybind_t kb;
	kb.key = key;
	kb.command = cmd;

	lt_darr_push(keybinds, kb);
}

lstr_t lookup_keybind(u32 key) {
	for (usz i = 0; i < lt_darr_count(keybinds); ++i) {
		if (keybinds[i].key == key)
			return keybinds[i].command;
	}
	return NLSTR();
}

