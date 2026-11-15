/* SHA-256 (FIPS 180-4), fed as a download comes in: the updater checks a
 * new version's file against the release's own digest (update.json,
 * docs/engine/updates.md) before it puts the file in place. */
#ifndef CW_UPDATE_SHA256_H
#define CW_UPDATE_SHA256_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t h[8];
	uint64_t bytes;      /* fed so far */
	uint8_t block[64];   /* ... the last block's, not yet hashed */
} Sha256;

void sha256_begin(Sha256 *s);
void sha256_feed(Sha256 *s, const void *data, size_t n);
/* The digest in lower-case hex */
void sha256_end(Sha256 *s, char hex[65]);

#endif
