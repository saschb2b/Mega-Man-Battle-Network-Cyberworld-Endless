#include "online_trade_wire.h"

#include <string.h>

#include "online_wire.h"

static bool present(const uint8_t *bytes, size_t size) {
	if (!bytes) return false;
	for (size_t i = 0; i < size; ++i) if (bytes[i]) return true;
	return false;
}

static bool chip_entry(uint16_t entry, bool optional) {
	return (!entry && optional) || ((entry & 511u) && (entry >> 9) <= 26);
}

static uint16_t read16(const uint8_t *p) { return (uint16_t)((unsigned)p[0] << 8 | p[1]); }

static bool read_caps(OnlineTradeMessage *out, const uint8_t *bytes, size_t size) {
	if (size != 36 || read16(bytes + 1) != ONLINE_TRADE_WIRE_VERSION ||
	    !present(bytes + 3, 32) || bytes[35] > 1) return false;
	memcpy(out->value.caps.server, bytes + 3, 32);
	out->value.caps.enabled = bytes[35] != 0;
	return true;
}

static bool read_listing(OnlineTradeListing *item, const uint8_t *p) {
	memcpy(item->identity, p, 16);
	memcpy(item->name, p + 16, 16);
	item->name[16] = 0;
	item->have = read16(p + 32); item->want = read16(p + 34); item->rules = p[36];
	if (!present(item->identity, 16) || !online_wire_name(item->name) ||
	    !chip_entry(item->have, false) || !chip_entry(item->want, true) ||
	    item->have == item->want || item->rules > 3) return false;
	/* A NUL ends the name; the fixed-width remainder must be padding. */
	for (size_t i = strlen(item->name); i < 16; ++i) if (p[16 + i]) return false;
	return !(item->rules & ONLINE_TRADE_RULE_ALL_STAR) ||
		((item->have >> 9) == 26 && (!item->want || (item->want >> 9) == 26));
}

static bool read_board(OnlineTradeMessage *out, const uint8_t *bytes, size_t size) {
	if (size < 2 || bytes[1] > ONLINE_TRADE_LISTINGS_MAX ||
	    size != 2 + (size_t)bytes[1] * ONLINE_TRADE_LISTING_BYTES) return false;
	out->value.board.count = bytes[1];
	for (size_t i = 0; i < out->value.board.count; ++i) {
		OnlineTradeListing *item = &out->value.board.items[i];
		if (!read_listing(item, bytes + 2 + i * ONLINE_TRADE_LISTING_BYTES)) return false;
		for (size_t j = 0; j < i; ++j)
			if (!memcmp(item->identity, out->value.board.items[j].identity, 16)) return false;
	}
	return true;
}

static bool read_decision(OnlineTradeMessage *out, const uint8_t *bytes, size_t size) {
	OnlineTradeDecision *d = &out->value.decision;
	if (size != ONLINE_TRADE_DECISION_BYTES ||
	    !online_trade_terms_decode(&d->terms, bytes + 1, ONLINE_TRADE_TERMS_BYTES)) return false;
	d->state = bytes[191]; d->prepared = bytes[192]; d->applied = bytes[193];
	if (d->state > ONLINE_TRADE_ABORTED || d->prepared > 3 || d->applied > 3) return false;
	if (d->state == ONLINE_TRADE_COMMITTED) return d->prepared == 3;
	return d->prepared < 3 && d->applied == 0;
}

bool online_trade_wire_read(OnlineTradeMessage *out, const uint8_t *bytes, size_t size) {
	if (!out || !bytes || !size) return false;
	OnlineTradeMessage parsed = { .kind = bytes[0] };
	bool ok = false;
	switch (parsed.kind) {
	case ONLINE_TRADE_CAPS: ok = read_caps(&parsed, bytes, size); break;
	case ONLINE_TRADE_BOARD: ok = read_board(&parsed, bytes, size); break;
	case ONLINE_TRADE_DECISION: ok = read_decision(&parsed, bytes, size); break;
	default: break;
	}
	if (ok) *out = parsed;
	return ok;
}

bool online_trade_wire_ready(uint8_t out[54], const OnlineTradeReady *r) {
	if (!out || !r || !present(r->run_id, 16) || !present(r->checkpoint, 32) ||
	    r->revision == UINT32_MAX || r->rules > 3) return false;
	out[0] = ONLINE_TRADE_READY;
	memcpy(out + 1, r->run_id, 16); online_put32(out + 17, r->revision);
	memcpy(out + 21, r->checkpoint, 32); out[53] = r->rules;
	return true;
}

bool online_trade_wire_offer(uint8_t out[5], uint16_t have, uint16_t want) {
	if (!out || !chip_entry(have, true) || !chip_entry(want, true) ||
	    (!have && want) || (have && have == want)) return false;
	out[0] = ONLINE_TRADE_OFFER;
	online_put16(out + 1, have); online_put16(out + 3, want);
	return true;
}

bool online_trade_wire_request(uint8_t out[21], const uint8_t target[16], uint16_t give, uint16_t receive) {
	if (!out || !present(target, 16) || !chip_entry(give, false) ||
	    !chip_entry(receive, false) || give == receive) return false;
	out[0] = ONLINE_TRADE_REQUEST;
	memcpy(out + 1, target, 16);
	online_put16(out + 17, give); online_put16(out + 19, receive);
	return true;
}

bool online_trade_wire_prepare(uint8_t out[49], const uint8_t transaction[16], const uint8_t digest[32]) {
	if (!out || !present(transaction, 16) || !present(digest, 32)) return false;
	out[0] = ONLINE_TRADE_PREPARE;
	memcpy(out + 1, transaction, 16); memcpy(out + 17, digest, 32);
	return true;
}

bool online_trade_wire_transaction(uint8_t out[17], uint8_t kind, const uint8_t transaction[16]) {
	if (!out || !present(transaction, 16) ||
	    (kind != ONLINE_TRADE_CANCEL && kind != ONLINE_TRADE_STATUS && kind != ONLINE_TRADE_ACK)) return false;
	out[0] = kind; memcpy(out + 1, transaction, 16);
	return true;
}
