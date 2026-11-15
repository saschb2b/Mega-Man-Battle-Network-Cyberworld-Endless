/* update_sha256.h. FIPS 180-4's SHA-256, its constants the standard's. */
#include "update_sha256.h"

#include <stdio.h>
#include <string.h>

static const uint32_t K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static uint32_t ror(uint32_t x, int n) { return x >> n | x << (32 - n); }

/* one 64-byte block into the state */
static void block(uint32_t h[8], const uint8_t *p) {
	uint32_t w[64], v[8];
	for (int i = 0; i < 16; ++i) w[i] = (uint32_t)p[4 * i] << 24 | (uint32_t)p[4 * i + 1] << 16 | (uint32_t)p[4 * i + 2] << 8 | p[4 * i + 3];
	for (int i = 16; i < 64; ++i) {
		uint32_t s0 = ror(w[i - 15], 7) ^ ror(w[i - 15], 18) ^ w[i - 15] >> 3, s1 = ror(w[i - 2], 17) ^ ror(w[i - 2], 19) ^ w[i - 2] >> 10;
		w[i] = w[i - 16] + s0 + w[i - 7] + s1;
	}
	memcpy(v, h, sizeof v);
	for (int i = 0; i < 64; ++i) {
		uint32_t s1 = ror(v[4], 6) ^ ror(v[4], 11) ^ ror(v[4], 25), ch = (v[4] & v[5]) ^ (~v[4] & v[6]);
		uint32_t t1 = v[7] + s1 + ch + K[i] + w[i];
		uint32_t s0 = ror(v[0], 2) ^ ror(v[0], 13) ^ ror(v[0], 22), maj = (v[0] & v[1]) ^ (v[0] & v[2]) ^ (v[1] & v[2]);
		memmove(v + 1, v, 7 * sizeof *v);
		v[4] += t1;
		v[0] = t1 + s0 + maj;
	}
	for (int i = 0; i < 8; ++i) h[i] += v[i];
}

void sha256_begin(Sha256 *s) {
	static const uint32_t start[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
	memcpy(s->h, start, sizeof s->h);
	s->bytes = 0;
}

void sha256_feed(Sha256 *s, const void *data, size_t n) {
	const uint8_t *p = data;
	while (n) {
		size_t at = (size_t)(s->bytes % 64), take = 64 - at < n ? 64 - at : n;
		memcpy(s->block + at, p, take);
		s->bytes += take;
		p += take;
		n -= take;
		if (at + take == 64) block(s->h, s->block);
	}
}

void sha256_end(Sha256 *s, char hex[65]) {
	uint64_t bits = s->bytes * 8;
	uint8_t pad[72] = { 0x80 }, len[8];
	size_t at = (size_t)(s->bytes % 64), fill = at < 56 ? 56 - at : 120 - at;
	for (int i = 0; i < 8; ++i) len[i] = (uint8_t)(bits >> (56 - 8 * i));
	sha256_feed(s, pad, fill);
	sha256_feed(s, len, 8);
	for (int i = 0; i < 8; ++i) snprintf(hex + 8 * i, 9, "%08x", (unsigned)s->h[i]);
}
