#include "online_trade_terms.h"

#include <string.h>

#include "sha256.h"

static bool nonzero(const uint8_t *p, size_t n) {
	uint8_t any = 0;
	for (size_t i = 0; i < n; ++i) any |= p[i];
	return any != 0;
}

static bool side_valid(const OnlineTradeSide *side) {
	return nonzero(side->identity, 16) && nonzero(side->run_id, 16) && nonzero(side->checkpoint, 32) &&
		side->revision < UINT32_MAX && (side->chip & 511u) && (side->chip >> 9) <= 26 && !(side->rules & ~3u) &&
		(!(side->rules & ONLINE_TRADE_RULE_ALL_STAR) || (side->chip >> 9) == 26);
}

bool online_trade_terms_valid(const OnlineTradeTerms *terms) {
	return terms && nonzero(terms->server, 32) && nonzero(terms->transaction, 16) &&
		side_valid(&terms->side[0]) && side_valid(&terms->side[1]) &&
		memcmp(terms->side[0].identity, terms->side[1].identity, 16) &&
		memcmp(terms->side[0].run_id, terms->side[1].run_id, 16) &&
		terms->side[0].chip != terms->side[1].chip &&
		!((terms->side[0].rules ^ terms->side[1].rules) & ONLINE_TRADE_RULE_ALL_STAR);
}

bool online_trade_terms_encode(const OnlineTradeTerms *terms, uint8_t out[ONLINE_TRADE_TERMS_BYTES]) {
	if (!out || !online_trade_terms_valid(terms)) return false;
	memcpy(out, terms->server, 32);
	memcpy(out + 32, terms->transaction, 16);
	for (unsigned i = 0; i < 2; ++i) {
		const OnlineTradeSide *side = &terms->side[i];
		uint8_t *p = out + 48 + i * ONLINE_TRADE_SIDE_BYTES;
		memcpy(p, side->identity, 16);
		memcpy(p + 16, side->run_id, 16);
		for (unsigned j = 0; j < 4; ++j) p[32 + j] = (uint8_t)(side->revision >> (24 - 8 * j));
		memcpy(p + 36, side->checkpoint, 32);
		p[68] = (uint8_t)(side->chip >> 8); p[69] = (uint8_t)side->chip; p[70] = side->rules;
	}
	return true;
}

bool online_trade_terms_decode(OnlineTradeTerms *terms, const uint8_t *bytes, size_t n) {
	if (!terms) return false;
	memset(terms, 0, sizeof *terms);
	if (!bytes || n != ONLINE_TRADE_TERMS_BYTES) return false;
	memcpy(terms->server, bytes, 32);
	memcpy(terms->transaction, bytes + 32, 16);
	for (unsigned i = 0; i < 2; ++i) {
		OnlineTradeSide *side = &terms->side[i];
		const uint8_t *p = bytes + 48 + i * ONLINE_TRADE_SIDE_BYTES;
		memcpy(side->identity, p, 16);
		memcpy(side->run_id, p + 16, 16);
		for (unsigned j = 0; j < 4; ++j) side->revision = side->revision << 8 | p[32 + j];
		memcpy(side->checkpoint, p + 36, 32);
		side->chip = (uint16_t)((unsigned)p[68] << 8 | p[69]); side->rules = p[70];
	}
	return online_trade_terms_valid(terms);
}

bool online_trade_terms_digest(const OnlineTradeTerms *terms, uint8_t out[32]) {
	uint8_t bytes[ONLINE_TRADE_TERMS_BYTES];
	if (!out || !online_trade_terms_encode(terms, bytes)) return false;
	static const char domain[] = "Cyberworld trade 1";
	CwSha256 hash;
	cw_sha256_init(&hash);
	cw_sha256_update(&hash, domain, sizeof domain);
	cw_sha256_update(&hash, bytes, sizeof bytes);
	cw_sha256_final(&hash, out);
	return true;
}
