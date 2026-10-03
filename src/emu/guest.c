/* Another game's battles on a second core (guest.h). The guest is BN5: its
 * ROM, read beside BN6's (rom.c, XR), padded to EMU_ROM_SIZE as BN6's copy
 * is, in an mGBA core of its own. It boots once to a playable state (its
 * title, NEW GAME, and the intro pressed through to Lan's room) kept in
 * the data directory, and runs only while BN6's core waits, on the main
 * thread and without hooks: its encounter roll, patched in its ROM copy,
 * returns the record a battle is given, and the battle's end is read from
 * its game state (docs/ROM_DATA.md, BN5 guest battles). */
#include "guest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "emu.h"
#include "rom.h"

#if !defined(__3DS__) && !defined(__EMSCRIPTEN__)
#define CW_GUEST 1
#endif

/* ---- its battle records, from the ROM file ---- */

static uint32_t rom32(const uint8_t *d, uint32_t a) {
	if (a < 0x08000000u || a - 0x08000000u + 4 > ROM_SIZE) return 0;
	const uint8_t *p = d + (a - 0x08000000u);
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

/* the first record of net map (group, number), 0 for none */
static uint32_t records_at(int xrom, int group, int number) {
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data || group < 0x80 || number < 0 || number > 0xFF) return 0;
	const uint8_t *d = XR[xrom].data;
	uint32_t groups = rom32(d, BN5_BATTLE_TABLES + 4), maps = groups ? rom32(d, groups + 4u * (uint32_t)(group - 0x80)) : 0;
	return maps ? rom32(d, maps + 4u * (uint32_t)number) : 0;
}

int guest_records(int xrom, int group, int number) {
	uint32_t r = records_at(xrom, group, number);
	int n = 0;
	while (r && n < 64 && r - 0x08000000u + 16u * (uint32_t)(n + 1) <= ROM_SIZE && XR[xrom].data[r - 0x08000000u + 16u * (uint32_t)n] != 0xFF) ++n;
	return n;
}

uint32_t guest_record(int xrom, int group, int number, int i) {
	return i >= 0 && i < guest_records(xrom, group, number) ? records_at(xrom, group, number) + 16u * (uint32_t)i : 0;
}

#ifdef CW_GUEST

#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba-util/vfs.h>

#include "compat.h"
#include "game.h"

static struct mCore *core;
static bool ready, failed, active, result_due;
static uint32_t video[EMU_W * EMU_H];
static GuestResult result;
static int frames;
/* a battle's course: asked for, begun, its GAME OVER playing */
enum { PH_IDLE, PH_ASKED, PH_BATTLE, PH_OVER };
static int phase;

static uint8_t rd8(uint32_t a) { return (uint8_t)core->rawRead8(core, a, -1); }
static uint32_t rd32(uint32_t a) { return core->rawRead32(core, a, -1); }
static int main_mode(void) { return rd8(rd32(BN5_TOOLKIT)); }
static int sub_mode(void) { return rd8(BN5_GAMESTATE); }
static bool on_map(void) { return main_mode() == BN5_MODE_GAME && sub_mode() == BN5_SUB_MAP; }

static void run(int n, uint32_t keys) {
	core->setKeys(core, keys);
	for (int i = 0; i < n; ++i) core->runFrame(core);
}

static void tap(uint32_t key) {
	run(2, key);
	run(8, 0);
}

/* The roll answers `record` (0: no battle): ldr r0,=record; tst r0,r0;
 * bx lr, the record in the literal after it. */
static void patch_roll(uint32_t record) {
	static const uint16_t code[4] = { 0x4801, 0x4200, 0x4770, 0x46C0 };
	for (uint32_t i = 0; i < 4; ++i) core->rawWrite16(core, BN5_ROLL + 2 * i, -1, code[i]);
	core->rawWrite32(core, BN5_ROLL + 8, -1, record);
}

/* bump the number when the boot changes */
static void state_path(char *out, size_t n) { snprintf(out, n, "%s/guest-bn5-1.state", g_data_dir); }

static bool load_boot(void) {
	char path[600];
	state_path(path, sizeof path);
	struct VFile *vf = VFileOpen(path, O_RDONLY);
	if (!vf) return false;
	bool ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA);
	vf->close(vf);
	return ok && on_map();
}

/* Its title, NEW GAME, then A pressed through the intro until Lan stands
 * in his room: the same frames every time (emulation is deterministic) */
static bool boot(void) {
	if (load_boot()) return true;
	core->reset(core);
	run(600, 0);
	for (int i = 0; i < 6; ++i) {
		tap(KEY_START);
		run(60, 0);
	}
	tap(KEY_A);
	run(120, 0);
	for (int i = 0; i < 120; ++i) {
		tap(KEY_A);
		run(40, 0);
	}
	run(60, 0);
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

bool guest_start(int xrom) {
	if (ready || failed) return ready;
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data) return false;
	failed = true;
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
	blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), emu_audio_out_rate());
	blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), emu_audio_out_rate());
	core->reset(core);
	if (!boot()) {
		fprintf(stderr, "guest: %s did not reach its map\n", XR[xrom].layout->tag);
		return false;
	}
	patch_roll(0);
	failed = false;
	ready = true;
	printf("guest: %s ready for battles\n", XR[xrom].layout->tag);
	return true;
}

bool guest_battle(uint32_t record) {
	if (!ready || active || !record) return false;
	emu_sync();   /* (BN6's frame done first, where its core has a thread) */
	/* (from where its boot left it: a GAME OVER put it back on its title) */
	if (!on_map() && !load_boot()) return false;
	patch_roll(record);
	active = true;
	phase = PH_ASKED;
	frames = 0;
	return true;
}

bool guest_active(void) { return active; }

static void finish(int outcome) {
	patch_roll(0);
	active = false;
	phase = PH_IDLE;
	result = (GuestResult){ outcome, frames };
	result_due = true;
}

void guest_frame(uint32_t keys) {
	if (!active) return;
	core->setKeys(core, keys);
	core->runFrame(core);
	emu_audio_from(core);
	++frames;
	int mode = main_mode(), sub = sub_mode();
	if (phase == PH_ASKED && (sub == BN5_SUB_BATTLE_INIT || sub == BN5_SUB_BATTLE)) {
		phase = PH_BATTLE;
		core->rawWrite32(core, BN5_ROLL + 8, -1, 0);   /* (one battle: the roll answers none again) */
	} else if (phase == PH_ASKED && frames > 600) finish(GUEST_WON);   /* (never began: nothing lost) */
	else if (phase == PH_BATTLE && mode == BN5_MODE_GAME_OVER) phase = PH_OVER;
	else if (phase == PH_BATTLE && mode == BN5_MODE_GAME && sub == BN5_SUB_MAP) finish(GUEST_WON);
	else if (phase == PH_OVER && mode != BN5_MODE_GAME_OVER) finish(GUEST_LOST);
}

const uint32_t *guest_video(void) { return video; }

bool guest_take_result(GuestResult *out) {
	if (!result_due) return false;
	*out = result;
	result_due = false;
	return true;
}

#else   /* (one ROM: the 3DS, the browser) */

bool guest_start(int xrom) { (void)xrom; return false; }
bool guest_battle(uint32_t record) { (void)record; return false; }
bool guest_active(void) { return false; }
void guest_frame(uint32_t keys) { (void)keys; }
const uint32_t *guest_video(void) { return NULL; }
bool guest_take_result(GuestResult *out) { (void)out; return false; }

#endif
