/* Streaming SHA-256, FIPS 180-4. Digests detect changed bytes; they do not
 * authenticate a peer or replace the transport's server verification. */
#ifndef CW_SHA256_H
#define CW_SHA256_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t h[8];
	uint64_t bytes;
	uint8_t block[64];
	size_t used;
} CwSha256;

void cw_sha256_init(CwSha256 *s);
void cw_sha256_update(CwSha256 *s, const void *data, size_t n);
void cw_sha256_final(CwSha256 *s, uint8_t out[32]);
void cw_sha256(const void *data, size_t n, uint8_t out[32]);

#endif
