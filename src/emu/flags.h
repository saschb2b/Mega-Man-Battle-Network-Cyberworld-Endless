/* The game's event flags (eEventFlags): flag n is bit 0x80 >> (n & 7) of
 * byte n / 8. */
#ifndef CW_FLAGS_H
#define CW_FLAGS_H

#include <stdbool.h>

bool flag_get(int flag);
void flag_set(int flag);
void flag_clear(int flag);

#endif
