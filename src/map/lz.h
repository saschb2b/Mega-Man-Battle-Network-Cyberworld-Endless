/* LZ77 data for the game's own decompressor. */
#ifndef CW_LZ_H
#define CW_LZ_H

#include <stddef.h>
#include <stdint.h>

/* GBA LZ77 (type 0x10) of `n` bytes made of literal blocks only, padded to
 * 4 bytes; `out` needs 4 + n + n / 8 + 8 bytes. Returns the size written. */
size_t lz_literal(const uint8_t *src, size_t n, uint8_t *out);

#endif
