/* Hooks (src/emu/hook.c) on mGBA, without the game's ROM: a ROM of our own
 * bytes that calls three routines and stores what they return, run for a
 * frame with hooks of every kind on them (issue #27), an event posted from
 * an answer hook and a halt from one (issues #30, #35), and the table full
 * (HOOK_MAX), its hooks found as the code meets them. Then two cores side
 * by side, as BN6's and the guest's run (issue #61): BN6's made by
 * src/emu/emu.c with our BKPT handler, the guest's as src/emu/guest.c makes
 * it, without, on a ROM that also sounds a tone, then with hooks of its own
 * as the differential gives it (issue #82); and the guest's made and
 * run on a thread of its own while BN6's frames run, as its boot is. Then
 * a block of Thumb code moved into BN6's copy, its calls out through
 * veneers (src/emu/thumb_reloc.c, issue #82), and the second-level
 * dispatch's code (src/emu/xvirus_dispatch.c) run against spawners and
 * routines of the test's own. Then RAM the engine keeps patched, the
 * game's own bytes in the states it saves (emu.c, emu_keep); last, the
 * installers' claims through emu.c's writes and hooks (claims.h).
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

#include <SDL.h>
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
#include "bn6.h"
#include "claims.h"
#include "hook.h"
#include "platform.h"
#include "save_queue.h"
#include "sound_heard.h"
#include "sound_quiet.h"
#include "thumb_reloc.h"
#include "xinstall_log.h"
#include "xvirus_dispatch.h"

/* (what src/emu/emu.c takes from the platform: nothing is shown or kept) */
Platform P;
void platform_persist(void) {}

#define ROM_SIZE 0x800   /* (room for a full table of hooks on its zeros from 0x300: HOOK_MAX 192, issue #82) */
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

/* The table full (HOOK_MAX): three hooks the code meets (its calls at 0xF8,
 * 0xFC and 0x100, each r0 doubled) put in first, the rest on the ROM's
 * zeros that never run, every halfword from 0x300 on (one of them in
 * 0x100's bucket, which is found then by the table read through); one more
 * refused; each of the three run as the code meets it; all taken off */
#define FULL_AT 0x08000300u

static void full_table(struct mCore *core) {
	/* (0x08000100's bucket is FULL_AT + 2's, which takes it: that hit goes
	 * through the table read through, hook.c's find) */
	static const uint32_t met[3] = { 0x080000F8, 0x080000FC, 0x08000100 };
	int added = 0;
	for (int i = 0; i < 3; ++i) added += hook_add(met[i], doubled, NULL);
	for (uint32_t i = 0; i < HOOK_MAX - 3; ++i) added += hook_add(FULL_AT + 2 * i, doubled, NULL);
	CHECK(added == HOOK_MAX && !hook_add(FULL_AT - 0x100, doubled, NULL), "the table full: %d hooks put in of %d, one more refused", added,
		HOOK_MAX);
	uint32_t hits = hook_hits, strays = hook_strays;
	core->reset(core);
	core->runFrame(core);
	/* (5 x 2 + 10, 7 x 2 + 20, the bkpt #1's 3, 9 x 2 + 30) */
	CHECK(stored(core, 0) == 20 && stored(core, 1) == 34 && stored(core, 2) == 3 && stored(core, 3) == 48 && hook_hits - hits == 3 &&
		hook_strays == strays, "the full table's hooks: %u %u %u %u, %u hits, %u strays", stored(core, 0), stored(core, 1), stored(core, 2),
		stored(core, 3), hook_hits - hits, hook_strays - strays);
	for (int i = 0; i < 3; ++i) hook_remove(met[i]);
	for (uint32_t i = 0; i < HOOK_MAX - 3; ++i) hook_remove(FULL_AT + 2 * i);
	CHECK(core->rawRead16(core, 0x080000F8, -1) == 0x300A && core->rawRead16(core, FULL_AT, -1) == 0 &&
		core->rawRead16(core, FULL_AT + 2 * (HOOK_MAX - 4), -1) == 0, "the full table's originals back");
}

/* ... and the guest's own (the differential's, issue #82): one tripling r0
 * where BN6's doubles it, one reading what the core it runs on holds */
static HookAct tripled(HookRegs *r, void *user) { r->r[0] *= 3; ++*(int *)user; return HOOK_CONTINUE; }
static HookAct marked(HookRegs *r, void *user) { (void)r; *(uint32_t *)user = hook_read32(0x02000000); return HOOK_CONTINUE; }

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

/* Each core its own hooks (hook_core): the guest's hook at the address of
 * BN6's runs in the guest's frames alone, BN6's in BN6's alone, each
 * reading its own core's memory; detached, the guest runs its own code
 * again */
static void cores_apart(struct mCore *g) {
	HookCore *gh = hook_core(g);
	int guest_hits = 0;
	uint32_t seen = 0;
	CHECK(gh && hook_on(gh, 0x080000F8, tripled, &guest_hits) && hook_on(gh, 0x080000E8, marked, &seen), "the guest's own hooks at 0xF8 and 0xE8");
	CHECK(!hook_on(gh, 0x080000F8, tripled, &guest_hits), "a second hook at the guest's 0xF8 refused");
	CHECK(emu_read16(0x080000F8) == 0xBECE && emu_read16(0x080000E8) == 0x2009 && g->rawRead16(g, 0x080000E8, -1) == 0xBECE,
		"the BKPTs each in its own core's copy: BN6's %04x at 0xF8 and %04x at 0xE8, the guest's %04x at 0xE8", emu_read16(0x080000F8),
		emu_read16(0x080000E8), g->rawRead16(g, 0x080000E8, -1));
	g->reset(g);
	g->rawWrite32(g, 0x02000000, -1, 0xB5B5B5B5);
	guest_run(g, 1);
	CHECK(stored(g, 0) == 25 && stored(g, 3) == 39 && guest_hits == 1, "the guest's frame: %u %u, its hook %d times (not 25 39, once)", stored(g, 0),
		stored(g, 3), guest_hits);
	CHECK(seen == 0xB5B5B5B5, "the guest's hook read %08x, not its own core's B5B5B5B5", seen);
	uint32_t strays = hook_strays;
	emu_reset();
	emu_frame(0);
	emu_sync();
	CHECK(emu_read32(IWRAM) == 20 && emu_read32(IWRAM + 12) == 39 && guest_hits == 1 && hook_strays == strays,
		"BN6's frame: %u %u, the guest's hook %d times, %u strays (its own hook alone)", emu_read32(IWRAM), emu_read32(IWRAM + 12), guest_hits,
		hook_strays - strays);
	hook_detach(g);
	g->reset(g);
	guest_run(g, 1);
	CHECK(stored(g, 0) == 15 && guest_hits == 1 && g->rawRead16(g, 0x080000F8, -1) == 0x300A, "the guest detached: %u, its code %04x at 0xF8",
		stored(g, 0), g->rawRead16(g, 0x080000F8, -1));
	emu_frame(0);
	emu_sync();
	CHECK(emu_read32(IWRAM) == 20, "BN6's hook after the guest's detach: %u", emu_read32(IWRAM));
}

/* ---- the installs' record (xinstall_log.h): the Arena's pair under full rules ---- */

/* What emu.c's writes and hooks put in BN6's core while an install records,
 * made again in a core of the ROM's bytes (as arena_pair.c makes its two):
 * the ROM's bytes in its copy, RAM's and those outside the record left
 * out; the hooks on its own table, each reading the core it runs on; the
 * record's hash the same for the same installs */
static void recorded_installs(const uint8_t *rom, uint32_t *video) {
	static uint8_t copy[ROM_SIZE];
	static const uint8_t b[4] = { 1, 2, 3, 4 };
	uint32_t seen = 0;
	xlog_reset();
	emu_write8(0x08000700u, 0x11);
	xlog_begin();
	emu_write(0x08000710u, b, sizeof b);
	emu_write32(0x08000714u, 0x08070605u);
	emu_write8(0x0A000720u, 0x22);   /* (the ROM's mirror: its byte at 0x08000720) */
	emu_write8(0x02000100u, 0x33);
	CHECK(emu_hook(0x080000E8, marked, &seen), "a recorded hook at 0xE8");
	xlog_end();
	uint64_t h = xlog_hash();
	CHECK(xlog_bytes() == 9 && xlog_hooks() == 1 && h, "the record: %zu bytes, %d hooks, hash %016llx", xlog_bytes(), xlog_hooks(),
		(unsigned long long)h);
	memcpy(copy, rom, ROM_SIZE);
	xlog_apply(copy, ROM_SIZE);
	CHECK(copy[0x710] == 1 && copy[0x717] == 8 && copy[0x720] == 0x22 && copy[0x700] == rom[0x700] && copy[0xE8] == rom[0xE8],
		"the record's bytes in the copy: %02x %02x %02x, outside %02x, the hook's place %02x", copy[0x710], copy[0x717], copy[0x720],
		copy[0x700], copy[0xE8]);
	struct mCore *c = guest_core(copy, video);
	CHECK(c && xlog_hook_core(c), "the record's hooks on a core of their own");
	if (c) {
		c->reset(c);
		c->rawWrite32(c, 0x02000000, -1, 0xC3C3C3C3u);
		guest_run(c, 1);
		CHECK(seen == 0xC3C3C3C3u, "the recorded hook read %08x in the other core, not its C3C3C3C3", seen);
		hook_detach(c);
		mCoreConfigDeinit(&c->config);
		c->deinit(c);
	}
	emu_unhook(0x080000E8);
	CHECK(xlog_hash() == h, "the record's hash changed after it ended");
	xlog_reset();
	CHECK(!xlog_hash() && !xlog_bytes(), "the record forgotten");
}

/* ---- the saves' files written off the frame (save_queue.c) ---- */

/* a file's bytes, or NULL (*n its length) */
static uint8_t *file_bytes(const char *path, size_t *n) {
	FILE *f = fopen(path, "rb");
	*n = 0;
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t *b = len > 0 ? malloc((size_t)len) : NULL;
	if (b && fread(b, 1, (size_t)len, f) == (size_t)len) *n = (size_t)len;
	fclose(f);
	if (!*n) { free(b); return NULL; }
	return b;
}

/* a block of n bytes, each the mark */
static void *filled(uint8_t mark, size_t n) {
	uint8_t *b = malloc(n);
	if (b) memset(b, mark, n);
	return b;
}

static bool file_is(const char *path, uint8_t mark, size_t n) {
	size_t got = 0;
	uint8_t *b = file_bytes(path, &got);
	bool ok = b && got == n;
	for (size_t i = 0; ok && i < n; ++i) ok = b[i] == mark;
	free(b);
	return ok;
}

static void save_queue_checks(void) {
	char dir[] = "/tmp/cw-saves-XXXXXX", a[64], b[64], tmp[80];
	if (!mkdtemp(dir)) { ++failures; printf("FAIL: no folder for the save queue's files\n"); return; }
	snprintf(a, sizeof a, "%s/a.sav", dir);
	snprintf(b, sizeof b, "%s/b.sav", dir);
	/* in the order they came: the last of each file stands, whole */
	bool queued = true;
	for (int k = 1; k <= 40; ++k)
		queued = save_queue_file(k % 2 ? a : b, filled((uint8_t)k, 4096 + (size_t)k), 4096 + (size_t)k) && queued;
	CHECK(queued, "the save queue took the files");
	save_queue_wait();
	CHECK(file_is(a, 39, 4096 + 39) && file_is(b, 40, 4096 + 40), "each file the last queued for it, whole");
	snprintf(tmp, sizeof tmp, "%s.tmp", a);
	FILE *left = fopen(tmp, "rb");
	CHECK(!left, "no file left beside its place");
	if (left) fclose(left);
	/* the game's end writes what is queued before the thread goes */
	for (int k = 41; k <= 60; ++k) save_queue_file(k % 2 ? a : b, filled((uint8_t)k, 300000), 300000);
	save_queue_quit();
	CHECK(file_is(a, 59, 300000) && file_is(b, 60, 300000), "a quit writes every file queued first");
	/* and a file queued after it starts the thread again */
	CHECK(save_queue_file(a, filled(61, 10), 10), "queued after a quit");
	save_queue_wait();
	CHECK(file_is(a, 61, 10), "written by a thread started again");
	/* written at once, the same way */
	uint8_t now[5] = { 62, 62, 62, 62, 62 };
	CHECK(save_file_write(b, now, sizeof now) && file_is(b, 62, sizeof now), "a file written at once");
	unsigned before = save_write_failures();
	CHECK(!save_file_write(dir, now, sizeof now), "a directory cannot become a save");
	CHECK(save_write_failures() != before, "a synchronous write failure is observable");
	before = save_write_failures();
	CHECK(save_queue_file(dir, filled(63, 10), 10), "a failing write can still be accepted by the queue");
	save_queue_wait();
	CHECK(save_write_failures() != before, "a worker failure is observable after wait");
	before = save_write_failures();
	CHECK(save_queue_file(a, filled(64, 10), 10), "a later write can recover");
	save_queue_wait();
	CHECK(save_write_failures() == before && file_is(a, 64, 10), "a successful group has no new failures");
	save_queue_quit();
	remove(a);
	remove(b);
	rmdir(dir);
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
	/* a rollback's sound (issue #167): the ring cut back to a mark, what is
	 * not yet heard replaced by what is written next, what was heard skipped
	 * in it, the ring's timeline kept */
	uint64_t mark = emu_audio_mark();
	guest_run(g, 2);
	CHECK(emu_audio_mark() > mark, "the ring's mark stood still under the guest's sound");
	emu_audio_rewind(mark);
	CHECK(emu_audio_mark() == mark && ring_drain(NULL) == 0, "the ring not cut back to a mark none of it was heard past");
	guest_run(g, 2);
	int16_t heard[2 * 300];
	CHECK(emu_audio_read(heard, 300) == 300, "300 of the guest's frames not heard");
	emu_audio_rewind(mark);
	CHECK(emu_audio_mark() == mark + 300 && ring_drain(NULL) == 0, "the ring not cut to what was heard");
	guest_run(g, 2);
	int again = ring_drain(NULL);
	CHECK(again > 0 && emu_audio_mark() == mark + 300 + (uint64_t)again, "the sound run again: %d heard, the mark %llu past",
		again, (unsigned long long)(emu_audio_mark() - mark));
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
	/* BN6's state written from memory by the save queue, byte for byte the
	 * state written to its file at once, and loaded back */
	char sdir[] = "/tmp/cw-state-XXXXXX", qp[64], dp[64];
	if (mkdtemp(sdir)) {
		snprintf(qp, sizeof qp, "%s/run.state", sdir);
		snprintf(dp, sizeof dp, "%s/direct.state", sdir);
		bool q = emu_save_state_queued(qp), d = emu_save_state(dp);
		save_queue_wait();
		size_t nq = 0, nd = 0;
		uint8_t *bq = file_bytes(qp, &nq), *bd = file_bytes(dp, &nd);
		CHECK(q && d && bq && bd && nq == nd && !memcmp(bq, bd, nq), "the queued state the state written at once: %zu and %zu bytes", nq, nd);
		CHECK(emu_load_state(qp), "the queued state loaded");
		free(bq);
		free(bd);
		remove(qp);
		remove(dp);
		rmdir(sdir);
	}
	cores_apart(g);
	recorded_installs(rom, video);
	emu_unhook(0x080000F8);
	mCoreConfigDeinit(&g->config);
	g->deinit(g);
}

/* ---- the guest's boot on a thread of its own (guest.c's boot_main) ---- */

typedef struct {
	const uint8_t *rom;
	uint32_t *video;
	int frames;
	struct mCore *core;
	SDL_atomic_t done;
} BootJob;

/* its core made and its frames run on the thread, as the boot's are: no
 * sound pulled, nothing of BN6's touched */
static int boot_thread(void *arg) {
	BootJob *b = arg;
	b->core = guest_core(b->rom, b->video);
	for (int i = 0; b->core && i < b->frames; ++i) b->core->runFrame(b->core);
	SDL_AtomicSet(&b->done, 1);
	return 0;
}

/* a core's memory, a word at a time, as a sum (two cores alike or not) */
static uint32_t ram_sum(struct mCore *c, uint32_t base, uint32_t size) {
	uint32_t sum = 0;
	for (uint32_t a = 0; a < size; a += 4) sum = sum * 31u + c->rawRead32(c, base + a, -1);
	return sum;
}

static void two_threads(void) {
	static uint8_t tone[ROM_SIZE];
	static uint32_t video[240 * 160], video2[240 * 160];
	make_sound_rom(tone);
	CHECK(emu_hook(0x080000F8, doubled, NULL), "BN6's hook at 0xF8 again");
	/* BN6's frames on this thread while the guest's core is made and runs
	 * on its own: each of BN6's from its reset (its code once through 0xF8,
	 * a hit each), none of the guest's frames a hit */
	uint32_t hits = hook_hits, strays = hook_strays;
	BootJob job = { .rom = tone, .video = video, .frames = 600 };
	SDL_AtomicSet(&job.done, 0);
	SDL_Thread *t = SDL_CreateThread(boot_thread, "guest boot", &job);
	CHECK(t != NULL, "no thread for the guest's boot");
	uint32_t ours = 0;
	while (t && !SDL_AtomicGet(&job.done)) {
		emu_reset();
		emu_frame(0);
		emu_sync();
		++ours;
	}
	if (t) SDL_WaitThread(t, NULL);
	struct mCore *g = job.core;
	CHECK(g != NULL, "no guest core made on its thread");
	if (!g) return;
	CHECK(ours > 0, "no frame of BN6's ran beside the guest's thread");
	CHECK(hook_hits - hits == ours && hook_strays == strays, "BN6's hook: %u hits in its %u frames beside the guest's thread, %u strays", hook_hits - hits,
		ours, hook_strays - strays);
	CHECK(emu_read32(IWRAM) == 20, "BN6's core beside the guest's thread: %u, not 20", emu_read32(IWRAM));
	CHECK(stored(g, 0) == 15 && stored(g, 1) == 27 && stored(g, 3) == 39, "the guest's on its thread: %u %u %u, not 15 27 39", stored(g, 0), stored(g, 1),
		stored(g, 3));
	/* the same frames on this thread come to the same memory: what the
	 * thread made is the game's, wherever it ran */
	struct mCore *g2 = guest_core(tone, video2);
	CHECK(g2 != NULL, "no second guest core");
	if (g2) {
		for (int i = 0; i < job.frames; ++i) g2->runFrame(g2);
		CHECK(ram_sum(g, IWRAM, 0x8000) == ram_sum(g2, IWRAM, 0x8000) && ram_sum(g, 0x02000000, 0x40000) == ram_sum(g2, 0x02000000, 0x40000),
			"the guest's memory after its frames on a thread differs from the same frames' here");
		mCoreConfigDeinit(&g2->config);
		g2->deinit(g2);
	}
	emu_unhook(0x080000F8);
	mCoreConfigDeinit(&g->config);
	g->deinit(g);
}

/* ---- a block moved, its calls out through veneers (issue #82) ---- */

/* As a routine of another game's is moved into BN6's free space: a block of
 * our own bytes, made as if it stood at SRC in that game, calling two of
 * its routines there (SRC_EXT1, SRC_EXT2), one immediate of it patched,
 * copied to MOVED in BN6's copy of the ROM (emu_write, as an installer
 * writes), those calls aimed at two routines of this ROM's (the stubs),
 * 11 MiB away: so far that a BL reaches them only through a veneer. Each
 * stub, ARM code, keeps the registers and the flags it met, which must be
 * the ones the block set before its BL, with lr its way back into the
 * block. The BLs and the patch are thumb_reloc.c's own, run by the CPU. */
#define SRC 0x08001000u
#define SRC_EXT1 0x08001800u
#define SRC_EXT2 0x08001C00u
#define STUB1 0x08000200u
#define STUB2 0x08000240u
#define MOVED 0x08B34000u
#define VENEERS 0x08B31000u
#define LOAD 0x26                /* the block's routine that loads the registers */
#define PATCHED 0x20             /* its movs r1,#0x11, made #0x5A as it moves */
#define POOL 0x4C                /* its literal pool: r0-r12's values, then three words of its own */
#define BLOCK (POOL + 16 * 4)    /* its bytes */
#define REC1 (IWRAM + 0x100)     /* what each stub met: r0-r12, lr, sp, CPSR */
#define REC2 (IWRAM + 0x140)
#define DONE (IWRAM + 0x80)      /* the block's last stores: a word, then the patched byte at +8 */
#define SP_SEEN (IWRAM + 0x84)   /* its sp, as it began */
#define DONE_MARK 0x600DF00Du

/* r6 and r7 for the flags: cmp r6,r7 is N and V (0x7FFFFFFF - -1), cmp r6,r6 Z and C */
static uint32_t reg_value(int i) { return i == 6 ? 0x7FFFFFFFu : i == 7 ? 0xFFFFFFFFu : 0x01010101u * (uint32_t)(i + 1); }

/* ldr rd,[pc,#..] at `at` of the word at `lit` (both in the block) */
static void ldr_pc(uint8_t *b, uint32_t at, int rd, uint32_t lit) { put16(b, at, (uint16_t)(0x4800 | rd << 8 | (lit - ((at + 4) & ~3u)) / 4)); }

/* The block, its BLs encoded where it stood (at SRC):
 *   00 mov r0,sp; ldr r1,=SP_SEEN; str r0,[r1]
 *   06 bl load;   cmp r6,r7;  bl SRC_EXT1 (N V)
 *   10 bl load;   cmp r6,r6;  bl SRC_EXT2 (Z C)
 *   1A ldr r0,=DONE; ldr r1,=DONE_MARK; str r1,[r0]
 *   20 movs r1,#0x11; strb r1,[r0,#8]; b .
 *   26 load: r8-r12 through r0, then r0-r7, from the pool; bx lr
 * Its BLs' offsets in bls; how many (-1 where one would not encode). */
static int make_block(uint8_t *b, uint32_t *bls) {
	static const struct { uint32_t at, to; } calls[4] = { { 0x06, SRC + LOAD }, { 0x0C, SRC_EXT1 }, { 0x10, SRC + LOAD }, { 0x16, SRC_EXT2 } };
	memset(b, 0, BLOCK);
	put16(b, 0x00, 0x4668);   /* mov r0,sp */
	ldr_pc(b, 0x02, 1, POOL + 13 * 4);
	put16(b, 0x04, 0x6008);   /* str r0,[r1] */
	put16(b, 0x0A, 0x42BE);   /* cmp r6,r7 */
	put16(b, 0x14, 0x42B6);   /* cmp r6,r6 */
	ldr_pc(b, 0x1A, 0, POOL + 14 * 4);
	ldr_pc(b, 0x1C, 1, POOL + 15 * 4);
	put16(b, 0x1E, 0x6001);   /* str r1,[r0] */
	put16(b, PATCHED, 0x2111);   /* movs r1,#0x11 */
	put16(b, 0x22, 0x7201);   /* strb r1,[r0,#8] */
	put16(b, 0x24, 0xE7FE);   /* b . */
	uint32_t at = LOAD;
	for (int r = 8; r <= 12; ++r) {
		ldr_pc(b, at, 0, POOL + 4u * (uint32_t)r);
		put16(b, at + 2, (uint16_t)(0x4680 | (r - 8)));   /* mov rN,r0 */
		at += 4;
	}
	for (int r = 0; r <= 7; ++r, at += 2) ldr_pc(b, at, r, POOL + 4u * (uint32_t)r);
	put16(b, at, 0x4770);     /* bx lr */
	for (int r = 0; r <= 12; ++r) put32(b, POOL + 4u * (uint32_t)r, reg_value(r));
	put32(b, POOL + 13 * 4, SP_SEEN);
	put32(b, POOL + 14 * 4, DONE);
	put32(b, POOL + 15 * 4, DONE_MARK);
	for (int i = 0; i < 4; ++i) {
		if (!thumb_bl_write(b, calls[i].at, SRC, calls[i].to)) return -1;
		bls[i] = calls[i].at;
	}
	return 4;
}

/* A stub at `at`, entered in Thumb: on into ARM, every register and the
 * CPSR kept at rec (r0 at rec, rN at rec + 4N, lr, sp and the CPSR after),
 * back by bx lr:
 *   bx pc; nop
 *   stmfd sp!,{r0}; ldr r0,=rec+4; stmia r0,{r1-r12,lr}; ldmfd sp!,{r1}
 *   str r1,[r0,#-4]; str sp,[r0,#0x34]; mrs r1,cpsr; str r1,[r0,#0x38]; bx lr */
static void stub(uint32_t at, uint32_t rec) {
	static const uint32_t arm[9] = { 0xE92D0001, 0xE59F0018, 0xE8805FFE, 0xE8BD0002, 0xE5001004, 0xE580D034, 0xE10F1000, 0xE5801038, 0xE12FFF1E };
	uint8_t s[0x2C];
	put16(s, 0, 0x4778);
	put16(s, 2, 0x46C0);
	for (int i = 0; i < 9; ++i) put32(s, 4 + 4u * (uint32_t)i, arm[i]);
	put32(s, 0x28, rec + 4);
	emu_write(at, s, sizeof s);
}

/* the block's calls out: to the stub that stands for each, through its veneer */
typedef struct {
	ThumbVeneers pool;
	int asked;
} CallsOut;

static uint32_t to_stub(uint32_t target, void *user) {
	CallsOut *c = user;
	++c->asked;
	uint32_t to = target == SRC_EXT1 ? STUB1 : target == SRC_EXT2 ? STUB2 : 0;
	return to ? thumb_veneer_get(&c->pool, to) : 0;
}

/* a record's register, r0-r12, then lr (13), sp (14), the CPSR (15) */
static uint32_t met(uint32_t rec, int i) { return emu_read32(rec + 4u * (uint32_t)i); }

static void moved_block(void) {
	static uint8_t block[BLOCK], veneers[4 * THUMB_VENEER_SIZE];
	uint32_t made[8], found[8];
	int n = make_block(block, made);
	int scanned = thumb_bl_scan(block, POOL, found, 8);
	CHECK(n == 4 && scanned == n && !memcmp(found, made, sizeof *made * (size_t)n), "the block's BLs: %d found, %d made", scanned, n);
	uint16_t hi, lo;
	CHECK(!thumb_bl_encode(MOVED + 0x0C, STUB1, &hi, &lo), "a BL from the moved block reaches the stub");
	/* its movs r1,#0x11 made #0x5A, as a recipe's immediates are */
	static const ThumbImm was = { THUMB_MOVS, 1, 0, 0x11 };
	uint16_t insn = (uint16_t)(block[PATCHED] | block[PATCHED + 1] << 8);
	CHECK(thumb_imm_patch(&insn, &was, 0x5A) && insn == 0x215A, "the block's movs patched: %04x", insn);
	put16(block, PATCHED, insn);
	CallsOut calls = { .pool = { .bytes = veneers, .base = VENEERS, .cap = 4 } };
	ThumbBlock b = { block, BLOCK, SRC, MOVED };
	bool moved = thumb_block_rebase(&b, found, scanned, to_stub, &calls);
	CHECK(moved && calls.asked == 2 && calls.pool.n == 2, "the block moved: %d, its calls out asked %d, %d veneers", moved, calls.asked, calls.pool.n);
	if (!moved) return;
	emu_write(MOVED, block, BLOCK);
	emu_write(VENEERS, veneers, (size_t)calls.pool.n * THUMB_VENEER_SIZE);
	stub(STUB1, REC1);
	stub(STUB2, REC2);
	for (uint32_t a = DONE; a < REC2 + 0x40; a += 4) emu_write32(a, 0);
	/* (the ROM's own start goes on to the moved block, after a reset: the
	 * core's copy keeps what was written into it) */
	emu_write32(0x080000C8, MOVED | 1);
	emu_reset();
	emu_frame(0);
	emu_sync();
	uint32_t sp = emu_read32(SP_SEEN);
	CHECK(emu_read32(DONE) == DONE_MARK, "the moved block did not run to its end: %08x", emu_read32(DONE));
	CHECK(sp >> 24 == 0x03, "the block's sp: %08x", sp);
	CHECK(emu_read8(DONE + 8) == 0x5A, "the patched movs stored %02x, not 5a", emu_read8(DONE + 8));
	/* each stub as the block called it: every register, lr back into the
	 * block after its BL, sp, and the flags of its cmp */
	static const struct { uint32_t rec, back, nzcv; } call[2] = { { REC1, 0x10, 0x9 }, { REC2, 0x1A, 0x6 } };
	for (int k = 0; k < 2; ++k) {
		uint32_t rec = call[k].rec;
		for (int i = 0; i <= 12; ++i) CHECK(met(rec, i) == reg_value(i), "call %d: r%d %08x, not %08x", k + 1, i, met(rec, i), reg_value(i));
		CHECK(met(rec, 13) == ((MOVED + call[k].back) | 1), "call %d: lr %08x, not %08x", k + 1, met(rec, 13), (MOVED + call[k].back) | 1);
		CHECK(met(rec, 14) == sp, "call %d: sp %08x, not the block's %08x", k + 1, met(rec, 14), sp);
		CHECK(met(rec, 15) >> 28 == call[k].nzcv, "call %d: flags %x, not %x", k + 1, met(rec, 15) >> 28, call[k].nzcv);
	}
	/* the veneers as the core reads them */
	uint8_t v[THUMB_VENEER_SIZE];
	uint32_t t1 = 0, t2 = 0;
	for (uint32_t i = 0; i < THUMB_VENEER_SIZE; ++i) v[i] = emu_read8(VENEERS + i);
	bool v1 = thumb_veneer_target(v, &t1);
	for (uint32_t i = 0; i < THUMB_VENEER_SIZE; ++i) v[i] = emu_read8(VENEERS + THUMB_VENEER_SIZE + i);
	bool v2 = thumb_veneer_target(v, &t2);
	CHECK(v1 && v2 && t1 == STUB1 && t2 == STUB2, "the veneers in the ROM: %08x %08x", t1, t2);
	emu_write32(0x080000C8, 0x080000D1);
}

/* ---- the second-level dispatch's code run (issue #82) ----
 *
 * Its code (xvirus_dispatch_code) at XV_STUBS, its tables as xvirus_grow.c
 * leaves them, a family's entries set, and in their places routines of the
 * test's own: BN6's spawner of the third kind (keeps the registers it met,
 * the two words above its sp, and gives back an object or none, r0 and r1
 * changed), object_freeMemory (keeps r5) and a moved routine (keeps r0, r1,
 * r5 and lr). A driver near the stubs calls the dispatch's spawner as a
 * moved family's code does (r0 its game's type, r1-r4 the object's words),
 * keeps the registers it gets back, then runs the dispatch's type as BN6's
 * runner does (mov lr,pc; bx r0, r5 the object). */
#define DRV (XV_STUBS + 0x400u)           /* the driver, */
#define DRV_BACK 0x1A                     /* ... where the spawner returns to (after its BL), */
#define DRV_RAN 0x2A                      /* ... where the dispatch's run returns to, */
#define DRV_LITS 0x34                     /* ... its words: the object, the slot's side byte, the type, the record, run or not, the
                                             dispatch, the mark's place and the mark */
#define MOVED_RT (DRV + 0x80u)            /* the moved routine */
#define FAKE_SPAWN BN6_SPAWN_3_AT         /* BN6's spawner, the one XV_SPAWN_3 calls */
#define BN6_SPAWN_3_AT 0x08003358u
#define FAKE_FREE 0x08003458u             /* BN6's object_freeMemory, the one the stray's routine goes to */
#define REC_SPAWN (IWRAM + 0x400)         /* what the spawner met: r0-r4, then the two words above its sp */
#define REC_AFTER (IWRAM + 0x440)         /* r0-r5 as the driver got them back */
#define REC_MOVED (IWRAM + 0x480)         /* r0, r1, r5 and lr the moved routine met */
#define REC_FREE (IWRAM + 0x4A0)          /* r5 object_freeMemory met */
#define RAN_MARK (IWRAM + 0x4B0)
#define OBJ (IWRAM + 0x500)               /* the object, its slot at +3: */
#define SLOT 5
#define SIDE3 (0x0203FF20u + SLOT)        /* the third kind's side table's byte for it (BN6_DISPATCH_SIDE + 0x20) */

/* ldr rd,[pc,#..] at `at` (an address) of the word at `lit` */
static uint16_t ldr_lit(uint32_t at, int rd, uint32_t lit) { return (uint16_t)(0x4800 | rd << 8 | (lit - ((at + 4) & ~3u)) / 4); }

static void code_at(uint32_t at, const uint16_t *ops, int n, const uint32_t *lits, int nlits, uint32_t lit_off) {
	uint8_t b[0x80] = { 0 };
	for (int i = 0; i < n; ++i) put16(b, 2u * (uint32_t)i, ops[i]);
	for (int i = 0; i < nlits; ++i) put32(b, lit_off + 4u * (uint32_t)i, lits[i]);
	emu_write(at, b, lit_off + 4u * (uint32_t)nlits);
}

/* the driver, for one run: its game's type, the object the spawner gives back, whether the dispatch's type runs, the mark */
static void driver(uint32_t type, uint32_t obj, bool run, uint32_t mark) {
	uint16_t ops[DRV_LITS / 2];
	int n = 0;
	uint32_t lit = DRV + DRV_LITS;
	ops[n++] = ldr_lit(DRV + 0x00, 0, lit);            /* ldr r0,=OBJ */
	ops[n++] = 0x2100 | SLOT;                          /* movs r1,#SLOT */
	ops[n++] = 0x70C1;                                 /* strb r1,[r0,#3] */
	ops[n++] = ldr_lit(DRV + 0x06, 0, lit + 4);        /* ldr r0,=SIDE3 */
	ops[n++] = 0x21EE;                                 /* movs r1,#0xEE */
	ops[n++] = 0x7001;                                 /* strb r1,[r0] */
	ops[n++] = ldr_lit(DRV + 0x0C, 0, lit + 8);        /* ldr r0,=type */
	ops[n++] = 0x2111;                                 /* movs r1..r4 */
	ops[n++] = 0x2222;
	ops[n++] = 0x2333;
	ops[n++] = 0x2444;
	n += 2;                                            /* bl XV_SPAWN_3 (0x16) */
	ops[n++] = ldr_lit(DRV + 0x1A, 7, lit + 12);       /* ldr r7,=REC_AFTER */
	ops[n++] = 0xC73F;                                 /* stmia r7!,{r0-r5} */
	ops[n++] = ldr_lit(DRV + 0x1E, 0, lit + 16);       /* ldr r0,=run */
	ops[n++] = 0x2800;                                 /* cmp r0,#0 */
	ops[n++] = 0xD002;                                 /* beq 0x2A */
	ops[n++] = ldr_lit(DRV + 0x24, 0, lit + 20);       /* ldr r0,=the dispatch's type's routine */
	ops[n++] = 0x46FE;                                 /* mov lr,pc */
	ops[n++] = 0x4700;                                 /* bx r0 */
	ops[n++] = ldr_lit(DRV + 0x2A, 0, lit + 24);       /* ldr r0,=RAN_MARK */
	ops[n++] = ldr_lit(DRV + 0x2C, 1, lit + 28);       /* ldr r1,=mark */
	ops[n++] = 0x6001;                                 /* str r1,[r0] */
	ops[n++] = 0xE7FE;                                 /* b . */
	ops[n++] = 0x46C0;
	const uint32_t lits[8] = { OBJ, SIDE3, type, REC_AFTER, run, xvirus_dispatch_routine(3) | 1, RAN_MARK, mark };
	uint16_t hi, lo;
	thumb_bl_encode(DRV + 0x16, XV_SPAWN_3, &hi, &lo);
	ops[11] = hi;
	ops[12] = lo;
	code_at(DRV, ops, n, lits, 8, DRV_LITS);
	/* BN6's spawner, giving back `obj`:
	 *   ldr r5,=REC_SPAWN; stmia r5!,{r0-r4}; ldr r0,[sp]; str r0,[r5]; ldr r0,[sp,#4]; str r0,[r5,#4]
	 *   ldr r5,=obj; movs r0,#0x77; movs r1,#0x66; mov pc,lr */
	const uint16_t spawn[10] = { ldr_lit(FAKE_SPAWN, 5, FAKE_SPAWN + 0x14), 0xC51F, 0x9800, 0x6028, 0x9801, 0x6068,
		ldr_lit(FAKE_SPAWN + 0x0C, 5, FAKE_SPAWN + 0x18), 0x2077, 0x2166, 0x46F7 };
	const uint32_t spawn_lits[2] = { REC_SPAWN, obj };
	code_at(FAKE_SPAWN, spawn, 10, spawn_lits, 2, 0x14);
}

static uint32_t rec(uint32_t at, int i) { return emu_read32(at + 4u * (uint32_t)i); }

static void dispatch_run(void) {
	static uint8_t code[XV_STUBS_SIZE];
	xvirus_dispatch_code(code);
	emu_write(XV_STUBS, code, sizeof code);
	/* its tables blank, as xvirus_grow.c makes them, then a family's entries: the third kind's 0x25 moved, 0x30 BN6's 0x4B */
	static uint8_t moved[3 * XV_STEP], twin[3 * 0x100];
	for (uint32_t i = 0; i < sizeof moved; i += 4) put32(moved, i, XV_STUB_NONE | 1);
	memset(twin, XV_TYPE_MOVED, sizeof twin);
	emu_write(XV_MOVED_TABLES, moved, sizeof moved);
	emu_write(XV_TWIN_TABLES, twin, sizeof twin);
	emu_write32(xvirus_dispatch_moved_at(3, 0x25), MOVED_RT | 1);
	uint8_t b = 0x4B;
	emu_write(xvirus_dispatch_twin_at(3, 0x30), &b, 1);
	/* the moved routine: ldr r6,=REC_MOVED; stmia r6!,{r0,r1,r5}; mov r0,lr; str r0,[r6]; mov pc,lr */
	const uint16_t moved_rt[6] = { ldr_lit(MOVED_RT, 6, MOVED_RT + 0x0C), 0xC623, 0x4670, 0x6030, 0x46F7, 0x46C0 };
	const uint32_t moved_lit = REC_MOVED;
	code_at(MOVED_RT, moved_rt, 6, &moved_lit, 1, 0x0C);
	/* object_freeMemory: ldr r6,=REC_FREE; str r5,[r6]; mov pc,lr */
	const uint16_t free_rt[4] = { ldr_lit(FAKE_FREE, 6, FAKE_FREE + 0x08), 0x6035, 0x46F7, 0x46C0 };
	const uint32_t free_lit = REC_FREE;
	code_at(FAKE_FREE, free_rt, 4, &free_lit, 1, 0x08);
	emu_write32(0x080000C8, DRV | 1);
	static const struct { const char *what; uint32_t type, obj; bool run; uint32_t r0, side; } runs[4] = {
		{ "moved", 0x1225, OBJ, true, XV_T3_DISPATCH, 0x25 },   /* (its low byte the type, as the spawn stores it) */
		{ "BN6's twin", 0x30, OBJ, false, 0x4B, 0x30 },
		{ "none spawned", 0x25, 0, false, XV_T3_DISPATCH, 0xEE },
		{ "a stray", 0x26, OBJ, true, XV_T3_DISPATCH, 0x26 },
	};
	for (int k = 0; k < 4; ++k) {
		driver(runs[k].type, runs[k].obj, runs[k].run, 0xD15A0000u + (uint32_t)k);
		for (uint32_t a = REC_SPAWN; a < RAN_MARK + 4; a += 4) emu_write32(a, 0);
		emu_reset();
		emu_frame(0);
		emu_sync();
		const char *w = runs[k].what;
		CHECK(rec(RAN_MARK, 0) == 0xD15A0000u + (uint32_t)k, "dispatch, %s: the driver did not run to its end (%08x)", w, rec(RAN_MARK, 0));
		/* BN6's spawner met BN6's type, the object's words, and the frame above it: its game's type and the way back */
		CHECK(rec(REC_SPAWN, 0) == runs[k].r0 && rec(REC_SPAWN, 1) == 0x11 && rec(REC_SPAWN, 2) == 0x22 && rec(REC_SPAWN, 3) == 0x33 &&
			rec(REC_SPAWN, 4) == 0x44, "dispatch, %s: the spawner met r0 %#x, r1-r4 %#x %#x %#x %#x", w, rec(REC_SPAWN, 0),
			rec(REC_SPAWN, 1), rec(REC_SPAWN, 2), rec(REC_SPAWN, 3), rec(REC_SPAWN, 4));
		CHECK(rec(REC_SPAWN, 5) == runs[k].type && rec(REC_SPAWN, 6) == ((DRV + DRV_BACK) | 1), "dispatch, %s: the frame above the spawner: %#x, %08x",
			w, rec(REC_SPAWN, 5), rec(REC_SPAWN, 6));
		/* back: r0 and r1 as the spawner left them, r2-r4 kept, r5 the object; its side byte its game's type */
		CHECK(rec(REC_AFTER, 0) == 0x77 && rec(REC_AFTER, 1) == 0x66 && rec(REC_AFTER, 2) == 0x22 && rec(REC_AFTER, 4) == 0x44 &&
			rec(REC_AFTER, 5) == runs[k].obj, "dispatch, %s: back r0 %#x r1 %#x r5 %08x", w, rec(REC_AFTER, 0), rec(REC_AFTER, 1), rec(REC_AFTER, 5));
		CHECK(emu_read8(SIDE3) == runs[k].side, "dispatch, %s: the side byte %#x, not %#x", w, emu_read8(SIDE3), runs[k].side);
	}
	/* the moved routine ran on the dispatch's type as BN5's runner runs it (the last run's: the stray's freed instead) */
	CHECK(rec(REC_FREE, 0) == OBJ, "dispatch: the stray not let go (%08x)", rec(REC_FREE, 0));
	CHECK(rec(REC_MOVED, 0) == 0, "dispatch: the stray ran the moved routine");
	driver(0x25, OBJ, true, 0xD15A0010u);
	emu_reset();
	emu_frame(0);
	emu_sync();
	CHECK(rec(REC_MOVED, 0) == (MOVED_RT | 1) && rec(REC_MOVED, 1) == 0x25 * 4 && rec(REC_MOVED, 2) == OBJ && rec(REC_MOVED, 3) == DRV + DRV_RAN,
		"dispatch: the moved routine met r0 %08x, r1 %#x, r5 %08x, lr %08x", rec(REC_MOVED, 0), rec(REC_MOVED, 1), rec(REC_MOVED, 2), rec(REC_MOVED, 3));
	emu_write32(0x080000C8, 0x080000D1);
}

/* ---- the music or the effects silenced (sound_quiet.c) ---- */

/* BN6's player rows (BN6_MPLAYS) for two players, an effect's (row 0) and
 * the music's (row 31), a track each; a stand-in for TrkVolPitSet that
 * works a track's volume out as 0x40, right and left, where its flags ask
 * (BN6_TRACK_VOL_SET) and clears what it did; a driver that calls it on
 * the music's track and then the effect's */
#define Q_EFFECT 0x02006E08u
#define Q_MUSIC 0x02007EE8u
#define Q_DRV 0x08151000u
#define Q_MARK (IWRAM + 0x4C0)

static void quiet_rom(void) {
	static uint8_t rows[BN6_MPLAY_COUNT * BN6_MPLAY_ROW];
	put32(rows, 0, 0x02010690);
	put32(rows, BN6_MPLAY_TRACKS, Q_EFFECT);
	rows[BN6_MPLAY_TRACK_COUNT] = 1;
	uint32_t m = (BN6_MPLAY_COUNT - 1) * BN6_MPLAY_ROW;
	put32(rows, m, BN6_MUSIC_PLAYER);
	put32(rows, m + BN6_MPLAY_TRACKS, Q_MUSIC);
	rows[m + BN6_MPLAY_TRACK_COUNT] = 1;
	emu_write(BN6_MPLAYS, rows, sizeof rows);
	/* ldrb r2,[r1]; lsls r3,r2,#31; beq; movs r3,#0x40; strb r3,[r1,#0x10]; strb r3,[r1,#0x11];
	 * movs r3,#0xF0; ands r2,r3; strb r2,[r1]; mov pc,lr (the change's bits cleared, as MPlayMain clears them after) */
	const uint16_t vol[10] = { 0x780A, 0x07D3, 0xD002, 0x2340, 0x740B, 0x744B, 0x23F0, 0x401A, 0x700A, 0x46F7 };
	code_at(BN6_M4A_TRK_VOL_PIT_SET, vol, 10, NULL, 0, 0x14);
	/* ldr r1,=Q_MUSIC; bl; ldr r1,=Q_EFFECT; bl; ldr r0,=Q_MARK; movs r1,#0x5A; str r1,[r0]; b . */
	uint16_t ops[12] = { ldr_lit(Q_DRV, 1, Q_DRV + 0x18), 0, 0, ldr_lit(Q_DRV + 0x06, 1, Q_DRV + 0x1C), 0, 0,
		ldr_lit(Q_DRV + 0x0C, 0, Q_DRV + 0x20), 0x215A, 0x6001, 0xE7FE, 0x46C0, 0x46C0 };
	thumb_bl_encode(Q_DRV + 0x02, BN6_M4A_TRK_VOL_PIT_SET, &ops[1], &ops[2]);
	thumb_bl_encode(Q_DRV + 0x08, BN6_M4A_TRK_VOL_PIT_SET, &ops[4], &ops[5]);
	const uint32_t lits[3] = { Q_MUSIC, Q_EFFECT, Q_MARK };
	code_at(Q_DRV, ops, 12, lits, 3, 0x18);
}

/* the driver's frame from the reset, both tracks playing with their
 * volume to work out (0x81) and none yet */
static void quiet_run(bool music, bool effects) {
	sound_heard_set(music, effects);
	emu_reset();
	for (int k = 0; k < 2; ++k) {
		uint32_t t = k ? Q_EFFECT : Q_MUSIC;
		emu_write8(t + BN6_TRACK_FLAGS, BN6_TRACK_EXISTS | BN6_TRACK_VOL_SET);
		emu_write8(t + BN6_TRACK_VOL_RIGHT, 0);
		emu_write8(t + BN6_TRACK_VOL_LEFT, 0);
	}
	emu_write32(Q_MARK, 0);
	emu_frame(0);
	emu_sync();
}

static void quiet_players(void) {
	quiet_rom();
	emu_write32(0x080000C8, Q_DRV | 1);
	sound_quiet_install();
	CHECK(emu_read16(BN6_M4A_TRK_VOL_PIT_SET) == 0xBECE, "the hook on TrkVolPitSet");
	static const struct { bool music, effects; uint8_t music_vol, effect_vol; } runs[4] = {
		{ true, true, 0x40, 0x40 }, { false, true, 0, 0x40 }, { true, false, 0x40, 0 }, { false, false, 0, 0 },
	};
	for (int k = 0; k < 4; ++k) {
		quiet_run(runs[k].music, runs[k].effects);
		CHECK(emu_read32(Q_MARK) == 0x5A, "quiet %d: the driver did not run to its end", k);
		CHECK(emu_read8(Q_MUSIC + BN6_TRACK_VOL_RIGHT) == runs[k].music_vol && emu_read8(Q_MUSIC + BN6_TRACK_VOL_LEFT) == runs[k].music_vol &&
			emu_read8(Q_EFFECT + BN6_TRACK_VOL_RIGHT) == runs[k].effect_vol && emu_read8(Q_EFFECT + BN6_TRACK_VOL_LEFT) == runs[k].effect_vol,
			"quiet %d: the music's volume %#x %#x, the effect's %#x %#x", k, emu_read8(Q_MUSIC + BN6_TRACK_VOL_RIGHT),
			emu_read8(Q_MUSIC + BN6_TRACK_VOL_LEFT), emu_read8(Q_EFFECT + BN6_TRACK_VOL_RIGHT), emu_read8(Q_EFFECT + BN6_TRACK_VOL_LEFT));
		/* (worked out, silenced or not: nothing left to do) */
		CHECK(emu_read8(Q_MUSIC + BN6_TRACK_FLAGS) == BN6_TRACK_EXISTS && emu_read8(Q_EFFECT + BN6_TRACK_FLAGS) == BN6_TRACK_EXISTS,
			"quiet %d: the flags %#x %#x", k, emu_read8(Q_MUSIC + BN6_TRACK_FLAGS), emu_read8(Q_EFFECT + BN6_TRACK_FLAGS));
	}
	/* a change tells the players it touches before the next frame (the
	 * driver loops at its end since), and a state's load tells them all */
	sound_heard_set(false, true);
	emu_frame(0);
	emu_sync();
	CHECK(emu_read8(Q_EFFECT + BN6_TRACK_FLAGS) == (BN6_TRACK_EXISTS | BN6_TRACK_VOL_CHANGED) &&
		emu_read8(Q_MUSIC + BN6_TRACK_FLAGS) == BN6_TRACK_EXISTS, "the effects turned on: the flags %#x (music) %#x (effect)",
		emu_read8(Q_MUSIC + BN6_TRACK_FLAGS), emu_read8(Q_EFFECT + BN6_TRACK_FLAGS));
	sound_quiet_loaded();
	emu_frame(0);
	emu_sync();
	CHECK(emu_read8(Q_MUSIC + BN6_TRACK_FLAGS) == (BN6_TRACK_EXISTS | BN6_TRACK_VOL_CHANGED), "after a load: the music's flags %#x",
		emu_read8(Q_MUSIC + BN6_TRACK_FLAGS));
	/* (a track that does not play is left alone) */
	emu_write8(Q_MUSIC + BN6_TRACK_FLAGS, 0);
	sound_heard_set(true, true);
	emu_frame(0);
	emu_sync();
	CHECK(emu_read8(Q_MUSIC + BN6_TRACK_FLAGS) == 0, "a track that does not play told: %#x", emu_read8(Q_MUSIC + BN6_TRACK_FLAGS));
	emu_write32(0x080000C8, 0x080000D1);
}

/* ---- RAM kept patched (emu.c's emu_keep, issue #82) ---- */

/* As the engine keeps IWRAM's copy of the game's code patched: the
 * engine's bytes made where the game's own are, a state saved with the
 * game's own there (another core reads them so), the engine's made again
 * as one is loaded, and bytes that read neither left as they are */
#define KEPT (IWRAM + 0x200)
#define KEPT_STATE "build/host/test_emu.state"

static uint32_t state_word(uint32_t addr) {
	static uint8_t rom[ROM_SIZE];
	static uint32_t video[240 * 160];
	make_rom(rom);
	struct mCore *g = guest_core(rom, video);
	struct VFile *vf = VFileOpen(KEPT_STATE, O_RDONLY);
	uint32_t v = g && vf && mCoreLoadStateNamed(g, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC) ? g->rawRead32(g, addr, -1) : 0xDEADBEEF;
	if (vf) vf->close(vf);
	if (g) {
		mCoreConfigDeinit(&g->config);
		g->deinit(g);
	}
	return v;
}

static void kept_ram(void) {
	static const uint8_t own[4] = { 0x11, 0x11, 0x11, 0x11 }, ours[4] = { 0x22, 0x22, 0x22, 0x22 };
	emu_write32(KEPT, 0x11111111);
	CHECK(emu_keep(KEPT, own, ours, sizeof own), "a patch kept");
	CHECK(emu_read32(KEPT) == 0x22222222, "the patch made: %08x", emu_read32(KEPT));
	CHECK(emu_save_state(KEPT_STATE), "a state saved");
	CHECK(emu_read32(KEPT) == 0x22222222, "the patch after the save: %08x", emu_read32(KEPT));
	CHECK(state_word(KEPT) == 0x11111111, "the state holds %08x, not the game's own 11111111", state_word(KEPT));
	emu_write32(KEPT, 0x33333333);
	CHECK(emu_load_state(KEPT_STATE) && emu_read32(KEPT) == 0x22222222, "the patch after the state loaded: %08x", emu_read32(KEPT));
	/* (another's bytes there: neither the game's nor the patch's) */
	emu_write32(KEPT, 0x44444444);
	CHECK(emu_save_state(KEPT_STATE) && state_word(KEPT) == 0x44444444 && emu_read32(KEPT) == 0x44444444, "another's bytes saved as %08x, left %08x",
		state_word(KEPT), emu_read32(KEPT));
	CHECK(emu_load_state(KEPT_STATE) && emu_read32(KEPT) == 0x44444444, "another's bytes loaded made %08x", emu_read32(KEPT));
	remove(KEPT_STATE);
}

/* ---- the installers' claims through emu.c (claims.h, issue #82) ----
 * emu_write's, emu_write32's and emu_write8's bytes the installer's that
 * writes them, another's over them told, a write outside any installer
 * nobody's, and a hook set on an installer's bytes told */
/* What hooks keep a core between calls, named once, copied out and back by slot */
static void kept_slots(void) {
	static uint32_t ways[HOOK_CORES][2];
	static uint8_t dues[HOOK_CORES];
	size_t before = hook_kept_size();
	CHECK(hook_keep_slots(ways, sizeof *ways) && hook_keep_slots(dues, sizeof *dues) && hook_keep_slots(ways, sizeof *ways),
		"the kept arrays named");
	CHECK(hook_kept_size() == before + 9, "a core's kept bytes: %zu", hook_kept_size() - before);
	ways[1][0] = 0x080000F8;
	ways[1][1] = 0x03007F00;
	dues[1] = 7;
	ways[2][0] = 5;
	uint8_t snap[64];
	hook_kept_save(1, snap);
	ways[1][0] = dues[1] = 0;
	ways[1][1] = 1;
	hook_kept_load(1, snap);
	CHECK(ways[1][0] == 0x080000F8 && ways[1][1] == 0x03007F00 && dues[1] == 7 && ways[2][0] == 5, "slot 1's put back, slot 2's left");
	hook_kept_load(HOOK_CORES, snap);   /* (no such slot: nothing) */
	CHECK(ways[2][0] == 5, "a slot past the table touches nothing");
}

static void claimed_writes(void) {
	static const uint8_t b[16] = { 0 };
	int sits = 0;
	claims_reset();
	claims_quiet(true);
	claims_begin("x");
	emu_write(0x08900000u, b, sizeof b);
	claims_end();
	CHECK(claims_who(0x0890000Fu) && !strcmp(claims_who(0x0890000Fu), "x"), "claims: emu_write's last byte not the installer's");
	claims_begin("y");
	emu_write32(0x0890000Cu, 0);
	emu_write8(0x08900020u, 0);
	claims_end();
	emu_write8(0x08900030u, 0);
	CHECK(claims_overlaps() == 1 && claims_who(0x08900020u) && !strcmp(claims_who(0x08900020u), "y") && !claims_who(0x08900030u),
		"claims: %d overlaps told, the bytes %s's and %s's", claims_overlaps(), claims_who(0x08900020u) ? claims_who(0x08900020u) : "no one",
		claims_who(0x08900030u) ? claims_who(0x08900030u) : "no one");
	CHECK(emu_hook(0x08900004u, counted, &sits) && claims_overlaps() == 2, "claims: a hook on an installer's bytes not told (%d)", claims_overlaps());
	emu_unhook(0x08900004u);
	claims_reset();
	claims_quiet(false);
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

	full_table(core);

	/* a halt from a hook: the loop at 0xF0 met once, then the CPU idle for
	 * the rest of the frame (no interrupt is on to end it) */
	int halts = 0;
	CHECK(hook_add(0x080000F0, halting, &halts), "a hook on the loop at 0xF0");
	core->reset(core);
	core->runFrame(core);
	CHECK(halts == 1 && ((struct GBA *)core->board)->cpu->halted, "halted: the hook ran %d times, halted %d", halts, ((struct GBA *)core->board)->cpu->halted);
	CHECK(stored(core, 3) == 39, "the code before the halt ran: %u", stored(core, 3));
	hook_remove(0x080000F0);
	hook_detach(core);
	mCoreConfigDeinit(&core->config);
	core->deinit(core);

	two_cores();
	two_threads();
	save_queue_checks();
	moved_block();
	dispatch_run();
	quiet_players();
	kept_ram();
	claimed_writes();
	kept_slots();
	if (failures) { printf("test_emu: %d failed\n", failures); return 1; }
	printf("all hook checks passed, the two cores kept apart, their hooks each their own, on one thread and on two, a moved block's calls through veneers, "
		"the second-level dispatch's code, the music and the effects silenced apart, RAM kept patched but in its states, the installers' writes claimed, what hooks keep a core put back\n");
	return 0;
}
