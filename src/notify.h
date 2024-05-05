#ifndef NOTIFY_H
#define NOTIFY_H 1

#include <lt/lt.h>

void notify_init(void);

void notify(char* fmt, ...);
b8 pop_notification(lstr_t out_notify[static 1]);

#endif
