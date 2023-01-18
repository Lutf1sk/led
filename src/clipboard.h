#ifndef CLIPBOARD_H
#define CLIPBOARD_H 1

#include <lt/strstream.h>

extern lt_strstream_t clipboard;

void clipboard_clear(void);
void clipboard_init(void);

#endif
