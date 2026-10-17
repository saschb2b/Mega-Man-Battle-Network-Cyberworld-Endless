#include "online_trade_admit.h"

#include <stdlib.h>

#include "network_prefs.h"
#include "online_trade_files.h"
#include "online_trade_history.h"
#include "online_trade_identity.h"

bool online_trade_admit_namespace(const char *root) {
	uint8_t id[NETWORK_ID_BYTES];
	OnlineTradeHistoryResult status = online_trade_history_namespace(root, NULL);
	if (status == TRADE_HISTORY_OK) return true;
	return status == TRADE_HISTORY_MISSING && network_identity(id) && online_trade_history_create(root, id);
}

static bool admit(const char *root, const char *directory, bool remember) {
	if (!trade_files_supported()) return true;
	char path[TRADE_PATH_MAX];
	if (!trade_files_path(path, directory, "run.trade")) return false;
	int receipt = trade_files_kind(path);
	uint32_t seed;
	if (!online_trade_identity_seed_dir(directory, &seed)) return receipt == 0;
	if (!receipt) {
		OnlineTradeHistoryResult status = online_trade_history_missing(root, seed);
		return status == TRADE_HISTORY_OK || status == TRADE_HISTORY_MISSING;
	}
	if (receipt != 1) return false;
	size_t n = 0;
	uint8_t *record = trade_files_read(path, &n);
	bool ok = online_trade_record_read(record, n, NULL, NULL) &&
		trade_files_path(path, directory, "run.state") && trade_files_kind(path) == 1;
	if (ok) {
		OnlineTradeHistoryResult status = online_trade_history_check(root, seed, record, n);
		ok = status == TRADE_HISTORY_OK || status == TRADE_HISTORY_MISSING;
	}
	if (ok && remember) ok = online_trade_admit_namespace(root) &&
		online_trade_history_remember(root, seed, record, n) == TRADE_HISTORY_OK;
	free(record);
	return ok;
}

bool online_trade_admit_check(const char *root, const char *directory) { return admit(root, directory, false); }
bool online_trade_admit_remember(const char *root, const char *directory) { return admit(root, directory, true); }
