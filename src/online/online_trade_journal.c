#include "online_trade_journal.h"

#include <stdlib.h>
#include <string.h>

#include "online_trade_files.h"
#include "online_trade_record.h"
#include "sha256.h"

#define JOURNAL_BYTES (10 + ONLINE_TRADE_TERMS_BYTES + 96 + 32)
#define HASH_OFFSET (10 + ONLINE_TRADE_TERMS_BYTES)
static const uint8_t magic[8] = { 'C', 'W', 'T', 'J', '0', '1', '\r', '\n' };

typedef struct {
	char root[TRADE_PATH_MAX], current[TRADE_PATH_MAX], stage[TRADE_PATH_MAX], previous[TRADE_PATH_MAX];
	char pending[TRADE_PATH_MAX], temporary[TRADE_PATH_MAX];
} Paths;

static bool paths(Paths *p, const char *root) {
	if (!root || strlen(root) >= sizeof p->root) return false;
	memcpy(p->root, root, strlen(root) + 1);
	return trade_files_path(p->current, root, "savedata") && trade_files_path(p->stage, root, "trade.stage") &&
		trade_files_path(p->previous, root, "trade.previous") && trade_files_path(p->pending, root, "trade.pending") &&
		trade_files_path(p->temporary, root, "trade.pending.tmp");
}

static bool nonzero(const uint8_t *p, size_t n) {
	uint8_t any = 0;
	for (size_t i = 0; i < n; ++i) any |= p[i];
	return any != 0;
}

static bool journal_read(const Paths *p, OnlineTradeJournal *journal) {
	size_t n;
	uint8_t *data = trade_files_read(p->pending, &n), hash[32];
	bool ok = data && n == JOURNAL_BYTES;
	if (ok) {
		cw_sha256(data, n - 32, hash);
		ok = !memcmp(data, magic, sizeof magic) && !memcmp(hash, data + n - 32, 32) &&
			data[8] >= TRADE_JOURNAL_PREPARED && data[8] <= TRADE_JOURNAL_ABORTED && data[9] <= 1 &&
			online_trade_terms_decode(&journal->terms, data + 10, ONLINE_TRADE_TERMS_BYTES);
	}
	if (ok) {
		journal->state = (OnlineTradeJournalState)data[8]; journal->side = data[9];
		memcpy(journal->source, data + HASH_OFFSET, 32);
		memcpy(journal->staged, data + HASH_OFFSET + 32, 32);
		memcpy(journal->post_content, data + HASH_OFFSET + 64, 32);
		ok = !memcmp(journal->source, journal->terms.side[journal->side].checkpoint, 32) &&
			nonzero(journal->staged, 32) && nonzero(journal->post_content, 32);
	}
	free(data);
	return ok;
}

static bool journal_write(const Paths *p, const OnlineTradeJournal *journal) {
	uint8_t data[JOURNAL_BYTES] = { 0 };
	memcpy(data, magic, sizeof magic);
	data[8] = (uint8_t)journal->state; data[9] = (uint8_t)journal->side;
	if (!online_trade_terms_encode(&journal->terms, data + 10)) return false;
	memcpy(data + HASH_OFFSET, journal->source, 32);
	memcpy(data + HASH_OFFSET + 32, journal->staged, 32);
	memcpy(data + HASH_OFFSET + 64, journal->post_content, 32);
	cw_sha256(data, sizeof data - 32, data + sizeof data - 32);
	if (!trade_files_write(p->pending, data, sizeof data) || !trade_files_sync(p->root)) return false;
	size_t n;
	uint8_t *readback = trade_files_read(p->pending, &n);
	bool ok = readback && n == sizeof data && !memcmp(readback, data, sizeof data);
	free(readback);
	return ok;
}

static bool matches(const char *directory, const uint8_t expected[32]) {
	uint8_t hash[32];
	return trade_files_hash(directory, false, hash) && !memcmp(hash, expected, 32);
}

static bool certified_matches(const char *directory, const uint8_t expected[32]) {
	return trade_files_certify(directory) && matches(directory, expected);
}

static bool same_terms(const OnlineTradeTerms *a, const OnlineTradeTerms *b) {
	uint8_t aa[ONLINE_TRADE_TERMS_BYTES], bb[ONLINE_TRADE_TERMS_BYTES];
	return online_trade_terms_encode(a, aa) && online_trade_terms_encode(b, bb) && !memcmp(aa, bb, sizeof aa);
}

static bool finish_cleanup(const Paths *p) {
	/* The installed receipt already accompanies the new state; or the
	 * durable decision is ABORTED. Delete the journal last. */
	return trade_files_remove_dir(p->previous) && trade_files_remove_dir(p->stage) && trade_files_remove(p->temporary) &&
		trade_files_sync(p->root) && trade_files_remove(p->pending) && trade_files_sync(p->root);
}

static OnlineTradeJournalState install(const Paths *p, OnlineTradeJournal *journal) {
	int current = trade_files_kind(p->current), stage = trade_files_kind(p->stage), previous = trade_files_kind(p->previous);
	if (current == 2 && matches(p->current, journal->staged)) {
		if (stage || (previous && (previous != 2 || !matches(p->previous, journal->source)))) return TRADE_JOURNAL_BROKEN;
	} else {
		if (stage != 2 || !matches(p->stage, journal->staged)) return TRADE_JOURNAL_BROKEN;
		if (current == 2) {
			if (previous || !matches(p->current, journal->source)) return TRADE_JOURNAL_BROKEN;
			if (!trade_files_move(p->current, p->previous) || !trade_files_sync(p->root)) return TRADE_JOURNAL_COMMITTED;
		} else if (current || previous != 2 || !matches(p->previous, journal->source)) return TRADE_JOURNAL_BROKEN;
		if (!trade_files_move(p->stage, p->current) || !trade_files_sync(p->root)) return TRADE_JOURNAL_COMMITTED;
		if (!matches(p->current, journal->staged)) return TRADE_JOURNAL_BROKEN;
	}
	journal->state = TRADE_JOURNAL_APPLIED;
	return journal_write(p, journal) ? TRADE_JOURNAL_APPLIED : TRADE_JOURNAL_COMMITTED;
}

static OnlineTradeJournalState recover(const Paths *p, OnlineTradeJournal *journal) {
	memset(journal, 0, sizeof *journal);
	char identity[TRADE_PATH_MAX];
	if (!trade_files_path(identity, p->root, "trade.newrun") || trade_files_kind(identity) ||
		!trade_files_path(identity, p->root, "trade.newrun.tmp") || trade_files_kind(identity)) return TRADE_JOURNAL_BROKEN;
	int pending = trade_files_kind(p->pending);
	if (!pending) {
		/* PREPARED is sent only after journal+directory flush succeeds.
		 * Without it, a leftover stage was never promised to the server.
		 * A previous directory, however, proves an interrupted install. */
		if (trade_files_kind(p->previous)) return TRADE_JOURNAL_BROKEN;
		return trade_files_remove_dir(p->stage) && trade_files_remove(p->temporary) && trade_files_sync(p->root) ?
			TRADE_JOURNAL_CLEAR : TRADE_JOURNAL_BROKEN;
	}
	if (pending != 1 || !journal_read(p, journal)) return TRADE_JOURNAL_BROKEN;
	if (journal->state == TRADE_JOURNAL_ABORTED) {
		if (trade_files_kind(p->previous) || !matches(p->current, journal->source)) return TRADE_JOURNAL_BROKEN;
		return finish_cleanup(p) ? TRADE_JOURNAL_CLEAR : TRADE_JOURNAL_ABORTED;
	}
	if (journal->state == TRADE_JOURNAL_COMMITTED) return install(p, journal);
	if (journal->state == TRADE_JOURNAL_APPLIED)
		return matches(p->current, journal->staged) ? TRADE_JOURNAL_APPLIED : TRADE_JOURNAL_BROKEN;
	if (!matches(p->current, journal->source) || !matches(p->stage, journal->staged) || trade_files_kind(p->previous))
		return TRADE_JOURNAL_BROKEN;
	/* A process can see a renamed PREPARED journal whose final directory
	 * flush failed. Both abort and commit need durable checkpoint files;
	 * visible bytes alone are not a durability promise. */
	if (!certified_matches(p->current, journal->source) || !certified_matches(p->stage, journal->staged) || !journal_write(p, journal))
		return TRADE_JOURNAL_BROKEN;
	return TRADE_JOURNAL_PREPARED;
}

OnlineTradeJournalState online_trade_journal_recover(const char *root, OnlineTradeJournal *out) {
	if (out) memset(out, 0, sizeof *out);
	if (!trade_files_supported()) return TRADE_JOURNAL_UNSUPPORTED;
	Paths p;
	if (!paths(&p, root)) return TRADE_JOURNAL_BROKEN;
	int fd = trade_files_lock(root);
	if (fd < 0) return TRADE_JOURNAL_BROKEN;
	OnlineTradeJournal journal;
	OnlineTradeJournalState state = recover(&p, &journal);
	journal.state = state;
	if (out) *out = journal;
	trade_files_unlock(fd);
	return state;
}

static bool replacements_valid(const OnlineTradeReplacement *replacement, size_t count) {
	if (!replacement || count < 1 || count > 2) return false;
	unsigned found = 0;
	for (size_t i = 0; i < count; ++i) {
		if (!replacement[i].name || !replacement[i].bytes || !replacement[i].size || replacement[i].size > TRADE_FILE_LIMIT) return false;
		unsigned which = !strcmp(replacement[i].name, "run.state") ? 1u : !strcmp(replacement[i].name, "profile.sav") ? 2u : 0u;
		if (!which || (found & which)) return false;
		found |= which;
	}
	return (found & 1u) != 0;
}

static bool source_valid(const Paths *p, const OnlineTradeSide *side) {
	static const char *const required[] = { "profile.sav", "run.sav", "run.state", "run.trade" };
	char path[TRADE_PATH_MAX];
	for (unsigned i = 0; i < sizeof required / sizeof *required; ++i)
		if (!trade_files_path(path, p->current, required[i]) || trade_files_kind(path) != 1) return false;
	if (!trade_files_path(path, p->current, "run.trade")) return false;
	size_t n;
	uint8_t *record = trade_files_read(path, &n), id[16];
	uint32_t revision;
	bool ok = online_trade_record_read(record, n, id, &revision) && revision == side->revision && !memcmp(id, side->run_id, 16);
	free(record);
	return ok;
}

static bool write_stage(const Paths *p, OnlineTradeJournal *journal, const OnlineTradeReplacement *replacement, size_t count) {
	if (!trade_files_mkdir(p->stage) || !trade_files_copy(p->current, p->stage) || !matches(p->stage, journal->source)) return false;
	char path[TRADE_PATH_MAX];
	for (size_t i = 0; i < count; ++i) {
		if (!trade_files_path(path, p->stage, replacement[i].name) || trade_files_kind(path) != 1 ||
			!trade_files_write(path, replacement[i].bytes, replacement[i].size)) return false;
	}
	/* The transfer summary is derived from the bytes being replaced.
	 * Export recreates it; preserving the stale summary would lie. */
	if (!trade_files_path(path, p->stage, "transfer.manifest") || !trade_files_remove(path) ||
		!trade_files_hash(p->stage, true, journal->post_content) || !trade_files_path(path, p->stage, "run.trade")) return false;
	size_t old_n, new_n;
	uint8_t *old = trade_files_read(path, &old_n);
	uint8_t *record = online_trade_record_append(old, old_n, &journal->terms, journal->side, journal->post_content, &new_n);
	free(old);
	bool ok = record && trade_files_write(path, record, new_n);
	free(record);
	return ok && trade_files_sync(p->stage) && trade_files_hash(p->stage, false, journal->staged) &&
		matches(p->current, journal->source) && trade_files_sync(p->root);
}

bool online_trade_journal_prepare(const char *root, const OnlineTradeTerms *terms, unsigned side,
	const OnlineTradeReplacement *replacement, size_t count) {
	if (!trade_files_supported() || side > 1 || !online_trade_terms_valid(terms) || !replacements_valid(replacement, count)) return false;
	Paths p;
	if (!paths(&p, root)) return false;
	int fd = trade_files_lock(root);
	if (fd < 0) return false;
	OnlineTradeJournal journal;
	bool ok = recover(&p, &journal) == TRADE_JOURNAL_CLEAR;
	journal.terms = *terms; journal.side = side; journal.state = TRADE_JOURNAL_PREPARED;
	memcpy(journal.source, terms->side[side].checkpoint, 32);
	if (ok) ok = source_valid(&p, &terms->side[side]) && certified_matches(p.current, journal.source) &&
		write_stage(&p, &journal, replacement, count) && journal_write(&p, &journal);
	trade_files_unlock(fd);
	return ok;
}

OnlineTradeJournalState online_trade_journal_decide(const char *root, const OnlineTradeTerms *terms, bool committed) {
	if (!trade_files_supported()) return TRADE_JOURNAL_UNSUPPORTED;
	Paths p;
	if (!paths(&p, root)) return TRADE_JOURNAL_BROKEN;
	int fd = trade_files_lock(root);
	if (fd < 0) return TRADE_JOURNAL_BROKEN;
	OnlineTradeJournal journal;
	OnlineTradeJournalState result = TRADE_JOURNAL_BROKEN;
	if (journal_read(&p, &journal) && same_terms(&journal.terms, terms)) {
		if (journal.state == TRADE_JOURNAL_PREPARED) {
			journal.state = committed ? TRADE_JOURNAL_COMMITTED : TRADE_JOURNAL_ABORTED;
			if (journal_write(&p, &journal)) result = recover(&p, &journal);
		} else if (committed == (journal.state != TRADE_JOURNAL_ABORTED)) result = recover(&p, &journal);
	}
	trade_files_unlock(fd);
	return result;
}

bool online_trade_journal_acknowledged(const char *root, const OnlineTradeTerms *terms) {
	if (!trade_files_supported()) return false;
	Paths p;
	if (!paths(&p, root)) return false;
	int fd = trade_files_lock(root);
	if (fd < 0) return false;
	OnlineTradeJournal journal;
	bool ok;
	if (!trade_files_kind(p.pending)) {
		char path[TRADE_PATH_MAX];
		size_t n = 0;
		uint8_t *record = trade_files_path(path, p.current, "run.trade") ? trade_files_read(path, &n) : NULL;
		ok = !trade_files_kind(p.previous) && !trade_files_kind(p.stage) && online_trade_record_has(record, n, terms);
		free(record);
	} else ok = journal_read(&p, &journal) && journal.state == TRADE_JOURNAL_APPLIED &&
		same_terms(&journal.terms, terms) && matches(p.current, journal.staged) && finish_cleanup(&p);
	trade_files_unlock(fd);
	return ok;
}
