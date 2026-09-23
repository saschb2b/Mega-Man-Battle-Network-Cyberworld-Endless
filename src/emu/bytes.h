/* Little-endian byte packing for data written into the game's memory. */
#ifndef CW_BYTES_H
#define CW_BYTES_H

#include <stdint.h>

static inline void put16(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static inline void put32(uint8_t *p, uint32_t v) { put16(p, v); put16(p + 2, v >> 16); }

#endif
