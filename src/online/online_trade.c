#include "online_trade.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "chips.h"
#include "compat.h"
#include "emu.h"
#include "game.h"
#include "gamecall.h"
#include "online_transport.h"
#include "trade_words.h"

#define FOLDERS 3        /* Folder1, Folder2 and the ExtraFolder: 30 entries each, 0x3C apart */
#define PENDING "trade.pending"

static TradeView T;

static uint16_t get16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }
static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static void put16(uint8_t *p, unsigned v) { p[0] = (uint8_t)(v >> 8); p[1] = (uint8_t)v; }
static void put32(uint8_t *p, uint32_t v) { put16(p, v >> 16); put16(p + 2, v); }

void online_trade_reset(void) { memset(&T, 0, sizeof T); }

const TradeView *online_trade_view(void) { return &T; }

static bool send(int action, uint32_t ref, TradeChip give, TradeChip get);

/* a chip only a game with BN5's ROM holds: past BN6's 319, or on one of
 * its blank records, where BN5's are installed (the server's rule too,
 * protocol.json's bn6_blank) */
static bool online_trade_bn5(unsigned id) { return id > ONLINE_BN6_CHIP_LAST || BN6_CHIP_BLANK(id); }

/* a chip this game holds: one of BN6's own, or one of BN5's (on BN6's blank
 * records and past its 319) the registry has; in a code of its record's,
 * where the core has them */
static bool chip_ok(TradeChip c) {
	if (c.id < 1 || c.id > ONLINE_CHIP_LAST || c.code > ONLINE_CODE_LAST) return false;
	if (online_trade_bn5(c.id) && !chips_bn5(c.id)) return false;
	const uint8_t *rec = chips_record(c.id);
	for (unsigned s = 0; rec && s < 4; ++s)
		if (rec[BN6_CHIP_CODES + s] == c.code) return true;
	return !rec;
}

bool online_trade_holds(TradeChip c) { return chip_ok(c); }

bool online_trade_mail(const uint8_t *data, size_t size) {
	if (size != ONLINE_TRADE_STATE_BYTES || data[0] != ONLINE_TRADE_STATE || data[1] < TRADE_OFFERED || data[1] > TRADE_CANCELLED) return false;
	TradeView v = { .stage = data[1], .id = get32(data + 2), .other = get32(data + 6) };
	memcpy(v.identity, data + 10, ONLINE_IDENTITY_BYTES);
	v.mine = (TradeChip){ get16(data + 26), data[28] };
	v.theirs = (TradeChip){ get16(data + 29), data[31] };
	if (v.mine.id > ONLINE_CHIP_LAST || v.theirs.id > ONLINE_CHIP_LAST) return false;
	/* (a chip this game cannot hold, one the registry has not: the trade
	 * called off, never kept) */
	if ((v.mine.id && !chip_ok(v.mine)) || (v.theirs.id && !chip_ok(v.theirs))) {
		fprintf(stderr, "online: trade %u names a chip this game cannot hold (%u %u for %u %u): called off\n", v.id,
			v.mine.id, v.mine.code, v.theirs.id, v.theirs.code);
		if (v.stage != TRADE_DONE && v.stage != TRADE_CANCELLED) send(4, v.id, (TradeChip){ 0, 0 }, (TradeChip){ 0, 0 });
		return true;
	}
	/* (a trade first heard of answered began with the board's offer taken) */
	v.taken = v.id == T.id ? T.taken : v.stage == TRADE_ANSWERED;
	v.serial = T.serial + 1;
	T = v;
	return true;
}

static bool send(int action, uint32_t ref, TradeChip give, TradeChip get) {
	uint8_t m[ONLINE_TRADE_BYTES] = { ONLINE_TRADE, (uint8_t)action };
	put32(m + 2, ref);
	put16(m + 6, give.id); m[8] = give.code;
	put16(m + 9, get.id); m[11] = get.code;
	return ref && online_transport_send(m, sizeof m);
}

bool online_trade_propose(uint32_t other, TradeChip give) { return chip_ok(give) && send(1, other, give, (TradeChip){ 0, 0 }); }

bool online_trade_take(uint32_t poster, TradeChip give, TradeChip get) {
	return chip_ok(give) && chip_ok(get) && send(5, poster, give, get);
}

bool online_trade_answer(TradeChip give) {
	return T.stage == TRADE_OFFERED && chip_ok(give) && send(2, T.id, give, T.theirs);
}

bool online_trade_accept(void) { return T.stage == TRADE_ANSWERED && send(3, T.id, T.mine, T.theirs); }

bool online_trade_cancel(void) {
	bool open = T.stage && T.stage != TRADE_DONE && T.stage != TRADE_CANCELLED;
	return open && send(4, T.id, (TradeChip){ 0, 0 }, (TradeChip){ 0, 0 });
}

/* ---- the Pack ---- */

/* (ids from BN6_PACK_IDS in the pack's extension, where a game with BN5's
 * ROM counts its chips: docs/EMULATION.md) */
static uint32_t pack_entry(unsigned id) {
	uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	if (pack < BN6_EWRAM || pack > BN6_EWRAM_END - BN6_PACK_IDS * BN6_PACK_ENTRY || id > ONLINE_CHIP_LAST) return 0;
	return id < BN6_PACK_IDS ? pack + id * BN6_PACK_ENTRY : BN6_PACK_EXT + (id - BN6_PACK_IDS) * BN6_PACK_ENTRY;
}

/* how many copies of (id, code) MegaMan's three folders hold */
static int in_folders(TradeChip c) {
	uint32_t folders = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	if (folders < BN6_EWRAM || folders > BN6_EWRAM_END - FOLDERS * 0x3C) return 99;
	int n = 0;
	for (unsigned i = 0; i < FOLDERS * BN6_FOLDER_ENTRIES; ++i) {
		uint16_t e = emu_read16(folders + 2 * i);
		n += (e & 0x1FF) == c.id && e >> 9 == c.code;
	}
	return n;
}

int online_trade_spares(TradeChip *out, uint8_t *count, int cap) {
	int n = 0;
	for (unsigned id = 1; id <= ONLINE_CHIP_LAST && n < cap; ++id) {
		const uint8_t *rec = chips_record((int)id);
		uint32_t at = pack_entry(id);
		if (!rec || !at || (online_trade_bn5(id) && !chips_bn5((int)id))) continue;
		for (unsigned s = 0; s < 4 && n < cap; ++s) {
			uint8_t code = rec[BN6_CHIP_CODES + s], have = emu_read8(at + s);
			if (code > ONLINE_CODE_LAST || !have) continue;
			TradeChip c = { (uint16_t)id, code };
			int spare = have - in_folders(c);
			if (spare <= 0 || !chip_ok(c)) continue;
			out[n] = c;
			count[n++] = (uint8_t)(spare > 99 ? 99 : spare);
		}
	}
	return n;
}

/* ---- the committed swap, on disk until the run has it ---- */

static void pending_path(char *out, size_t n) { snprintf(out, n, "%s/%s", g_data_dir, PENDING); }

bool online_trade_keep(const TradeView *done) {
	if (!chip_ok(done->mine) || !chip_ok(done->theirs)) return false;
	char path[1100], tmp[1110];
	pending_path(path, sizeof path);
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *f = fopen(tmp, "w");
	if (!f) return false;
	fprintf(f, "# A Net Square trade the server committed; the run takes it at the next CONTINUE.\n"
		"trade=%u\ngive=%u %u\nget=%u %u\n", done->id, done->mine.id, done->mine.code, done->theirs.id, done->theirs.code);
	bool ok = !ferror(f) && cw_fsync_file(f);
	if (fclose(f)) ok = false;
	if (ok) ok = cw_rename(tmp, path);
	if (!ok) remove(tmp);
	else cw_fsync_dir(g_data_dir);
	return ok;
}

bool online_trade_pending(TradeChip *give, TradeChip *get) {
	char path[1100], line[128];
	pending_path(path, sizeof path);
	FILE *f = fopen(path, "r");
	if (!f) return false;
	unsigned gi = 0, gc = 99, ti = 0, tc = 99;
	while (fgets(line, sizeof line, f)) {
		if (sscanf(line, "give=%u %u", &gi, &gc) == 2) continue;
		(void)sscanf(line, "get=%u %u", &ti, &tc);
	}
	fclose(f);
	*give = (TradeChip){ (uint16_t)gi, (uint8_t)gc };
	*get = (TradeChip){ (uint16_t)ti, (uint8_t)tc };
	return gi <= 0xFFFF && ti <= 0xFFFF && chip_ok(*give) && chip_ok(*get);
}

/* the pack's count of (id, code), -1 where the chip has no such code */
static int pack_count(TradeChip c) {
	const uint8_t *rec = chips_record(c.id);
	uint32_t at = pack_entry(c.id);
	for (unsigned s = 0; rec && at && s < 4; ++s)
		if (rec[BN6_CHIP_CODES + s] == c.code) return emu_read8(at + s);
	return -1;
}

static void chip_words(TradeChip c, char *out, size_t n) {
	char name[24];
	chips_name(c.id, name, sizeof name);
	snprintf(out, n, "%s %c", name, c.code == 26 ? '*' : 'A' + c.code);
}

const char *online_trade_settle(void) {
	TradeChip give, get;
	if (!online_trade_pending(&give, &get)) return NULL;
	int had = pack_count(give);
	/* (the checkpoint before the visit always holds the spare it offered,
	 * a copy past its folders'; where it does not, the run keeps its own
	 * and takes none, never a folder's copy) */
	if (had - in_folders(give) <= 0 || pack_count(get) < 0) {
		fprintf(stderr, "online: trade %u %u for %u %u not settled: the Pack holds %d\n", give.id, give.code, get.id, get.code, had);
		online_trade_settled();
		return NULL;
	}
	int got_had = pack_count(get);
	game_call_ret(BN6_TAKE_CHIPS, give.id, give.code, 1, NULL);
	game_call_ret(BN6_GIVE_CHIPS, get.id, get.code, 1, NULL);
	/* (ids from 320 count in the pack's extension: BN6's routines patched to, docs/EMULATION.md) */
	fprintf(stderr, "online: trade settled: %u %u given, %u %u got (the Pack's %d to %d of the one, %d to %d of the other)\n",
		give.id, give.code, get.id, get.code, had, pack_count(give), got_had, pack_count(get));
	char gave[32], got[32];
	chip_words(give, gave, sizeof gave);
	chip_words(get, got, sizeof got);
	return trade_settled(gave, got);
}

void online_trade_settled(void) {
	char path[1100];
	pending_path(path, sizeof path);
	remove(path);
}
