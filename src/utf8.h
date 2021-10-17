#ifndef UTF8_H
#define UTF8_H 1

#include "common.h"

usz utf8_encode(char* out, u32 v);
usz utf8_decode(char* str, u32* out);

usz utf8_decode_len(u8 v);
usz utf8_encode_len(u32 v);

#endif
