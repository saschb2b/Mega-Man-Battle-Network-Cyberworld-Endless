/* Isolated durable local trade storage. No game/menu capability is
 * enabled by this component; callers must gate all progress and transfer
 * writers while a journal exists, and own a settled source checkpoint. */
#ifndef CW_ONLINE_TRADE_JOURNAL_H
#define CW_ONLINE_TRADE_JOURNAL_H

#include "online_trade_terms.h"

typedef enum {
	TRADE_JOURNAL_CLEAR, TRADE_JOURNAL_PREPARED, TRADE_JOURNAL_COMMITTED,
	TRADE_JOURNAL_APPLIED, TRADE_JOURNAL_ABORTED, TRADE_JOURNAL_BROKEN,
	TRADE_JOURNAL_UNSUPPORTED
} OnlineTradeJournalState;

typedef struct {
	OnlineTradeTerms terms;
	unsigned side;
	OnlineTradeJournalState state;
	uint8_t source[32], staged[32], post_content[32];
} OnlineTradeJournal;

typedef struct {
	const char *name; /* run.state or profile.sav only */
	const uint8_t *bytes;
	size_t size;
} OnlineTradeReplacement;

/* True only once source, full stage, receipt and PREPARED journal are flushed
 * and read back. Source remains untouched. A false return may still leave
 * a pending journal: recover before allowing any offline progress. */
bool online_trade_journal_prepare(const char *root, const OnlineTradeTerms *terms, unsigned side,
	const OnlineTradeReplacement *replacement, size_t count);
/* Reads the journal and completes only already-recorded COMMITTED/APPLIED
 * or ABORTED work. PREPARED/unknown/corrupt stays reserved. No timeouts. */
OnlineTradeJournalState online_trade_journal_recover(const char *root, OnlineTradeJournal *out);
/* Only an authoritative, matching server decision may call this. Exact
 * terms are compared; COMMITTED can never become ABORTED. */
OnlineTradeJournalState online_trade_journal_decide(const char *root, const OnlineTradeTerms *terms, bool committed);
/* After the server durably acknowledges local APPLIED, remember its run
 * revision in local history before releasing the journal. Repeated
 * acknowledgement after success is harmless; missing history blocks. */
bool online_trade_journal_acknowledged(const char *root, const OnlineTradeTerms *terms);

#endif
