/* Local native-editor oracle. No ROM bytes, states or dumps belong in git. */
#include "online_trade_capture.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba/core/serialize.h>
#include <mgba-util/vfs.h>

#include "chip_pool.h"
#include "chips.h"
#include "emu.h"
#include "rom.h"
#include "run.h"

Run run; /* Metadata readers see the plain, helper-free native control. */

#define PROBE_IWRAM 0x03000000u
#define PROBE_IWRAM_BYTES 0x8000u
#define PROBE_CODES 27
#define PROBE_SAMPLES 100

static struct mCore *core;
static FILE *trace;
static unsigned frame, samples[PROBE_SAMPLES], nsamples;
static const char *directory;

uint8_t emu_read8(uint32_t address) { return core->rawRead8(core, address, -1); }
uint16_t emu_read16(uint32_t address) { return core->rawRead16(core, address, -1); }
uint32_t emu_read32(uint32_t address) { return core->rawRead32(core, address, -1); }
void emu_write8(uint32_t address, uint8_t value) { core->rawWrite8(core, address, -1, value); }
static void write16(uint32_t address, uint16_t value) { core->rawWrite16(core, address, -1, value); }

static FILE *output(const char *name) {
	char path[2048];
	if (snprintf(path, sizeof path, "%s/%s", directory, name) >= (int)sizeof path) return NULL;
	return fopen(path, "wb");
}

static void save_bytes(const char *name, const void *data, size_t length) {
	FILE *f = output(name);
	if (!f || fwrite(data, 1, length, f) != length || fclose(f)) exit(2);
}

static void sample(void) {
	if (nsamples == PROBE_SAMPLES) exit(2);
	samples[nsamples++] = frame;
	for (unsigned i = 0; i < BN6_EWRAM_END - BN6_EWRAM; ++i) fputc(emu_read8(BN6_EWRAM + i), trace);
	for (unsigned i = 0; i < PROBE_IWRAM_BYTES; ++i) fputc(emu_read8(PROBE_IWRAM + i), trace);
}

static void advance(unsigned frames, unsigned keys) {
	core->setKeys(core, keys);
	for (unsigned i = 0; i < frames; ++i) { core->runFrame(core); ++frame; }
	sample();
}

static void tap(unsigned key) { advance(2, key); advance(8, 0); }

static const uint8_t *native_record(unsigned id) {
	return R.data + R.layout->chip_data + id * BN6_CHIP_RECORD_SIZE;
}

static unsigned slot_of(uint16_t entry) {
	unsigned id = entry & (ONLINE_PACK_IDS - 1u);
	if (!id || id >= BN6_PACK_IDS) { fprintf(stderr, "fixture: non-native folder ID %u\n", id); exit(2); }
	const uint8_t *record = native_record(id);
	for (unsigned i = 0; i < ONLINE_TRADE_CODES; ++i) if (record[BN6_CHIP_CODES + i] == (entry >> 9)) return i;
	fprintf(stderr, "fixture: invalid native folder entry %u\n", entry); exit(2);
}

/* Copy the player's native first folder, substituting two native records
 * to distinguish the other slots. Build totals only to SET UP a fixture;
 * the oracle later reads the original editor's own resulting counts. */
static void fixture(unsigned folders, bool boundary) {
	uint32_t at = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	uint32_t context = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_STEPS);
	uint8_t totals[BN6_PACK_IDS][BN6_PACK_ENTRY] = {{0}};
	uint16_t first[BN6_FOLDER_ENTRIES];
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) first[i] = emu_read16(at + 2 * i);
	for (unsigned f = 0; f < folders; ++f) {
		for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
			uint16_t entry = first[i];
			if (f && (entry & 511u) == 1 && !(entry >> 9))
				entry = (uint16_t)((f + 1) | native_record(f + 1)[BN6_CHIP_CODES] << 9);
			write16(at + 2 * (f * BN6_FOLDER_ENTRIES + i), entry);
			++totals[entry & 511u][slot_of(entry)];
		}
		emu_write8(BN6_FOLDER_KINDS + f, (uint8_t)(f << 4));
		for (unsigned i = 0; i < BN6_FOLDER_NAME_BYTES; ++i)
			emu_write8(BN6_FOLDER_NAMES + f * BN6_FOLDER_NAME_BYTES + i, emu_read8(BN6_FOLDER_NAMES + i));
	}
	++totals[1][0]; totals[1][1] += 2; totals[2][0] += 3;
	if (boundary) totals[1][0] = ONLINE_TRADE_COUNT_MAX;
	for (unsigned id = 0; id < BN6_PACK_IDS; ++id)
		for (unsigned i = 0; i < BN6_PACK_ENTRY; ++i) emu_write8(pack + id * BN6_PACK_ENTRY + i, totals[id][i]);
	emu_write8(context + BN6_FOLDER_COUNT_FIELD, (uint8_t)folders);
	for (unsigned id = 1; id <= BN6_FOLDERS_MAX; ++id) {
		uint32_t marks = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIP_MARKS);
		emu_write8(marks + id, emu_read8(BN6_CHIP_KEYS + id) ^ BN6_CHIP_KEY_XOR);
	}
}

static void equip(unsigned active) {
	if (!active) return;
	tap(KEY_START); advance(40, 0); tap(KEY_A); advance(50, 0);
	for (unsigned i = 0; i < active; ++i) tap(KEY_RIGHT);
	tap(KEY_A); advance(30, 0);
	if (active < 2) tap(KEY_DOWN); /* Editable folders: EDIT before EQUIP. */
	tap(KEY_A); advance(30, 0); tap(KEY_B); advance(40, 0); tap(KEY_B); advance(40, 0);
}

static void editor(unsigned active) {
	tap(KEY_START); advance(40, 0); tap(KEY_A); advance(50, 0);
	for (unsigned i = 0; i < active; ++i) tap(KEY_LEFT);
	tap(KEY_A); advance(30, 0); tap(KEY_A); advance(60, 0);
}

static void counts(const char *name, const OnlineTradePrepared *prepared) {
	uint8_t values[ONLINE_PACK_IDS * PROBE_CODES] = {0};
	if (prepared) {
		OnlineTradeInventory view = online_trade_inventory_view(prepared);
		for (unsigned id = 1; id < ONLINE_PACK_IDS; ++id)
			for (unsigned code = 0; code < PROBE_CODES; ++code)
				values[id * PROBE_CODES + code] = (uint8_t)online_trade_spares(&view, (uint16_t)(id | code << 9));
	} else {
		uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
		for (unsigned id = 1; id < BN6_PACK_IDS; ++id)
			for (unsigned i = 0; i < ONLINE_TRADE_CODES; ++i) {
				unsigned code = native_record(id)[BN6_CHIP_CODES + i];
				if (code < PROBE_CODES) values[id * PROBE_CODES + code] = emu_read8(pack + id * BN6_PACK_ENTRY + i);
			}
	}
	save_bytes(name, values, sizeof values);
}

static bool lifetime(OnlineTradePrepared *prepared, bool live_mutation) {
	uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	uint32_t at = pack + BN6_PACK_ENTRY;
	uint8_t previous = emu_read8(at);
	save_bytes("prepared-before.bin", prepared, sizeof *prepared);
	emu_write8(at, previous ? (uint8_t)(previous - 1) : 1);
	bool changed = emu_read8(at) != previous;
	/* A deliberately broken live-backed capture, for the lifetime guard. */
	if (live_mutation) (void)online_trade_capture(prepared, true, 0, false);
	counts("retained-counts.bin", prepared);
	save_bytes("prepared-after.bin", prepared, sizeof *prepared);
	emu_write8(at, previous);
	return changed;
}

static void nolog(struct mLogger *logger, int category, enum mLogLevel level, const char *format, va_list args) {
	(void)logger; (void)category; (void)level; (void)format; (void)args;
}

static bool start(const char *rom, const char *state) {
	static struct mLogger quiet = { .log = nolog };
	char message[512];
	if (!rom_load_file(rom, message, sizeof message)) { fprintf(stderr, "%s\n", message); return false; }
	mLogSetDefaultLogger(&quiet);
	core = mCoreFind(rom);
	if (!core || !core->init(core)) return false;
	mCoreInitConfig(core, NULL);
	static color_t video[EMU_W * EMU_H];
	core->setVideoBuffer(core, video, EMU_W);
	if (!mCoreLoadFile(core, rom)) return false;
	core->reset(core);
	struct VFile *file = VFileOpen(state, O_RDONLY);
	if (!file) return false;
	bool ok = mCoreLoadStateNamed(core, file, SAVESTATE_RTC);
	file->close(file);
	return ok;
}

static unsigned number(const char *value, unsigned limit) {
	char *end;
	errno = 0;
	unsigned long parsed = strtoul(value, &end, 10);
	if (errno || !*value || *end || parsed > limit) exit(2);
	return (unsigned)parsed;
}

int main(int argc, char **argv) {
	if (argc != 9) { fprintf(stderr, "usage: %s ROM STATE OUT FOLDERS ACTIVE BOUNDARY CAPTURE LIVE_MUTATION\n", argv[0]); return 2; }
	unsigned folders = number(argv[4], BN6_FOLDERS_MAX), active = number(argv[5], BN6_FOLDERS_MAX - 1);
	bool boundary = number(argv[6], 1), capture = number(argv[7], 1), live_mutation = number(argv[8], 1);
	if (!folders || active >= folders || !start(argv[1], argv[2])) return 2;
	directory = argv[3]; trace = output("ram.bin");
	if (!trace) return 2;
	fixture(folders, boundary); advance(30, 0); equip(active);
	unsigned map_mode = emu_read8(emu_read32(BN6_TOOLKIT));
	unsigned actual_active = emu_read8(BN6_NAVI_FOLDER);
	unsigned actual_count = emu_read8(emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_STEPS) + BN6_FOLDER_COUNT_FIELD);
	OnlineTradePrepared prepared;
	bool captured = !capture || online_trade_capture(&prepared, true, 0, false), changed = false, full = false;
	if (capture && captured) {
		counts("counts.bin", &prepared);
		OnlineTradeInventory view = online_trade_inventory_view(&prepared);
		full = online_trade_check(&view, (uint16_t)(1 | native_record(1)[1] << 9), 1, false) == ONLINE_TRADE_PACK_FULL;
		changed = lifetime(&prepared, live_mutation);
	}
	advance(1, 0); editor(active);
	if (!capture) counts("counts.bin", NULL);
	uint32_t menu = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU);
	FILE *report = output("observation.json");
	if (!report || fclose(trace)) return 2;
	fprintf(report, "{\"folder_count\":%u,\"active_folder\":%u,\"map_mode\":%u,\"editor_mode\":%u,\"editor_state\":[%u,%u],\"frames\":%u,\"capture_ok\":%s,\"live_pack_changed\":%s,\"capacity_full\":%s,\"sample_frames\":[",
		actual_count, actual_active, map_mode, emu_read8(emu_read32(BN6_TOOLKIT)), emu_read8(menu), emu_read8(menu + 1), frame,
		captured ? "true" : "false", changed ? "true" : "false", full ? "true" : "false");
	for (unsigned i = 0; i < nsamples; ++i) fprintf(report, "%s%u", i ? "," : "", samples[i]);
	fprintf(report, "]}\n");
	bool ok = !fclose(report);
	core->deinit(core);
	return ok ? 0 : 2;
}
