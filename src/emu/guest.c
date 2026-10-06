/* Another game's battles on a second core (guest.h). The guest is BN5: its
 * ROM, read beside BN6's (rom.c, XR), padded to EMU_ROM_SIZE as BN6's copy
 * is, in an mGBA core of its own. It boots once to a playable state (its
 * title, NEW GAME, and the intro pressed through to Lan's room) kept in
 * the data directory, in the background from the game's start on: on a
 * thread of its own on native builds, a slice of each frame's spare time in
 * the browser, whose page runs one frame at a time (guest_tick). Its battles
 * run only while BN6's core waits, on the main thread and without hooks:
 * its encounter roll, patched in its ROM copy, returns the record a battle
 * is given, and the battle's end is read from its game state
 * (docs/ROM_DATA.md, BN5 guest battles). */
#include "guest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "data.h"
#include "debug.h"
#include "emu.h"
#include "guest_records.h"
#include "rom.h"

/* (every build but the 3DS's, whose heap holds one ROM) */
#if !defined(__3DS__)
#define CW_GUEST 1
#endif

bool guest_dev_worried, guest_dev_gem;
int guest_dev_slowboot;
int guest_backdrop = -1;

const char *guest_dark_name(int k) {
	static char name[16];
	if (k < 0 || k >= GUEST_DARK_KINDS) return "";
	xrom_text(XROM_BN5_COLONEL_US, BN5_CHIP_NAMES_LOW, BN5_DARK_FIRST + k, name, sizeof name);
	return name;
}

#ifdef CW_GUEST

#include <SDL.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba-util/vfs.h>

#include "compat.h"
#include "game.h"
#include "guest_reward.h"
#include "platform.h"
#include "xchips.h"

/* ---- the run's chips as its game's: by name, from both ROMs ---- */

#define BN6_CHIPS 314   /* BN6's standard, Mega and Giga chips, those a folder holds */
#define BN6_STANDARD_CHIPS 203   /* ... its standard ones, 1-202 */
static uint16_t to_bn5[BN6_CHIPS], from_bn5[BN5_CHIPS];

/* Pairs BN6's chips with BN5's of the same name (xchips.c); how many
 * paired. */
static int chips_pair(int xrom) {
	static char names5[BN5_CHIPS][XCHIP_NAME], names6[BN6_CHIPS][XCHIP_NAME];
	for (int i = 1; i < BN5_CHIPS; ++i)
		xrom_text(xrom, i < 256 ? BN5_CHIP_NAMES_LOW : BN5_CHIP_NAMES_HIGH, i < 256 ? i : i - 256, names5[i], sizeof names5[i]);
	for (int id = 1; id < BN6_CHIPS; ++id) {
		ChipInfo ci;
		chip_info(id, &ci);
		snprintf(names6[id], sizeof names6[id], "%s", ci.name);
	}
	return xchips_pair((const char (*)[XCHIP_NAME])names6, BN6_CHIPS, (const char (*)[XCHIP_NAME])names5, BN5_CHIPS, to_bn5, from_bn5);
}

/* (the core is its boot's alone while that runs, on its thread where it
 * has one: the main thread touches it only once it is ready, or where the
 * boot runs in its own frames' slices) */
static struct mCore *core;
static bool ready, failed, active, result_due;
static int paired;   /* BN6's chips paired by name with its game's (chips_pair), on the main thread */
static uint32_t video[EMU_W * EMU_H];
static GuestResult result;
static int frames;
static int recoded, recode_ex[3];   /* chips that went in with another code, and the first: BN6 id, code, its code there */
static uint8_t dark_in[GUEST_DARK_KINDS];   /* the run's DarkChips as the battle began */
static bool dark_used;                       /* ... and one was used in it (latched from BN5_DARK_USED) */
static bool fell;                            /* ... and MegaMan's HP stood at 0 while it was fought on */
static bool buster_told;                     /* (debug) its battle's buster was printed */

/* a battle's course: waiting for the guest's boot, asked for, begun */
enum { PH_IDLE, PH_BOOT, PH_ASKED, PH_BATTLE };
static int phase;
/* (the battle that waits for the boot: its record, and MegaMan with his
 * folder as he went in) */
static uint32_t pend_record;
static GuestScale pend_sc;   /* ... its scaling to the act */
static GuestMegaMan pend_mm;
static GuestBoss pend_boss;   /* ... a guardian's, his (ai 0: none) */
static uint16_t pend_folder[30];
/* MegaMan's mood as a battle opens, from his HP: a battle begun calm, 0x80,
 * needed seven hits, each 10 off, to worry him (1-0x40), so a playtester at
 * 10 of 120 HP never saw his DarkChip (session 67). Worried from the start
 * at a quarter of his HP or less, else as much calmer as he is whole. BN5
 * sets 0x80 itself as the opening runs, so it is written over then. */
static int start_mood = 0x80;
static int mood_of(int hp, int max_hp) { return max_hp <= 0 ? 0x80 : hp * 4 <= max_hp ? 0x30 : 0x40 + 0x40 * hp / max_hp; }

static uint8_t rd8(uint32_t a) { return (uint8_t)core->rawRead8(core, a, -1); }
static uint16_t rd16(uint32_t a) { return (uint16_t)core->rawRead16(core, a, -1); }
static uint32_t rd32(uint32_t a) { return core->rawRead32(core, a, -1); }
static void wr16(uint32_t a, uint16_t v) { core->rawWrite16(core, a, -1, v); }

bool guest_possible(int xrom) { return xrom == XROM_BN5_COLONEL_US && XR[xrom].data && (ready || !failed); }

/* The chips of both games paired by name, once, where both ROMs are read:
 * on the main thread, before any boot (the director's words read them) */
static void pair(void) {
	if (!paired && XR[XROM_BN5_COLONEL_US].data && R.data) paired = chips_pair(XROM_BN5_COLONEL_US);
}

int guest_kind_chips(int kind, uint16_t *out, int max) {
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	if (!d || !R.data || kind < 0) return 0;
	pair();
	int n = 0;
	for (int id = 1; id < BN6_STANDARD_CHIPS && n < max; ++id)
		if (to_bn5[id] && d[BN5_CHIP_RECORDS + 0x2Cu * to_bn5[id] + BN5_CHIP_KIND] == kind) out[n++] = (uint16_t)id;
	return n;
}

/* (0 until it is ready: the autopilot reads its battle while one waits) */
uint8_t guest_read8(uint32_t a) { return ready ? rd8(a) : 0; }
uint16_t guest_read16(uint32_t a) { return ready ? rd16(a) : 0; }
uint32_t guest_read32(uint32_t a) { return ready ? rd32(a) : 0; }
void guest_write16(uint32_t a, uint16_t v) { if (ready) wr16(a, v); }
int guest_chip_bn6(int id) { return id > 0 && id < BN5_CHIPS ? from_bn5[id] : 0; }

/* The deck's compaction, replaced (BN5_COMPACT): as BN5's, the live chips
 * to the front, 0xFFFF after; but a DarkChip stays live only where BN5's
 * worried rule could have put it (at the hand's end while MegaMan is
 * worried or dark, or in the hand after the battle's first Custom screen),
 * the others shelved behind the padding, where no draw reaches and the
 * rule, which reads all 30, still finds them: DarkChips come only by its
 * rule (docs/META.md). Thumb, its literals 0x08012705 (emotion(side)),
 * the deck 0x0203C830 and 0xFFFF; called with r5 the Custom screen's state
 * and sl the toolkit, as BN5's (written and tested in romlab: the worried
 * MegaMan offered the shelved DarkCirc in his hand's last slot, the calm
 * one none). */
static const uint8_t shelf[188] = {
	0xf0, 0xb5, 0x84, 0xb0, 0x50, 0x46, 0x80, 0x69, 0xc1, 0x79, 0x00, 0x22, 0x01, 0x29, 0x00, 0xd9, 0x01, 0x22, 0x03, 0x92, 0x40, 0x7b,
	0x26, 0x4b, 0x00, 0xf0, 0x49, 0xf8, 0x01, 0x28, 0x01, 0xd0, 0x05, 0x28, 0x03, 0xd1, 0x03, 0x9a, 0x02, 0x21, 0x0a, 0x43, 0x03, 0x92,
	0xab, 0x79, 0x5b, 0x00, 0x20, 0x4c, 0x00, 0x26, 0x00, 0x27, 0x00, 0x20, 0x02, 0x90, 0xa0, 0x5b, 0x1e, 0x49, 0x88, 0x42, 0x1b, 0xd0,
	0xc1, 0x05, 0xc9, 0x0d, 0xbb, 0x29, 0x15, 0xd3, 0xc6, 0x29, 0x13, 0xd8, 0x03, 0x9a, 0x9e, 0x42, 0x04, 0xd3, 0x06, 0xd1, 0x02, 0x21,
	0x0a, 0x42, 0x0c, 0xd1, 0x02, 0xe0, 0x01, 0x21, 0x0a, 0x42, 0x08, 0xd1, 0x02, 0x99, 0x04, 0x29, 0x05, 0xd2, 0x4a, 0x00, 0x01, 0x31,
	0x02, 0x91, 0x69, 0x46, 0x88, 0x52, 0x01, 0xe0, 0xe0, 0x53, 0x02, 0x37, 0x02, 0x36, 0x3c, 0x2e, 0xdc, 0xd3, 0x02, 0x99, 0x1e, 0x22,
	0x52, 0x1a, 0x52, 0x00, 0x0b, 0x48, 0x97, 0x42, 0x02, 0xd2, 0xe0, 0x53, 0x02, 0x37, 0xfa, 0xe7, 0x00, 0x26, 0x02, 0x98, 0x40, 0x00,
	0x86, 0x42, 0x05, 0xd2, 0x69, 0x46, 0x88, 0x5b, 0xe0, 0x53, 0x02, 0x37, 0x02, 0x36, 0xf5, 0xe7, 0x04, 0xb0, 0xf0, 0xbd, 0x18, 0x47,
	0x05, 0x27, 0x01, 0x08, 0x30, 0xc8, 0x03, 0x02, 0xff, 0xff, 0x00, 0x00,
};
#define SHELF_AT (BN5_FREE + 0x100)

/* BN5's compaction pointed at the shelf (ldr r3, =shelf + 1; bx r3; nop) */
static void shelf_install(void) {
	for (uint32_t i = 0; i < sizeof shelf; i += 2) core->rawWrite16(core, SHELF_AT + i, -1, (uint16_t)(shelf[i] | shelf[i + 1] << 8));
	static const uint16_t jump[3] = { 0x4B01, 0x4718, 0x46C0 };
	for (uint32_t i = 0; i < 3; ++i) core->rawWrite16(core, BN5_COMPACT + 2 * i, -1, jump[i]);
	core->rawWrite32(core, BN5_COMPACT + 6, -1, SHELF_AT + 1);
}

/* The All * helper in the guest (docs/META.md): its chips' records hold *
 * alone in its ROM copy, as BN6's then do, so the run's chips come into its
 * Custom screen in * (a code its record lacks it draws as nothing), and its
 * check of a Program Advance of codes in a row takes three * (as BN6's,
 * BN5_PA_STAR_LIMIT); else its ROM's own again, where a run before had it. */
static bool starred;
static void star_records(bool on) {
	if (on == starred) return;
	starred = on;
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	static const uint8_t star[4] = { CHIP_CODE_STAR, 0xFF, 0xFF, 0xFF };
	for (uint32_t x = 1; x < BN5_CHIPS; ++x)
		for (uint32_t i = 0, rec = BN5_CHIP_RECORDS + 0x2Cu * x; i < 4; ++i)
			core->rawWrite8(core, 0x08000000u + rec + i, -1, on ? star[i] : d[rec + i]);
	uint32_t at = BN5_PA_STAR_LIMIT - 0x08000000u;
	uint16_t limit = (uint16_t)(d[at] | d[at + 1] << 8);
	wr16(BN5_PA_STAR_LIMIT, on && limit == 0x2A01 ? 0x2A03 : limit);
}

/* the run's DarkChips into the guest's folder (three at most, one of each,
 * as BN5's folder takes them), the kinds in `first` (BN6's folder holds
 * them: docs/META.md, BN6's own DarkChips) before the rest, each in its own
 * code (in * with All *), where chips sat out: one empty slot stays, the
 * shelf's edge */
static void dark_in_folder(uint32_t folders, uint32_t marks, unsigned first) {
	static const uint16_t entry[GUEST_DARK_KINDS] = { 0x22BB, 0x32BC, 0x10BD, 0x34BE, 0x2CBF, 0x26C0, 0x18C1, 0x0EC2, 0x0AC3, 0x06C4, 0x26C5, 0x08C6 };
	int empty[30], ne = 0, put = 0;
	for (int i = 29; i >= 0; --i) if (rd16(folders + 2u * (uint32_t)i) == 0xFFFF) empty[ne++] = i;
	for (int pass = 0; pass < 2; ++pass)
		for (int k = 0; k < GUEST_DARK_KINDS && put < 3 && put + 1 < ne; ++k) {
			if (!dark_in[k] || (int)(first >> k & 1) == pass) continue;
			wr16(folders + 2u * (uint32_t)empty[put++], starred ? chip_entry_star(entry[k]) : entry[k]);
			uint32_t id = entry[k] & 0x1FF;
			core->rawWrite8(core, marks + id, -1, (uint8_t)(rd8(BN5_CHIP_KEYS + id) ^ BN5_CHIP_KEY_XOR));
		}
}
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

/* ---- its boot: its title, NEW GAME, then A pressed through the intro
 * until Lan stands in his room, the same frames every time (emulation is
 * deterministic), once; then a state. It runs in the background, never in
 * a frame's way: on a thread of its own on native builds (boot_main), which
 * alone touches the guest core until it is ready and nothing of BN6's (the
 * chips are paired, the logger set and the sound's rate read before it
 * starts), or in slices of the frames' spare time where there is none (the
 * browser's, or --dev slowboot), guest_tick ---- */

/* Its frames: the title's 600, START tapped six times 70 frames apart, A
 * once and 130 frames, A every 50 frames through the intro 120 times, and
 * 60 more (a tap is held two frames) */
#define BOOT_FRAMES (600 + 6 * 70 + 130 + 120 * 50 + 60)

/* ... the keys held in its frame f */
static uint32_t boot_keys(int f) {
	if ((f -= 600) < 0) return 0;
	if (f < 6 * 70) return f % 70 < 2 ? KEY_START : 0;
	if ((f -= 6 * 70) < 130) return f < 2 ? KEY_A : 0;
	if ((f -= 130) < 120 * 50) return f % 50 < 2 ? KEY_A : 0;
	return 0;
}

enum { BOOT_NONE, BOOT_THREAD, BOOT_SLICES };   /* how it runs (the main thread's word) */
enum { BOOT_RUNNING, BOOT_READY, BOOT_FAILED };  /* how the thread's run ended */
static int boot_how;
static int boot_at = -1;          /* its next frame while it runs (its runner's), -1 none */
static uint64_t boot_ticks;       /* (the time its frames took, for its line) */
static SDL_atomic_t boot_seen;    /* its frames run, as the main thread reads them */
static uint64_t boot_t0;          /* when it began, for its pace (the main thread's) */
static int boot_slices, boot_sliced;   /* (in slices: the frames that ran one, and its frames they ran) */
#ifndef __EMSCRIPTEN__
static SDL_Thread *boot_thread;
static SDL_atomic_t boot_end;     /* the thread's last word, BOOT_RUNNING until then */
static SDL_atomic_t boot_stop;    /* the main thread's to it: stop (the game ends) */
static int boot_rate;             /* the sound's rate, read for it */
#endif

static bool boot_stopped(void) {
#ifndef __EMSCRIPTEN__
	return SDL_AtomicGet(&boot_stop) != 0;
#else
	return false;
#endif
}

/* Its frames from boot_at on, until the performance counter's `until` (0:
 * no end) or `count` of them (-1: no end); true once all have run */
static bool boot_frames(uint64_t until, int count) {
	uint64_t t0 = SDL_GetPerformanceCounter();
	while (boot_at < BOOT_FRAMES && count-- != 0 && (!until || SDL_GetPerformanceCounter() < until) && !boot_stopped()) {
		core->setKeys(core, boot_keys(boot_at));
		core->runFrame(core);
		SDL_AtomicSet(&boot_seen, ++boot_at);
	}
	boot_ticks += SDL_GetPerformanceCounter() - t0;
	return boot_at >= BOOT_FRAMES;
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
	printf("guest: booted in %d frames, %.2f s of its frames' time\n", BOOT_FRAMES, (double)boot_ticks / (double)SDL_GetPerformanceFrequency());
	return true;
}

/* The core for extra ROM `xrom`: its copy padded to EMU_ROM_SIZE as BN6's
 * is, its picture, and its sound at `rate` (BN6's) */
static bool core_make(int xrom, int rate) {
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
	blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), rate);
	blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), rate);
	core->reset(core);
	return true;
}

/* Booted: its roll answers no battle until one is asked, and the deck's
 * shelf is in (after its boot, which runs its ROM as it is). The first
 * write copies the ROM (mGBA's copy on write: 32 MB mapped, a while on a
 * handheld, two frames' time at the browser's title): written by its thread
 * where it has one, else as its first battle begins, behind the switch's
 * white. */
static bool patched;
static void boot_patch(void) {
	patch_roll(0);
	shelf_install();
	patched = true;
}

#ifndef __EMSCRIPTEN__
/* The thread: the core made, its kept state loaded or its frames run and
 * kept, its patches written; its last word in boot_end. Its priority low:
 * where cores are few, BN6's frames come first. */
static int boot_main(void *arg) {
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);
	bool ok = core_make((int)(intptr_t)arg, boot_rate);
	if (ok && !load_boot()) {
		core->reset(core);
		boot_at = 0;
		ok = boot_frames(0, -1) && boot_kept();
	}
	if (ok) boot_patch();
	SDL_AtomicSet(&boot_end, ok ? BOOT_READY : BOOT_FAILED);
	return 0;
}
#endif

/* A guardian's battle under way (guest_boss_battle): his id, and the zenny
 * his reward rows' chips pay (BN6's guardians' battles pay so where their
 * row holds their chip, which their Guardian Data gives) */
static int boss_id, boss_zenny;

/* A guardian's HP held to his act's band, the Net's clock's on it: his
 * stats row's HP written in the ROM copy before his battle, as he spawns
 * from it (seen in romlab: KnightMan's V1 row written 450 spawned him at
 * 450 of 450), and its own written back as the battle ends */
static uint32_t capped_at;
static uint16_t capped_was;

static void boss_uncap(void) {
	if (capped_at) wr16(capped_at, capped_was);
	capped_at = 0;
}

static void boss_cap(const GuestBoss *b) {
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	uint32_t at = guest_navi_stats(d, b->ai, b->version);
	uint16_t w = at ? (uint16_t)(d[at] | d[at + 1] << 8) : 0;
	if (!at || b->hp_cap <= 0 || (w & 0xFFF) == b->hp_cap) return;
	capped_at = 0x08000000u + at;
	capped_was = w;
	wr16(capped_at, (uint16_t)((w & 0xF000) | (b->hp_cap & 0xFFF)));
}

/* `record` copied past BN5's ROM without its GAME OVER: a loss ends the
 * battle on the map with the result 2, and the run's own GAME OVER follows
 * (BN6's); its copy's address. Its entities follow the record's pointer at
 * +0xC, four bytes each (0x11 an enemy, its id in the last two), 0xF0
 * ending them: the original's, or copied where its viruses are scaled.
 * (And its background the dressed area's where it leaves it to the map:
 * every guest battle stood in front of the room its boot state stands in,
 * yellow rings for each area.) */
#define ENTITIES_AT (BN5_FREE + 0x20)   /* a copied record's entity list, 16 entries and the 0xF0 at most */

/* (dev: a green Mystery Data where the record sets none, its byte 2 0x0F:
 * there every battle, its find of row 0, on the first panel of the enemies'
 * side none of its entities takes, `taken` bit y * 8 + x; --dev gem, as
 * loot_add_gem puts one in BN6's battles) */
static uint32_t gem_put(uint32_t at, uint32_t taken) {
	static const uint8_t panels[] = { 0x16, 0x36, 0x26, 0x15, 0x35, 0x25 };
	for (unsigned p = 0; p < sizeof panels; ++p) {
		if (taken >> ((panels[p] >> 4) * 8 + (panels[p] & 7)) & 1) continue;
		const uint8_t e[4] = { BN5_ENTITY_FIND << 4, panels[p], 0x0F, 0 };
		for (uint32_t j = 0; j < 4; ++j) core->rawWrite8(core, at + j, -1, e[j]);
		return 1;
	}
	return 0;
}

static uint32_t record_copy(uint32_t record, GuestScale sc) {
	for (uint32_t i = 0; i < 16; i += 2) {
		uint16_t v = rd16(record + i);
		if (i == 8) v = (uint16_t)(v & ~BN5_OPT_GAME_OVER);
		/* (a guardian's: no running from him, as from BN6's, and his results
		 * screen on, which his V1 story records leave off) */
		if (i == 8 && boss_id) v = (uint16_t)((v & ~BN5_OPT_RUN) | BN5_OPT_RESULTS);
		if (i == BN5_RECORD_BACKDROP && (v & 0xFF) == 0xFF && guest_backdrop >= 0) v = (uint16_t)((v & 0xFF00) | (guest_backdrop & 0xFF));
		wr16(BN5_FREE + i, v);
	}
	/* (and its entities, a byte at a time, each virus at its scaled version:
	 * the copy pointed at them; dev: its Mystery Data there every battle) */
	if (sc.up <= 0 && !guest_dev_gem) return BN5_FREE;
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	uint32_t from = rd32(record + 0xC), k = 0, taken = 0;
	bool gem = false;
	for (; k < 16 && from - 0x08000000u + 4u * k + 4 <= ROM_SIZE && d[from - 0x08000000u + 4u * k] != 0xF0; ++k) {
		const uint8_t *e = d + (from - 0x08000000u) + 4u * k;
		int id = e[0] == 0x11 ? guest_version_up(d, e[2] | e[3] << 8, sc) : e[2] | e[3] << 8;
		if (guest_dev_gem && e[0] >> 4 == BN5_ENTITY_FIND) { id |= 0x0F; gem = true; }
		taken |= 1u << ((e[1] >> 4 & 3) * 8 + (e[1] & 7));
		const uint8_t to[4] = { e[0], e[1], (uint8_t)id, (uint8_t)(id >> 8) };
		for (uint32_t j = 0; j < 4; ++j) core->rawWrite8(core, ENTITIES_AT + 4u * k + j, -1, to[j]);
	}
	if (guest_dev_gem && !gem && k < 16) k += gem_put(ENTITIES_AT + 4u * k, taken);
	core->rawWrite8(core, ENTITIES_AT + 4u * k, -1, 0xF0);
	core->rawWrite32(core, BN5_FREE + 0xC, -1, ENTITIES_AT);
	return BN5_FREE;
}

/* The run's folder as the guest's first, each chip as its game's of the
 * same name (its own code), owned; a chip it has none of sits out, its
 * slot empty. How many went in. */
/* A chip's code as BN5's chip of its name has it: the same letter where it
 * has that one, else its '*', else its first. BN5's Custom screen draws a
 * chip with a code it lacks as nothing, which can be chosen and does
 * nothing (a playtester's Storm folder had nine of them, its Thunder S
 * among them: BN5's Thunder, its chip 19, has B, L, P and *; session 65;
 * the Storm folder's ElcPuls1 and DolThdr1 BN5 has none of). */
static int bn5_code(int x, int code) {
	/* (its record as its ROM copy has it: in * alone with All *) */
	uint8_t rec[4];
	for (uint32_t i = 0; i < 4; ++i) rec[i] = rd8(0x08000000u + BN5_CHIP_RECORDS + 0x2Cu * (uint32_t)x + i);
	return xchips_code(rec, code);
}

static int folder_in(const uint16_t *folder) {
	uint32_t folders = rd32(BN5_TOOLKIT + BN5_TOOLKIT_CHIPS), marks = rd32(BN5_TOOLKIT + BN5_TOOLKIT_CHIP_MARKS);
	if (folders < 0x02000000u || folders >= 0x02040000u || marks < 0x02000000u || marks >= 0x02040000u) return 0;
	int in = 0;
	unsigned dark = 0;
	recoded = 0;
	for (uint32_t i = 0; i < 30; ++i) {
		int id = folder[i] & 0x1FF, x = id > 0 && id < BN6_CHIPS ? to_bn5[id] : 0, code = folder[i] >> 9;
		/* (a DarkChip of BN6's folder, one of BN6's own: the run's one of
		 * its kind, which comes by BN5's own rule, first among the run's,
		 * never as a folder chip beside it; docs/META.md) */
		if (x >= BN5_DARK_FIRST && x < BN5_DARK_FIRST + GUEST_DARK_KINDS) {
			dark_in[x - BN5_DARK_FIRST] = 1;
			dark |= 1u << (x - BN5_DARK_FIRST);
			x = 0;
		}
		int there = x ? bn5_code(x, code) : code;
		core->rawWrite16(core, folders + 2 * i, -1, x ? (uint16_t)(x | there << 9) : 0xFFFF);
		if (!x) continue;
		if (there != code && !recoded++) { recode_ex[0] = id; recode_ex[1] = code; recode_ex[2] = there; }
		core->rawWrite8(core, marks + (uint32_t)x, -1, (uint8_t)(rd8(BN5_CHIP_KEYS + (uint32_t)x) ^ BN5_CHIP_KEY_XOR));
		++in;
	}
	core->rawWrite8(core, BN5_NAVI_FOLDER, -1, 0);
	dark_in_folder(folders, marks, dark);
	if (dark && emu_debug_on()) fprintf(stderr, "guest: the DarkChips of BN6's folder come by BN5's own rule, first: kinds %#x\n", dark);
	return in;
}

int guest_sitting_out(const uint16_t *folder, uint16_t *out, int max) {
	pair();
	return paired ? xchips_out(folder, 30, to_bn5, BN6_CHIPS, out, max) : 0;
}

/* (debug: the folder's chips that sat out, slot by slot) */
static void out_tell(const uint16_t *folder) {
	char line[600];
	int k = 0, n = 0;
	for (int i = 0; folder && i < 30 && k < (int)sizeof line - 24; ++i) {
		int id = folder[i] & 0x1FF, code = folder[i] >> 9;
		if (!id || (id < BN6_CHIPS && to_bn5[id])) continue;
		ChipInfo ci;
		chip_info(id, &ci);
		k += snprintf(line + k, sizeof line - (size_t)k, "%s%s %c", n++ ? ", " : "", ci.name, code == 26 ? '*' : code < 26 ? 'A' + code : '?');
	}
	if (n) fprintf(stderr, "guest: %d sat out: %s\n", n, line);
}

/* One of its event flags set or cleared (BN5_TOOLKIT_FLAGS) */
static void flag_put(uint32_t flag, bool on) {
	uint32_t flags = rd32(BN5_TOOLKIT + BN5_TOOLKIT_FLAGS);
	if (flags < 0x02000000u || flags >= 0x02040000u) return;
	uint32_t a = flags + (flag >> 3);
	uint8_t bit = (uint8_t)(0x80u >> (flag & 7)), v = rd8(a);
	core->rawWrite8(core, a, -1, (uint8_t)(on ? v | bit : v & ~bit));
}

/* The run's Souls as its game's own (docs/META.md, Souls in BN5 territory):
 * each one's flag, and Double Soul and Chaos Unison learned while it holds
 * one, so its Custom screen offers UNITE by its own rule; its boot state,
 * early in its story, holds none of them */
static int souls_seen;   /* (debug) frames its Custom screen has been up, -1 told */
static void souls_in(uint8_t souls) {
	flag_put(BN5_FLAG_DOUBLE_SOUL, souls != 0);
	flag_put(BN5_FLAG_CHAOS, souls != 0);
	for (uint32_t k = 0; k < 6; ++k) flag_put(BN5_FLAG_SOUL + k, souls >> k & 1);
	souls_seen = souls && emu_debug_on() ? 0 : -1;
}

/* (debug: once a battle with Souls held, whether UNITE stood on its first
 * Custom screen, half a second up, as its opening sets it up; and each
 * unison as it begins) */
static void souls_tell(void) {
	static uint8_t united;
	static uint32_t used;   /* (the unisons told: a Chaos Unison is the one whose bit is new) */
	if (phase != PH_BATTLE || sub_mode() != BN5_SUB_BATTLE) { united = 0; used = 0; return; }
	if (souls_seen >= 0 && rd8(BN5_BATTLE_PHASE) == BN5_PHASE_CUSTOM && ++souls_seen == 30) {
		fprintf(stderr, "guest: UNITE %s on its first Custom screen\n", rd8(BN5_CUSTOM_UNITE) == 2 ? "stands" : "does not stand");
		souls_seen = -1;
	}
	uint8_t soul = rd8(BN5_BATTLE_SOUL + BN5_BATTLE_NAVI_SIZE * rd8(BN5_BATTLE_SIDE));
	if (soul != united && soul && emu_debug_on()) {
		uint32_t now = rd32(BN5_SOULS_USED);
		fprintf(stderr, "guest: MegaMan united with Soul %d%s\n", soul, (now & ~used) >> (16 + soul) & 1 ? ", Chaos Unison" : "");
		used = now;
	}
	united = soul;
}

/* His buster as the run's NaviCust makes it: BN5's boot state's fired 1 a
 * shot and 10 a charge, whatever programs the run had run */
static void buster_in(const uint8_t buster[3]) {
	static const uint32_t at[3] = { BN5_NAVI_ATTACK, BN5_NAVI_SPEED, BN5_NAVI_CHARGE };
	for (int i = 0; i < 3; ++i) core->rawWrite8(core, at[i], -1, buster[i] > BN5_BUSTER_MAX ? BN5_BUSTER_MAX : buster[i]);
	buster_told = !emu_debug_on();
}

/* (debug: the buster the battle took, from its own copy, once its Custom
 * screen is up) */
static void buster_tell(void) {
	if (buster_told || phase != PH_BATTLE || sub_mode() != BN5_SUB_BATTLE || rd8(BN5_BATTLE_STATE + 1) != BN5_PHASE_CUSTOM) return;
	buster_told = true;
	uint32_t s = BN5_BATTLE_NAVI + BN5_BATTLE_NAVI_SIZE * rd8(BN5_BATTLE_SIDE);
	fprintf(stderr, "guest: its battle's buster Attack %d, Speed %d, Charge %d\n", rd8(s + (BN5_NAVI_ATTACK - BN5_NAVI_STATS)) + 1,
		rd8(s + (BN5_NAVI_SPEED - BN5_NAVI_STATS)) + 1, rd8(s + (BN5_NAVI_CHARGE - BN5_NAVI_STATS)) + 1);
}

static bool battle_begin(uint32_t record, GuestScale sc, const GuestMegaMan *mm, const GuestBoss *boss) {
	memcpy(dark_in, mm->dark, sizeof dark_in);
	emu_sync();   /* (BN6's frame done first, where its core has a thread) */
	if (!on_map() && !load_boot()) return false;
	if (!patched) boot_patch();
	/* (a guardian's: his band, his rows' zenny, his options; none else) */
	boss_uncap();
	boss_id = boss ? guest_navi_id(XR[XROM_BN5_COLONEL_US].data, boss->ai, boss->version) : 0;
	boss_zenny = boss ? boss->zenny : 0;
	if (boss) boss_cap(boss);
	souls_in(mm->souls);
	star_records(mm->star);
	/* MegaMan as the run has him: his HP (BN5 copies it back after a battle
	 * whose options carry 0x40, as its random battles' do) */
	int hp = mm->hp, max_hp = mm->max_hp;
	wr16(BN5_NAVI_BASE_MAX_HP, (uint16_t)max_hp);
	wr16(BN5_NAVI_MAX_HP, (uint16_t)max_hp);
	wr16(BN5_NAVI_HP, (uint16_t)(hp < 1 ? 1 : hp > max_hp ? max_hp : hp));
	buster_in(mm->buster);
	/* (calm at its start, as each of the run's battles begins: BN5's dark
	 * meter and mood would carry its last DarkChip's darkness on; then, as
	 * its opening runs, as worried as his HP says, start_mood) */
	core->rawWrite8(core, BN5_NAVI_MOOD, -1, 0x80);
	start_mood = mood_of(hp, max_hp);
	wr16(BN5_NAVI_METER, 500);
	uint32_t check = rd32(BN5_TOOLKIT + BN5_TOOLKIT_METER_CHECK);
	if (check >= 0x02000000u && check < 0x02040000u) core->rawWrite32(core, check, -1, 500u ^ rd32(BN5_METER_KEY));
	dark_used = fell = false;
	const uint16_t *folder = mm->folder;
	int in = folder ? folder_in(folder) : 0;
	int ids[16], n = guest_record_foes_scaled(XROM_BN5_COLONEL_US, record, sc, ids, 16);
	/* (its results screen's rows as the run gets them: guest_reward.c) */
	GuestRowsFit how = { mm->star, { mm->codes[0], mm->codes[1], mm->codes[2] }, boss_id, boss_zenny };
	guest_rows_fit(ids, n, &how);
	guest_finds_fit(&how);
	if (emu_debug_on()) {
		fprintf(stderr, "guest: battle %08X, HP %d/%d, mood %#x, %d of the folder's 30 in, buster Attack %d, Speed %d, Charge %d\n", record, hp, max_hp,
			start_mood, in,
			rd8(BN5_NAVI_ATTACK) + 1, rd8(BN5_NAVI_SPEED) + 1, rd8(BN5_NAVI_CHARGE) + 1);
		out_tell(folder);
	}
	patch_roll(record_copy(record, sc));
	active = true;
	phase = PH_ASKED;
	frames = 0;
	return true;
}

/* ---- its boot from the main thread: begun, taken in at its end ---- */

/* Its end, on the main thread: ready for battles (its state kept in the
 * browser's storage too) or failed; and the battle that waited for it
 * begins, or, where it cannot, ends unfought, MegaMan as he went in */
static void boot_over(bool ok) {
	boot_how = BOOT_NONE;
	boot_at = -1;
	ready = ok;
	failed = !ok;
	platform_persist();
	if (ok) printf("guest: %s ready for battles, %d of BN6's chips by name\n", XR[XROM_BN5_COLONEL_US].layout->tag, paired);
	else fprintf(stderr, "guest: %s did not reach its map\n", XR[XROM_BN5_COLONEL_US].layout->tag);
	if (phase != PH_BOOT) return;
	active = false;
	phase = PH_IDLE;
	if (ready && battle_begin(pend_record, pend_sc, &pend_mm, pend_boss.ai ? &pend_boss : NULL)) return;
	result = (GuestResult){ .outcome = GUEST_ESCAPED, .hp = pend_mm.hp, .reward = guest_reward_of(0), .find = guest_reward_of(0) };
	memcpy(result.dark, pend_mm.dark, sizeof result.dark);
	result_due = true;
}

#ifndef __EMSCRIPTEN__
/* Its thread ended (stopped, where `stop`) and waited for: its end taken
 * in, where it ran to one */
static void boot_join(bool stop) {
	if (boot_how != BOOT_THREAD) return;
	if (stop) SDL_AtomicSet(&boot_stop, 1);
	SDL_WaitThread(boot_thread, NULL);
	boot_thread = NULL;
	if (stop) { boot_how = BOOT_NONE; return; }
	boot_over(SDL_AtomicGet(&boot_end) == BOOT_READY);
}
#endif

/* Its boot begun: on a thread of its own where it can be, else on this
 * one, its core made and its kept state loaded at once, or its frames
 * begun, a slice of each frame from then on (guest_tick) */
static bool boot_begin(int xrom) {
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data) return false;
	emu_log_quiet();   /* (the logger every core reads: set here, before any thread) */
	pair();
	SDL_AtomicSet(&boot_seen, 0);
	boot_ticks = 0;
	boot_t0 = SDL_GetPerformanceCounter();
	boot_slices = boot_sliced = 0;
#ifndef __EMSCRIPTEN__
	if (guest_dev_slowboot <= 0) {
		boot_rate = emu_audio_out_rate();
		SDL_AtomicSet(&boot_end, BOOT_RUNNING);
		SDL_AtomicSet(&boot_stop, 0);
		boot_thread = SDL_CreateThread(boot_main, "guest boot", (void *)(intptr_t)xrom);
		if (boot_thread) {
			boot_how = BOOT_THREAD;
			printf("guest: %s boots on a thread of its own\n", XR[xrom].layout->tag);
			return true;
		}
	}
#endif
	failed = true;
	if (!core_make(xrom, emu_audio_out_rate())) return false;
	failed = false;
	if (load_boot()) {
		boot_over(true);
		return true;
	}
	core->reset(core);
	boot_at = 0;
	boot_how = BOOT_SLICES;
	printf("guest: %s boots a slice a frame\n", XR[xrom].layout->tag);
	return true;
}

bool guest_start(int xrom) {
	if (ready || failed) return ready;
	if (boot_how != BOOT_NONE) return true;   /* (its boot under way) */
	return boot_begin(xrom);
}

void guest_warm(void) {
	static bool looked;
	if (looked) return;
	looked = true;
	if (!XR[XROM_BN5_COLONEL_US].data || ready || failed || boot_how != BOOT_NONE) return;
#ifdef __EMSCRIPTEN__
	/* (kept already: a layer loads it, as quick as a state loads, and the
	 * page's memory stays BN6's until then) */
	char path[600];
	state_path(path, sizeof path);
	FILE *f = fopen(path, "rb");
	if (f) { fclose(f); return; }
#endif
	guest_start(XROM_BN5_COLONEL_US);
}

void guest_tick(int ms) {
#ifndef __EMSCRIPTEN__
	if (boot_how == BOOT_THREAD && SDL_AtomicGet(&boot_end) != BOOT_RUNNING) boot_join(false);
#endif
	if (boot_how != BOOT_SLICES) return;
	int before = boot_at;
	bool done = guest_dev_slowboot > 0 ? boot_frames(0, guest_dev_slowboot)
		: ms > 0 && boot_frames(SDL_GetPerformanceCounter() + SDL_GetPerformanceFrequency() * (uint64_t)ms / 1000, -1);
	if (boot_at > before) { ++boot_slices; boot_sliced += boot_at - before; }
	if (done) boot_over(boot_kept());
}

void guest_quit(void) {
#ifndef __EMSCRIPTEN__
	boot_join(true);
#endif
}

int guest_boot_progress(void) {
	if (boot_how == BOOT_NONE) return -1;
	int pc = SDL_AtomicGet(&boot_seen) * 100 / BOOT_FRAMES;
	return pc > 99 ? 99 : pc;
}

int guest_boot_left_ms(void) {
	int done = SDL_AtomicGet(&boot_seen);
	if (boot_how == BOOT_NONE) return -1;
	if (done < 60) return 0;   /* (its pace not known yet) */
	double left = BOOT_FRAMES - done;
	/* (in slices by the frames' pace, in the game's 1/60 s; on its thread
	 * by the clock's) */
	if (boot_how == BOOT_SLICES) return boot_sliced ? (int)(left * boot_slices / boot_sliced * 1000.0 / 60.0) : 0;
	double secs = (double)(SDL_GetPerformanceCounter() - boot_t0) / (double)SDL_GetPerformanceFrequency();
	return (int)(left * secs / done * 1000.0);
}

bool guest_boot_waiting(void) { return active && phase == PH_BOOT; }

/* (begun at once, or, its boot still under way, waiting for it, BN6's core
 * with it: guest_tick; a headless run waits for a thread's then and there,
 * its frames the same from run to run, as tools/play.py's replays need) */
static bool battle_ask(uint32_t record, GuestScale sc, const GuestMegaMan *mm, const GuestBoss *boss) {
	if (active || !record) return false;
#ifndef __EMSCRIPTEN__
	if (P.headless) boot_join(false);
#endif
	if (boot_how == BOOT_NONE) return ready && battle_begin(record, sc, mm, boss);
	pend_record = record;
	pend_sc = sc;
	pend_mm = *mm;
	pend_boss = boss ? *boss : (GuestBoss){ 0 };
	if (mm->folder) {
		memcpy(pend_folder, mm->folder, sizeof pend_folder);
		pend_mm.folder = pend_folder;
	}
	active = true;
	phase = PH_BOOT;
	return true;
}

bool guest_battle(uint32_t record, GuestScale sc, const GuestMegaMan *mm) { return battle_ask(record, sc, mm, NULL); }

bool guest_boss_battle(const GuestBoss *boss, const GuestMegaMan *mm) {
	uint32_t record = guest_navi_record(XROM_BN5_COLONEL_US, boss->ai, boss->version);
	if (emu_debug_on())
		fprintf(stderr, "guest: guardian %d at version %d, record %08X, his HP %d, at most %d, his rows %d zenny\n", boss->ai, boss->version, record,
			guest_navi_hp(XROM_BN5_COLONEL_US, boss->ai, boss->version, NULL), boss->hp_cap, boss->zenny);
	return battle_ask(record, (GuestScale){ 0, 0 }, mm, boss);
}

bool guest_active(void) { return active; }

bool guest_on_screen(void) { return active && phase == PH_BATTLE && sub_mode() == BN5_SUB_BATTLE; }

int guest_custom_gauge(void) {
	return guest_on_screen() && rd8(BN5_BATTLE_PHASE) == BN5_PHASE_FIGHT ? rd16(BN5_CUSTOM_GAUGE) : -1;
}

bool guest_fight_hp(int *hp, int *max) {
	if (!guest_on_screen()) return false;
	*hp = rd16(BN5_FIGHT_HP);
	*max = rd16(BN5_FIGHT_HP + 2);
	return *max > 0;   /* (0/0 in the battle's first frames, before it sets MegaMan) */
}

static void finish(int outcome) {
	result = (GuestResult){ .outcome = outcome, .frames = frames, .hp = phase == PH_BATTLE ? rd16(BN5_BATTLE_HP) : rd16(BN5_NAVI_HP), .recoded = recoded,
		.recode_chip = recode_ex[0], .recode_from = recode_ex[1], .recode_to = recode_ex[2] };
	memcpy(result.dark, dark_in, sizeof result.dark);   /* (BN5 keeps a DarkChip once used: the run's stay) */
	result.dark_used = dark_used;
	result.dark_rose = fell && outcome != GUEST_LOST;
	/* (what its results screen gave, as the run gets it, HP+N in the HP
	 * above; and its second, the find of a green Mystery Data left on the
	 * field, which BN5 gives as it gives the first: a playtester's screen
	 * said "BugFrag 1" and the run kept none, session 69) */
	uint16_t v = outcome == GUEST_WON ? rd16(BN5_REWARD) : 0, f = outcome == GUEST_WON ? rd16(BN5_REWARD_FIND) : 0;
	result.reward = guest_reward_of(v);
	result.find = guest_reward_of(f);
	if (emu_debug_on()) fprintf(stderr, "guest: its results screen gave %04X and %04X\n", v, f);
	result_due = true;
	patch_roll(0);
	boss_uncap();
	boss_id = boss_zenny = 0;
	active = false;
	phase = PH_IDLE;
}

/* CYBERWORLD_EMU_DEBUG's line every 30 of its frames, as BN6's core's
 * (debug.c): its modes, its battle's phase, MegaMan's HP, the gauge and
 * the battle's clock */
static void debug_line(int mode, int sub) {
	if (frames % 30 || !emu_debug_on()) return;
	fprintf(stderr, "guest t%d mode %02x sub %02x phase %02x hp %d/%d gauge %d%% timer %u mood %02x\n", frames, mode, sub, rd8(BN5_BATTLE_PHASE),
		rd16(BN5_T1_OBJECTS + BN6_T1_HP), rd16(BN5_T1_OBJECTS + BN6_T1_MAX_HP), rd16(BN5_CUSTOM_GAUGE) * 100 / 0x4000,
		(unsigned)rd32(BN5_BATTLE_TIMER), rd8(BN5_BATTLE_MOOD));
}

static void step(uint32_t keys, bool quiet);

/* (MegaMan at 0 HP while his battle is fought on: BN5 gets him up again,
 * at 1 HP, after a DarkChip was used, the darkness fighting with his body
 * a while: seen in a playtest, session 68; MegaMan says so after it) */
static void fall_watch(int sub) {
	if (phase == PH_BATTLE && sub == BN5_SUB_BATTLE && dark_used && rd16(BN5_T1_OBJECTS + BN6_T1_HP) == 0) fell = true;
}

/* (the battle's opening: its mood as worried as his HP says, start_mood,
 * over the calm BN5 sets as it runs) */
static void mood_open(int sub) {
	if (phase == PH_BATTLE && sub == BN5_SUB_BATTLE && rd8(BN5_BATTLE_STATE + 1) == BN5_PHASE_INTRO && rd8(BN5_BATTLE_MOOD) > start_mood)
		core->rawWrite8(core, BN5_BATTLE_MOOD, -1, (uint8_t)start_mood);
}

void guest_frame(uint32_t keys) { step(keys, false); }
void guest_frame_quiet(uint32_t keys) { step(keys, true); }

static void step(uint32_t keys, bool quiet) {
	if (!active || phase == PH_BOOT) return;
	core->setKeys(core, keys);
	core->runFrame(core);
	if (quiet) emu_audio_drop_from(core);
	else emu_audio_from(core);
	++frames;
	buster_tell();
	souls_tell();
	int mode = main_mode(), sub = sub_mode();
	debug_line(mode, sub);
	if (phase == PH_ASKED && (sub == BN5_SUB_BATTLE_INIT || sub == BN5_SUB_BATTLE)) {
		phase = PH_BATTLE;
		core->rawWrite32(core, BN5_ROLL + 8, -1, 0);   /* (one battle: the roll answers none again) */
		core->rawWrite8(core, BN5_BATTLE_RESULT + 1, -1, 0);
		wr16(BN5_REWARD, 0);
		wr16(BN5_REWARD_FIND, 0);
	} else if (phase == PH_ASKED && frames > 600) finish(GUEST_ESCAPED);   /* (never began: nothing happened) */
	/* (a DarkChip used: latched while its battle is fought, as leaving it
	 * wipes the flag; not in the battle's first frames, which hold the last
	 * battle's bytes until BN5 clears them: every battle had cost 20 max HP) */
	else if (phase == PH_BATTLE && sub == BN5_SUB_BATTLE && rd8(BN5_BATTLE_PHASE) == BN5_PHASE_FIGHT &&
		rd8(BN5_DARK_USED + 8u * rd8(BN5_BATTLE_SIDE)))
		dark_used = true;
	fall_watch(sub);
	mood_open(sub);
	if (guest_dev_worried && phase == PH_BATTLE && sub == BN5_SUB_BATTLE && rd8(BN5_BATTLE_MOOD) > 0x40) core->rawWrite8(core, BN5_BATTLE_MOOD, -1, 0x20);
	/* (back on the map: how it ended, from BN5's own result, and MegaMan's
	 * HP as the battle left it: BN5 copies it back to his NaviStats only on
	 * its own maps' terms, which a forced battle does not meet) */
	if (phase == PH_BATTLE && mode == BN5_MODE_GAME && sub == BN5_SUB_MAP) {
		int r = rd8(BN5_BATTLE_RESULT + 1), hp = rd16(BN5_BATTLE_HP);
		finish(r == BN5_RESULT_LOST || hp == 0 ? GUEST_LOST : r == BN5_RESULT_ESCAPED ? GUEST_ESCAPED : GUEST_WON);
	}
	/* (a GAME OVER all the same: its records' copies leave it out) */
	else if (phase == PH_BATTLE && mode == BN5_MODE_GAME_OVER) finish(GUEST_LOST);
}

const uint32_t *guest_video(void) { return video; }

bool guest_take_result(GuestResult *out) {
	if (!result_due) return false;
	*out = result;
	result_due = false;
	return true;
}

#else   /* (one ROM: the 3DS) */

bool guest_start(int xrom) { (void)xrom; return false; }
bool guest_battle(uint32_t record, GuestScale sc, const GuestMegaMan *mm) {
	(void)record; (void)sc; (void)mm;
	return false;
}
void guest_warm(void) {}
void guest_tick(int ms) { (void)ms; }
void guest_quit(void) {}
int guest_boot_progress(void) { return -1; }
int guest_boot_left_ms(void) { return -1; }
bool guest_boot_waiting(void) { return false; }
bool guest_boss_battle(const GuestBoss *boss, const GuestMegaMan *mm) { (void)boss; (void)mm; return false; }
bool guest_possible(int xrom) { (void)xrom; return false; }
int guest_kind_chips(int kind, uint16_t *out, int max) { (void)kind; (void)out; (void)max; return 0; }
int guest_sitting_out(const uint16_t *folder, uint16_t *out, int max) { (void)folder; (void)out; (void)max; return 0; }
bool guest_active(void) { return false; }
bool guest_on_screen(void) { return false; }
bool guest_fight_hp(int *hp, int *max) { (void)hp; (void)max; return false; }
int guest_custom_gauge(void) { return -1; }
void guest_frame(uint32_t keys) { (void)keys; }
void guest_frame_quiet(uint32_t keys) { (void)keys; }
const uint32_t *guest_video(void) { return NULL; }
bool guest_take_result(GuestResult *out) { (void)out; return false; }
uint8_t guest_read8(uint32_t a) { (void)a; return 0; }
uint16_t guest_read16(uint32_t a) { (void)a; return 0; }
uint32_t guest_read32(uint32_t a) { (void)a; return 0; }
void guest_write16(uint32_t a, uint16_t v) { (void)a; (void)v; }
int guest_chip_bn6(int id) { (void)id; return 0; }

#endif
