#include "online_trade_identity.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "online_trade_files.h"
#include "online_trade_history_internal.h"
#include "run.h"
#include "save_format.h"
#include "sha256.h"

#define MARKER_BYTES 93u
static const uint8_t magic[8] = { 'C', 'W', 'T', 'N', '0', '1', '\r', '\n' };
typedef struct { bool bound; uint32_t seed; uint8_t id[16], content[32]; } Marker;
typedef struct { char current[TRADE_PATH_MAX], marker[TRADE_PATH_MAX], temporary[TRADE_PATH_MAX], receipt[TRADE_PATH_MAX], receipt_temporary[TRADE_PATH_MAX]; } Paths;

static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static void put32(uint8_t *p, uint32_t n) { for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(n >> (24 - 8 * i)); }
static bool nonzero(const uint8_t *p, size_t n) {
	uint8_t any = 0;
	for (size_t i = 0; i < n; ++i) any |= p[i];
	return any != 0;
}
static bool paths(Paths *p, const char *root) {
	return trade_files_path(p->current, root, "savedata") && trade_files_path(p->marker, root, "trade.newrun") &&
		trade_files_path(p->temporary, root, "trade.newrun.tmp") && trade_files_path(p->receipt, p->current, "run.trade") && trade_files_path(p->receipt_temporary, p->current, "run.trade.tmp");
}
static bool no_transaction(const char *root) {
	static const char *const names[] = { "trade.pending", "trade.pending.tmp", "trade.stage", "trade.previous" };
	char path[TRADE_PATH_MAX];
	for (unsigned i = 0; i < sizeof names / sizeof *names; ++i)
		if (!trade_files_path(path, root, names[i]) || trade_files_kind(path)) return false;
	return true;
}

bool online_trade_identity_seed(const char *root, uint32_t *seed) {
	Paths p;
	char path[TRADE_PATH_MAX];
	if (!seed || !paths(&p, root) || !trade_files_path(path, p.current, "run.sav")) return false;
	size_t n = 0;
	uint8_t *data = trade_files_read(path, &n);
	bool ok = data && n == 12 + sizeof(Run);
	if (ok) {
		uint32_t header[3], checksum = 2166136261u;
		memcpy(header, data, sizeof header);
		for (size_t i = 12; i < n; ++i) checksum = (checksum ^ data[i]) * 16777619u;
		ok = header[0] == RUN_MAGIC && header[1] == sizeof(Run) && header[2] == checksum && data[12 + offsetof(Run, active)] == 1;
		if (ok) memcpy(seed, data + 12 + offsetof(Run, seed), sizeof *seed);
	}
	free(data);
	return ok;
}

static bool read_marker(const Paths *p, Marker *m) {
	size_t n = 0;
	uint8_t *data = trade_files_read(p->marker, &n), hash[32];
	bool ok = data && n == MARKER_BYTES;
	if (ok) {
		cw_sha256(data, n - 32, hash);
		ok = !memcmp(data, magic, 8) && data[8] <= 1 && nonzero(data + 13, 16) &&
			!memcmp(hash, data + n - 32, 32) && (data[8] ? nonzero(data + 29, 32) : !nonzero(data + 29, 32));
	}
	if (ok) {
		m->bound = data[8] != 0; m->seed = get32(data + 9);
		memcpy(m->id, data + 13, 16); memcpy(m->content, data + 29, 32);
	}
	free(data);
	return ok;
}

static bool write_marker(const char *root, const Paths *p, const Marker *m) {
	uint8_t data[MARKER_BYTES] = { 0 };
	memcpy(data, magic, 8); data[8] = m->bound; put32(data + 9, m->seed);
	memcpy(data + 13, m->id, 16); memcpy(data + 29, m->content, 32);
	cw_sha256(data, sizeof data - 32, data + sizeof data - 32);
	return trade_files_write(p->marker, data, sizeof data) && trade_files_sync(root);
}

static bool matching_receipt(const Paths *p, const uint8_t initial[ONLINE_TRADE_RECORD_INITIAL]) {
	size_t n = 0;
	uint8_t *bytes = trade_files_read(p->receipt, &n);
	bool ok = bytes && n == ONLINE_TRADE_RECORD_INITIAL && !memcmp(bytes, initial, n);
	free(bytes);
	return ok;
}

static bool source_valid(const char *root, const Paths *p, uint32_t expected_seed) {
	char path[TRADE_PATH_MAX];
	uint32_t seed;
	return online_trade_identity_seed(root, &seed) && seed == expected_seed &&
		trade_files_path(path, p->current, "profile.sav") && trade_files_kind(path) == 1 &&
		trade_files_path(path, p->current, "run.state") && trade_files_kind(path) == 1;
}

static bool source_matches(const char *root, const Paths *p, const Marker *m) {
	uint8_t hash[32];
	return source_valid(root, p, m->seed) && trade_files_certify(p->current) &&
		trade_files_hash(p->current, true, hash) && !memcmp(hash, m->content, 32);
}

static bool finish(const char *root, const Paths *p, const Marker *m) {
	uint8_t initial[ONLINE_TRADE_RECORD_INITIAL];
	if (!online_trade_record_new(m->id, initial) || !trade_files_remove(p->receipt_temporary)) return false;
	int receipt = trade_files_kind(p->receipt);
	bool installed = receipt == 1 && matching_receipt(p, initial);
	if (receipt && !installed) return false;
	if (!source_valid(root, p, m->seed)) return false;
	if (!source_matches(root, p, m) && !(installed && trade_files_certify(p->current))) return false;
	if (online_trade_history_remember_locked(root, m->seed, initial, sizeof initial) != TRADE_HISTORY_OK) return false;
	if (!installed && !trade_files_write(p->receipt, initial, sizeof initial)) return false;
	return trade_files_sync(p->current) && matching_receipt(p, initial) && trade_files_remove(p->temporary) &&
		trade_files_sync(root) && trade_files_remove(p->marker) && trade_files_sync(root);
}

static OnlineTradeIdentityState recover(const char *root, const Paths *p) {
	int kind = trade_files_kind(p->marker);
	if (!kind) return trade_files_remove(p->temporary) && trade_files_sync(root) ? TRADE_IDENTITY_CLEAR : TRADE_IDENTITY_BLOCKED;
	Marker m;
	if (kind != 1 || !no_transaction(root) || !read_marker(p, &m)) return TRADE_IDENTITY_BLOCKED;
	if (!m.bound) return TRADE_IDENTITY_UNBOUND;
	return finish(root, p, &m) ? TRADE_IDENTITY_CLEAR : TRADE_IDENTITY_BLOCKED;
}

OnlineTradeIdentityState online_trade_identity_recover(const char *root) {
	if (!trade_files_supported()) return TRADE_IDENTITY_UNSUPPORTED;
	Paths p;
	if (!paths(&p, root)) return TRADE_IDENTITY_BLOCKED;
	int lock = trade_files_lock(root);
	if (lock < 0) return TRADE_IDENTITY_BLOCKED;
	OnlineTradeIdentityState result = recover(root, &p);
	trade_files_unlock(lock);
	return result;
}

bool online_trade_identity_intent(const char *root, uint32_t seed, const uint8_t random_id[16]) {
	Paths p;
	if (!trade_files_supported() || !random_id || !nonzero(random_id, 16) || !paths(&p, root)) return false;
	int lock = trade_files_lock(root);
	if (lock < 0) return false;
	Marker m = { .seed = seed };
	memcpy(m.id, random_id, 16);
	bool ok = no_transaction(root) && !trade_files_kind(p.marker) && !trade_files_kind(p.temporary) &&
		online_trade_history_namespace_locked(root, NULL) == TRADE_HISTORY_OK && write_marker(root, &p, &m);
	if (ok && trade_files_kind(p.current)) ok = trade_files_remove(p.receipt) && trade_files_remove(p.receipt_temporary) && trade_files_sync(p.current);
	trade_files_unlock(lock);
	return ok;
}

bool online_trade_identity_bind(const char *root, const uint8_t expected_content[32]) {
	Paths p;
	if (!trade_files_supported() || !expected_content || !paths(&p, root)) return false;
	int lock = trade_files_lock(root);
	if (lock < 0) return false;
	Marker m;
	bool ok = no_transaction(root) && read_marker(&p, &m) && !m.bound && trade_files_kind(p.receipt) == 0;
	if (ok) {
		m.bound = true; memcpy(m.content, expected_content, 32);
		ok = source_matches(root, &p, &m) && write_marker(root, &p, &m) && finish(root, &p, &m);
	}
	trade_files_unlock(lock);
	return ok;
}

bool online_trade_identity_migrate(const char *root, uint32_t seed, const uint8_t expected_content[32]) {
	Paths p;
	if (!trade_files_supported() || !expected_content || !paths(&p, root)) return false;
	int lock = trade_files_lock(root);
	if (lock < 0) return false;
	Marker m = { .bound = true, .seed = seed };
	memcpy(m.content, expected_content, 32);
	uint8_t initial[ONLINE_TRADE_RECORD_INITIAL];
	bool ok = no_transaction(root) && !trade_files_kind(p.marker) && !trade_files_kind(p.temporary) && trade_files_kind(p.receipt) == 0 &&
		source_matches(root, &p, &m) && online_trade_history_legacy_locked(root, seed, initial) == TRADE_HISTORY_OK;
	if (ok) {
		memcpy(m.id, initial + 8, 16);
		ok = write_marker(root, &p, &m) && finish(root, &p, &m);
	}
	trade_files_unlock(lock);
	return ok;
}

bool online_trade_identity_discard(const char *root) {
	static const char *const names[] = { "run.sav", "run.state", "run.trade", "battle.sav", "run.make", "run.area",
		"run.seen", "run.folder", "run.act", "run.dark", "run.souls", "run.board", "transfer.manifest" };
	Paths p;
	if (!trade_files_supported() || !paths(&p, root)) return false;
	int lock = trade_files_lock(root);
	if (lock < 0) return false;
	Marker m;
	bool ok = no_transaction(root) && read_marker(&p, &m) && !m.bound;
	for (unsigned i = 0; ok && i < sizeof names / sizeof *names; ++i) {
		char path[TRADE_PATH_MAX];
		ok = trade_files_path(path, p.current, names[i]) && trade_files_remove(path);
		if (ok) {
			char temporary[TRADE_PATH_MAX];
			int n = snprintf(temporary, sizeof temporary, "%s.tmp", path);
			ok = n > 0 && n < (int)sizeof temporary && trade_files_remove(temporary);
		}
	}
	char profile_temporary[TRADE_PATH_MAX];
	if (ok) ok = trade_files_path(profile_temporary, p.current, "profile.sav.tmp") && trade_files_remove(profile_temporary);
	if (ok) ok = (trade_files_kind(p.current) == 0 || trade_files_sync(p.current)) && trade_files_remove(p.temporary) &&
		trade_files_sync(root) && trade_files_remove(p.marker) && trade_files_sync(root);
	trade_files_unlock(lock);
	return ok;
}
