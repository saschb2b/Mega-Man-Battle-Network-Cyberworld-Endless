/* Developer fixture for disposable copies only. The pending stage keeps
 * run.state identical; it demonstrates recovery UI, not a native swap. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "online_trade_files.h"
#include "online_trade_history.h"
#include "online_trade_identity.h"
#include "online_trade_journal.h"
int main(int argc, char **argv) {
	if (argc != 3 || (strcmp(argv[2], "unbound") && strcmp(argv[2], "pending") && strcmp(argv[2], "blocked"))) return 2;
	const char *root = argv[1], *mode = argv[2];
	uint8_t namespace_id[16] = {21}, fresh[16] = {33};
	if (!online_trade_history_create(root, namespace_id)) return 3;
	if (!strcmp(mode, "unbound")) return online_trade_identity_intent(root, 42, fresh) ? 0 : 4;
	char current[TRADE_PATH_MAX], path[TRADE_PATH_MAX];
	if (!trade_files_path(current, root, "savedata")) return 5;
	if (!strcmp(mode, "blocked")) {
		const uint8_t damaged[] = {1,2,3};
		return trade_files_path(path, root, "trade.pending") && trade_files_write(path, damaged, sizeof damaged) && trade_files_sync(root) ? 0 : 6;
	}
	uint32_t seed;
	uint8_t content[32];
	if (!online_trade_identity_seed(root, &seed) || !trade_files_hash(current, true, content) || !online_trade_identity_migrate(root, seed, content)) return 7;
	size_t n = 0;
	if (!trade_files_path(path, current, "run.trade")) return 8;
	uint8_t *record = trade_files_read(path, &n);
	OnlineTradeTerms terms = { .server = {1}, .transaction = {2}, .side = { {.identity={3},.chip=1}, {.identity={4},.run_id={6},.checkpoint={8},.chip=2} } };
	bool valid = online_trade_record_read(record, n, terms.side[0].run_id, &terms.side[0].revision);
	free(record);
	if (!valid) return 9;
	if (!trade_files_hash(current, false, terms.side[0].checkpoint) || !trade_files_path(path, current, "run.state")) return 10;
	uint8_t *state = trade_files_read(path, &n);
	OnlineTradeReplacement same = {"run.state", state, n};
	bool ok = state && online_trade_journal_prepare(root, &terms, 0, &same, 1);
	free(state);
	return ok ? 0 : 11;
}
