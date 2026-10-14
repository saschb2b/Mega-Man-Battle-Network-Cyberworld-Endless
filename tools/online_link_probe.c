/* Phase 0, issue #120: two mGBA cores on the pinned core's serial cable,
 * using a tiny ROM written here, and the cost of local rollback states.
 * This is a hardware/API spike, not a claim that BN6 NetBattle works.
 * Build with NDEBUG, as docker/mgba.sh builds its Release library. */
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <mgba/core/blip_buf.h>
#include <mgba/core/config.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/io.h>
#include <mgba/internal/gba/sio/lockstep.h>
#include <mgba-util/vfs.h>

#include "emu.h"
#include "online_snapshot.h"

#define PLAYERS 2
#define TRANSFERS 256
#define BENCH_COUNT 200
#define RAM_BYTES (0x40000u + 0x8000u)

typedef struct {
	struct GBASIOLockstep cable;
	struct GBASIOLockstepNode node[PLAYERS];
	struct mCore *core[PLAYERS];
	color_t video[PLAYERS][240 * 160];
	int32_t credit[PLAYERS];
	unsigned waiting;
	int running;
} Probe;

static struct mCore *snapshot_core;

uint8_t emu_read8(uint32_t address) { return (uint8_t)snapshot_core->rawRead8(snapshot_core, address, -1); }
uint16_t emu_read16(uint32_t address) { return (uint16_t)snapshot_core->rawRead16(snapshot_core, address, -1); }
uint32_t emu_read32(uint32_t address) { return snapshot_core->rawRead32(snapshot_core, address, -1); }

static void quiet_log(struct mLogger *logger, int category, enum mLogLevel level, const char *format, va_list args) {
	(void)logger; (void)category; (void)level; (void)format; (void)args;
}

static struct mLogger quiet = { .log = quiet_log };

static double millis(void) {
#ifdef _WIN32
	LARGE_INTEGER now, frequency;
	QueryPerformanceCounter(&now);
	QueryPerformanceFrequency(&frequency);
	return (double)now.QuadPart * 1000 / (double)frequency.QuadPart;
#else
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (double)now.tv_sec * 1000 + (double)now.tv_nsec / 1000000;
#endif
}

static bool cable_signal(struct mLockstep *lock, unsigned mask) {
	Probe *p = lock->context;
	if (p->running != 0) p->waiting &= ~mask;
	return true;
}

static bool cable_wait(struct mLockstep *lock, unsigned mask) {
	Probe *p = lock->context;
	p->waiting = mask;
	return true;
}

static void cable_add(struct mLockstep *lock, int id, int32_t cycles) {
	Probe *p = lock->context;
	if (id == 0) p->credit[1] += cycles;
	else p->credit[id] += cycles;
}

static int32_t cable_use(struct mLockstep *lock, int id, int32_t cycles) {
	Probe *p = lock->context;
	p->credit[id] -= cycles;
	return p->credit[id];
}

static int32_t cable_unused(struct mLockstep *lock, int id) {
	Probe *p = lock->context;
	return p->credit[id];
}

static void cable_unload(struct mLockstep *lock, int id) {
	Probe *p = lock->context;
	p->waiting &= ~(1u << (unsigned)id);
}

static struct mCore *make_core(color_t *video, const char *rom_path) {
	/* An ARM branch from the entry point to an infinite branch at 0xC0.
	 * Both instructions and the minimum identifying header are ours. */
	uint8_t rom[256] = {0};
	rom[0] = 0x2E; rom[3] = 0xEA; rom[0xB2] = 0x96;
	rom[0xC0] = 0xFE; rom[0xC1] = 0xFF; rom[0xC2] = 0xFF; rom[0xC3] = 0xEA;
	struct VFile *file = rom_path ? VFileOpen(rom_path, O_RDONLY) : VFileMemChunk(rom, sizeof rom);
	struct mCore *c = file ? mCoreFindVF(file) : NULL;
	if (!c || !c->init(c)) { if (file) file->close(file); return NULL; }
	mCoreInitConfig(c, NULL);
	c->setVideoBuffer(c, video, 240);
	if (!c->loadROM(c, file)) { c->deinit(c); return NULL; }
	c->loadSave(c, VFileMemChunk(NULL, 0));
	c->setAudioBufferSize(c, 1024);
	blip_set_rates(c->getAudioChannel(c, 0), c->frequency(c), 48000);
	blip_set_rates(c->getAudioChannel(c, 1), c->frequency(c), 48000);
	c->reset(c);
	return c;
}

static void cable_tick(Probe *p) {
	for (int i = 0; i < PLAYERS; ++i) {
		if (i == 0 && p->waiting) continue;
		p->running = i;
		p->core[i]->step(p->core[i]);
	}
}

static bool cable_start(Probe *p) {
	mLockstepInit(&p->cable.d);
	GBASIOLockstepInit(&p->cable);
	p->cable.d.context = p;
	p->cable.d.signal = cable_signal;
	p->cable.d.wait = cable_wait;
	p->cable.d.addCycles = cable_add;
	p->cable.d.useCycles = cable_use;
	p->cable.d.unusedCycles = cable_unused;
	p->cable.d.unload = cable_unload;
	for (int i = 0; i < PLAYERS; ++i) {
		struct mCore *c = make_core(p->video[i], NULL);
		if (!c) return false;
		p->core[i] = c;
		GBASIOLockstepNodeCreate(&p->node[i]);
		if (!GBASIOLockstepAttachNode(&p->cable, &p->node[i])) return false;
		GBASIOSetDriver(&((struct GBA *)c->board)->sio, &p->node[i].d, SIO_MULTI);
		c->busWrite16(c, 0x04000000u + REG_RCNT, 0);
		c->busWrite16(c, 0x04000000u + REG_SIOCNT, 0x2003);
	}
	for (int i = 0; i < 10000; ++i) cable_tick(p);
	return true;
}

static bool cable_transfer(Probe *p, unsigned sequence) {
	uint16_t sent[PLAYERS] = {(uint16_t)(sequence ^ 0xA5A5u), (uint16_t)(sequence ^ 0x5A5Au)};
	for (int i = 0; i < PLAYERS; ++i)
		p->core[i]->busWrite16(p->core[i], 0x04000000u + REG_SIOMLT_SEND, sent[i]);
	p->core[0]->busWrite16(p->core[0], 0x04000000u + REG_SIOCNT, 0x2083);
	for (int i = 0; i < 200000; ++i) {
		cable_tick(p);
		if (p->cable.d.transferActive == TRANSFER_IDLE) break;
	}
	if (p->cable.d.transferActive != TRANSFER_IDLE) {
		fprintf(stderr, "serial timeout at %u, phase=%d waiting=%u credit=%d\n", sequence,
			p->cable.d.transferActive, p->waiting, p->credit[1]);
		return false;
	}
	for (int i = 0; i < PLAYERS; ++i) {
		struct mCore *c = p->core[i];
		for (int j = 0; j < PLAYERS; ++j)
			if (c->busRead16(c, 0x04000000u + REG_SIOMULTI0 + 2u * (unsigned)j) != sent[j]) return false;
		if ((c->busRead16(c, 0x04000000u + REG_SIOCNT) & 0xF0) != (unsigned)i * 16) return false;
	}
	return true;
}

static void read_ram(struct mCore *c, uint8_t *out) {
	const struct GBA *gba = c->board;
	memcpy(out, gba->memory.wram, 0x40000);
	memcpy(out + 0x40000, gba->memory.iwram, 0x8000);
}

static bool loadout_test(struct mCore *c) {
	OnlineLoadout got;
	snapshot_core = c;
	uint32_t folder = BN6_EWRAM_END - BN6_FOLDER_ENTRIES * 2 - 64;
	uint32_t pack = folder - BN6_PACK_IDS * BN6_PACK_ENTRY;
	c->rawWrite32(c, BN6_TOOLKIT + BN6_TOOLKIT_CHIPS, -1, folder);
	c->rawWrite32(c, BN6_TOOLKIT + BN6_TOOLKIT_PACK, -1, pack);
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) c->rawWrite16(c, folder + 2 * i, -1, (uint16_t)(i + 1));
	for (unsigned i = 0; i < ONLINE_PACK_IDS; ++i) {
		uint32_t at = i < BN6_PACK_IDS ? pack + i * BN6_PACK_ENTRY : BN6_PACK_EXT + (i - BN6_PACK_IDS) * BN6_PACK_ENTRY;
		for (unsigned j = 0; j < BN6_PACK_ENTRY; ++j) c->rawWrite8(c, at + j, -1, (uint8_t)(i + j));
	}
	for (unsigned i = 0; i < ONLINE_BOARD_BYTES; ++i) c->rawWrite8(c, BN6_NAVICUST_PLACED + i, -1, (uint8_t)(i ^ 0x65));
	for (unsigned i = 0; i < ONLINE_BOARD_CELLS; ++i) c->rawWrite8(c, BN6_NAVICUST_GRID + i, -1, (uint8_t)(i + 7));
	c->rawWrite16(c, BN6_NAVI_HP, -1, 173);
	c->rawWrite16(c, BN6_NAVI_MAX_HP, -1, 280);
	c->rawWrite16(c, BN6_NAVI_BASE_MAX_HP, -1, 220);
	c->rawWrite8(c, BN6_NAVI_ATTACK, -1, 4);
	c->rawWrite8(c, BN6_NAVI_SPEED, -1, 3);
	c->rawWrite8(c, BN6_NAVI_CHARGE, -1, 2);
	bool ok = online_snapshot_read(&got, 0x2A) && got.hp == 173 && got.max_hp == 280 && got.base_hp == 220 &&
		got.attack == 4 && got.speed == 3 && got.charge == 2 && got.souls == 0x2A;
	if (!ok) fprintf(stderr, "loadout fields hp=%u max=%u base=%u attack=%u speed=%u charge=%u souls=%u folder=%08x pack=%08x\n",
		got.hp, got.max_hp, got.base_hp, got.attack, got.speed, got.charge, got.souls,
		emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS), emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK));
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES && ok; ++i) ok = got.folder[i] == i + 1;
	for (unsigned i = 0; i < ONLINE_PACK_IDS && ok; ++i)
		for (unsigned j = 0; j < BN6_PACK_ENTRY && ok; ++j) ok = got.pack[i][j] == (uint8_t)(i + j);
	for (unsigned i = 0; i < ONLINE_BOARD_BYTES && ok; ++i) ok = got.board[i] == (uint8_t)(i ^ 0x65);
	for (unsigned i = 0; i < ONLINE_BOARD_CELLS && ok; ++i) ok = got.grid[i] == (uint8_t)(i + 7);
	if (!ok) fprintf(stderr, "loadout arrays differ\n");
	c->rawWrite32(c, BN6_TOOLKIT + BN6_TOOLKIT_CHIPS, -1, folder + 1);
	ok = !online_snapshot_read(&got, 0) && ok;
	c->rawWrite32(c, BN6_TOOLKIT + BN6_TOOLKIT_CHIPS, -1, folder);
	c->rawWrite32(c, BN6_TOOLKIT + BN6_TOOLKIT_PACK, -1, BN6_EWRAM_END - BN6_PACK_IDS * BN6_PACK_ENTRY + 1);
	ok = !online_snapshot_read(&got, 0) && ok;
	c->rawWrite32(c, BN6_TOOLKIT + BN6_TOOLKIT_PACK, -1, pack);
	printf("loadout synthetic_fields_and_bounds=%s\n", ok ? "yes" : "NO");
	return ok;
}

static bool complete_state_probe(struct mCore *c) {
	struct VFile *vf = VFileMemChunk(NULL, 0);
	if (!vf) return false;
	bool ok = mCoreSaveStateNamed(c, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	void *before = NULL, *after = NULL;
	size_t sram_size = c->savedataClone(c, &before);
	uint8_t *changed = sram_size ? malloc(sram_size) : NULL;
	if (sram_size && !changed) ok = false;
	if (changed) {
		memset(changed, 0xA6, sram_size);
		ok = c->savedataRestore(c, changed, sram_size, false) && ok;
	}
	double start = millis();
	for (int i = 0; i < BENCH_COUNT && ok; ++i) ok = mCoreLoadStateNamed(c, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	double load_ms = (millis() - start) / BENCH_COUNT;
	size_t restored_size = c->savedataClone(c, &after);
	ok = ok && sram_size == restored_size && (!sram_size || memcmp(before, after, sram_size) == 0);
	printf("complete_state bytes=%jd savedata_bytes=%zu load_ms=%.4f savedata_restored=%s\n",
		(intmax_t)vf->size(vf), sram_size, load_ms, ok ? "yes" : "NO");
	free(before); free(after); free(changed);
	vf->close(vf);
	return ok;
}

static bool state_probe(struct mCore *c, const char *state_path) {
	if (state_path) {
		struct VFile *vf = VFileOpen(state_path, O_RDONLY);
		bool loaded = vf && mCoreLoadStateNamed(c, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
		if (vf) vf->close(vf);
		if (!loaded) return false;
	}
	size_t bytes = c->stateSize(c);
	uint8_t *state = calloc(1, bytes), *before = malloc(RAM_BYTES), *after = malloc(RAM_BYTES);
	if (!state || !before || !after) { free(state); free(before); free(after); return false; }
	OnlineLoadout carried_before, carried_after;
	snapshot_core = c;
	bool carried = state_path && online_snapshot_read(&carried_before, 0);
	read_ram(c, before);
	double start = millis();
	bool ok = true;
	for (int i = 0; i < BENCH_COUNT && ok; ++i) ok = c->saveState(c, state);
	double save_ms = (millis() - start) / BENCH_COUNT;
	/* Mutate every RAM byte: the test would fail if the restore were a no-op. */
	struct GBA *gba = c->board;
	memset(gba->memory.wram, 0xA5, 0x40000);
	memset(gba->memory.iwram, 0x5A, 0x8000);
	start = millis();
	for (int i = 0; i < BENCH_COUNT && ok; ++i) ok = c->loadState(c, state);
	double load_ms = (millis() - start) / BENCH_COUNT;
	read_ram(c, after);
	ok = ok && memcmp(before, after, RAM_BYTES) == 0;
	if (state_path) {
		bool same = carried && online_snapshot_read(&carried_after, 0) &&
			memcmp(&carried_before, &carried_after, sizeof carried_before) == 0;
		printf("carried bytes=%zu hp=%u max_hp=%u folder_entries=%u restored=%s souls=caller-supplied\n",
			sizeof carried_before, carried ? carried_before.hp : 0, carried ? carried_before.max_hp : 0,
			BN6_FOLDER_ENTRIES, same ? "yes" : "NO");
		ok = ok && same;
	}
	printf("state bytes=%zu ring_8_bytes=%zu save_ms=%.4f load_ms=%.4f ram_bytes=%u restored=%s\n",
		bytes, bytes * 8, save_ms, load_ms, RAM_BYTES, ok ? "yes" : "NO");
	free(state); free(before); free(after);
	return complete_state_probe(c) && ok;
}

int main(int argc, char **argv) {
	if (argc != 1 && argc != 3) {
		fprintf(stderr, "usage: %s [ROM STATE]\n", argv[0]);
		return 2;
	}
	mLogSetDefaultLogger(&quiet);
	Probe *p = calloc(1, sizeof *p);
	if (!p) return 1;
	bool ok = cable_start(p);
	double start = millis();
	unsigned transfers = 0;
	while (ok && transfers < TRANSFERS) { ok = cable_transfer(p, transfers); if (ok) ++transfers; }
	printf("serial cores=2 transfers=%u words_checked=%u elapsed_ms=%.2f passed=%s\n",
		transfers, transfers * 4, millis() - start, ok ? "yes" : "NO");
	for (int i = 0; i < PLAYERS; ++i) {
		if (!p->core[i]) continue;
		GBASIOSetDriver(&((struct GBA *)p->core[i]->board)->sio, NULL, SIO_MULTI);
		GBASIOLockstepDetachNode(&p->cable, &p->node[i]);
		p->core[i]->deinit(p->core[i]);
	}
	struct mCore *c = make_core(p->video[0], argc == 3 ? argv[1] : NULL);
	if (c) {
		if (argc == 1) { c->runFrame(c); ok = loadout_test(c) && ok; }
		ok = state_probe(c, argc == 3 ? argv[2] : NULL) && ok;
		c->deinit(c);
	}
	else ok = false;
	free(p);
	return ok ? 0 : 1;
}
