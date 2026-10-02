/* Hooks (src/emu/hook.c) on mGBA, without the game's ROM: a ROM of our own
 * bytes that calls three routines and stores what they return, run for a
 * frame with hooks of every kind on them (issue #27).
 *
 * The ROM: an ARM branch at 0 to 0xC0 (mGBA's GBAIsROM wants 0xEA at 3 and
 * 0x96 at 0xB2, nothing more), which switches to Thumb at 0xD0:
 *   D0 movs r0,#5;  bl F8 (r0 += 10); ldr r1,=0x03000000; str r0,[r1]
 *   DA movs r0,#7;  bl FC (r0 += 20); str r0,[r1,#4]
 *   E2 bkpt #1 (the cheat device's: the board's, a no-op here)
 *   E4 movs r2,#3;  str r2,[r1,#8]
 *   E8 movs r0,#9;  bl 100 (r0 += 30); str r0,[r1,#12]
 *   F0 b F0 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/config.h>
#include <mgba/core/core.h>
#include <mgba-util/vfs.h>

#include "hook.h"

#define ROM_SIZE 0x400
#define IWRAM 0x03000000u

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static void put16(uint8_t *rom, uint32_t at, uint16_t v) { rom[at] = (uint8_t)v; rom[at + 1] = (uint8_t)(v >> 8); }
static void put32(uint8_t *rom, uint32_t at, uint32_t v) { put16(rom, at, (uint16_t)v); put16(rom, at + 2, (uint16_t)(v >> 16)); }
/* bl from `at` to `to`, in its two halves */
static void bl(uint8_t *rom, uint32_t at, uint32_t to) {
	int32_t off = (int32_t)to - (int32_t)(at + 4);
	put16(rom, at, (uint16_t)(0xF000 | ((off >> 12) & 0x7FF)));
	put16(rom, at + 2, (uint16_t)(0xF800 | ((off >> 1) & 0x7FF)));
}

static void make_rom(uint8_t *rom) {
	memset(rom, 0, ROM_SIZE);
	put32(rom, 0x00, 0xEA00002E);   /* b 0xC0 */
	rom[0xB2] = 0x96;
	put32(rom, 0xC0, 0xE59F0000);   /* ldr r0,[pc,#0] */
	put32(rom, 0xC4, 0xE12FFF10);   /* bx r0 */
	put32(rom, 0xC8, 0x080000D1);   /* the Thumb code */
	put16(rom, 0xD0, 0x2005);       /* movs r0,#5 */
	bl(rom, 0xD2, 0xF8);
	put16(rom, 0xD6, 0x4907);       /* ldr r1,[pc,#28]: 0xF4 */
	put16(rom, 0xD8, 0x6008);       /* str r0,[r1] */
	put16(rom, 0xDA, 0x2007);       /* movs r0,#7 */
	bl(rom, 0xDC, 0xFC);
	put16(rom, 0xE0, 0x6048);       /* str r0,[r1,#4] */
	put16(rom, 0xE2, 0xBE01);       /* bkpt #1 */
	put16(rom, 0xE4, 0x2203);       /* movs r2,#3 */
	put16(rom, 0xE6, 0x608A);       /* str r2,[r1,#8] */
	put16(rom, 0xE8, 0x2009);       /* movs r0,#9 */
	bl(rom, 0xEA, 0x100);
	put16(rom, 0xEE, 0x60C8);       /* str r0,[r1,#12] */
	put16(rom, 0xF0, 0xE7FE);       /* b . */
	put16(rom, 0xF2, 0x46C0);       /* nop */
	put32(rom, 0xF4, IWRAM);
	put16(rom, 0xF8, 0x300A);       /* adds r0,#10 */
	put16(rom, 0xFA, 0x4770);       /* bx lr */
	put16(rom, 0xFC, 0x3014);       /* adds r0,#20 */
	put16(rom, 0xFE, 0x4770);
	put16(rom, 0x100, 0x301E);      /* adds r0,#30 */
	put16(rom, 0x102, 0x4770);
	put16(rom, 0x104, 0x3028);      /* adds r0,#40 */
	put16(rom, 0x106, 0x4770);
}

/* the hooks: one of each kind */
static HookAct doubled(HookRegs *r, void *user) { (void)user; r->r[0] *= 2; return HOOK_CONTINUE; }
static HookAct answered(HookRegs *r, void *user) { (void)user; r->r[0] = 99; return HOOK_RETURN; }
static HookAct sent_on(HookRegs *r, void *user) { (void)user; r->r12 = 0x08000105; return HOOK_JUMP; }
static HookAct counted(HookRegs *r, void *user) { (void)r; ++*(int *)user; return HOOK_CONTINUE; }

static uint32_t stored(struct mCore *core, int i) { return core->rawRead32(core, IWRAM + 4u * (uint32_t)i, -1); }

int main(void) {
	setvbuf(stdout, NULL, _IONBF, 0);
	static uint8_t rom[ROM_SIZE];
	static uint32_t video[240 * 160];
	make_rom(rom);
	struct VFile *vf = VFileMemChunk(rom, ROM_SIZE);
	struct mCore *core = vf ? mCoreFindVF(vf) : NULL;
	if (!core || !core->init(core)) { printf("FAIL: no core for the test ROM\n"); return 1; }
	mCoreInitConfig(core, NULL);
	core->setVideoBuffer(core, (color_t *)video, 240);
	if (!core->loadROM(core, vf)) { printf("FAIL: the test ROM did not load\n"); return 1; }
	hook_attach(core);
	/* (straight into the ROM, past the BIOS's start) */
	core->opts.skipBios = true;
	core->reset(core);

	/* plain: 5 + 10, 7 + 20, 3, 9 + 30 */
	core->runFrame(core);
	CHECK(stored(core, 0) == 15 && stored(core, 1) == 27 && stored(core, 2) == 3 && stored(core, 3) == 39,
		"no hooks: %u %u %u %u", stored(core, 0), stored(core, 1), stored(core, 2), stored(core, 3));

	int arm_hits = 0;
	CHECK(hook_add(0x080000F8, doubled, NULL), "a hook at 0xF8");
	CHECK(hook_add(0x080000FD, answered, NULL), "a hook at 0xFC (given with its Thumb bit)");
	CHECK(hook_add(0x08000100, sent_on, NULL), "a hook at 0x100");
	CHECK(hook_add_event(0x080000E8, 7), "an event hook at 0xE8");
	CHECK(hook_add_arm(0x080000C4, counted, &arm_hits), "an ARM hook at 0xC4");
	CHECK(!hook_add(0x080000F8, doubled, NULL), "a second hook at 0xF8 refused");
	uint32_t hits = hook_hits;
	core->reset(core);
	core->runFrame(core);
	/* CONTINUE with r0 doubled first: 5 * 2 + 10; RETURN 99; the bkpt #1
	 * passed on (the next store happened); JUMP to 0x104: 9 + 40 */
	CHECK(stored(core, 0) == 20, "continue: %u, not 20", stored(core, 0));
	CHECK(stored(core, 1) == 99, "return: %u, not 99", stored(core, 1));
	CHECK(stored(core, 2) == 3, "the board's bkpt #1: %u, not 3", stored(core, 2));
	CHECK(stored(core, 3) == 49, "jump: %u, not 49", stored(core, 3));
	CHECK(arm_hits == 1, "the ARM hook ran %d times, not once", arm_hits);
	CHECK(hook_hits - hits == 5, "%u hits, not 5", hook_hits - hits);
	HookEvent ev[4];
	int n = hook_drain(ev, 4);
	CHECK(n == 1 && ev[0].addr == 0x080000E8 && ev[0].kind == 7 && ev[0].r[0] == 99,
		"the event: %d queued, at %08x, kind %d, r0 %u", n, n ? ev[0].addr : 0, n ? ev[0].kind : 0, n ? ev[0].r[0] : 0);
	CHECK(hook_strays == 0 && hook_dropped == 0, "%u strays, %u dropped", hook_strays, hook_dropped);

	/* removed, the code is the ROM's again */
	hook_remove(0x080000F8);
	hook_remove(0x080000FC);
	hook_remove(0x08000100);
	hook_remove(0x080000E8);
	hook_remove(0x080000C4);
	CHECK(core->rawRead16(core, 0x080000F8, -1) == 0x300A && core->rawRead32(core, 0x080000C4, -1) == 0xE12FFF10, "the originals back");
	core->reset(core);
	core->runFrame(core);
	CHECK(stored(core, 0) == 15 && stored(core, 1) == 27 && stored(core, 3) == 39,
		"unhooked: %u %u %u", stored(core, 0), stored(core, 1), stored(core, 3));
	CHECK(hook_drain(ev, 4) == 0, "no event once unhooked");

	/* a hook in RAM: written again over what a reset (or a state) put
	 * there, and taken off with what it found */
	core->rawWrite16(core, IWRAM + 0x100, -1, 0x300A);
	CHECK(hook_add(IWRAM + 0x100, doubled, NULL), "a hook in IWRAM");
	CHECK(core->rawRead16(core, IWRAM + 0x100, -1) == 0xBECE, "its BKPT written");
	core->reset(core);
	core->rawWrite16(core, IWRAM + 0x100, -1, 0x3014);
	hook_reapply();
	CHECK(core->rawRead16(core, IWRAM + 0x100, -1) == 0xBECE, "written again after the reset");
	hook_remove(IWRAM + 0x100);
	CHECK(core->rawRead16(core, IWRAM + 0x100, -1) == 0x3014, "the reset's code back as it is taken off");
	mCoreConfigDeinit(&core->config);
	core->deinit(core);
	if (failures) { printf("test_emu: %d failed\n", failures); return 1; }
	printf("all hook checks passed\n");
	return 0;
}
