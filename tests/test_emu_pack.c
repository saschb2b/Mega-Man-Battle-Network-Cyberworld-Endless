/* The chip pack's routines (src/emu/xchips_pack.c) run on mGBA without the
 * game's ROM, through emu.c's core on test_emu.c's ROM: the block written
 * at CHIPS_CODE as the installer writes it, and drivers of the test's own
 * beside it that call it as BN6's code does.
 * - A chip's entry for every id to 1099, against bn6_pack_entry.
 * - A chip given as GiveChips gives one (the count's address from the
 *   offset routine, a count stored by a byte, its order stamp set), then
 *   read back by the stamp's reader, the total and the totals' count: in
 *   BN6's own pack, in the extension and in the cart's save RAM, where a
 *   halfword read gives one byte twice and BN6's own stamp routines would
 *   read 0x1212 for 0x7F12.
 * - The pack's clear: BN6's ZeroFillByWord called on the pack and on the
 *   extension, the save RAM's entries zeroed by the routine's own loop, and
 *   by its hook with one byte left to the loop.
 * - The save RAM's entries through a state written and loaded (as a
 *   checkpoint's and a .cwsave's are: emu_save_state), a state from before
 *   them (0xFF there) zeroed as it loads, and beside a core's raw state,
 *   which leaves the save RAM out (a NetBattle's rollback snapshot). */
#include "test_emu_pack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/config.h>
#include <mgba/core/core.h>
#include <mgba-util/vfs.h>

#include "chips.h"
#include "emu.h"
#include "hook.h"
#include "thumb_reloc.h"
#include "xchips_pack.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

#define TK 0x02000100u        /* the toolkit, r10: its +0x4C the pack */
#define PACK 0x02002230u
#define REC 0x02030000u       /* what a driver stores */
#define DONE 0x02034000u      /* ... and its mark, once at its end */
#define ZLOG 0x02034010u      /* the stand-in ZeroFillByWord's calls: its next place, then r0 and r1 a call */
#define ZERO_FILL 0x080008E0u /* (the block's literal, BN6's ZeroFillByWord) */
#define WRAP (CHIPS_CODE + 0x180u)   /* BN6's callers' frames: the routines are their tails */
#define DRV (CHIPS_CODE + 0x200u)
#define IDS 1100u
#define PACK_STATE "build/host/test_emu_pack.state"

/* a little Thumb, its literals after it */
typedef struct {
	uint32_t base;
	uint16_t ops[120];
	int n, nlits;
	struct { int at, rd; uint32_t v; } lits[32];
} Asm;

static void op(Asm *a, uint16_t v) { a->ops[a->n++] = v; }
static void lit(Asm *a, int rd, uint32_t v) {
	a->lits[a->nlits].at = a->n; a->lits[a->nlits].rd = rd; a->lits[a->nlits++].v = v;
	op(a, 0);
}
static void call(Asm *a, uint32_t to) {
	uint16_t hi = 0, lo = 0;
	if (!thumb_bl_encode(a->base + 2u * (uint32_t)a->n, to, &hi, &lo)) { ++failures; printf("FAIL: a call past a bl's reach\n"); }
	op(a, hi);
	op(a, lo);
}
static uint32_t here(const Asm *a) { return a->base + 2u * (uint32_t)a->n; }
static void put(Asm *a) {
	if (a->n & 1) op(a, 0x46C0);
	uint32_t pool = here(a);
	uint8_t b[sizeof a->ops + 4 * 32];
	for (int i = 0; i < a->nlits; ++i) {
		uint32_t at = a->base + 2u * (uint32_t)a->lits[i].at, to = pool + 4u * (uint32_t)i;
		a->ops[a->lits[i].at] = (uint16_t)(0x4800 | a->lits[i].rd << 8 | (to - ((at + 4) & ~3u)) / 4);
		for (int k = 0; k < 4; ++k) b[2 * a->n + 4 * i + k] = (uint8_t)(a->lits[i].v >> (8 * k));
	}
	for (int i = 0; i < a->n; ++i) { b[2 * i] = (uint8_t)a->ops[i]; b[2 * i + 1] = (uint8_t)(a->ops[i] >> 8); }
	emu_write(a->base, b, (size_t)(2 * a->n + 4 * a->nlits));
}

/* r10 the toolkit, its pack; the stand-in's log at its start */
static void begin(Asm *a, uint32_t base) {
	memset(a, 0, sizeof *a);
	a->base = base;
	lit(a, 0, TK); op(a, 0x4682);                    /* mov r10,r0 */
	lit(a, 1, PACK); op(a, 0x64C1);                  /* str r1,[r0,#0x4C] */
	lit(a, 0, ZLOG); lit(a, 1, ZLOG + 4); op(a, 0x6001);   /* str r1,[r0] */
}
static void end(Asm *a, uint32_t mark) {
	lit(a, 0, DONE); lit(a, 1, mark); op(a, 0x6001);
	op(a, 0xE7FE);                                   /* b . */
}

static uint32_t w_offset, w_total, w_clear;

/* the block, BN6's ZeroFillByWord's stand-in (its calls written down), and the callers' frames */
static void install(void) {
	uint8_t code[XPACK_SIZE];
	xchips_pack_code(code);
	emu_write(CHIPS_CODE, code, sizeof code);
	Asm z = { .base = ZERO_FILL };
	op(&z, 0xB410); lit(&z, 3, ZLOG); op(&z, 0x681C);   /* push {r4}; ldr r3,=log; ldr r4,[r3] */
	op(&z, 0x6020); op(&z, 0x6061); op(&z, 0x3408);     /* str r0,[r4]; str r1,[r4,#4]; adds r4,#8 */
	op(&z, 0x601C); op(&z, 0xBC10); op(&z, 0x4770);     /* str r4,[r3]; pop {r4}; bx lr */
	put(&z);
	Asm w = { .base = WRAP };
	w_offset = here(&w); op(&w, 0xB590); lit(&w, 7, (CHIPS_CODE + XPACK_OFFSET) | 1); op(&w, 0x4738);   /* push {r4,r7,lr}; bx r7 */
	w_total = here(&w); op(&w, 0xB580); lit(&w, 7, (CHIPS_CODE + XPACK_TOTAL) | 1); op(&w, 0x4738);     /* push {r7,lr}; bx r7 */
	w_clear = here(&w); op(&w, 0xB500); lit(&w, 0, PACK); lit(&w, 1, 0xF00);                            /* push {lr}; as zeroFill_e2002230 */
	lit(&w, 2, (CHIPS_CODE + XPACK_CLEAR) | 1); op(&w, 0x4710);                                         /* bx r2 */
	put(&w);
}

static bool run(uint32_t mark, int frames) {
	emu_write32(0x080000C8, DRV | 1);
	emu_reset();
	for (int i = 0; i < frames; ++i) emu_frame(0);
	emu_sync();
	emu_write32(0x080000C8, 0x080000D1);
	return emu_read32(DONE) == mark;
}

static void entries(void) {
	Asm a;
	begin(&a, DRV);
	op(&a, 0x2400); lit(&a, 5, REC); lit(&a, 6, IDS);   /* movs r4,#0 */
	int loop = a.n;
	op(&a, 0x1C20); call(&a, CHIPS_CODE + XPACK_ENTRY);  /* adds r0,r4,#0 */
	op(&a, 0x6028); op(&a, 0x3504); op(&a, 0x3401); op(&a, 0x42B4);   /* str r0,[r5]; adds r5,#4; adds r4,#1; cmp r4,r6 */
	op(&a, (uint16_t)(0xDB00 | ((loop - (a.n + 2)) & 0xFF)));          /* blt loop */
	end(&a, 0xE0000001u);
	put(&a);
	CHECK(run(0xE0000001u, 2), "pack: the entries' driver did not run to its end");
	int wrong = 0;
	unsigned first = 0;
	for (unsigned id = 0; id < IDS; ++id) {
		uint32_t want = bn6_pack_entry(PACK, id), got = emu_read32(REC + 4 * id);
		if (!want) want = CHIPS_CODE + 0xD8;   /* (the 12 zero bytes in the ROM) */
		if (got != want && !wrong++) first = id;
	}
	CHECK(!wrong, "pack: %d ids' entries not bn6_pack_entry's, the first %u (%08x)", wrong, first, emu_read32(REC + 4 * first));
	CHECK(emu_read32(REC + 4 * 700) == 0x0E006D1Cu && emu_read32(REC + 4 * 634) == CHIPS_CODE + 0xD8 && emu_read32(REC + 4 * 1024) == CHIPS_CODE + 0xD8,
		"pack: 700's entry %08x, 634's %08x, 1024's %08x", emu_read32(REC + 4 * 700), emu_read32(REC + 4 * 634), emu_read32(REC + 4 * 1024));
}

/* chip `id` given `count` in its record's code `place`, as GiveChips gives one, and read back; `zeroed`: its stamp zeroed before the last read */
static void give(unsigned id, unsigned place, unsigned count, bool zeroed, uint32_t mark) {
	Asm a;
	begin(&a, DRV);
	lit(&a, 0, BN6_PACK_STAMP_NEXT); lit(&a, 1, 0x7F12); op(&a, 0x8001);            /* strh r1,[r0] */
	lit(&a, 2, id); op(&a, (uint16_t)(0x2300 | place)); call(&a, w_offset); op(&a, 0x1C04);   /* the count's address, kept in r4 */
	op(&a, (uint16_t)(0x2100 | count)); op(&a, 0x7021);                              /* strb r1,[r4] */
	op(&a, 0x1C20); op(&a, (uint16_t)(0x2100 | place)); call(&a, CHIPS_CODE + XPACK_STAMP_SET);
	op(&a, 0x1C20); op(&a, (uint16_t)(0x2100 | place)); call(&a, CHIPS_CODE + XPACK_STAMP_GET);
	lit(&a, 5, REC); op(&a, 0x6028); op(&a, 0x606C);                                 /* the stamp read, the count's address */
	lit(&a, 0, id); call(&a, w_total); op(&a, 0x60A8);                               /* its total */
	lit(&a, 2, id); op(&a, (uint16_t)(0x2300 | place)); call(&a, CHIPS_CODE + XPACK_COUNT); op(&a, 0x60E9);   /* the totals' count */
	if (zeroed) { op(&a, 0x1C20); op(&a, (uint16_t)(0x2100 | place)); call(&a, CHIPS_CODE + XPACK_STAMP_CLEAR); }
	op(&a, 0x1C20); op(&a, (uint16_t)(0x2100 | place)); call(&a, CHIPS_CODE + XPACK_STAMP_GET); op(&a, 0x6128);
	end(&a, mark);
	put(&a);
	CHECK(run(mark, 2), "pack: chip %u's driver did not run to its end", id);
}

static void given(unsigned id, bool in_save) {
	const unsigned place = 1, count = 5;
	uint32_t at = bn6_pack_entry(PACK, id);
	if (in_save) for (uint32_t k = 0; k < BN6_PACK_ENTRY; ++k) emu_write8(at + k, 0);
	give(id, place, count, false, 0xE0000100u + id);
	CHECK(emu_read32(REC + 4) == at + place, "pack, chip %u: the count's address %08x, not %08x", id, emu_read32(REC + 4), at + place);
	CHECK(emu_read8(at + place) == count && !emu_read8(at) && !emu_read8(at + 2) && !emu_read8(at + 3), "pack, chip %u: its counts %u %u %u %u", id,
		emu_read8(at), emu_read8(at + 1), emu_read8(at + 2), emu_read8(at + 3));
	CHECK(emu_read8(at + 4 + 2 * place) == 0x12 && emu_read8(at + 5 + 2 * place) == 0x7F && !emu_read8(at + 4) && !emu_read8(at + 8),
		"pack, chip %u: its stamp's bytes %02x %02x", id, emu_read8(at + 4 + 2 * place), emu_read8(at + 5 + 2 * place));
	CHECK(emu_read32(REC) == 0x7F12 && emu_read32(REC + 16) == 0x7F12, "pack, chip %u: its stamp read as %04x, %04x", id, emu_read32(REC), emu_read32(REC + 16));
	CHECK(emu_read16(BN6_PACK_STAMP_NEXT) == 0x7F11, "pack, chip %u: the next stamp %04x, not 7f11", id, emu_read16(BN6_PACK_STAMP_NEXT));
	CHECK(emu_read32(REC + 8) == count && emu_read32(REC + 12) == count, "pack, chip %u: its total %u, the totals' count %u", id, emu_read32(REC + 8),
		emu_read32(REC + 12));
	if (!in_save) return;
	/* (the save RAM alone outlives the reset a driver starts with: given again, the count adds nothing here, the stamp zeroed) */
	give(id, place, count, true, 0xE0000200u + id);
	CHECK(emu_read32(REC) == 0x7F12 && !emu_read32(REC + 16) && !emu_read8(at + 4 + 2 * place) && !emu_read8(at + 5 + 2 * place) &&
		emu_read8(at + place) == count, "pack, chip %u: its stamp zeroed reads %04x", id, emu_read32(REC + 16));
}

static void cleared(bool hooked) {
	const uint32_t first = BN6_PACK_SAVE, last = BN6_PACK_SAVE + BN6_PACK_SAVE_BYTES - 1;
	for (uint32_t a = first; a <= last; ++a) emu_write8(a, 0x5A);
	emu_write8(BN6_PACK_SAVE_MARK + 15, 1);
	emu_write8(last + 1, 0x77);   /* (the byte past them, the region's own still: left alone) */
	Asm a;
	begin(&a, DRV);
	call(&a, w_clear);
	end(&a, 0xE0000300u + hooked);
	put(&a);
	uint32_t hits = hook_hits;
	CHECK(run(0xE0000300u + hooked, 2), "pack: the clear's driver did not run to its end");
	int left = 0;
	for (uint32_t k = first; k <= last; ++k) left += emu_read8(k) != 0;
	CHECK(!left && emu_read8(last + 1) == 0x77 && emu_read8(BN6_PACK_SAVE_MARK) == 'C' && emu_read8(BN6_PACK_SAVE_MARK + 15) == 1,
		"pack: the clear%s left %d of the save RAM's entries' bytes, the byte past them %02x", hooked ? ", hooked," : "", left, emu_read8(last + 1));
	CHECK(emu_read32(ZLOG) == ZLOG + 4 + 16 && emu_read32(ZLOG + 4) == PACK && emu_read32(ZLOG + 8) == 0xF00 && emu_read32(ZLOG + 12) == BN6_PACK_EXT &&
		emu_read32(ZLOG + 16) == 0x900, "pack: the clear's two calls of BN6's fill: %08x %x, %08x %x", emu_read32(ZLOG + 4), emu_read32(ZLOG + 8),
		emu_read32(ZLOG + 12), emu_read32(ZLOG + 16));
	CHECK(hook_hits - hits == (hooked ? 1u : 0u), "pack: the clear's hook met %u times", hook_hits - hits);
	emu_write8(last + 1, 0);
}

static void states(void) {
	const uint32_t at = bn6_pack_entry(PACK, 700);
	give(700, 1, 5, false, 0xE0000400u);
	CHECK(emu_read8(at + 1) == 5 && emu_read8(at + 6) == 0x12, "pack: chip 700 given before the state");
	CHECK(emu_save_state(PACK_STATE), "pack: a state saved");
	emu_write8(at + 1, 0);
	emu_write8(at + 6, 0);
	CHECK(emu_load_state(PACK_STATE) && emu_read8(at + 1) == 5 && emu_read8(at + 6) == 0x12 && emu_read8(at + 7) == 0x7F,
		"pack: the state loaded gives chip 700's count %u, its stamp %02x%02x", emu_read8(at + 1), emu_read8(at + 7), emu_read8(at + 6));
	/* a state from before the entries: 0xFF there, as mGBA fills a save RAM nothing wrote */
	for (uint32_t a = BN6_PACK_SAVE_MARK; a < BN6_PACK_SAVE + BN6_PACK_SAVE_BYTES; ++a) emu_write8(a, 0xFF);
	emu_write8(BN6_PACK_SAVE_MARK - 1, 0xFF);
	CHECK(emu_save_state(PACK_STATE) && emu_read8(at + 1) == 0xFF, "pack: an old state saved");
	int left = 0;
	bool loaded = emu_load_state(PACK_STATE);
	for (uint32_t a = BN6_PACK_SAVE; a < BN6_PACK_SAVE + BN6_PACK_SAVE_BYTES; ++a) left += emu_read8(a) != 0;
	CHECK(loaded && !left && emu_read8(BN6_PACK_SAVE_MARK) == 'C' && emu_read8(BN6_PACK_SAVE_MARK - 1) == 0xFF,
		"pack: an old state loaded leaves %d bytes of the entries, the mark's first %02x", left, emu_read8(BN6_PACK_SAVE_MARK));
	remove(PACK_STATE);
}

/* a core's raw state, as a rollback snapshot keeps one, and the entries beside it */
static void snapshot(const uint8_t *rom, uint32_t rom_size, uint32_t *video) {
	struct VFile *vf = VFileMemChunk(rom, rom_size);
	struct mCore *c = vf ? mCoreFindVF(vf) : NULL;
	if (!c || !c->init(c)) { ++failures; printf("FAIL: pack: no core for the snapshot\n"); return; }
	mCoreInitConfig(c, NULL);
	c->setVideoBuffer(c, (color_t *)video, 240);
	bool made = c->loadROM(c, vf) && c->loadSave(c, VFileMemChunk(NULL, 0));
	c->reset(c);
	const uint32_t at = bn6_pack_entry(PACK, 733);
	CHECK(made && c->rawRead8(c, at, -1) == 0xFF, "pack: a new core's save RAM reads %02x before the entries are made", (unsigned)c->rawRead8(c, at, -1));
	CHECK(xchips_pack_ready(c) && !c->rawRead8(c, at, -1) && c->rawRead8(c, BN6_PACK_SAVE_MARK, -1) == 'C' && xchips_pack_ready(c),
		"pack: the entries made on a new core");
	c->rawWrite8(c, at, -1, 7);
	CHECK(xchips_pack_ready(c) && c->rawRead8(c, at, -1) == 7, "pack: the entries made twice keep what they hold");
	uint8_t kept[XPACK_KEPT];
	xchips_pack_keep(c, kept);
	size_t n = c->stateSize(c);
	void *state = malloc(n);
	CHECK(state && c->saveState(c, state), "pack: a raw state");
	c->rawWrite8(c, at, -1, 9);
	CHECK(state && c->loadState(c, state) && c->rawRead8(c, at, -1) == 9, "pack: a raw state loaded changed the save RAM (%02x)",
		(unsigned)c->rawRead8(c, at, -1));
	xchips_pack_put(c, kept);
	CHECK(c->rawRead8(c, at, -1) == 7 && c->rawRead8(c, BN6_PACK_SAVE_MARK, -1) == 'C', "pack: the entries put back beside the state: %02x",
		(unsigned)c->rawRead8(c, at, -1));
	free(state);
	mCoreConfigDeinit(&c->config);
	c->deinit(c);
}

int pack_checks(const uint8_t *rom, uint32_t rom_size, uint32_t *video) {
	/* (emu_init made the entries: a fresh save RAM's 0xFF zeroed, the mark before them) */
	int left = 0;
	for (uint32_t a = BN6_PACK_SAVE; a < BN6_PACK_SAVE + BN6_PACK_SAVE_BYTES; ++a) left += emu_read8(a) != 0;
	CHECK(!left && emu_read8(BN6_PACK_SAVE_MARK) == 'C' && emu_read8(BN6_PACK_SAVE_MARK - 1) == 0xFF && emu_read8(BN6_PACK_SAVE + BN6_PACK_SAVE_BYTES) == 0xFF,
		"pack: the game's core's entries made as it starts (%d bytes not zero)", left);
	install();
	entries();
	given(5, false);     /* BN6's own pack */
	given(600, false);   /* the extension, in the entries of 320 on */
	given(700, true);    /* the cart's save RAM */
	given(1023, true);
	cleared(false);
	CHECK(emu_hook(CHIPS_CODE + XPACK_CLEAR_HOOK, xchips_pack_cleared, NULL), "pack: the clear's hook");
	cleared(true);
	emu_unhook(CHIPS_CODE + XPACK_CLEAR_HOOK);
	states();
	snapshot(rom, rom_size, video);
	return failures;
}
