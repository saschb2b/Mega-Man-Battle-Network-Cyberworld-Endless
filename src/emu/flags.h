/* The game's event flags (eEventFlags): flag n is bit 0x80 >> (n & 7) of
 * byte n / 8. */
#ifndef CW_FLAGS_H
#define CW_FLAGS_H

#include <stdbool.h>

#include "bn6.h"

/* the flags there are (the NaviCust's compressions among the last: 0x2660 on) */
#define FLAG_COUNT (BN6_EVENT_FLAG_BYTES * 8)

bool flag_get(int flag);
void flag_set(int flag);
void flag_clear(int flag);

#endif
