#include "clipboard.h"

#include <lt/mem.h>

lt_strstream_t clipboards[CLIPBOARD_COUNT];

void clipboard_init(void) {
	for (usz i = 0; i < CLIPBOARD_COUNT; ++i)
		LT_ASSERT(lt_strstream_create(&clipboards[i], lt_libc_heap) == LT_SUCCESS);
}

void clipboard_clear(usz clipboard) {
	if (clipboard >= CLIPBOARD_COUNT)
		return;
	lt_strstream_clear(&clipboards[clipboard]);
}
