/* Little-endian byte packing for data written into (and read from) the
 * game's memory. */
#ifndef CW_BYTES_H
#define CW_BYTES_H

#include <stdint.h>

static inline void put16(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static inline void put32(uint8_t *p, uint32_t v) { put16(p, v); put16(p + 2, v >> 16); }
static inline uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }

#endif
