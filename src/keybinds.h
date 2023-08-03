#ifndef KEYBINDS_H
#define KEYBINDS_H 1

#include <lt/lt.h>
#include <lt/fwd.h>

void keybind_init(void);

void reg_keybind_command(u32 key, lstr_t command);

u32 modstr_to_key(lstr_t mod);
u32 keystr_to_key(lstr_t key);

lstr_t lookup_keybind(u32 key);

void keybinds_load(lt_conf_t* kbs);

#endif
