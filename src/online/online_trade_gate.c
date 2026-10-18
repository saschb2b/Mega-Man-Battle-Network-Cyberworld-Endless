#include "online_trade_gate.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "network_prefs.h"
#include "online_trade_admit.h"
#include "online_trade_files.h"
#include "online_trade_identity.h"
#include "online_trade_history.h"

static char active_root[TRADE_PATH_MAX];
static OnlineTradeGateState state;
static OnlineTradeJournal pending;

/* Unsupported backends can still recognize reserved native files copied
 * into their data directory. They must never play past such a marker. */
static bool artifacts(const char *root) {
	static const char *const names[] = { "trade.newrun", "trade.newrun.tmp", "trade.pending", "trade.pending.tmp", "trade.stage", "trade.previous" };
	for (unsigned i = 0; i < sizeof names / sizeof *names; ++i) {
		char path[TRADE_PATH_MAX];
		int n = snprintf(path, sizeof path, "%s/%s", root, names[i]);
		struct stat st;
		if (n < 0 || n >= (int)sizeof path || !stat(path, &st) || errno != ENOENT) return true;
	}
	return false;
}

OnlineTradeGateState online_trade_gate_state(void) { return state; }
bool online_trade_gate_playable(void) { return state == TRADE_GATE_CLEAR || state == TRADE_GATE_NEW_RUN; }

bool online_trade_gate_recover(void) {
	memset(&pending, 0, sizeof pending);
	state = TRADE_GATE_BLOCKED;
	if (!active_root[0]) return false;
	if (!trade_files_supported()) {
		if (!artifacts(active_root)) state = TRADE_GATE_CLEAR;
		return state == TRADE_GATE_CLEAR;
	}
	OnlineTradeIdentityState identity = online_trade_identity_recover(active_root);
	if (identity == TRADE_IDENTITY_UNBOUND) { state = TRADE_GATE_UNBOUND; return false; }
	if (identity != TRADE_IDENTITY_CLEAR) return false;
	OnlineTradeJournalState journal = online_trade_journal_recover(active_root, &pending);
	if (journal == TRADE_JOURNAL_PREPARED || journal == TRADE_JOURNAL_COMMITTED || journal == TRADE_JOURNAL_APPLIED || journal == TRADE_JOURNAL_ABORTED) {
		state = TRADE_GATE_PENDING;
		return false;
	}
	if (journal != TRADE_JOURNAL_CLEAR) return false;
	OnlineTradeHistoryResult history = online_trade_history_namespace(active_root, NULL);
	if (history != TRADE_HISTORY_OK && history != TRADE_HISTORY_MISSING) return false;
	char temporary[TRADE_PATH_MAX];
	if (history == TRADE_HISTORY_MISSING && (!trade_files_path(temporary, active_root, "trade.history.tmp") || trade_files_kind(temporary))) return false;
	state = TRADE_GATE_CLEAR;
	return true;
}

void online_trade_gate_start(const char *root) {
	if (root && !strcmp(active_root, root)) return;
	state = TRADE_GATE_BLOCKED;
	memset(&pending, 0, sizeof pending);
	if (!root || !*root || strlen(root) >= sizeof active_root) { active_root[0] = 0; return; }
	snprintf(active_root, sizeof active_root, "%s", root);
	(void)online_trade_gate_recover();
}

bool online_trade_gate_pending(OnlineTradeJournal *out) {
	if (!out || state != TRADE_GATE_PENDING) return false;
	*out = pending;
	return true;
}

bool online_trade_gate_new_run(uint32_t seed) {
	if (state != TRADE_GATE_CLEAR) return false;
	if (!trade_files_supported()) return true;
	uint8_t id[NETWORK_ID_BYTES];
	state = TRADE_GATE_BLOCKED;
	if (!online_trade_admit_namespace(active_root) || !network_identity(id) || !online_trade_identity_intent(active_root, seed, id)) {
		(void)online_trade_gate_recover();
		if (state == TRADE_GATE_CLEAR) state = TRADE_GATE_BLOCKED;
		return false;
	}
	state = TRADE_GATE_NEW_RUN;
	return true;
}

bool online_trade_gate_checkpoint(bool writes_succeeded) {
	if (!online_trade_gate_playable() || !writes_succeeded) return false;
	if (!trade_files_supported() || state == TRADE_GATE_CLEAR) return true;
	char current[TRADE_PATH_MAX];
	uint8_t hash[32];
	state = TRADE_GATE_BLOCKED;
	bool ok = trade_files_path(current, active_root, "savedata") && trade_files_hash(current, true, hash) && online_trade_identity_bind(active_root, hash);
	if (ok) { state = TRADE_GATE_CLEAR; return true; }
	/* No bound marker means the same still-live core may retry its first
	 * complete checkpoint. Once bound, recovery owns those exact bytes. */
	OnlineTradeIdentityState recovered = online_trade_identity_recover(active_root);
	if (recovered == TRADE_IDENTITY_UNBOUND) state = TRADE_GATE_NEW_RUN;
	else if (recovered == TRADE_IDENTITY_CLEAR) state = TRADE_GATE_CLEAR;
	return state == TRADE_GATE_CLEAR;
}

bool online_trade_gate_discard(void) {
	if (state != TRADE_GATE_UNBOUND || !online_trade_identity_discard(active_root)) return false;
	return online_trade_gate_recover();
}

bool online_trade_gate_end_run(void) {
	if (!online_trade_gate_playable()) return false;
	if (state == TRADE_GATE_NEW_RUN) {
		state = TRADE_GATE_BLOCKED;
		if (!online_trade_identity_discard(active_root)) return false;
		return online_trade_gate_recover();
	}
	char current[TRADE_PATH_MAX];
	int n = snprintf(current, sizeof current, "%s/savedata", active_root);
	state = TRADE_GATE_BLOCKED;
	if (n < 0 || n >= (int)sizeof current || !online_trade_admit_remember(active_root, current)) return false;
	static const char *const names[] = { "run.sav", "battle.sav", "run.state", "run.trade", "transfer.manifest" };
	for (unsigned i = 0; i < sizeof names / sizeof *names; ++i) {
		char path[TRADE_PATH_MAX];
		n = snprintf(path, sizeof path, "%s/%s", current, names[i]);
		if (n < 0 || n >= (int)sizeof path) return false;
		if (trade_files_supported()) { if (!trade_files_remove(path)) return false; }
		else if (remove(path) && errno != ENOENT) return false;
	}
	if (trade_files_supported() && trade_files_kind(current) && !trade_files_sync(current)) return false;
	state = TRADE_GATE_CLEAR;
	return true;
}

bool online_trade_gate_abandon_previous(void) {
	return state == TRADE_GATE_NEW_RUN || online_trade_gate_end_run();
}

bool online_trade_gate_read(const char *name) {
	if (!online_trade_gate_playable()) return false;
	return state != TRADE_GATE_NEW_RUN || (name && (!strcmp(name, "profile.sav") || !strcmp(name, "rivals.sav")));
}

static bool savedata_path(const char *path) {
	if (!path || !active_root[0]) return false;
	size_t n = strlen(active_root);
	return !strncmp(path, active_root, n) && !strncmp(path + n, "/savedata/", 10);
}

bool online_trade_gate_write(const char *path) { return !savedata_path(path) || online_trade_gate_playable(); }
bool online_trade_gate_load(const char *path) { return !savedata_path(path) || online_trade_gate_read("run.state"); }

bool online_trade_gate_transfer(const char *root) {
	if (!root) return false;
	if (!strcmp(root, active_root) && state != TRADE_GATE_CLEAR) return false;
	return !artifacts(root);
}

bool online_trade_gate_admit(const char *root) {
	char current[TRADE_PATH_MAX];
	if (!online_trade_gate_transfer(root)) return false;
	bool ok = !trade_files_supported() || (trade_files_path(current, root, "savedata") && online_trade_admit_remember(root, current));
	if (!ok && !strcmp(root, active_root)) state = TRADE_GATE_BLOCKED;
	return ok;
}
