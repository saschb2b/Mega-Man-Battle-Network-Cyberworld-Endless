/* Another game's battles on a second core (guest.h): the differential's
 * reference. The guest is BN5 or BN4 Red Sun (a GuestGame each, below):
 * its ROM, read beside BN6's (rom.c, XR), padded to EMU_ROM_SIZE as BN6's
 * copy is, in an mGBA core of its own. It boots once to a playable state
 * (its title, NEW GAME, and the intro pressed through to Lan's room) kept
 * in the data directory. Its battles run while BN6's core waits, on the
 * main thread: its encounter roll (BN5's) or check (BN4's), patched in its
 * ROM copy, returns the record a battle is given (docs/rom/bn5-battles.md,
 * BN5 guest battles; docs/rom/bn4.md, The encounter check). Runs fought on
 * it until issue #82 brought every battle into BN6's engine
 * (docs/multirom/one-net.md). */
#include "guest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn4.h"
#include "bn5.h"
#include "emu.h"
#include "rom.h"

/* (every build but the 3DS's, whose heap holds no second core) */
#if !defined(__3DS__)

#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba-util/vfs.h>

#include "battle_rng.h"
#include "compat.h"
#include "game.h"

static struct mCore *core;
static bool ready, failed, active;
static uint32_t video[EMU_W * EMU_H];
static int frames;

/* a battle's course: asked for, begun */
enum { PH_IDLE, PH_ASKED, PH_BATTLE };
static int phase;
static uint32_t battle_seed;
static int battle_hp;

static uint8_t rd8(uint32_t a) { return (uint8_t)core->rawRead8(core, a, -1); }
static uint16_t rd16(uint32_t a) { return (uint16_t)core->rawRead16(core, a, -1); }
static uint32_t rd32(uint32_t a) { return core->rawRead32(core, a, -1); }
static void wr16(uint32_t a, uint16_t v) { core->rawWrite16(core, a, -1, v); }

uint8_t guest_read8(uint32_t a) { return ready ? rd8(a) : 0; }
uint16_t guest_read16(uint32_t a) { return ready ? rd16(a) : 0; }
uint32_t guest_read32(uint32_t a) { return ready ? rd32(a) : 0; }
void guest_write16(uint32_t a, uint16_t v) { if (ready) wr16(a, v); }
void guest_write32(uint32_t a, uint32_t v) { if (ready) core->rawWrite32(core, a, -1, v); }

/* Its boot: `taps` presses of `key` (none: a wait), each held two frames at
 * the start of a period of `period` frames; the same every time (emulation
 * is deterministic) */
typedef struct { uint32_t key; int taps, period; } BootStep;

/* A game the guest runs: its boot, where its battle is asked and its
 * record written, and how it reads its modes */
typedef struct {
	int xrom;
	const char *state;          /* its boot's state in the data directory (bump the name when the boot changes) */
	const BootStep *boot;       /* ... its presses, ended by a step of no taps */
	uint32_t ask;               /* the routine patched to answer a record: `ask_code`, the record in its literal at `ask_literal` */
	const uint16_t *ask_code;
	uint32_t ask_len, ask_literal;
	uint32_t record_at;         /* where a dev battle's record goes, past its game's 8 MB in its ROM copy */
	uint32_t record_min;        /* the shortest record: its header and its entities' end */
	uint32_t entities_ptr;      /* the record's pointer to its entities, which follow the pointer */
	bool (*on_map)(void);
	bool (*begun)(void);        /* its battle begun */
	bool (*over)(void);         /* ... over: back on its map, or a GAME OVER */
	void (*hp_in)(int hp);      /* MegaMan's HP and max HP on the map (NULL: as its battle begins, `begin`'s) */
	void (*begin)(void);        /* as its battle begins: its result cleared, its RNG words the seed's */
} GuestGame;

static const GuestGame *G;

/* ---- BN5 Team Colonel: its main mode at [eToolkit], its sub-mode at BN5_GAMESTATE (fixed) ---- */

static int bn5_mode(void) { return rd8(rd32(BN5_TOOLKIT)); }
static int bn5_sub(void) { return rd8(BN5_GAMESTATE); }
static bool bn5_on_map(void) { return bn5_mode() == BN5_MODE_GAME && bn5_sub() == BN5_SUB_MAP; }
static bool bn5_begun(void) { return bn5_sub() == BN5_SUB_BATTLE_INIT || bn5_sub() == BN5_SUB_BATTLE; }
static bool bn5_over(void) { return bn5_on_map() || bn5_mode() == BN5_MODE_GAME_OVER; }

static void bn5_hp_in(int hp) {
	const uint32_t at[3] = { BN5_NAVI_BASE_MAX_HP, BN5_NAVI_MAX_HP, BN5_NAVI_HP };
	for (int i = 0; i < 3; ++i) wr16(at[i], (uint16_t)hp);
}

/* (its result and rewards cleared, as the runs' battles began; its random
 * numbers from the spec's seed, docs/rom/bn5-battles.md, BN5's random
 * numbers: both words written as BN5 begins the battle, so its rolls are
 * the seed's whatever frames its map took before) */
static void bn5_begin(void) {
	core->rawWrite8(core, BN5_BATTLE_RESULT + 1, -1, 0);
	wr16(BN5_REWARD, 0);
	wr16(BN5_REWARD_FIND, 0);
	core->rawWrite32(core, BN5_RNG_PRIMARY, -1, battle_rng_word(battle_seed));
	core->rawWrite32(core, BN5_RNG_SECONDARY, -1, battle_rng_word(battle_seed ^ 0x5EC0DA27u));
}

/* (the roll: ldr r0,=record; tst r0,r0; bx lr, the record in the literal after it) */
static const uint16_t bn5_ask[4] = { 0x4801, 0x4200, 0x4770, 0x46C0 };
/* (the title's 600 frames, START tapped six times 70 frames apart, A once and 130 frames, A every 50 frames through
 * the intro 120 times, and 60 more) */
static const BootStep bn5_boot[] = { { 0, 1, 600 }, { KEY_START, 6, 70 }, { KEY_A, 1, 130 }, { KEY_A, 120, 50 }, { 0, 1, 60 }, { 0, 0, 0 } };

static const GuestGame bn5 = { XROM_BN5_COLONEL_US, "xdiff-bn5.state", bn5_boot, BN5_ROLL, bn5_ask, 4, BN5_ROLL + 8,
	BN5_FREE + 0x200, 0x11, 0xC, bn5_on_map, bn5_begun, bn5_over, bn5_hp_in, bn5_begin };

/* ---- BN4 Red Sun: its main mode at [eToolkit] (4 the map, 8 a battle), its save block moving in RAM at each change
 * of mode, so read through eToolkit at every access (docs/rom/bn4.md, The battle in RAM) ---- */

static int bn4_mode(void) { return rd8(rd32(BN4_TOOLKIT)); }
static bool bn4_on_map(void) { return bn4_mode() == BN4_MODE_MAP; }
static bool bn4_begun(void) { return bn4_mode() == BN4_MODE_BATTLE; }
static bool bn4_over(void) { return bn4_mode() == BN4_MODE_MAP || bn4_mode() == BN4_MODE_GAME_OVER; }

/* (MegaMan's HP and max HPs in a NaviStats) */
static void bn4_hp_at(uint32_t navi, int hp) {
	if (navi < 0x02000000u || navi >= 0x02040000u) return;
	wr16(navi + BN4_NAVI_HP, (uint16_t)hp);
	wr16(navi + BN4_NAVI_MAX_HPS, (uint16_t)hp);
	wr16(navi + BN4_NAVI_MAX_HPS + 2, (uint16_t)hp);
}

/* (its HP as its battle begins, in its NaviStats moved and the battle's copy, before MegaMan's object takes it: BN4
 * writes the NaviStats afresh as a battle is asked, two frames before its mode changes, so a write on the map is lost;
 * its result and reward cleared; both RNG words the seed's, as BN5's) */
static void bn4_begin(void) {
	bn4_hp_at(rd32(BN4_TOOLKIT + BN4_TOOLKIT_NAVI_STATS), battle_hp);
	bn4_hp_at(BN4_BATTLE_NAVI, battle_hp);
	core->rawWrite8(core, BN4_BATTLE_RESULT, -1, 0);
	wr16(BN4_BATTLE_REWARD, 0);
	core->rawWrite32(core, BN4_RNG_PRIMARY, -1, battle_rng_word(battle_seed));
	core->rawWrite32(core, BN4_RNG_SECONDARY, -1, battle_rng_word(battle_seed ^ 0x5EC0DA27u));
}

/* (the check: ldr r0,=record; bx lr, the record in the literal after it; the map loop tests r0, 0x080053EA) */
static const uint16_t bn4_ask[2] = { 0x4800, 0x4770 };
/* (tools/bn4_lab.py's boot: the title's 700 frames, START, 120 frames, A, 200 frames, A every 30 frames 60 times, then
 * every 20 frames 480 times through the opening's text, a tap held two frames and eight let go as romlab's) */
static const BootStep bn4_boot[] = { { 0, 1, 700 }, { KEY_START, 1, 10 }, { 0, 1, 120 }, { KEY_A, 1, 10 }, { 0, 1, 200 },
	{ KEY_A, 60, 30 }, { KEY_A, 480, 20 }, { 0, 0, 0 } };

static const GuestGame bn4 = { XROM_BN4_RED_SUN_US, "xdiff-bn4.state", bn4_boot, BN4_ENCOUNTER_CHECK, bn4_ask, 2,
	BN4_ENCOUNTER_CHECK + 4, BN4_FREE + 0x200, 0xD, BN4_RECORD_ENTITIES, bn4_on_map, bn4_begun, bn4_over, NULL, bn4_begin };

/* ---- the guest, either game ---- */

/* Its ask answers `record` (0: no battle) */
static void patch_ask(uint32_t record) {
	for (uint32_t i = 0; i < G->ask_len; ++i) core->rawWrite16(core, G->ask + 2 * i, -1, G->ask_code[i]);
	core->rawWrite32(core, G->ask_literal, -1, record);
}

static void state_path(char *out, size_t n) { snprintf(out, n, "%s/%s", g_data_dir, G->state); }

static bool load_boot(void) {
	char path[600];
	state_path(path, sizeof path);
	struct VFile *vf = VFileOpen(path, O_RDONLY);
	if (!vf) return false;
	bool ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA);
	vf->close(vf);
	return ok && G->on_map();
}

/* Its boot's frames, and the keys held in its frame f */
static int boot_frames(void) {
	int n = 0;
	for (const BootStep *b = G->boot; b->taps; ++b) n += b->taps * b->period;
	return n;
}

static uint32_t boot_keys(int f) {
	for (const BootStep *b = G->boot; b->taps; ++b) {
		if (f < b->taps * b->period) return f % b->period < 2 ? b->key : 0;
		f -= b->taps * b->period;
	}
	return 0;
}

/* Its end: Lan stands in his room, kept as the state; false where he does
 * not */
static bool boot_kept(void) {
	if (!G->on_map()) return false;
	char path[600], tmp[640];
	state_path(path, sizeof path);
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	struct VFile *vf = VFileOpen(tmp, O_CREAT | O_TRUNC | O_RDWR);
	if (vf) {
		bool ok = mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA);
		vf->close(vf);
		if (ok) cw_rename(tmp, path);
		else remove(tmp);
	}
	return true;
}

/* The core for extra ROM `xrom`: its copy padded to EMU_ROM_SIZE as BN6's
 * is, its picture, and its sound at BN6's rate */
static bool core_make(int xrom) {
	uint8_t *copy = malloc(EMU_ROM_SIZE);
	if (!copy) return false;
	memcpy(copy, XR[xrom].data, ROM_SIZE);
	memset(copy + ROM_SIZE, 0xFF, EMU_ROM_SIZE - ROM_SIZE);
	struct VFile *vf = VFileMemChunk(copy, EMU_ROM_SIZE);
	free(copy);
	if (!vf) return false;
	core = mCoreFindVF(vf);
	if (!core || !core->init(core)) { vf->close(vf); core = NULL; return false; }
	mCoreInitConfig(core, NULL);
	core->setVideoBuffer(core, (color_t *)video, EMU_W);
	if (!core->loadROM(core, vf)) { core->deinit(core); core = NULL; return false; }
	core->loadSave(core, VFileMemChunk(NULL, 0));
	core->setAudioBufferSize(core, 1024);
	int rate = emu_audio_out_rate();
	blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), rate);
	blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), rate);
	core->reset(core);
	return true;
}

bool guest_start(int xrom) {
	const GuestGame *g = xrom == XROM_BN5_COLONEL_US ? &bn5 : xrom == XROM_BN4_RED_SUN_US ? &bn4 : NULL;
	if (ready || failed) return ready && G == g;   /* (one game a process) */
	failed = true;
	if (!g) return false;
	/* (BN4's ROM is read only for research: asked for here, beside BN6's) */
	if (g == &bn4 && !XR[xrom].data) {
		xrom_research = true;
		xrom_find_beside();
	}
	if (!XR[xrom].data) return false;
	G = g;
	emu_log_quiet();
	if (!core_make(xrom)) return false;
	if (!load_boot()) {
		core->reset(core);
		for (int f = 0, n = boot_frames(); f < n; ++f) {
			core->setKeys(core, boot_keys(f));
			core->runFrame(core);
		}
		if (!boot_kept()) {
			fprintf(stderr, "guest: %s did not reach its map\n", XR[xrom].layout->tag);
			return false;
		}
	}
	patch_ask(0);
	ready = true;
	failed = false;
	printf("guest: %s ready for the differential's battles\n", XR[xrom].layout->tag);
	return true;
}

bool guest_dev_battle(const uint8_t *record, uint32_t len, int hp, uint32_t seed) {
	if (!ready || active || len < G->record_min || len > 0x80 || hp < 1) return false;
	if (!G->on_map() && !load_boot()) return false;
	for (uint32_t i = 0; i < len; ++i) core->rawWrite8(core, G->record_at + i, -1, record[i]);
	core->rawWrite32(core, G->record_at + G->entities_ptr, -1, G->record_at + G->entities_ptr + 4);
	if (G->hp_in) G->hp_in(hp);
	battle_hp = hp;
	battle_seed = seed;
	patch_ask(G->record_at);
	active = true;
	phase = PH_ASKED;
	frames = 0;
	return true;
}

bool guest_battling(void) { return active; }

bool guest_hook(uint32_t addr, EmuHook fn, void *user) { return ready && hook_on(hook_core(core), addr, fn, user); }

static void finish(void) {
	patch_ask(0);
	active = false;
	phase = PH_IDLE;
}

void guest_frame_quiet(uint32_t keys) {
	if (!active) return;
	core->setKeys(core, keys);
	core->runFrame(core);
	emu_audio_drop_from(core);
	++frames;
	if (phase == PH_ASKED && G->begun()) {
		phase = PH_BATTLE;
		core->rawWrite32(core, G->ask_literal, -1, 0);   /* (one battle: the ask answers none again) */
		G->begin();   /* (its random numbers the seed's, from the frame it began) */
	} else if (phase == PH_ASKED && frames > 600) finish();   /* (never began) */
	if (phase == PH_BATTLE && G->over()) finish();
}

const uint32_t *guest_video(void) { return video; }

#else   /* (one core: the 3DS) */

bool guest_start(int xrom) { (void)xrom; return false; }
uint8_t guest_read8(uint32_t a) { (void)a; return 0; }
uint16_t guest_read16(uint32_t a) { (void)a; return 0; }
uint32_t guest_read32(uint32_t a) { (void)a; return 0; }
void guest_write16(uint32_t a, uint16_t v) { (void)a; (void)v; }
void guest_write32(uint32_t a, uint32_t v) { (void)a; (void)v; }
bool guest_dev_battle(const uint8_t *record, uint32_t len, int hp, uint32_t seed) { (void)record; (void)len; (void)hp; (void)seed; return false; }
bool guest_battling(void) { return false; }
bool guest_hook(uint32_t addr, EmuHook fn, void *user) { (void)addr; (void)fn; (void)user; return false; }
void guest_frame_quiet(uint32_t keys) { (void)keys; }
const uint32_t *guest_video(void) { return NULL; }

#endif

