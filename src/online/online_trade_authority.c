#include "online_trade_authority.h"

#include <string.h>

static bool nonzero(const uint8_t *bytes, size_t size) {
	uint8_t any = 0;
	for (size_t i = 0; i < size; ++i) any |= bytes[i];
	return any != 0;
}

static bool participant(const OnlineTradeTerms *terms, const uint8_t identity[16], unsigned *side) {
	if (!online_trade_terms_valid(terms)) return false;
	for (unsigned i = 0; i < 2; ++i) {
		if (!memcmp(terms->side[i].identity, identity, 16)) { *side = i; return true; }
	}
	return false;
}

static bool decision_valid(const OnlineTradeDecision *decision) {
	if (!decision || !online_trade_terms_valid(&decision->terms) || decision->state > ONLINE_TRADE_ABORTED ||
	    decision->prepared > 3 || decision->applied > 3) return false;
	if (decision->state == ONLINE_TRADE_COMMITTED) return decision->prepared == 3;
	return decision->prepared < 3 && !decision->applied;
}

bool online_trade_authority_begin(OnlineTradeAuthority *out, const uint8_t identity[16]) {
	if (!out) return false;
	memset(out, 0, sizeof *out);
	if (!identity || !nonzero(identity, 16)) return false;
	memcpy(out->identity, identity, 16);
	return true;
}

bool online_trade_authority_caps(OnlineTradeAuthority *authority, const OnlineTradeMessage *message,
	const OnlineTradeTerms *pending) {
	if (!authority || !nonzero(authority->identity, 16) || !message || message->kind != ONLINE_TRADE_CAPS ||
	    !nonzero(message->value.caps.server, 32)) return false;
	const uint8_t *server = message->value.caps.server;
	if (authority->known && (memcmp(authority->server, server, 32) || authority->enabled != message->value.caps.enabled)) return false;
	unsigned side;
	if (pending && (!participant(pending, authority->identity, &side) || memcmp(pending->server, server, 32))) return false;
	memcpy(authority->server, server, 32);
	authority->known = true;
	authority->enabled = message->value.caps.enabled;
	return true;
}

static bool bound(const OnlineTradeAuthority *authority, const OnlineTradeDecision *decision) {
	return authority && authority->known && nonzero(authority->identity, 16) && decision_valid(decision) &&
		!memcmp(authority->server, decision->terms.server, 32);
}

bool online_trade_authority_offer(const OnlineTradeAuthority *authority, const OnlineTradeDecision *decision,
	const OnlineTradeReady *ready, uint16_t have, uint16_t want, unsigned *side) {
	if (!bound(authority, decision) || !authority->enabled || !ready || !side || decision->state != ONLINE_TRADE_OFFERED) return false;
	unsigned own;
	if (!participant(&decision->terms, authority->identity, &own) || (decision->prepared & (1u << own))) return false;
	const OnlineTradeSide *local = &decision->terms.side[own];
	if (memcmp(local->run_id, ready->run_id, 16) || memcmp(local->checkpoint, ready->checkpoint, 32) ||
	    local->revision != ready->revision || local->rules != ready->rules || local->chip != have ||
	    (want && decision->terms.side[1 - own].chip != want)) return false;
	*side = own;
	return true;
}

static bool equal_terms(const OnlineTradeTerms *a, const OnlineTradeTerms *b) {
	uint8_t left[ONLINE_TRADE_TERMS_BYTES], right[ONLINE_TRADE_TERMS_BYTES];
	return online_trade_terms_encode(a, left) && online_trade_terms_encode(b, right) && !memcmp(left, right, sizeof left);
}

bool online_trade_authority_recovery(const OnlineTradeAuthority *authority, const OnlineTradeDecision *decision,
	const OnlineTradeJournal *journal) {
	if (!bound(authority, decision) || !journal || journal->side > 1 ||
	    memcmp(journal->terms.side[journal->side].identity, authority->identity, 16) ||
	    memcmp(journal->source, journal->terms.side[journal->side].checkpoint, 32) ||
	    !nonzero(journal->staged, 32) || !nonzero(journal->post_content, 32) ||
	    !equal_terms(&decision->terms, &journal->terms)) return false;
	bool acknowledged = (decision->applied & (1u << journal->side)) != 0;
	switch (journal->state) {
	case TRADE_JOURNAL_PREPARED: return !acknowledged;
	case TRADE_JOURNAL_COMMITTED: return decision->state == ONLINE_TRADE_COMMITTED && !acknowledged;
	case TRADE_JOURNAL_APPLIED: return decision->state == ONLINE_TRADE_COMMITTED;
	case TRADE_JOURNAL_ABORTED: return decision->state == ONLINE_TRADE_ABORTED;
	default: return false;
	}
}
