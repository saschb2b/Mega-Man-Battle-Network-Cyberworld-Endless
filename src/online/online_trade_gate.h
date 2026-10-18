/* Frame-thread admission of ordinary saves and play. The disk journal
 * remains authoritative; cached state never supplies a server decision. */
#ifndef CW_ONLINE_TRADE_GATE_H
#define CW_ONLINE_TRADE_GATE_H

#include "online_trade_journal.h"

typedef enum {
	TRADE_GATE_CLEAR, TRADE_GATE_NEW_RUN, TRADE_GATE_UNBOUND,
	TRADE_GATE_PENDING, TRADE_GATE_BLOCKED
} OnlineTradeGateState;

/* Called before import recovery or any save reader/writer, with the save
 * queue drained. Repeated calls for this root preserve its live intent. */
void online_trade_gate_start(const char *root);
OnlineTradeGateState online_trade_gate_state(void);
bool online_trade_gate_playable(void);
/* Retry durable recovery. NEW_RUN becomes UNBOUND if its live session
 * has ended; title entry uses this before offering any continuation. */
bool online_trade_gate_recover(void);
bool online_trade_gate_pending(OnlineTradeJournal *out);
/* Explicit NEW GAME after final seed selection, before any save writes.
 * Queue must be drained. Unsupported storage keeps offline behavior. */
bool online_trade_gate_new_run(uint32_t seed);
/* First complete checkpoint only: caller has drained the queue and
 * supplies whether its failure sequence stayed unchanged. On failure
 * keep the live core; do not claim a checkpoint or start an online visit. */
bool online_trade_gate_checkpoint(bool writes_succeeded);
bool online_trade_gate_discard(void);
/* NEW GAME's director startup abandons the old checkpoint but must keep
 * the live first-save intent. Unsupported storage retires the old files. */
bool online_trade_gate_abandon_previous(void);
/* Explicit completed/lost run only, queue drained. Retains durable
 * receipt history before removing the ended checkpoint and its receipt. */
bool online_trade_gate_end_run(void);

/* Ordinary I/O seams. Run readers cannot see an abandoned old checkpoint
 * while NEW_RUN is waiting for its first complete save. */
bool online_trade_gate_read(const char *name);
bool online_trade_gate_write(const char *path);
/* Export/import/Undo/mirror must not move a live unbound checkpoint. */
bool online_trade_gate_transfer(const char *root);
/* After ordinary import recovery/install: validate and persist history
 * before readers can use its checkpoint. Failure blocks the active root. */
bool online_trade_gate_admit(const char *root);
bool online_trade_gate_load(const char *path);

#endif
