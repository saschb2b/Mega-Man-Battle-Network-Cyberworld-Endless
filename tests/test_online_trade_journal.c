#include "test_online_trade_journal.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "online_trade_files.h"
#include "online_trade_journal.h"
#include "online_trade_record.h"
#include "sha256.h"

static int failures;
#define CHECK(value) do { if (!(value)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #value); ++failures; } } while (0)

typedef struct {
	char root[TRADE_PATH_MAX], current[TRADE_PATH_MAX], undo[TRADE_PATH_MAX];
	OnlineTradeTerms terms;
	uint8_t source[32], staged[32], undo_hash[32];
} Fixture;
static const uint8_t original[] = { 1, 7, 23, 91 }, replacement[] = { 0, 6, 24, 91 };
static const OnlineTradeReplacement change = { "run.state", replacement, sizeof replacement };

static bool put(const char *directory, const char *name, const uint8_t *bytes, size_t n) {
	char path[TRADE_PATH_MAX];
	return trade_files_path(path, directory, name) && trade_files_write(path, bytes, n);
}

static uint8_t *get(const char *directory, const char *name, size_t *n) {
	char path[TRADE_PATH_MAX];
	return trade_files_path(path, directory, name) ? trade_files_read(path, n) : NULL;
}

static void erase(const char *path) {
	struct stat st;
	if (lstat(path, &st)) return;
	if (!S_ISDIR(st.st_mode)) { CHECK(unlink(path) == 0); return; }
	DIR *dir = opendir(path);
	CHECK(dir != NULL);
	if (!dir) return;
	for (struct dirent *entry; (entry = readdir(dir));) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
		char child[TRADE_PATH_MAX];
		CHECK(trade_files_path(child, path, entry->d_name));
		erase(child);
	}
	CHECK(closedir(dir) == 0);
	CHECK(rmdir(path) == 0);
}

static bool fixture(Fixture *f) {
	memset(f, 0, sizeof *f);
	memcpy(f->root, "/tmp/cw-trade-XXXXXX", sizeof "/tmp/cw-trade-XXXXXX");
	if (!mkdtemp(f->root) || !trade_files_path(f->current, f->root, "savedata") ||
		!trade_files_path(f->undo, f->root, "savedata.old") || !trade_files_mkdir(f->current) || !trade_files_mkdir(f->undo)) return false;
	memset(f->terms.server, 1, 32); memset(f->terms.transaction, 2, 16);
	for (unsigned i = 0; i < 2; ++i) {
		memset(f->terms.side[i].identity, (int)i + 3, 16);
		memset(f->terms.side[i].run_id, (int)i + 5, 16);
		memset(f->terms.side[i].checkpoint, (int)i + 7, 32);
		f->terms.side[i].chip = (uint16_t)(i + 1);
	}
	uint8_t record[ONLINE_TRADE_RECORD_INITIAL];
	return online_trade_record_new(f->terms.side[0].run_id, record) &&
		put(f->current, "run.trade", record, sizeof record) && put(f->current, "profile.sav", original, sizeof original) &&
		put(f->current, "run.sav", original, sizeof original) && put(f->current, "run.state", original, sizeof original) &&
		put(f->current, "run.act", original, sizeof original) && put(f->current, "transfer.manifest", original, sizeof original) &&
		put(f->undo, "run.state", original, sizeof original) && trade_files_sync(f->current) && trade_files_sync(f->root) &&
		trade_files_hash(f->current, false, f->source) && trade_files_hash(f->undo, false, f->undo_hash) &&
		(memcpy(f->terms.side[0].checkpoint, f->source, 32), true);
}

static void unchanged(const char *directory, const uint8_t expected[32]) {
	uint8_t actual[32];
	CHECK(trade_files_hash(directory, false, actual));
	CHECK(!memcmp(actual, expected, 32));
}

static void prepared(Fixture *f) {
	CHECK(online_trade_journal_prepare(f->root, &f->terms, 0, &change, 1));
	OnlineTradeJournal journal;
	CHECK(online_trade_journal_recover(f->root, &journal) == TRADE_JOURNAL_PREPARED);
	memcpy(f->staged, journal.staged, 32);
	unchanged(f->current, f->source);
}

static void installed(Fixture *f) {
	unchanged(f->current, f->staged);
	unchanged(f->undo, f->undo_hash);
	size_t n;
	uint8_t *record = get(f->current, "run.trade", &n), id[16];
	uint32_t revision = 0;
	CHECK(online_trade_record_read(record, n, id, &revision));
	CHECK(revision == 1 && !memcmp(id, f->terms.side[0].run_id, 16));
	CHECK(online_trade_record_has(record, n, &f->terms));
	free(record);
	uint8_t *state = get(f->current, "run.state", &n);
	CHECK(state && n == sizeof replacement && !memcmp(state, replacement, n));
	free(state);
	char manifest[TRADE_PATH_MAX];
	CHECK(trade_files_path(manifest, f->current, "transfer.manifest"));
	CHECK(trade_files_kind(manifest) == 0);
}

static void commit_and_retry(void) {
	Fixture f;
	CHECK(fixture(&f));
	prepared(&f);
	CHECK(!online_trade_journal_prepare(f.root, &f.terms, 0, &change, 1));
	OnlineTradeTerms other = f.terms; other.transaction[0] ^= 1;
	CHECK(online_trade_journal_decide(f.root, &other, true) == TRADE_JOURNAL_BROKEN);
	CHECK(online_trade_journal_decide(f.root, &f.terms, true) == TRADE_JOURNAL_APPLIED);
	installed(&f);
	CHECK(online_trade_journal_decide(f.root, &f.terms, false) == TRADE_JOURNAL_BROKEN);
	CHECK(online_trade_journal_decide(f.root, &f.terms, true) == TRADE_JOURNAL_APPLIED);
	CHECK(online_trade_journal_acknowledged(f.root, &f.terms));
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_CLEAR);
	CHECK(online_trade_journal_acknowledged(f.root, &f.terms));
	CHECK(!online_trade_journal_acknowledged(f.root, &other));
	/* An ordinary later checkpoint need not match a historic receipt's
	 * post-content digest, and replayed acknowledgement remains harmless. */
	CHECK(put(f.current, "run.state", original, sizeof original));
	CHECK(online_trade_journal_acknowledged(f.root, &f.terms));
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_CLEAR);
	unchanged(f.undo, f.undo_hash);
	erase(f.root);
}

static void abort_and_invalid_source(void) {
	Fixture f;
	CHECK(fixture(&f));
	prepared(&f);
	CHECK(online_trade_journal_decide(f.root, &f.terms, false) == TRADE_JOURNAL_CLEAR);
	unchanged(f.current, f.source);
	unchanged(f.undo, f.undo_hash);
	OnlineTradeTerms wrong = f.terms; ++wrong.side[0].revision;
	CHECK(!online_trade_journal_prepare(f.root, &wrong, 0, &change, 1));
	wrong = f.terms; wrong.side[0].checkpoint[0] ^= 1;
	CHECK(!online_trade_journal_prepare(f.root, &wrong, 0, &change, 1));
	OnlineTradeReplacement bad = { "run.sav", replacement, sizeof replacement };
	CHECK(!online_trade_journal_prepare(f.root, &f.terms, 0, &bad, 1));
	char path[TRADE_PATH_MAX];
	CHECK(trade_files_path(path, f.current, "run.trade") && trade_files_remove(path));
	CHECK(!online_trade_journal_prepare(f.root, &f.terms, 0, &change, 1));
	CHECK(trade_files_kind(path) == 0);
	CHECK(put(f.current, "run.trade", original, sizeof original));
	CHECK(!online_trade_journal_prepare(f.root, &f.terms, 0, &change, 1));
	size_t n;
	uint8_t *still = trade_files_read(path, &n);
	CHECK(still && n == sizeof original && !memcmp(still, original, n));
	free(still);
	erase(f.root);
}

static void malformed_storage(void) {
	Fixture f;
	CHECK(fixture(&f));
	char path[TRADE_PATH_MAX];
	uint8_t hash[32];
	CHECK(!trade_files_path(path, f.root, ".."));
	CHECK(!trade_files_path(path, f.root, "nested/file"));
	CHECK(put(f.current, "unknown", original, sizeof original));
	CHECK(!trade_files_hash(f.current, false, hash));
	CHECK(trade_files_path(path, f.current, "unknown") && trade_files_remove(path));
	CHECK(trade_files_path(path, f.current, "run.area") && symlink("run.sav", path) == 0);
	CHECK(!trade_files_hash(f.current, false, hash));
	CHECK(unlink(path) == 0 && mkdir(path, 0700) == 0);
	CHECK(!trade_files_hash(f.current, false, hash));
	CHECK(rmdir(path) == 0);
	int lock = trade_files_lock(f.root);
	CHECK(lock >= 0);
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_BROKEN);
	trade_files_unlock(lock);
	prepared(&f);
	CHECK(put(f.root, "trade.pending", original, sizeof original));
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_BROKEN);
	CHECK(online_trade_journal_decide(f.root, &f.terms, false) == TRADE_JOURNAL_BROKEN);
	unchanged(f.current, f.source);
	erase(f.root);
	CHECK(fixture(&f));
	CHECK(trade_files_path(path, f.root, "trade.stage") && trade_files_mkdir(path));
	CHECK(put(path, "unknown", original, sizeof original));
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_BROKEN);
	size_t n;
	uint8_t *untouched = get(path, "unknown", &n);
	CHECK(untouched && n == sizeof original && !memcmp(untouched, original, n));
	free(untouched);
	erase(f.root);
}

static void record_integrity(void) {
	Fixture f;
	CHECK(fixture(&f));
	uint8_t initial[ONLINE_TRADE_RECORD_INITIAL], post[32] = { 1 };
	CHECK(online_trade_record_new(f.terms.side[0].run_id, initial));
	size_t n;
	uint8_t *one = online_trade_record_append(initial, sizeof initial, &f.terms, 0, post, &n);
	CHECK(one != NULL);
	if (!one) { erase(f.root); return; }
	for (size_t i = 0; i < n; ++i) {
		one[i] ^= 1;
		CHECK(!online_trade_record_read(one, n, NULL, NULL));
		one[i] ^= 1;
	}
	CHECK(!online_trade_record_read(one, n - 1, NULL, NULL));
	size_t next_n;
	CHECK(!online_trade_record_append(one, n, &f.terms, 0, post, &next_n));
	OnlineTradeTerms second = f.terms;
	++second.side[0].revision; ++second.transaction[0];
	uint8_t *two = online_trade_record_append(one, n, &second, 0, post, &next_n);
	CHECK(two != NULL);
	uint32_t revision = 0;
	CHECK(online_trade_record_read(two, next_n, NULL, &revision) && revision == 2);
	CHECK(online_trade_record_has(two, next_n, &f.terms) && online_trade_record_has(two, next_n, &second));
	/* A checksum alone must not make malformed revision chains valid. */
	one[28 + ONLINE_TRADE_TERMS_BYTES] = 2;
	cw_sha256(one, n - 32, one + n - 32);
	CHECK(!online_trade_record_read(one, n, NULL, NULL));
	free(one); free(two);
	erase(f.root);
}

#ifdef CW_TRADE_FILES_TEST
static unsigned boundary, target;
static bool crash;
static const char *last_point;

static bool fault(const char *name) {
	last_point = name;
	if (++boundary != target) return true;
	if (crash) _exit(77);
	return false;
}

typedef enum { PREPARE, COMMIT, ACK, CERTIFY } Operation;
static void operate(Fixture *f, Operation op) {
	if (op == PREPARE) (void)online_trade_journal_prepare(f->root, &f->terms, 0, &change, 1);
	if (op == COMMIT) (void)online_trade_journal_decide(f->root, &f->terms, true);
	if (op == ACK) (void)online_trade_journal_acknowledged(f->root, &f->terms);
	if (op == CERTIFY) (void)online_trade_journal_recover(f->root, NULL);
}

static void ready(Fixture *f, Operation op) {
	CHECK(fixture(f));
	if (op != PREPARE) prepared(f);
	if (op == ACK) CHECK(online_trade_journal_decide(f->root, &f->terms, true) == TRADE_JOURNAL_APPLIED);
}

static void recovered(Fixture *f, Operation op) {
	OnlineTradeJournalState state = online_trade_journal_recover(f->root, NULL);
	if (op == PREPARE || op == CERTIFY) {
		CHECK(state == TRADE_JOURNAL_CLEAR || state == TRADE_JOURNAL_PREPARED);
		unchanged(f->current, f->source);
		if (state == TRADE_JOURNAL_PREPARED)
			CHECK(online_trade_journal_decide(f->root, &f->terms, false) == TRADE_JOURNAL_CLEAR);
	} else {
		if (state == TRADE_JOURNAL_PREPARED) state = online_trade_journal_decide(f->root, &f->terms, true);
		CHECK(state == TRADE_JOURNAL_APPLIED || (op == ACK && state == TRADE_JOURNAL_CLEAR));
		installed(f);
		CHECK(online_trade_journal_acknowledged(f->root, &f->terms));
		CHECK(online_trade_journal_recover(f->root, NULL) == TRADE_JOURNAL_CLEAR);
	}
	unchanged(f->undo, f->undo_hash);
}

static void at_boundary(Operation op, unsigned at, bool death) {
	Fixture f;
	ready(&f, op);
	boundary = 0; target = at; crash = death;
	trade_files_test_hook(fault);
	if (death) {
		pid_t pid = fork();
		CHECK(pid >= 0);
		if (!pid) { operate(&f, op); _exit(78); }
		int status = 0;
		CHECK(pid > 0 && waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 77);
	} else operate(&f, op);
	trade_files_test_hook(NULL);
	int before = failures;
	recovered(&f, op);
	if (failures != before) fprintf(stderr, "operation %u boundary %u death %d (%s)\n", op, at, death, last_point);
	erase(f.root);
}

static char source_stage_path[TRADE_PATH_MAX];
static bool source_flush_seen;
static bool fail_source_flush(const char *name) {
	if (!strcmp(name, "before-file-flush") && !trade_files_kind(source_stage_path)) {
		source_flush_seen = true;
		return false;
	}
	return true;
}

static void source_is_durable(void) {
	Fixture f;
	CHECK(fixture(&f));
	CHECK(trade_files_path(source_stage_path, f.root, "trade.stage"));
	source_flush_seen = false;
	trade_files_test_hook(fail_source_flush);
	CHECK(!online_trade_journal_prepare(f.root, &f.terms, 0, &change, 1));
	trade_files_test_hook(NULL);
	CHECK(source_flush_seen);
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_CLEAR);
	unchanged(f.current, f.source);
	erase(f.root);
}

static char pending_path[TRADE_PATH_MAX];
static bool fail_final_prepare_flush(const char *name) {
	return strcmp(name, "before-directory-flush") || trade_files_kind(pending_path) != 1;
}
static bool fail_file_flush(const char *name) { return strcmp(name, "before-file-flush") != 0; }
static bool fail_directory_flush(const char *name) { return strcmp(name, "before-directory-flush") != 0; }

static void visible_is_not_durable(void) {
	Fixture f;
	CHECK(fixture(&f));
	CHECK(trade_files_path(pending_path, f.root, "trade.pending"));
	trade_files_test_hook(fail_final_prepare_flush);
	CHECK(!online_trade_journal_prepare(f.root, &f.terms, 0, &change, 1));
	trade_files_test_hook(NULL);
	CHECK(trade_files_kind(pending_path) == 1);
	trade_files_test_hook(fail_file_flush);
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_BROKEN);
	trade_files_test_hook(fail_directory_flush);
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_BROKEN);
	trade_files_test_hook(NULL);
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_PREPARED);
	unchanged(f.current, f.source);
	erase(f.root);
}

static bool stop_install(const char *name) { return strcmp(name, "before-move") != 0; }

static void damaged_committed_stage(void) {
	Fixture f;
	CHECK(fixture(&f));
	prepared(&f);
	trade_files_test_hook(stop_install);
	CHECK(online_trade_journal_decide(f.root, &f.terms, true) == TRADE_JOURNAL_COMMITTED);
	trade_files_test_hook(NULL);
	char stage[TRADE_PATH_MAX];
	CHECK(trade_files_path(stage, f.root, "trade.stage"));
	CHECK(put(stage, "run.state", original, sizeof original));
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_BROKEN);
	CHECK(online_trade_journal_decide(f.root, &f.terms, false) == TRADE_JOURNAL_BROKEN);
	unchanged(f.current, f.source);
	/* The recorded commit cannot restore the outgoing chip to play.
	 * Only repairing the exact promised stage permits installation. */
	CHECK(put(stage, "run.state", replacement, sizeof replacement));
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_APPLIED);
	installed(&f);
	erase(f.root);
}

static char corrupt_path[TRADE_PATH_MAX];
static bool corrupt_readback(const char *name) {
	if (strcmp(name, "after-file-rename") || trade_files_kind(corrupt_path) != 1) return true;
	FILE *file = fopen(corrupt_path, "ab");
	CHECK(file != NULL);
	if (file) { CHECK(fputc(0, file) == 0); CHECK(fclose(file) == 0); }
	return true;
}

static void damaged_write_readback(void) {
	Fixture f;
	CHECK(fixture(&f));
	char stage[TRADE_PATH_MAX];
	CHECK(trade_files_path(stage, f.root, "trade.stage"));
	CHECK(trade_files_path(corrupt_path, stage, "profile.sav"));
	trade_files_test_hook(corrupt_readback);
	CHECK(!online_trade_journal_prepare(f.root, &f.terms, 0, &change, 1));
	trade_files_test_hook(NULL);
	CHECK(online_trade_journal_recover(f.root, NULL) == TRADE_JOURNAL_CLEAR);
	unchanged(f.current, f.source);
	erase(f.root);
}

static void fault_sweep(Operation op) {
	Fixture f;
	ready(&f, op);
	boundary = 0; target = 0; crash = false;
	trade_files_test_hook(fault);
	operate(&f, op);
	trade_files_test_hook(NULL);
	unsigned total = boundary;
	recovered(&f, op);
	erase(f.root);
	for (unsigned i = 1; i <= total; ++i) {
		at_boundary(op, i, false);
		at_boundary(op, i, true);
	}
	printf("trade journal operation %u: %u error and %u process-death boundaries\n", op, total, total);
}
#endif

int test_online_trade_journal(void) {
	failures = 0;
	CHECK(trade_files_supported());
	commit_and_retry(); abort_and_invalid_source(); malformed_storage(); record_integrity();
#ifdef CW_TRADE_FILES_TEST
	source_is_durable(); visible_is_not_durable(); damaged_committed_stage(); damaged_write_readback();
	fault_sweep(PREPARE); fault_sweep(COMMIT); fault_sweep(ACK); fault_sweep(CERTIFY);
#endif
	printf("trade journal: %s\n", failures ? "FAILED" : "passed");
	return failures;
}

#ifdef CW_TRADE_JOURNAL_TEST_MAIN
int main(void) { return test_online_trade_journal() ? 1 : 0; }
#endif
