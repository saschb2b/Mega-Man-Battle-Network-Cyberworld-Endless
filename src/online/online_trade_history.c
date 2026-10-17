#include "online_trade_history.h"

#include <stdlib.h>
#include <string.h>

#include "online_trade_files.h"
#include "online_trade_history_internal.h"
#include "sha256.h"

#define HISTORY_HEAD 28u
#define HISTORY_ENTRY 57u
#define HISTORY_MAX 4096u
#define ENTRY_SEED 16u
#define ENTRY_REVISION 20u
#define ENTRY_HASH 24u
#define ENTRY_LEGACY 56u
static const uint8_t magic[8] = { 'C', 'W', 'T', 'H', '0', '1', '\r', '\n' };
typedef struct { uint8_t *bytes; size_t size; uint32_t count; } History;

static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static void put32(uint8_t *p, uint32_t n) { for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(n >> (24 - 8 * i)); }
static bool nonzero(const uint8_t *p, size_t n) {
	uint8_t any = 0;
	for (size_t i = 0; i < n; ++i) any |= p[i];
	return any != 0;
}

static uint8_t *entry(const History *h, uint32_t index) { return h->bytes + HISTORY_HEAD + index * HISTORY_ENTRY; }

static void legacy_id(const uint8_t namespace_id[16], uint32_t seed, uint8_t out[16]) {
	static const char domain[] = "Cyberworld legacy run 1";
	uint8_t encoded[4], hash[32];
	put32(encoded, seed);
	CwSha256 digest;
	cw_sha256_init(&digest);
	cw_sha256_update(&digest, domain, sizeof domain);
	cw_sha256_update(&digest, namespace_id, 16);
	cw_sha256_update(&digest, encoded, sizeof encoded);
	cw_sha256_final(&digest, hash);
	memcpy(out, hash, 16);
}

static bool valid(const History *h) {
	if (h->size < HISTORY_HEAD + 32 || h->count > HISTORY_MAX || h->size != HISTORY_HEAD + h->count * HISTORY_ENTRY + 32 ||
		memcmp(h->bytes, magic, 8) || !nonzero(h->bytes + 8, 16)) return false;
	uint8_t hash[32];
	cw_sha256(h->bytes, h->size - 32, hash);
	if (memcmp(hash, h->bytes + h->size - 32, 32)) return false;
	for (uint32_t i = 0; i < h->count; ++i) {
		const uint8_t *e = entry(h, i);
		if (!nonzero(e, 16) || !nonzero(e + ENTRY_HASH, 32) || e[ENTRY_LEGACY] > 1 ||
			get32(e + ENTRY_REVISION) > (ONLINE_TRADE_RECORD_LIMIT - ONLINE_TRADE_RECORD_INITIAL) / (ONLINE_TRADE_TERMS_BYTES + 33u) ||
			(i && memcmp(entry(h, i - 1), e, 16) >= 0)) return false;
		if (e[ENTRY_LEGACY]) {
			legacy_id(h->bytes + 8, get32(e + ENTRY_SEED), hash);
			if (memcmp(e, hash, 16)) return false;
		}
	}
	return true;
}

static OnlineTradeHistoryResult read_history(const char *root, History *h) {
	memset(h, 0, sizeof *h);
	char path[TRADE_PATH_MAX];
	if (!trade_files_path(path, root, "trade.history")) return TRADE_HISTORY_BROKEN;
	int kind = trade_files_kind(path);
	if (!kind) return TRADE_HISTORY_MISSING;
	if (kind != 1) return TRADE_HISTORY_BROKEN;
	h->bytes = trade_files_read(path, &h->size);
	if (!h->bytes) return TRADE_HISTORY_IO;
	h->count = h->size >= HISTORY_HEAD ? get32(h->bytes + 24) : 0;
	return valid(h) ? TRADE_HISTORY_OK : TRADE_HISTORY_BROKEN;
}

static bool write_history(const char *root, History *h) {
	char path[TRADE_PATH_MAX];
	put32(h->bytes + 24, h->count);
	cw_sha256(h->bytes, h->size - 32, h->bytes + h->size - 32);
	return trade_files_path(path, root, "trade.history") && trade_files_write(path, h->bytes, h->size) && trade_files_sync(root);
}

bool online_trade_history_create(const char *root, const uint8_t namespace_id[16]) {
	if (!trade_files_supported() || !namespace_id || !nonzero(namespace_id, 16)) return false;
	int lock = trade_files_lock(root);
	if (lock < 0) return false;
	History h;
	OnlineTradeHistoryResult result = read_history(root, &h);
	bool ok = result == TRADE_HISTORY_OK && !memcmp(h.bytes + 8, namespace_id, 16);
	if (result == TRADE_HISTORY_MISSING) {
		char temporary[TRADE_PATH_MAX];
		ok = trade_files_path(temporary, root, "trade.history.tmp") && trade_files_kind(temporary) == 0;
		h.size = HISTORY_HEAD + 32;
		h.bytes = ok ? calloc(1, h.size) : NULL;
		ok = h.bytes != NULL;
		if (ok) { memcpy(h.bytes, magic, 8); memcpy(h.bytes + 8, namespace_id, 16); }
	}
	if (ok) ok = write_history(root, &h);
	free(h.bytes);
	trade_files_unlock(lock);
	return ok;
}

OnlineTradeHistoryResult online_trade_history_namespace_locked(const char *root, uint8_t out[16]) {
	History h;
	OnlineTradeHistoryResult result = read_history(root, &h);
	if (result == TRADE_HISTORY_OK && out) memcpy(out, h.bytes + 8, 16);
	free(h.bytes);
	return result;
}

OnlineTradeHistoryResult online_trade_history_namespace(const char *root, uint8_t out[16]) {
	if (!trade_files_supported()) return TRADE_HISTORY_UNSUPPORTED;
	int lock = trade_files_lock(root);
	if (lock < 0) return TRADE_HISTORY_IO;
	OnlineTradeHistoryResult result = online_trade_history_namespace_locked(root, out);
	trade_files_unlock(lock);
	return result;
}

static uint32_t find_entry(const History *h, const uint8_t id[16]) {
	uint32_t i = 0;
	while (i < h->count && memcmp(entry(h, i), id, 16) < 0) ++i;
	return i;
}

static OnlineTradeHistoryResult check_record(const History *h, uint32_t seed, const uint8_t *record, size_t n) {
	uint8_t id[16], prefix[32];
	uint32_t revision;
	if (!online_trade_record_read(record, n, id, &revision)) return TRADE_HISTORY_BROKEN;
	uint32_t i = find_entry(h, id);
	if (i == h->count || memcmp(entry(h, i), id, 16)) return TRADE_HISTORY_OK;
	const uint8_t *e = entry(h, i);
	uint32_t known = get32(e + ENTRY_REVISION);
	if (get32(e + ENTRY_SEED) != seed || revision < known ||
		!online_trade_record_history(record, n, known, prefix) || memcmp(prefix, e + ENTRY_HASH, 32)) return TRADE_HISTORY_STALE;
	return TRADE_HISTORY_OK;
}

static bool remember(History *h, uint32_t seed, const uint8_t *record, size_t n, bool legacy) {
	uint8_t id[16], hash[32];
	uint32_t revision;
	if (!online_trade_record_read(record, n, id, &revision) || !online_trade_record_history(record, n, revision, hash)) return false;
	uint32_t i = find_entry(h, id);
	if (i == h->count || memcmp(entry(h, i), id, 16)) {
		if (h->count == HISTORY_MAX) return false;
		uint8_t *grown = realloc(h->bytes, h->size + HISTORY_ENTRY);
		if (!grown) return false;
		h->bytes = grown;
		memmove(entry(h, i + 1), entry(h, i), h->size - HISTORY_HEAD - i * HISTORY_ENTRY);
		memset(entry(h, i), 0, HISTORY_ENTRY);
		h->size += HISTORY_ENTRY; ++h->count;
	}
	uint8_t *e = entry(h, i);
	memcpy(e, id, 16); put32(e + ENTRY_SEED, seed); put32(e + ENTRY_REVISION, revision);
	memcpy(e + ENTRY_HASH, hash, 32);
	if (legacy) e[ENTRY_LEGACY] = 1;
	return true;
}

static OnlineTradeHistoryResult record_locked(const char *root, uint32_t seed, const uint8_t *record, size_t n, bool write) {
	History h;
	OnlineTradeHistoryResult result = read_history(root, &h);
	if (result == TRADE_HISTORY_OK) result = check_record(&h, seed, record, n);
	if (result == TRADE_HISTORY_OK && write && (!remember(&h, seed, record, n, false) || !write_history(root, &h))) result = TRADE_HISTORY_IO;
	free(h.bytes);
	return result;
}

OnlineTradeHistoryResult online_trade_history_remember_locked(const char *root, uint32_t seed, const uint8_t *record, size_t n) {
	return record_locked(root, seed, record, n, true);
}

static OnlineTradeHistoryResult record_operation(const char *root, uint32_t seed, const uint8_t *record, size_t n, bool write) {
	if (!trade_files_supported()) return TRADE_HISTORY_UNSUPPORTED;
	int lock = trade_files_lock(root);
	if (lock < 0) return TRADE_HISTORY_IO;
	OnlineTradeHistoryResult result = record_locked(root, seed, record, n, write);
	trade_files_unlock(lock);
	return result;
}

OnlineTradeHistoryResult online_trade_history_check(const char *root, uint32_t seed, const uint8_t *record, size_t n) {
	return record_operation(root, seed, record, n, false);
}
OnlineTradeHistoryResult online_trade_history_remember(const char *root, uint32_t seed, const uint8_t *record, size_t n) {
	return record_operation(root, seed, record, n, true);
}

static OnlineTradeHistoryResult migrate(History *h, uint32_t seed, uint8_t initial[ONLINE_TRADE_RECORD_INITIAL]) {
	uint8_t id[16];
	legacy_id(h->bytes + 8, seed, id);
	uint32_t found = find_entry(h, id);
	if (found < h->count && !memcmp(entry(h, found), id, 16)) {
		const uint8_t *known = entry(h, found);
		if (!known[ENTRY_LEGACY] || get32(known + ENTRY_SEED) != seed || get32(known + ENTRY_REVISION)) return TRADE_HISTORY_STALE;
	}
	for (uint32_t i = 0; i < h->count; ++i) {
		const uint8_t *e = entry(h, i);
		if (get32(e + ENTRY_SEED) == seed && (!e[ENTRY_LEGACY] || get32(e + ENTRY_REVISION))) return TRADE_HISTORY_STALE;
	}
	if (!online_trade_record_new(id, initial) || !remember(h, seed, initial, ONLINE_TRADE_RECORD_INITIAL, true)) return TRADE_HISTORY_IO;
	return TRADE_HISTORY_OK;
}

OnlineTradeHistoryResult online_trade_history_legacy_locked(const char *root, uint32_t seed, uint8_t initial[ONLINE_TRADE_RECORD_INITIAL]) {
	if (!initial) return TRADE_HISTORY_BROKEN;
	memset(initial, 0, ONLINE_TRADE_RECORD_INITIAL);
	History h;
	OnlineTradeHistoryResult result = read_history(root, &h);
	if (result == TRADE_HISTORY_OK) result = migrate(&h, seed, initial);
	if (result == TRADE_HISTORY_OK && !write_history(root, &h)) result = TRADE_HISTORY_IO;
	if (result != TRADE_HISTORY_OK) memset(initial, 0, ONLINE_TRADE_RECORD_INITIAL);
	free(h.bytes);
	return result;
}

OnlineTradeHistoryResult online_trade_history_legacy(const char *root, uint32_t seed, uint8_t initial[ONLINE_TRADE_RECORD_INITIAL]) {
	if (!trade_files_supported()) return TRADE_HISTORY_UNSUPPORTED;
	int lock = trade_files_lock(root);
	if (lock < 0) return TRADE_HISTORY_IO;
	OnlineTradeHistoryResult result = online_trade_history_legacy_locked(root, seed, initial);
	trade_files_unlock(lock);
	return result;
}
