#include "notify.h"

#include <lt/darr.h>
#include <lt/io.h>
#include <lt/mem.h>

#include <stdarg.h>

lt_darr(lstr_t) notifications;

void notify_init(void) {
	notifications = lt_darr_create(lstr_t, 16, lt_libc_heap);
	LT_ASSERT(notifications);
}

void notify(char* fmt, ...) {
	lstr_t str;
	va_list al;
	va_start(al, fmt);
	LT_ASSERT(lt_vaprintf(&str, lt_libc_heap, fmt, al) >= 0);
	va_end(al);

	lt_darr_push(notifications, str);
}

b8 pop_notification(lstr_t out_notify[static 1]) {
	usz count = lt_darr_count(notifications);
	if (count <= 0) {
		return 0;
	}
	*out_notify = notifications[count - 1];
	lt_darr_pop(notifications);
	return 1;
}
