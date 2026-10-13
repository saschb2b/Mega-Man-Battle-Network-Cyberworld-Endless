/* Another game's battles on a second core (guest.h): the differential's
 * reference. The guest is BN5: its ROM, read beside BN6's (rom.c, XR),
 * padded to EMU_ROM_SIZE as BN6's copy is, in an mGBA core of its own. It
 * boots once to a playable state (its title, NEW GAME, and the intro
 * pressed through to Lan's room) kept in the data directory. Its battles
 * run while BN6's core waits, on the main thread: its encounter roll,
 * patched in its ROM copy, returns the record a battle is given
 * (docs/ROM_DATA.md, BN5 guest battles). Runs fought on it until issue #82
 * brought every battle into BN6's engine (docs/MULTIROM.md, One net). */
#include "guest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static uint8_t rd8(uint32_t a) { return (uint8_t)core->rawRead8(core, a, -1); }
static uint16_t rd16(uint32_t a) { return (uint16_t)core->rawRead16(core, a, -1); }
static uint32_t rd32(uint32_t a) { return core->rawRead32(core, a, -1); }
static void wr16(uint32_t a, uint16_t v) { core->rawWrite16(core, a, -1, v); }

uint8_t guest_read8(uint32_t a) { return ready ? rd8(a) : 0; }
uint16_t guest_read16(uint32_t a) { return ready ? rd16(a) : 0; }
uint32_t guest_read32(uint32_t a) { return ready ? rd32(a) : 0; }
void guest_write16(uint32_t a, uint16_t v) { if (ready) wr16(a, v); }
void guest_write32(uint32_t a, uint32_t v) { if (ready) core->rawWrite32(core, a, -1, v); }

static int main_mode(void) { return rd8(rd32(BN5_TOOLKIT)); }
static int sub_mode(void) { return rd8(BN5_GAMESTATE); }
static bool on_map(void) { return main_mode() == BN5_MODE_GAME && sub_mode() == BN5_SUB_MAP; }

/* The roll answers `record` (0: no battle): ldr r0,=record; tst r0,r0;
 * bx lr, the record in the literal after it. */
static void patch_roll(uint32_t record) {
	static const uint16_t code[4] = { 0x4801, 0x4200, 0x4770, 0x46C0 };
	for (uint32_t i = 0; i < 4; ++i) core->rawWrite16(core, BN5_ROLL + 2 * i, -1, code[i]);
	core->rawWrite32(core, BN5_ROLL + 8, -1, record);
}

/* (bump the name when the boot changes; the runs' own, guest-bn5-1.state,
 * is gone from players' data folders: startup.c) */
static void state_path(char *out, size_t n) { snprintf(out, n, "%s/xdiff-bn5.state", g_data_dir); }

static bool load_boot(void) {
	char path[600];
	state_path(path, sizeof path);
	struct VFile *vf = VFileOpen(path, O_RDONLY);
	if (!vf) return false;
	bool ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA);
	vf->close(vf);
	return ok && on_map();
}

/* Its boot's frames: the title's 600, START tapped six times 70 frames
 * apart, A once and 130 frames, A every 50 frames through the intro 120
 * times, and 60 more (a tap is held two frames); the same every time
 * (emulation is deterministic) */
#define BOOT_FRAMES (600 + 6 * 70 + 130 + 120 * 50 + 60)

/* ... the keys held in its frame f */
static uint32_t boot_keys(int f) {
	if ((f -= 600) < 0) return 0;
	if (f < 6 * 70) return f % 70 < 2 ? KEY_START : 0;
	if ((f -= 6 * 70) < 130) return f < 2 ? KEY_A : 0;
	if ((f -= 130) < 120 * 50) return f % 50 < 2 ? KEY_A : 0;
	return 0;
}

/* Its end: Lan stands in his room, kept as the state; false where he does
 * not */
static bool boot_kept(void) {
	if (!on_map()) return false;
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
	if (ready || failed) return ready;
	failed = true;
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data) return false;
	emu_log_quiet();
	if (!core_make(xrom)) return false;
	if (!load_boot()) {
		core->reset(core);
		for (int f = 0; f < BOOT_FRAMES; ++f) {
			core->setKeys(core, boot_keys(f));
			core->runFrame(core);
		}
		if (!boot_kept()) {
			fprintf(stderr, "guest: %s did not reach its map\n", XR[xrom].layout->tag);
			return false;
		}
	}
	patch_roll(0);
	ready = true;
	failed = false;
	printf("guest: %s ready for the differential's battles\n", XR[xrom].layout->tag);
	return true;
}

/* ---- its random numbers from the spec's seed (docs/ROM_DATA.md, BN5's
 * random numbers): both RNG words written as BN5 begins the battle, so its
 * rolls are the seed's whatever frames its map took before ---- */
static uint32_t battle_seed;

static void rng_in(void) {
	core->rawWrite32(core, BN5_RNG_PRIMARY, -1, battle_rng_word(battle_seed));
	core->rawWrite32(core, BN5_RNG_SECONDARY, -1, battle_rng_word(battle_seed ^ 0x5EC0DA27u));
}

/* (its game's battle as its game has it: the record at DEV_RECORD, past
 * BN5's 8 MB in its ROM copy) */
#define DEV_RECORD (BN5_FREE + 0x200)

bool guest_dev_battle(const uint8_t *record, uint32_t len, int hp, uint32_t seed) {
	if (active || len < 0x11 || len > 0x80 || hp < 1) return false;
	if (!ready || (!on_map() && !load_boot())) return false;
	for (uint32_t i = 0; i < len; ++i) core->rawWrite8(core, DEV_RECORD + i, -1, record[i]);
	core->rawWrite32(core, DEV_RECORD + 0xC, -1, DEV_RECORD + 0x10);
	const uint32_t at[3] = { BN5_NAVI_BASE_MAX_HP, BN5_NAVI_MAX_HP, BN5_NAVI_HP };
	for (int i = 0; i < 3; ++i) wr16(at[i], (uint16_t)hp);
	battle_seed = seed;
	patch_roll(DEV_RECORD);
	active = true;
	phase = PH_ASKED;
	frames = 0;
	return true;
}

bool guest_hook(uint32_t addr, EmuHook fn, void *user) { return ready && hook_on(hook_core(core), addr, fn, user); }

static void finish(void) {
	patch_roll(0);
	active = false;
	phase = PH_IDLE;
}

void guest_frame_quiet(uint32_t keys) {
	if (!active) return;
	core->setKeys(core, keys);
	core->runFrame(core);
	emu_audio_drop_from(core);
	++frames;
	int mode = main_mode(), sub = sub_mode();
	if (phase == PH_ASKED && (sub == BN5_SUB_BATTLE_INIT || sub == BN5_SUB_BATTLE)) {
		phase = PH_BATTLE;
		core->rawWrite32(core, BN5_ROLL + 8, -1, 0);   /* (one battle: the roll answers none again) */
		core->rawWrite8(core, BN5_BATTLE_RESULT + 1, -1, 0);   /* (its result and rewards cleared, as the runs' battles began) */
		wr16(BN5_REWARD, 0);
		wr16(BN5_REWARD_FIND, 0);
		rng_in();   /* (its random numbers the seed's, from the frame it began) */
	} else if (phase == PH_ASKED && frames > 600) finish();   /* (never began) */
	/* (back on the map, or a GAME OVER: its battle over) */
	if (phase == PH_BATTLE && ((mode == BN5_MODE_GAME && sub == BN5_SUB_MAP) || mode == BN5_MODE_GAME_OVER)) finish();
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
bool guest_hook(uint32_t addr, EmuHook fn, void *user) { (void)addr; (void)fn; (void)user; return false; }
void guest_frame_quiet(uint32_t keys) { (void)keys; }
const uint32_t *guest_video(void) { return NULL; }

#endif
