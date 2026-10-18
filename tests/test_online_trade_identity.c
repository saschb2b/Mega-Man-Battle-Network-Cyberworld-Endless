#include "test_online_trade_identity.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "online_trade_files.h"
#include "online_trade_gate.h"
#include "online_trade_history.h"
#include "online_trade_identity.h"
#include "online_trade_journal.h"
#include "run.h"
#include "save_format.h"

static int failures;
#define CHECK(value) do { if (!(value)) { fprintf(stderr, "identity FAIL line %d: %s\n", __LINE__, #value); ++failures; } } while (0)
typedef struct { char root[TRADE_PATH_MAX], current[TRADE_PATH_MAX]; uint8_t content[32]; } Fixture;
static const uint8_t namespace_id[16] = { 21 }, fresh_id[16] = { 33 };
static const uint8_t profile[] = { 1, 2, 3 }, state[] = { 91, 71, 59, 31 };

static bool put(const char *dir, const char *name, const uint8_t *data, size_t n) {
	char path[TRADE_PATH_MAX];
	return trade_files_path(path, dir, name) && trade_files_write(path, data, n);
}
static uint8_t *get(const char *dir, const char *name, size_t *n) {
	char path[TRADE_PATH_MAX];
	return trade_files_path(path, dir, name) ? trade_files_read(path, n) : NULL;
}
static bool remove_file(const char *dir, const char *name) {
	char path[TRADE_PATH_MAX];
	return trade_files_path(path, dir, name) && trade_files_remove(path);
}
static int kind(const char *dir, const char *name) {
	char path[TRADE_PATH_MAX];
	return trade_files_path(path, dir, name) ? trade_files_kind(path) : -1;
}
static void erase(const char *path) {
	DIR *dir = opendir(path);
	if (!dir) { CHECK(unlink(path) == 0); return; }
	struct dirent *entry;
	while ((entry = readdir(dir))) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
		char child[TRADE_PATH_MAX];
		CHECK(trade_files_path(child, path, entry->d_name));
		erase(child);
	}
	CHECK(closedir(dir) == 0);
	CHECK(rmdir(path) == 0);
}
static bool checkpoint(Fixture *f, uint32_t seed) {
	Run saved = { .active = true, .seed = seed, .depth = 4 };
	uint8_t data[12 + sizeof saved];
	uint32_t header[3] = { RUN_MAGIC, sizeof saved, 2166136261u };
	memcpy(data + 12, &saved, sizeof saved);
	for (size_t i = 12; i < sizeof data; ++i) header[2] = (header[2] ^ data[i]) * 16777619u;
	memcpy(data, header, sizeof header);
	return put(f->current, "run.sav", data, sizeof data) && put(f->current, "run.state", state, sizeof state) &&
		trade_files_hash(f->current, true, f->content);
}
static bool fixture(Fixture *f) {
	memset(f, 0, sizeof *f);
	memcpy(f->root, "/tmp/cw-identity-XXXXXX", sizeof "/tmp/cw-identity-XXXXXX");
	return mkdtemp(f->root) && trade_files_path(f->current, f->root, "savedata") && trade_files_mkdir(f->current) &&
		put(f->current, "profile.sav", profile, sizeof profile) && online_trade_history_create(f->root, namespace_id) && checkpoint(f, 17);
}
static void bytes_match(const char *dir, const char *name, const uint8_t *expected, size_t expected_n) {
	size_t n = 0;
	uint8_t *actual = get(dir, name, &n);
	CHECK(actual && n == expected_n && !memcmp(actual, expected, n));
	free(actual);
}
static void enrolled(Fixture *f, const uint8_t expected_id[16]) {
	size_t n = 0;
	uint8_t *record = get(f->current, "run.trade", &n), id[16];
	uint32_t revision = 99;
	CHECK(online_trade_record_read(record, n, id, &revision));
	CHECK(revision == 0 && !memcmp(id, expected_id, 16));
	CHECK(online_trade_history_check(f->root, 17, record, n) == TRADE_HISTORY_OK);
	free(record);
	CHECK(kind(f->root, "trade.newrun") == 0 && kind(f->root, "trade.newrun.tmp") == 0);
	bytes_match(f->current, "run.state", state, sizeof state);
	bytes_match(f->current, "profile.sav", profile, sizeof profile);
}

static void intent_and_binding(void) {
	Fixture f;
	CHECK(fixture(&f));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_CLEAR);
	CHECK(online_trade_identity_intent(f.root, 17, fresh_id));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_UNBOUND);
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_BROKEN);
	CHECK(!online_trade_identity_intent(f.root, 17, fresh_id));
	CHECK(!online_trade_identity_migrate(f.root, 17, f.content));
	uint8_t wrong[32]; memcpy(wrong, f.content, sizeof wrong); wrong[0] ^= 1;
	CHECK(!online_trade_identity_bind(f.root, wrong));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_UNBOUND);
	CHECK(online_trade_identity_bind(f.root, f.content));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_CLEAR);
	enrolled(&f, fresh_id);
	CHECK(!online_trade_identity_discard(f.root));
	CHECK(!online_trade_identity_migrate(f.root, 17, f.content));
	erase(f.root);
}

static void explicit_discard(void) {
	Fixture f;
	CHECK(fixture(&f));
	uint8_t old[ONLINE_TRADE_RECORD_INITIAL], id[16] = { 1 };
	CHECK(online_trade_record_new(id, old));
	CHECK(put(f.current, "run.trade", old, sizeof old));
	CHECK(online_trade_history_remember(f.root, 3, old, sizeof old) == TRADE_HISTORY_OK);
	CHECK(online_trade_identity_intent(f.root, 17, fresh_id));
	CHECK(kind(f.current, "run.trade") == 0);
	CHECK(put(f.current, "run.state.tmp", state, sizeof state));
	CHECK(put(f.current, "profile.sav.tmp", state, sizeof state));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_UNBOUND);
	CHECK(online_trade_identity_discard(f.root));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_CLEAR);
	CHECK(kind(f.current, "run.state") == 0 && kind(f.current, "run.sav") == 0);
	CHECK(kind(f.current, "run.state.tmp") == 0 && kind(f.current, "profile.sav.tmp") == 0);
	bytes_match(f.current, "profile.sav", profile, sizeof profile);
	CHECK(online_trade_history_check(f.root, 3, old, sizeof old) == TRADE_HISTORY_OK);
	erase(f.root);
}

static void legacy_migration(void) {
	Fixture f;
	CHECK(fixture(&f));
	CHECK(online_trade_identity_migrate(f.root, 17, f.content));
	size_t n = 0;
	uint8_t *initial = get(f.current, "run.trade", &n), id[16];
	CHECK(online_trade_record_read(initial, n, id, NULL));
	enrolled(&f, id);
	CHECK(remove_file(f.current, "run.trade"));
	CHECK(online_trade_identity_migrate(f.root, 17, f.content));
	bytes_match(f.current, "run.trade", initial, n);
	OnlineTradeTerms terms = { .server = { 1 }, .transaction = { 2 } };
	for (unsigned i = 0; i < 2; ++i) {
		memset(terms.side[i].identity, (int)i + 1, 16);
		memset(terms.side[i].run_id, (int)i + 3, 16);
		memset(terms.side[i].checkpoint, (int)i + 5, 32);
		terms.side[i].chip = (uint16_t)(i + 1);
	}
	memcpy(terms.side[0].run_id, id, 16);
	size_t receipt_n = 0;
	uint8_t *receipt = online_trade_record_append(initial, n, &terms, 0, f.content, &receipt_n);
	CHECK(receipt != NULL);
	CHECK(online_trade_history_remember(f.root, 17, receipt, receipt_n) == TRADE_HISTORY_OK);
	CHECK(remove_file(f.current, "run.trade"));
	CHECK(!online_trade_identity_migrate(f.root, 17, f.content));
	CHECK(kind(f.current, "run.trade") == 0);
	free(receipt); free(initial);
	erase(f.root);
}

static void invalid_source(void) {
	Fixture f;
	CHECK(fixture(&f));
	uint32_t seed = 0;
	CHECK(online_trade_identity_seed(f.root, &seed) && seed == 17);
	CHECK(online_trade_identity_intent(f.root, 18, fresh_id));
	CHECK(!online_trade_identity_bind(f.root, f.content));
	CHECK(online_trade_identity_discard(f.root));
	CHECK(checkpoint(&f, 17));
	CHECK(online_trade_identity_intent(f.root, 17, fresh_id));
	CHECK(remove_file(f.current, "run.state"));
	CHECK(trade_files_hash(f.current, true, f.content));
	CHECK(!online_trade_identity_bind(f.root, f.content));
	CHECK(put(f.current, "run.sav", state, sizeof state));
	CHECK(!online_trade_identity_seed(f.root, &seed));
	CHECK(put(f.root, "trade.newrun", state, sizeof state));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_BLOCKED);
	CHECK(!online_trade_identity_discard(f.root));
	erase(f.root);
}

static void gate_lifecycle(void) {
	Fixture f;
	CHECK(fixture(&f));
	online_trade_gate_start(f.root);
	CHECK(online_trade_gate_state() == TRADE_GATE_CLEAR && online_trade_gate_playable());
	CHECK(online_trade_gate_new_run(17));
	CHECK(online_trade_gate_state() == TRADE_GATE_NEW_RUN);
	/* director_start_run abandons the old checkpoint after title NEW
	 * GAME has reserved the new identity, before its first saved layer. */
	CHECK(online_trade_gate_abandon_previous());
	CHECK(online_trade_gate_state() == TRADE_GATE_NEW_RUN && kind(f.root, "trade.newrun") == 1);
	CHECK(online_trade_gate_read("profile.sav") && !online_trade_gate_read("run.sav"));
	CHECK(!online_trade_gate_transfer(f.root));
	CHECK(!online_trade_gate_checkpoint(false) && online_trade_gate_state() == TRADE_GATE_NEW_RUN);
	online_trade_gate_start(f.root);
	CHECK(online_trade_gate_state() == TRADE_GATE_NEW_RUN);
	CHECK(online_trade_gate_checkpoint(true));
	CHECK(online_trade_gate_state() == TRADE_GATE_CLEAR && online_trade_gate_admit(f.root));
	CHECK(kind(f.current, "run.trade") == 1 && kind(f.root, "trade.newrun") == 0);
	CHECK(online_trade_history_missing(f.root, 17) == TRADE_HISTORY_STALE);
	CHECK(online_trade_gate_end_run() && online_trade_gate_state() == TRADE_GATE_CLEAR);
	CHECK(kind(f.current, "run.sav") == 0 && kind(f.current, "run.state") == 0 && kind(f.current, "run.trade") == 0);
	CHECK(online_trade_history_missing(f.root, 17) == TRADE_HISTORY_STALE);
	CHECK(online_trade_gate_new_run(18));
	CHECK(!online_trade_gate_recover() && online_trade_gate_state() == TRADE_GATE_UNBOUND);
	char run_path[TRADE_PATH_MAX], boot_path[TRADE_PATH_MAX];
	CHECK(trade_files_path(run_path, f.current, "run.state") && trade_files_path(boot_path, f.root, "boot-3.state"));
	CHECK(!online_trade_gate_playable() && !online_trade_gate_read("profile.sav"));
	CHECK(!online_trade_gate_write(run_path) && !online_trade_gate_load(run_path));
	CHECK(online_trade_gate_write(boot_path) && online_trade_gate_load(boot_path));
	CHECK(!online_trade_gate_new_run(19));
	CHECK(online_trade_gate_discard() && online_trade_gate_state() == TRADE_GATE_CLEAR);
	bytes_match(f.current, "profile.sav", profile, sizeof profile);
	/* Deliberate loss in the initial town still retires an unbound run. */
	CHECK(online_trade_gate_new_run(19));
	CHECK(online_trade_gate_end_run() && online_trade_gate_state() == TRADE_GATE_CLEAR);
	CHECK(kind(f.root, "trade.newrun") == 0);
	CHECK(put(f.root, "trade.pending", state, sizeof state));
	CHECK(!online_trade_gate_recover() && online_trade_gate_state() == TRADE_GATE_BLOCKED);
	CHECK(!online_trade_gate_transfer(f.root) && !online_trade_gate_discard());
	CHECK(remove_file(f.root, "trade.pending") && online_trade_gate_recover());
	CHECK(put(f.root, "trade.history", state, sizeof state));
	CHECK(!online_trade_gate_recover() && online_trade_gate_state() == TRADE_GATE_BLOCKED);
	erase(f.root);
}


static void gate_pending_copy(void) {
	Fixture f;
	CHECK(fixture(&f));
	CHECK(online_trade_identity_intent(f.root, 17, fresh_id));
	CHECK(online_trade_identity_bind(f.root, f.content));
	OnlineTradeTerms terms = { .server = {1}, .transaction = {2}, .side = {
		{.identity = {3}, .chip = 1}, {.identity = {4}, .run_id = {6}, .checkpoint = {8}, .chip = 2} } };
	memcpy(terms.side[0].run_id, fresh_id, sizeof fresh_id);
	CHECK(trade_files_hash(f.current, false, terms.side[0].checkpoint));
	const OnlineTradeReplacement same = {"run.state", state, sizeof state};
	CHECK(online_trade_journal_prepare(f.root, &terms, 0, &same, 1));
	online_trade_gate_start(f.root);
	CHECK(online_trade_gate_state() == TRADE_GATE_PENDING && !online_trade_gate_playable());
	OnlineTradeJournal pending;
	CHECK(!online_trade_gate_pending(NULL) && online_trade_gate_pending(&pending));
	CHECK(pending.state == TRADE_JOURNAL_PREPARED && pending.side == 0);
	CHECK(!memcmp(pending.source, terms.side[0].checkpoint, sizeof pending.source));
	uint8_t expected[32], actual[32];
	CHECK(online_trade_terms_digest(&terms, expected) && online_trade_terms_digest(&pending.terms, actual));
	CHECK(!memcmp(expected, actual, sizeof expected));
	pending.terms.transaction[0] ^= 1;
	CHECK(online_trade_gate_pending(&pending));
	CHECK(online_trade_terms_digest(&pending.terms, actual) && !memcmp(expected, actual, sizeof expected));
	CHECK(online_trade_journal_decide(f.root, &terms, false) == TRADE_JOURNAL_CLEAR);
	CHECK(online_trade_gate_recover() && !online_trade_gate_pending(&pending));
	erase(f.root);
}

#ifdef CW_TRADE_FILES_TEST
static unsigned boundary, target;
static bool death;
static const char *last_point;
static bool fault(const char *point) {
	last_point = point;
	if (++boundary != target) return true;
	if (death) _exit(77);
	return false;
}
static char marker_path[TRADE_PATH_MAX];
static bool stop_after_bound(const char *point) {
	if (strcmp(point, "after-file-rename")) return true;
	FILE *file = fopen(marker_path, "rb");
	if (!file) return true;
	uint8_t data[9];
	size_t n = fread(data, 1, sizeof data, file);
	CHECK(fclose(file) == 0);
	return n != sizeof data || !data[8];
}
static void leave_bound(Fixture *f) {
	CHECK(trade_files_path(marker_path, f->root, "trade.newrun"));
	trade_files_test_hook(stop_after_bound);
	CHECK(!online_trade_identity_bind(f->root, f->content));
	trade_files_test_hook(NULL);
	CHECK(kind(f->current, "run.trade") == 0);
}
typedef enum { INTENT, BIND, MIGRATE, DISCARD, RECOVER } Operation;
static void ready(Fixture *f, Operation op) {
	CHECK(fixture(f));
	if (op == BIND || op == DISCARD || op == RECOVER) CHECK(online_trade_identity_intent(f->root, 17, fresh_id));
	if (op == RECOVER) leave_bound(f);
}
static void operate(Fixture *f, Operation op) {
	switch (op) {
		case INTENT: (void)online_trade_identity_intent(f->root, 17, fresh_id); break;
		case BIND: (void)online_trade_identity_bind(f->root, f->content); break;
		case MIGRATE: (void)online_trade_identity_migrate(f->root, 17, f->content); break;
		case DISCARD: (void)online_trade_identity_discard(f->root); break;
		case RECOVER: (void)online_trade_identity_recover(f->root); break;
	}
}
static void recovered(Fixture *f, Operation op) {
	OnlineTradeIdentityState status = online_trade_identity_recover(f->root);
	CHECK(status == TRADE_IDENTITY_CLEAR || status == TRADE_IDENTITY_UNBOUND);
	bytes_match(f->current, "profile.sav", profile, sizeof profile);
	if (status == TRADE_IDENTITY_UNBOUND) {
		CHECK(op == INTENT || op == BIND || op == DISCARD);
		CHECK(online_trade_identity_discard(f->root));
		CHECK(kind(f->current, "run.state") == 0 && kind(f->current, "run.sav") == 0);
		return;
	}
	if (op == DISCARD) {
		CHECK(kind(f->current, "run.state") == 0 && kind(f->current, "run.sav") == 0);
	} else if (op == INTENT) {
		CHECK(kind(f->current, "run.trade") == 0);
		bytes_match(f->current, "run.state", state, sizeof state);
	} else if (op == MIGRATE) {
		if (!kind(f->current, "run.trade")) CHECK(online_trade_identity_migrate(f->root, 17, f->content));
		uint8_t expected[ONLINE_TRADE_RECORD_INITIAL], id[16];
		CHECK(online_trade_history_legacy(f->root, 17, expected) == TRADE_HISTORY_OK);
		CHECK(online_trade_record_read(expected, sizeof expected, id, NULL));
		enrolled(f, id);
	} else enrolled(f, fresh_id);
}
static void interrupt_at(Operation op, unsigned at, bool process_death) {
	Fixture f;
	ready(&f, op);
	boundary = 0; target = at; death = process_death;
	trade_files_test_hook(fault);
	if (process_death) {
		pid_t pid = fork();
		CHECK(pid >= 0);
		if (!pid) { operate(&f, op); _exit(78); }
		int status = 0;
		CHECK(pid > 0 && waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 77);
	} else operate(&f, op);
	trade_files_test_hook(NULL);
	int before = failures;
	recovered(&f, op);
	if (before != failures) fprintf(stderr, "identity op %u boundary %u death %d (%s)\n", op, at, process_death, last_point);
	erase(f.root);
}
static void fault_sweep(Operation op) {
	Fixture f;
	ready(&f, op);
	boundary = 0; target = 0; death = false;
	trade_files_test_hook(fault);
	operate(&f, op);
	trade_files_test_hook(NULL);
	unsigned count = boundary;
	recovered(&f, op);
	erase(f.root);
	for (unsigned i = 1; i <= count; ++i) {
		interrupt_at(op, i, false);
		interrupt_at(op, i, true);
	}
	printf("identity operation %u: %u I/O errors + %u process deaths\n", op, count, count);
}
static bool refuse_flush(const char *point) { return strcmp(point, "before-file-flush") != 0; }
static void recovery_must_certify(void) {
	Fixture f;
	ready(&f, RECOVER);
	trade_files_test_hook(refuse_flush);
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_BLOCKED);
	trade_files_test_hook(NULL);
	CHECK(kind(f.current, "run.trade") == 0);
	CHECK(!online_trade_identity_discard(f.root));
	CHECK(put(f.current, "run.state", profile, sizeof profile));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_BLOCKED);
	CHECK(put(f.current, "run.state", state, sizeof state));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_CLEAR);
	enrolled(&f, fresh_id);
	erase(f.root);
}
static void installed_receipt_survives_progress(void) {
	Fixture f;
	ready(&f, RECOVER);
	size_t n = 0;
	uint8_t *marker = get(f.root, "trade.newrun", &n);
	CHECK(marker != NULL);
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_CLEAR);
	/* If the final marker deletion was not persisted, an already installed
	 * initial receipt may accompany later ordinary progress on restart. */
	CHECK(put(f.current, "run.state", profile, sizeof profile));
	CHECK(put(f.root, "trade.newrun", marker, n));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_CLEAR);
	bytes_match(f.current, "run.state", profile, sizeof profile);
	uint8_t other[ONLINE_TRADE_RECORD_INITIAL], id[16] = { 44 };
	CHECK(online_trade_record_new(id, other));
	CHECK(put(f.root, "trade.newrun", marker, n));
	CHECK(put(f.current, "run.trade", other, sizeof other));
	CHECK(online_trade_identity_recover(f.root) == TRADE_IDENTITY_BLOCKED);
	bytes_match(f.current, "run.trade", other, sizeof other);
	free(marker);
	erase(f.root);
}
#endif

int test_online_trade_identity(void) {
	failures = 0;
	intent_and_binding(); explicit_discard(); legacy_migration(); invalid_source(); gate_lifecycle(); gate_pending_copy();
#ifdef CW_TRADE_FILES_TEST
	recovery_must_certify(); installed_receipt_survives_progress();
	for (Operation op = INTENT; op <= RECOVER; ++op) fault_sweep(op);
#endif
	return failures;
}
#ifdef CW_TRADE_IDENTITY_TEST_MAIN
int main(void) { return test_online_trade_identity() ? 1 : 0; }
#endif
