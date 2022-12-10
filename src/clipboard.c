#include "clipboard.h"

#include <lt/mem.h>

lt_strstream_t clipboard;

void clipboard_init(void) {
	LT_ASSERT(!lt_strstream_create(&clipboard, lt_libc_heap));
}

void clipboard_clear(void) {
	lt_strstream_clear(&clipboard);
}

