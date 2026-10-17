/* Small byte operations shared by the durable trade records. */
#ifndef CW_ONLINE_TRADE_BYTES_H
#define CW_ONLINE_TRADE_BYTES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline uint32_t trade_bytes_get32(const uint8_t *p) {
	return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
}

static inline void trade_bytes_put32(uint8_t *p, uint32_t n) {
	for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(n >> (24 - 8 * i));
}

static inline bool trade_bytes_nonzero(const uint8_t *p, size_t n) {
	uint8_t any = 0;
	for (size_t i = 0; i < n; ++i) any |= p[i];
	return any != 0;
}

#endif
