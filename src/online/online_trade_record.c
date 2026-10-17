#include "online_trade_record.h"

#include <stdlib.h>
#include <string.h>

#include "sha256.h"

#define HEADER_BYTES 28
#define RECEIPT_BYTES (ONLINE_TRADE_TERMS_BYTES + 1 + 32)
static const uint8_t magic[8] = { 'C', 'W', 'T', 'R', 'D', '1', '\r', '\n' };

static bool nonzero(const uint8_t *p, size_t n) {
	uint8_t any = 0;
	for (size_t i = 0; i < n; ++i) any |= p[i];
	return any != 0;
}

static void put_revision(uint8_t *p, uint32_t revision) {
	for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(revision >> (24 - i * 8));
}

bool online_trade_record_new(const uint8_t run_id[16], uint8_t out[ONLINE_TRADE_RECORD_INITIAL]) {
	if (!run_id || !out || !nonzero(run_id, 16)) return false;
	memset(out, 0, ONLINE_TRADE_RECORD_INITIAL);
	memcpy(out, magic, sizeof magic);
	memcpy(out + 8, run_id, 16);
	cw_sha256(out, HEADER_BYTES, out + HEADER_BYTES);
	return true;
}

bool online_trade_record_read(const uint8_t *data, size_t n, uint8_t run_id[16], uint32_t *revision) {
	if (!data || n < ONLINE_TRADE_RECORD_INITIAL || n > ONLINE_TRADE_RECORD_LIMIT ||
		(n - ONLINE_TRADE_RECORD_INITIAL) % RECEIPT_BYTES || memcmp(data, magic, sizeof magic) || !nonzero(data + 8, 16)) return false;
	uint32_t count = (uint32_t)data[24] << 24 | (uint32_t)data[25] << 16 | (uint32_t)data[26] << 8 | data[27];
	if (count != (n - ONLINE_TRADE_RECORD_INITIAL) / RECEIPT_BYTES) return false;
	uint8_t hash[32];
	cw_sha256(data, n - 32, hash);
	if (memcmp(hash, data + n - 32, 32)) return false;
	for (uint32_t i = 0; i < count; ++i) {
		const uint8_t *p = data + HEADER_BYTES + i * RECEIPT_BYTES;
		OnlineTradeTerms terms;
		unsigned side = p[ONLINE_TRADE_TERMS_BYTES];
		if (side > 1 || !online_trade_terms_decode(&terms, p, ONLINE_TRADE_TERMS_BYTES) ||
			terms.side[side].revision != i || memcmp(terms.side[side].run_id, data + 8, 16) ||
			!nonzero(p + ONLINE_TRADE_TERMS_BYTES + 1, 32)) return false;
	}
	if (run_id) memcpy(run_id, data + 8, 16);
	if (revision) *revision = count;
	return true;
}

uint8_t *online_trade_record_append(const uint8_t *prior, size_t n, const OnlineTradeTerms *terms,
	unsigned side, const uint8_t post_content[32], size_t *written) {
	uint8_t id[16];
	uint32_t revision;
	if (written) *written = 0;
	if (!written || side > 1 || !post_content || !nonzero(post_content, 32) || !online_trade_terms_valid(terms) ||
		!online_trade_record_read(prior, n, id, &revision) || terms->side[side].revision != revision ||
		memcmp(id, terms->side[side].run_id, 16) || n > ONLINE_TRADE_RECORD_LIMIT - RECEIPT_BYTES) return NULL;
	uint8_t *out = malloc(n + RECEIPT_BYTES);
	if (!out) return NULL;
	memcpy(out, prior, n - 32);
	put_revision(out + 24, revision + 1);
	uint8_t *p = out + n - 32;
	online_trade_terms_encode(terms, p);
	p[ONLINE_TRADE_TERMS_BYTES] = (uint8_t)side;
	memcpy(p + ONLINE_TRADE_TERMS_BYTES + 1, post_content, 32);
	*written = n + RECEIPT_BYTES;
	cw_sha256(out, *written - 32, out + *written - 32);
	return out;
}

bool online_trade_record_has(const uint8_t *data, size_t n, const OnlineTradeTerms *terms) {
	uint32_t revision;
	uint8_t encoded[ONLINE_TRADE_TERMS_BYTES];
	if (!online_trade_record_read(data, n, NULL, &revision) || !online_trade_terms_encode(terms, encoded)) return false;
	for (uint32_t i = 0; i < revision; ++i)
		if (!memcmp(data + HEADER_BYTES + i * RECEIPT_BYTES, encoded, sizeof encoded)) return true;
	return false;
}
