#ifndef CW_ONLINE_TEXT_H
#define CW_ONLINE_TEXT_H

#include <stddef.h>

extern const char online_title[], online_map_failed[], online_connecting[];
extern const char online_waiting[];
void online_peer_text(char *out, size_t size, const char *name, unsigned count);

#endif
