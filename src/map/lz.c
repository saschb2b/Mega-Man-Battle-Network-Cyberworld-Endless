/* LZ77 as the GBA BIOS decompresses it (type 0x10). */
#include "lz.h"

size_t lz_literal(const uint8_t *src, size_t n, uint8_t *out) {
	/* GBA LZ77 (type 0x10) made of literal blocks only */
	size_t o = 0;
	out[o++] = 0x10; out[o++] = (uint8_t)n; out[o++] = (uint8_t)(n >> 8); out[o++] = (uint8_t)(n >> 16);
	for (size_t i = 0; i < n; i += 8) {
		out[o++] = 0;
		for (size_t k = 0; k < 8; ++k) out[o++] = i + k < n ? src[i + k] : 0;
	}
	while (o & 3) out[o++] = 0;
	return o;
}
