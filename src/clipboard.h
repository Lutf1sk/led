#ifndef CLIPBOARD_H
#define CLIPBOARD_H 1

#include <lt/strstream.h>

#define CLIPBOARD_COUNT 16

extern lt_strstream_t clipboards[CLIPBOARD_COUNT];

void clipboard_init(void);
void clipboard_clear(usz clipboard);

#endif
