#include "test_online_trade_wire.h"

#include <stdio.h>
#include <string.h>

#include "online_trade_wire.h"
#include "online_trade_inbox.h"
#include "online_negotiation.h"
#include "negotiation.h"

static int failures;
#define CHECK(value) do { if (!(value)) { fprintf(stderr, "trade wire line %d: %s\n", __LINE__, #value); ++failures; } } while (0)

static void refused(const uint8_t *bytes, size_t size) {
	OnlineTradeMessage message, before;
	memset(&message, 0xa5, sizeof message); before = message;
	CHECK(!online_trade_wire_read(&message, bytes, size));
	CHECK(!memcmp(&message, &before, sizeof message));
}

static void capabilities(void) {
	uint8_t bytes[37] = { ONLINE_TRADE_CAPS, 0, 1, 7 };
	OnlineTradeMessage message;
	CHECK(online_trade_wire_read(&message, bytes, 36));
	CHECK(message.kind == ONLINE_TRADE_CAPS && !message.value.caps.enabled && message.value.caps.server[0] == 7);
	bytes[35] = 1; CHECK(online_trade_wire_read(&message, bytes, 36) && message.value.caps.enabled);
	bytes[35] = 2; refused(bytes, 36); bytes[35] = 0;
	bytes[2] = 2; refused(bytes, 36); bytes[2] = 1;
	bytes[3] = 0; refused(bytes, 36); bytes[3] = 7;
	for (size_t size = 0; size < 36; ++size) refused(bytes, size);
	refused(bytes, 37);
}

static void listings(void) {
	uint8_t bytes[2 + ONLINE_TRADE_LISTINGS_MAX * ONLINE_TRADE_LISTING_BYTES] = { ONLINE_TRADE_BOARD, 8 };
	for (unsigned i = 0; i < 8; ++i) {
		uint8_t *p = bytes + 2 + i * ONLINE_TRADE_LISTING_BYTES;
		p[0] = (uint8_t)(i + 1); memcpy(p + 16, "Lan", 3);
		p[32] = 6; p[33] = 1; /* Cannon D */
	}
	OnlineTradeMessage message;
	CHECK(online_trade_wire_read(&message, bytes, sizeof bytes));
	CHECK(message.value.board.count == 8 && message.value.board.items[7].have == 1537);
	CHECK(!strcmp(message.value.board.items[0].name, "Lan"));
	for (size_t size = 0; size < sizeof bytes; ++size) refused(bytes, size);
	bytes[1] = 9; refused(bytes, sizeof bytes); bytes[1] = 8;
	bytes[39] = 1; refused(bytes, sizeof bytes); bytes[39] = 2;
	bytes[2] = 0; refused(bytes, sizeof bytes); bytes[2] = 1;
	bytes[22] = 'x'; refused(bytes, sizeof bytes); bytes[22] = 0;
	bytes[18] = ' '; refused(bytes, sizeof bytes); bytes[18] = 'L';
	bytes[34] = 54; refused(bytes, sizeof bytes); bytes[34] = 6;
	bytes[38] = ONLINE_TRADE_RULE_ALL_STAR; refused(bytes, sizeof bytes);
	bytes[34] = 52; CHECK(online_trade_wire_read(&message, bytes, sizeof bytes));
	bytes[37] = 2; refused(bytes, sizeof bytes); /* All * cannot ask for A. */
	bytes[37] = 0; bytes[36] = 52; bytes[37] = 2;
	CHECK(online_trade_wire_read(&message, bytes, sizeof bytes));
	bytes[37] = 1; refused(bytes, sizeof bytes); /* Identical have/want. */
	bytes[1] = 0; CHECK(online_trade_wire_read(&message, bytes, 2) && !message.value.board.count);
}

static void decisions(void) {
	OnlineTradeTerms terms = { .server = {1}, .transaction = {2}, .side = {
		{ .identity = {3}, .run_id = {4}, .checkpoint = {5}, .chip = 1 },
		{ .identity = {6}, .run_id = {7}, .checkpoint = {8}, .chip = 2 } } };
	uint8_t bytes[ONLINE_TRADE_DECISION_BYTES + 1] = { ONLINE_TRADE_DECISION };
	CHECK(online_trade_terms_encode(&terms, bytes + 1));
	OnlineTradeMessage message;
	for (unsigned state = 0; state < 4; ++state) {
		for (unsigned prepared = 0; prepared < 5; ++prepared) {
			for (unsigned applied = 0; applied < 5; ++applied) {
				bytes[191] = (uint8_t)state; bytes[192] = (uint8_t)prepared; bytes[193] = (uint8_t)applied;
				bool valid = state == 1 ? prepared == 3 && applied <= 3 :
					(state == 0 || state == 2) && prepared < 3 && !applied;
				if (!valid) refused(bytes, 194);
				else {
					CHECK(online_trade_wire_read(&message, bytes, 194));
					CHECK(message.value.decision.state == state && message.value.decision.prepared == prepared);
				}
			}
		}
	}
	bytes[191] = 1; bytes[192] = 3; bytes[193] = 0;
	for (size_t size = 0; size < 194; ++size) refused(bytes, size);
	refused(bytes, 195);
	bytes[1] = 0; refused(bytes, 194); bytes[1] = 1;
	bytes[49] = 6; refused(bytes, 194); /* Both identities now match. */
}

static void commands(void) {
	uint8_t out[54], before[54], id[16] = {1}, digest[32] = {2};
	OnlineTradeReady ready = { .run_id = {1}, .checkpoint = {2}, .revision = 0x12345678, .rules = 3 };
	memset(out, 0xa5, sizeof out);
	CHECK(online_trade_wire_ready(out, &ready));
	CHECK(out[0] == ONLINE_TRADE_READY && !memcmp(out + 17, "\x12\x34\x56\x78", 4) && out[53] == 3);
	memcpy(before, out, sizeof out);
	ready.revision = UINT32_MAX; CHECK(!online_trade_wire_ready(out, &ready) && !memcmp(out, before, sizeof out));
	ready.revision = 0; ready.rules = 4; CHECK(!online_trade_wire_ready(out, &ready));
	CHECK(online_trade_wire_request(out, id, 1537, 13314));
	CHECK(out[0] == ONLINE_TRADE_REQUEST && out[17] == 6 && out[18] == 1 && out[19] == 52 && out[20] == 2);
	CHECK(!online_trade_wire_request(out, id, 1537, 1537));
	CHECK(!online_trade_wire_request(out, id, 0, 1));
	CHECK(online_trade_wire_prepare(out, id, digest) && out[0] == ONLINE_TRADE_PREPARE && out[17] == 2);
	CHECK(!online_trade_wire_prepare(out, NULL, digest));
	for (unsigned kind = 0; kind < 256; ++kind) {
		bool expected = kind == ONLINE_TRADE_CANCEL || kind == ONLINE_TRADE_STATUS || kind == ONLINE_TRADE_ACK;
		CHECK(online_trade_wire_transaction(out, (uint8_t)kind, id) == expected);
	}
	CHECK(!online_trade_wire_transaction(out, ONLINE_TRADE_ACK, (const uint8_t[16]){0}));
	for (unsigned entry = 0; entry <= UINT16_MAX; ++entry) {
		bool expected = !entry || (entry % 512 != 0 && entry / 512 <= 26);
		CHECK(online_trade_wire_offer(out, (uint16_t)entry, 0) == expected);
	}
	CHECK(!online_trade_wire_offer(out, 0, 1));
	CHECK(!online_trade_wire_offer(out, 1, 1));
	CHECK(online_trade_wire_offer(out, 1, 513) && out[3] == 2 && out[4] == 1);
}

static void untrusted_bytes(void) {
	uint8_t bytes[513];
	for (unsigned kind = 0; kind < 256; ++kind) {
		memset(bytes, (int)kind, sizeof bytes);
		for (size_t size = 0; size <= sizeof bytes; ++size) refused(bytes, size);
	}
	OnlineTradeMessage message;
	CHECK(!online_trade_wire_read(NULL, bytes, 1));
	CHECK(!online_trade_wire_read(&message, NULL, 1));
}

static void inboxes(void) {
	OnlineTradeInbox inbox = {0};
	OnlineTradeMessage event = {0};
	uint8_t caps[36] = {ONLINE_TRADE_CAPS, 0, 1, 7}, board[2] = {ONLINE_TRADE_BOARD, 0};
	uint8_t command[54] = {ONLINE_TRADE_STATUS};
	CHECK(!online_trade_inbox_push(&inbox, board, sizeof board));
	CHECK(!online_trade_inbox_command(&inbox, command, 17));
	CHECK(online_trade_inbox_push(&inbox, caps, sizeof caps));
	CHECK(!online_trade_inbox_push(&inbox, caps, sizeof caps));
	CHECK(online_trade_inbox_command(&inbox, command, 17));
	CHECK(!online_trade_inbox_command(&inbox, command, 16));
	command[0] = ONLINE_TRADE_READY;
	CHECK(!online_trade_inbox_command(&inbox, command, sizeof command));
	for (unsigned i = 1; i < ONLINE_TRADE_INBOX_SIZE; ++i)
		CHECK(online_trade_inbox_push(&inbox, board, sizeof board));
	CHECK(!online_trade_inbox_push(&inbox, board, sizeof board));
	caps[3] = 0; /* The queued capability owns its bytes. */
	CHECK(online_trade_inbox_pop(&inbox, &event) && event.kind == ONLINE_TRADE_CAPS && event.value.caps.server[0] == 7);
	for (unsigned i = 1; i < ONLINE_TRADE_INBOX_SIZE; ++i)
		CHECK(online_trade_inbox_pop(&inbox, &event) && event.kind == ONLINE_TRADE_BOARD);
	CHECK(!online_trade_inbox_pop(&inbox, &event));
	CHECK(online_trade_inbox_push(&inbox, board, sizeof board) && online_trade_inbox_pop(&inbox, &event));
	memset(&inbox, 0, sizeof inbox); caps[3] = 7; caps[35] = 1;
	CHECK(online_trade_inbox_push(&inbox, caps, sizeof caps));
	CHECK(online_trade_inbox_command(&inbox, command, sizeof command));
	command[0] = ONLINE_TRADE_DECISION;
	CHECK(!online_trade_inbox_command(&inbox, command, sizeof command));
}

static void protocols(void) {
	OnlineNegotiation selected = {0};
	CHECK(!strcmp(online_negotiation_offer(&selected), ONLINE_SUBPROTOCOL));
	CHECK(!online_negotiation_accept(&selected, ONLINE_TRADE_SUBPROTOCOL) && !selected.trades);
	selected.offer_trades = true;
	CHECK(!strcmp(online_negotiation_offer(&selected), ONLINE_TRADE_PROTOCOLS));
	CHECK(online_negotiation_accept(&selected, ONLINE_TRADE_SUBPROTOCOL) && selected.trades);
	CHECK(online_negotiation_accept(&selected, ONLINE_SUBPROTOCOL) && !selected.trades);
	CHECK(!online_negotiation_accept(&selected, ONLINE_TRADE_PROTOCOLS) && !selected.trades);
	CHECK(!online_negotiation_accept(&selected, NULL));
}

int test_online_trade_wire(void) {
	capabilities(); listings(); decisions(); commands(); untrusted_bytes(); inboxes(); protocols();
	if (!failures) puts("Bounded trade messages reject malformed lengths, identities, states and chip codes");
	return failures;
}

#ifdef CW_TRADE_WIRE_TEST_MAIN
int main(void) { return test_online_trade_wire() ? 1 : 0; }
#endif
