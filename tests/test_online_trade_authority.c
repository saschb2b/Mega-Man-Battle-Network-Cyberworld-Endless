#include "test_online_trade_authority.h"

#include <stdio.h>
#include <string.h>

#include "online_trade_authority.h"

static int failures;
#define CHECK(value) do { if (!(value)) { fprintf(stderr, "trade authority line %d: %s\n", __LINE__, #value); ++failures; } } while (0)

static OnlineTradeDecision fixture(void) {
	OnlineTradeDecision value = { .terms = { .server = {11}, .transaction = {12}, .side = {
		{ .identity = {21}, .run_id = {22}, .checkpoint = {23}, .chip = 1, .revision = 4 },
		{ .identity = {31}, .run_id = {32}, .checkpoint = {33}, .chip = 2, .revision = 5 } } } };
	return value;
}

static OnlineTradeAuthority connection(const OnlineTradeDecision *decision, unsigned side, bool enabled) {
	OnlineTradeAuthority authority;
	OnlineTradeMessage caps = { .kind = ONLINE_TRADE_CAPS };
	memcpy(caps.value.caps.server, decision->terms.server, 32);
	caps.value.caps.enabled = enabled;
	CHECK(online_trade_authority_begin(&authority, decision->terms.side[side].identity));
	CHECK(online_trade_authority_caps(&authority, &caps, &decision->terms));
	return authority;
}

static void capabilities(void) {
	OnlineTradeDecision decision = fixture();
	OnlineTradeAuthority authority, before;
	CHECK(online_trade_authority_begin(&authority, decision.terms.side[0].identity));
	OnlineTradeMessage caps = { .kind = ONLINE_TRADE_CAPS };
	memcpy(caps.value.caps.server, decision.terms.server, 32);
	CHECK(online_trade_authority_caps(&authority, &caps, &decision.terms));
	CHECK(authority.known && !authority.enabled);
	CHECK(online_trade_authority_caps(&authority, &caps, &decision.terms));
	before = authority;
	caps.value.caps.enabled = true;
	CHECK(!online_trade_authority_caps(&authority, &caps, &decision.terms));
	CHECK(!memcmp(&before, &authority, sizeof authority));
	caps.value.caps.enabled = false; caps.value.caps.server[1] = 1;
	CHECK(!online_trade_authority_caps(&authority, &caps, NULL));
	CHECK(!memcmp(&before, &authority, sizeof authority));
	CHECK(online_trade_authority_begin(&authority, decision.terms.side[0].identity));
	CHECK(!online_trade_authority_caps(&authority, &caps, &decision.terms));
	CHECK(!authority.known);
	CHECK(online_trade_authority_begin(&authority, (uint8_t[16]){99}));
	memcpy(caps.value.caps.server, decision.terms.server, 32);
	CHECK(!online_trade_authority_caps(&authority, &caps, &decision.terms));
	CHECK(!online_trade_authority_begin(&authority, (uint8_t[16]){0}));
	CHECK(!authority.known && !authority.enabled && !authority.identity[0]);
}

static void new_offers(void) {
	for (unsigned own = 0; own < 2; ++own) {
		OnlineTradeDecision decision = fixture();
		OnlineTradeAuthority authority = connection(&decision, own, true);
		const OnlineTradeSide *local = &decision.terms.side[own];
		OnlineTradeReady ready = { .revision = local->revision, .rules = local->rules };
		memcpy(ready.run_id, local->run_id, 16); memcpy(ready.checkpoint, local->checkpoint, 32);
		unsigned side = 99;
		CHECK(online_trade_authority_offer(&authority, &decision, &ready, local->chip, 0, &side) && side == own);
		decision.prepared = (uint8_t)(1u << (1 - own));
		CHECK(online_trade_authority_offer(&authority, &decision, &ready, local->chip, decision.terms.side[1 - own].chip, &side));
		decision.prepared = (uint8_t)(1u << own); side = 99;
		CHECK(!online_trade_authority_offer(&authority, &decision, &ready, local->chip, 0, &side) && side == 99);
		decision.prepared = 0;
		CHECK(!online_trade_authority_offer(&authority, &decision, &ready, local->chip, 7, &side));
		CHECK(!online_trade_authority_offer(&authority, &decision, &ready, 7, 0, &side));
		ready.revision++;
		CHECK(!online_trade_authority_offer(&authority, &decision, &ready, local->chip, 0, &side));
		ready.revision--; ready.run_id[15] = 1;
		CHECK(!online_trade_authority_offer(&authority, &decision, &ready, local->chip, 0, &side));
		ready.run_id[15] = 0; ready.checkpoint[31] = 1;
		CHECK(!online_trade_authority_offer(&authority, &decision, &ready, local->chip, 0, &side));
		ready.checkpoint[31] = 0; ready.rules = ONLINE_TRADE_RULE_BN5;
		CHECK(!online_trade_authority_offer(&authority, &decision, &ready, local->chip, 0, &side));
		ready.rules = 0; authority.enabled = false;
		CHECK(!online_trade_authority_offer(&authority, &decision, &ready, local->chip, 0, &side));
		authority.enabled = true; decision.state = ONLINE_TRADE_COMMITTED; decision.prepared = 3;
		CHECK(!online_trade_authority_offer(&authority, &decision, &ready, local->chip, 0, &side));
		CHECK(side == 99);
	}
}

static void journal_decisions(void) {
	for (unsigned own = 0; own < 2; ++own) {
		OnlineTradeDecision decision = fixture();
		OnlineTradeAuthority authority = connection(&decision, own, false);
		OnlineTradeJournal journal = { .terms = decision.terms, .side = own, .state = TRADE_JOURNAL_PREPARED,
			.staged = {41}, .post_content = {42} };
		memcpy(journal.source, decision.terms.side[own].checkpoint, 32);
		CHECK(online_trade_authority_recovery(&authority, &decision, &journal));
		journal.source[31] = 1;
		CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
		journal.source[31] = 0; journal.staged[0] = 0;
		CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
		journal.staged[0] = 41; journal.post_content[0] = 0;
		CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
		journal.post_content[0] = 42;
		decision.state = ONLINE_TRADE_ABORTED;
		CHECK(online_trade_authority_recovery(&authority, &decision, &journal));
		decision.state = ONLINE_TRADE_COMMITTED; decision.prepared = 3;
		CHECK(online_trade_authority_recovery(&authority, &decision, &journal));
		decision.applied = (uint8_t)(1u << own);
		CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
		journal.state = TRADE_JOURNAL_COMMITTED;
		CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
		journal.state = TRADE_JOURNAL_APPLIED;
		CHECK(online_trade_authority_recovery(&authority, &decision, &journal));
		decision.applied = 0;
		CHECK(online_trade_authority_recovery(&authority, &decision, &journal));
		decision.state = ONLINE_TRADE_ABORTED; decision.prepared = 0;
		CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
		journal.state = TRADE_JOURNAL_COMMITTED;
		CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
		journal.state = TRADE_JOURNAL_ABORTED;
		CHECK(online_trade_authority_recovery(&authority, &decision, &journal));
		decision.state = ONLINE_TRADE_COMMITTED; decision.prepared = 3;
		CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
		for (unsigned state = TRADE_JOURNAL_BROKEN; state <= TRADE_JOURNAL_UNSUPPORTED; ++state) {
			journal.state = (OnlineTradeJournalState)state;
			CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
		}
		journal.state = TRADE_JOURNAL_CLEAR;
		CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
		journal.state = TRADE_JOURNAL_PREPARED; journal.side = 1 - own;
		CHECK(!online_trade_authority_recovery(&authority, &decision, &journal));
	}
}

static void altered_terms(void) {
	OnlineTradeDecision original = fixture();
	OnlineTradeAuthority authority = connection(&original, 0, true);
	OnlineTradeJournal journal = { .terms = original.terms, .side = 0, .state = TRADE_JOURNAL_PREPARED,
		.source = {23}, .staged = {41}, .post_content = {42} };
	uint8_t bytes[ONLINE_TRADE_TERMS_BYTES];
	CHECK(online_trade_terms_encode(&original.terms, bytes));
	for (size_t at = 0; at < sizeof bytes; ++at) {
		OnlineTradeDecision changed = original;
		bytes[at] ^= 1;
		(void)online_trade_terms_decode(&changed.terms, bytes, sizeof bytes);
		CHECK(!online_trade_authority_recovery(&authority, &changed, &journal));
		bytes[at] ^= 1;
	}
	authority.known = false;
	CHECK(!online_trade_authority_recovery(&authority, &original, &journal));
}

int test_online_trade_authority(void) {
	capabilities(); new_offers(); journal_decisions(); altered_terms();
	if (!failures) puts("Trade authority binds connection, participant, advertised checkpoint and durable journal");
	return failures;
}

#ifdef CW_TRADE_AUTHORITY_TEST_MAIN
int main(void) { return test_online_trade_authority() ? 1 : 0; }
#endif
