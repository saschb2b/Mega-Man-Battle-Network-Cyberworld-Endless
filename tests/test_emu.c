/* Hooks (src/emu/hook.c) on mGBA, without the game's ROM: a ROM of our own
 * bytes that calls three routines and stores what they return, run for a
 * frame with hooks of every kind on them (issue #27), an event posted from
 * an answer hook and a halt from one (issues #30, #35). Then two cores side
 * by side, as BN6's and the guest's run (issue #61): BN6's made by
 * src/emu/emu.c with our BKPT handler, the guest's as src/emu/guest.c makes
 * it, without, on a ROM that also sounds a tone.
 *
 * The ROM: an ARM branch at 0 to 0xC0 (mGBA's GBAIsROM wants 0xEA at 3 and
 * 0x96 at 0xB2, nothing more), which switches to Thumb at 0xD0:
 *   D0 movs r0,#5;  bl F8 (r0 += 10); ldr r1,=0x03000000; str r0,[r1]
 *   DA movs r0,#7;  bl FC (r0 += 20); str r0,[r1,#4]
 *   E2 bkpt #1 (the cheat device's: the board's, a no-op here)
 *   E4 movs r2,#3;  str r2,[r1,#8]
 *   E8 movs r0,#9;  bl 100 (r0 += 30); str r0,[r1,#12]
 *   F0 b F0
 * The guest's: its branch at 0 to 0x110 first, ARM code that turns the
 * sound on and plays a square wave on channel 2, then on to 0xC0. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/blip_buf.h>
#include <mgba/core/config.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/internal/gba/gba.h>
#include <mgba-util/vfs.h>
#ifdef __SANITIZE_ADDRESS__
#include <sanitizer/lsan_interface.h>
#endif

#include "emu.h"
#include "hook.h"
#include "platform.h"

/* (what src/emu/emu.c takes from the platform: nothing is shown or kept) */
Platform P;
void platform_persist(void) {}

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

/* ... and the guest's: the sound on first (SOUNDCNT_X, then its volumes and
 * channels, channel 2 a square wave at 256 Hz, full and steady) */
static void make_sound_rom(uint8_t *rom) {
	make_rom(rom);
	put32(rom, 0x00, 0xEA000042);   /* b 0x110 */
	put32(rom, 0x110, 0xE59F3028);  /* ldr r3,=0x04000060 */
	put32(rom, 0x114, 0xE3A00080);  /* mov r0,#0x80 */
	put32(rom, 0x118, 0xE1C302B4);  /* strh r0,[r3,#0x24]: SOUNDCNT_X, on */
	put32(rom, 0x11C, 0xE59F0020);  /* ldr r0,=0xFF77 */
	put32(rom, 0x120, 0xE1C302B0);  /* strh r0,[r3,#0x20]: SOUNDCNT_L, every channel both sides */
	put32(rom, 0x124, 0xE3A00002);  /* mov r0,#2 */
	put32(rom, 0x128, 0xE1C302B2);  /* strh r0,[r3,#0x22]: SOUNDCNT_H, the tones at full */
	put32(rom, 0x12C, 0xE59F0014);  /* ldr r0,=0xF080 */
	put32(rom, 0x130, 0xE1C300B8);  /* strh r0,[r3,#8]: SOUND2CNT_L, volume 15, half duty */
	put32(rom, 0x134, 0xE59F0010);  /* ldr r0,=0x8600 */
	put32(rom, 0x138, 0xE1C300BC);  /* strh r0,[r3,#0xC]: SOUND2CNT_H, 256 Hz, started */
	put32(rom, 0x13C, 0xEAFFFFDF);  /* b 0xC0 */
	put32(rom, 0x140, 0x04000060);
	put32(rom, 0x144, 0x0000FF77);
	put32(rom, 0x148, 0x0000F080);
	put32(rom, 0x14C, 0x00008600);
}

/* the hooks: one of each kind */
static HookAct doubled(HookRegs *r, void *user) { (void)user; r->r[0] *= 2; return HOOK_CONTINUE; }
/* (and an event of its own, with the registers it met) */
static HookAct answered(HookRegs *r, void *user) { (void)user; hook_post(r, 8); r->r[0] = 99; return HOOK_RETURN; }
static HookAct sent_on(HookRegs *r, void *user) { (void)user; r->r[12] = 0x08000105; return HOOK_JUMP; }
static HookAct counted(HookRegs *r, void *user) { (void)r; ++*(int *)user; return HOOK_CONTINUE; }
/* (and the CPU halted, with no interrupt on to wake it) */
static HookAct halting(HookRegs *r, void *user) { (void)r; ++*(int *)user; hook_halt(); return HOOK_CONTINUE; }

static uint32_t stored(struct mCore *core, int i) { return core->rawRead32(core, IWRAM + 4u * (uint32_t)i, -1); }

/* ---- two cores (issue #61) ---- */

/* A second core as src/emu/guest.c makes one: from its own copy of the
 * ROM, its sound at the rate BN6's plays at, no hooks of ours */
static struct mCore *guest_core(const uint8_t *rom, uint32_t *video) {
	struct VFile *vf = VFileMemChunk(rom, ROM_SIZE);
	struct mCore *c = vf ? mCoreFindVF(vf) : NULL;
	if (!c || !c->init(c)) return NULL;
	mCoreInitConfig(c, NULL);
	c->setVideoBuffer(c, (color_t *)video, 240);
	if (!c->loadROM(c, vf)) return NULL;
	c->loadSave(c, VFileMemChunk(NULL, 0));
	c->setAudioBufferSize(c, 1024);
	blip_set_rates(c->getAudioChannel(c, 0), c->frequency(c), emu_audio_out_rate());
	blip_set_rates(c->getAudioChannel(c, 1), c->frequency(c), emu_audio_out_rate());
	c->reset(c);
	return c;
}

static void guest_run(struct mCore *g, int frames) {
	for (int i = 0; i < frames; ++i) {
		g->runFrame(g);
		emu_audio_from(g);
	}
}

/* the ring emptied; the stereo frames it held, and how far apart their
 * samples lay (a tone, not silence) */
static int ring_drain(int *swing) {
	int16_t buf[2048];
	int total = 0, n, lo = 32767, hi = -32768;
	while ((n = emu_audio_read(buf, 1024)) > 0) {
		for (int i = 0; i < 2 * n; ++i) {
			if (buf[i] < lo) lo = buf[i];
			if (buf[i] > hi) hi = buf[i];
		}
		total += n;
	}
	if (swing) *swing = total > 0 ? hi - lo : 0;
	return total;
}

static void two_cores(void) {
	static uint8_t rom[ROM_SIZE], tone[ROM_SIZE];
	static uint32_t video[240 * 160];
	make_rom(rom);
	make_sound_rom(tone);
	/* BN6's, as the game makes it, its hooks through emu.c; it lives as long
	 * as the program, as the game's does (mGBA keeps some of what it holds
	 * in memory it maps itself, which LeakSanitizer does not look through) */
#ifdef __SANITIZE_ADDRESS__
	__lsan_disable();
#endif
	bool made = emu_init(rom, ROM_SIZE);
#ifdef __SANITIZE_ADDRESS__
	__lsan_enable();
#endif
	if (!made) { ++failures; printf("FAIL: emu_init on the test ROM\n"); return; }
	struct mCore *g = guest_core(tone, video);
	if (!g) { ++failures; printf("FAIL: no guest core for the test ROM\n"); return; }
	CHECK(emu_hook(0x080000F8, doubled, NULL), "BN6's hook at 0xF8");
	emu_frame(0);
	emu_frame(0);
	guest_run(g, 2);
	/* each runs its own code: BN6's hook in BN6's, none in the guest's */
	CHECK(emu_read32(IWRAM) == 20, "BN6's core: %u, not 20 (its hook)", emu_read32(IWRAM));
	CHECK(stored(g, 0) == 15 && stored(g, 1) == 27 && stored(g, 3) == 39, "the guest's: %u %u %u, not 15 27 39", stored(g, 0), stored(g, 1), stored(g, 3));
	CHECK(emu_read16(0x080000F8) == 0xBECE && g->rawRead16(g, 0x080000F8, -1) == 0x300A, "the BKPT in BN6's copy alone: %04x, the guest's %04x",
		emu_read16(0x080000F8), g->rawRead16(g, 0x080000F8, -1));
	/* run in turn, each from its reset (its code once through 0xF8): the
	 * hook's hits come with BN6's frames, never the guest's */
	uint32_t hits = hook_hits, strays = hook_strays;
	g->reset(g);
	guest_run(g, 2);
	CHECK(hook_hits == hits, "%u hits in the guest's frames", hook_hits - hits);
	emu_reset();
	emu_frame(0);
	emu_frame(0);
	emu_sync();
	CHECK(hook_hits == hits + 1, "%u hits in BN6's frames, not 1", hook_hits - hits);
	g->reset(g);
	guest_run(g, 2);
	CHECK(hook_hits == hits + 1 && stored(g, 0) == 15, "the guest after BN6's: %u hits, %u", hook_hits - hits, stored(g, 0));
	/* a BKPT of ours in the guest's code (none of its own hooks are) runs
	 * no hook of BN6's: the guest's board takes it, a no-op, so its add is
	 * skipped (5), where BN6's hook would have doubled it first (20) */
	hits = hook_hits;
	g->rawWrite16(g, 0x080000F8, -1, 0xBECE);
	g->reset(g);
	guest_run(g, 1);
	CHECK(hook_hits == hits && hook_strays == strays, "the guest's BKPT: %u hits, %u strays", hook_hits - hits, hook_strays - strays);
	CHECK(stored(g, 0) == 5, "the guest's BKPT: %u, not 5", stored(g, 0));
	g->rawWrite16(g, 0x080000F8, -1, 0x300A);
	/* their RAM apart */
	emu_write32(0x02000000, 0xB6B6B6B6);
	g->rawWrite32(g, 0x02000000, -1, 0xB5B5B5B5);
	CHECK(emu_read32(0x02000000) == 0xB6B6B6B6 && g->rawRead32(g, 0x02000000, -1) == 0xB5B5B5B5, "EWRAM shared: %08x %08x",
		emu_read32(0x02000000), g->rawRead32(g, 0x02000000, -1));
	/* the guest's sound into BN6's ring: its tone; and none let go */
	g->reset(g);
	ring_drain(NULL);   /* (what the guest's frames above put there) */
	emu_frame(0);
	emu_sync();
	int swing = 0;
	ring_drain(&swing);
	CHECK(swing < 256, "BN6's core, its sound off, in the ring: its samples %d apart", swing);
	guest_run(g, 4);
	int got = ring_drain(&swing);
	CHECK(got > 1000 && swing > 4096, "the guest's tone in the ring: %d frames, its samples %d apart", got, swing);
	/* (a frame run ahead unheard: its sound let go before the next pull) */
	g->runFrame(g);
	emu_audio_drop_from(g);
	emu_audio_from(g);
	CHECK(ring_drain(NULL) == 0, "a frame's sound let go reached the ring");
	/* its state saved and loaded, BN6's core untouched */
	g->rawWrite32(g, 0x02000000, -1, 0xB5B5B5B5);
	struct VFile *state = VFileMemChunk(NULL, 0);
	CHECK(state && mCoreSaveStateNamed(g, state, SAVESTATE_SAVEDATA), "the guest's state saved");
	g->rawWrite32(g, 0x02000000, -1, 0x12345678);
	guest_run(g, 1);
	if (state) {
		state->seek(state, 0, SEEK_SET);
		CHECK(mCoreLoadStateNamed(g, state, SAVESTATE_SAVEDATA), "the guest's state loaded");
		state->close(state);
	}
	CHECK(g->rawRead32(g, 0x02000000, -1) == 0xB5B5B5B5, "the guest's state: %08x, not B5B5B5B5", g->rawRead32(g, 0x02000000, -1));
	CHECK(emu_read32(0x02000000) == 0xB6B6B6B6, "BN6's EWRAM moved with the guest's state: %08x", emu_read32(0x02000000));
	emu_frame(0);
	CHECK(emu_read32(IWRAM) == 20, "BN6's hook after the guest's state: %u", emu_read32(IWRAM));
	emu_unhook(0x080000F8);
	mCoreConfigDeinit(&g->config);
	g->deinit(g);
}

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
	/* the answer hook's post (r0 still 7), then the event hook's (r0 the
	 * 99 it returned), in their order */
	HookEvent ev[4];
	int n = hook_drain(ev, 4);
	CHECK(n == 2 && ev[0].addr == 0x080000FC && ev[0].kind == 8 && ev[0].r[0] == 7,
		"the post: %d queued, at %08x, kind %d, r0 %u", n, n ? ev[0].addr : 0, n ? ev[0].kind : 0, n ? ev[0].r[0] : 0);
	CHECK(n == 2 && ev[1].addr == 0x080000E8 && ev[1].kind == 7 && ev[1].r[0] == 99,
		"the event: at %08x, kind %d, r0 %u", n > 1 ? ev[1].addr : 0, n > 1 ? ev[1].kind : 0, n > 1 ? ev[1].r[0] : 0);
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

	/* a halt from a hook: the loop at 0xF0 met once, then the CPU idle for
	 * the rest of the frame (no interrupt is on to end it) */
	int halts = 0;
	CHECK(hook_add(0x080000F0, halting, &halts), "a hook on the loop at 0xF0");
	core->reset(core);
	core->runFrame(core);
	CHECK(halts == 1 && ((struct GBA *)core->board)->cpu->halted, "halted: the hook ran %d times, halted %d", halts, ((struct GBA *)core->board)->cpu->halted);
	CHECK(stored(core, 3) == 39, "the code before the halt ran: %u", stored(core, 3));
	hook_remove(0x080000F0);
	mCoreConfigDeinit(&core->config);
	core->deinit(core);

	two_cores();
	if (failures) { printf("test_emu: %d failed\n", failures); return 1; }
	printf("all hook checks passed, the two cores kept apart\n");
	return 0;
}
