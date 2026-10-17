/* Bind decoded trade messages to this connection and its local checkpoint. */
#ifndef CW_ONLINE_TRADE_AUTHORITY_H
#define CW_ONLINE_TRADE_AUTHORITY_H

#include "online_trade_journal.h"
#include "online_trade_wire.h"

typedef struct {
	uint8_t identity[16], server[32];
	bool known, enabled;
} OnlineTradeAuthority;

/* identity is the authenticated WELCOME identity, never a chip-card field.
 * Begin again on every connection; no state carries across transports. */
bool online_trade_authority_begin(OnlineTradeAuthority *out, const uint8_t identity[16]);
/* A pending journal pins the server and participant across reconnection.
 * A disabled capability may still resolve that journal. Any refusal leaves
 * the previous authority intact; callers must close the refused connection. */
bool online_trade_authority_caps(OnlineTradeAuthority *authority, const OnlineTradeMessage *message,
	const OnlineTradeTerms *pending);
/* This only permits showing both cards, never confirmation or PREPARED.
 * Independently check the current peer, block list and immutable local
 * inventory before inviting the player to confirm. Failure leaves side alone. */
bool online_trade_authority_offer(const OnlineTradeAuthority *authority, const OnlineTradeDecision *decision,
	const OnlineTradeReady *ready, uint16_t have, uint16_t want, unsigned *side);
/* Only a recovered, certified local journal permits applying a decision.
 * Pass the result of journal_recover, never a fabricated in-memory record:
 * hash consistency here cannot certify that files were flushed to storage.
 * Broken/unsupported storage, other terms or an impossible state refuse.
 * The caller still owns durable installation/history and acknowledgement. */
bool online_trade_authority_recovery(const OnlineTradeAuthority *authority, const OnlineTradeDecision *decision,
	const OnlineTradeJournal *journal);

#endif
