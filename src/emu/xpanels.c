/* xpanels.h: BN5's lava and sea as BN6's engine runs them. Each of BN5's
 * routines for them (bn5.h: BN5_LAVA_STEP, BN5_PANEL_STEP, BN5_PANEL_HIT,
 * BN5_PANEL_UPDATE, BN5_SET_PANEL_TYPE) has a twin in BN6 that knows
 * neither: the twins are hooked where BN5's test or call for them, BN5's
 * rules in C, and what must run the game's own routines (the panel cooled,
 * its spark) a few instructions of the project's own Thumb. IWRAM's
 * routines are patched in their ROM image, which the game copies as it
 * powers on, and kept so in its RAM (emu_keep), out of its states. */
#include "xpanels.h"

#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"
#include "thumb_reloc.h"
#include "xpanels_look.h"
#include "xvirus_recipe.h"

#define BUS 0x08000000u

/* BN5's rules (bn5.h's routines; docs/rom/) */
#define ELEMENT_FIRE   1           /* BN6_COLL_ELEMENT: Fire */
#define ELEMENT_ELEC   3           /* ... Elec */
#define LAVA_DAMAGE    50u         /* BN5_LAVA_STEP: movs r1,#0x32, shifted by the weakness to Fire */
#define LAVA_HIT_BITS  3           /* ... ORed into the hit's bits */
#define FLAGS_SPARED_1 0x20u       /* ... the object's flags (BN6_COLL_FLAGS) that spare it: these before the panel is read, */
#define FLAGS_SPARED_2 0x88000206u /* ... these after, */
#define FLAGS_UNHURT   0x09u       /* ... and these the damage, not the cooling */
#define MADE_FRAMES    0x3C0       /* BN5_SET_PANEL_TYPE and its field's start: lava or sea, 960 frames (BN6's conveyors' 0x708) */

/* What the checks expect of BN6's code before it is patched: the IWRAM
 * table's literal, and the volcano's and conveyors' tests' bytes (their
 * CRC32) */
#define FLAGS_IMAGE   (BN6_IWRAM_IMAGE + (BN6_PANEL_FLAGS_REF - BN6_IWRAM_CODE))
#define COOL_IMAGE    (BN6_IWRAM_IMAGE + (BN6_PANEL_HIT_COOL - BN6_IWRAM_CODE))
#define COOL_CRC      0xB21608DAu
#define BONUS_IMAGE   (BN6_IWRAM_IMAGE + (BN6_PANEL_BONUS_TESTS - BN6_IWRAM_CODE))
#define BONUS_CRC     0xA63E1015u

static bool ready;
bool xpanels_ready(void) { return ready; }

static uint32_t rom32(uint32_t a) { return get32(R.data + (a - BUS)); }
static uint16_t rom16(uint32_t a) { return get16(R.data + (a - BUS)); }

/* the panel at (column, row) of the field, 0 off it (bn6f getPanelDataOffset, IWRAM 0x03007954) */
static uint32_t panel_at(uint32_t x, uint32_t y) {
	return x - 1 < 6 && y - 1 < 3 ? BN6_FIELD_PANELS + (y * 8 + x) * BN6_PANEL_SIZE : 0;
}

static void add16(uint32_t a, uint32_t v) { hook_write16(a, (uint16_t)(hook_read16(a) + v)); }
static void or8(uint32_t a, uint8_t v) { hook_write8(a, (uint8_t)(hook_read8(a) | v)); }

/* ---- lava underfoot (BN5_LAVA_STEP) ----
 * BN5 steps an object on lava from each actor's pre-step before its status
 * step, and from four obstacles' and one object's: BN6's twins of those
 * places are hooked. Where the lava acts, its damage is written here and
 * the panel's cooling and spark run as BN5's do, through the stub
 * (stub_make), which comes back to the hooked instruction: that time it
 * runs. */

/* the lava acting on the object r5 names: its damage dealt; false where it does not act */
static bool lava_acts(const HookRegs *r) {
	if (hook_read16(BN6_BATTLE_FLAGS) & BN6_TIME_STOP) return false;
	uint32_t coll = hook_read32(r->r[5] + BN6_T1_COLLISION);
	if (coll < BN6_EWRAM || coll >= BN6_EWRAM_END || !hook_read8(coll + BN6_COLL_REGION)) return false;
	uint8_t element = hook_read8(coll + BN6_COLL_ELEMENT);
	uint32_t flags = hook_read32(coll + BN6_COLL_FLAGS);
	if (element == ELEMENT_FIRE || (flags & FLAGS_SPARED_1)) return false;
	uint32_t p = panel_at(hook_read8(coll + BN6_COLL_PANEL_X), hook_read8(coll + BN6_COLL_PANEL_Y));
	if (!p || hook_read8(p + BN6_PANEL_TYPE) != XPANEL_LAVA || (flags & FLAGS_SPARED_2)) return false;
	if (!(flags & FLAGS_UNHURT)) {
		uint32_t damage = LAVA_DAMAGE << (hook_read8(BN6_WEAKNESS + 5u * element + ELEMENT_FIRE) & 31);
		add16(coll + BN6_COLL_DAMAGE_ADD, damage);
		add16(coll + BN6_COLL_FIRE_DAMAGE, damage);
		/* (BN5 ORs them into its +0x18 and +0x19 too, two bytes it ORs into
		 * +0x0F, where BN6 keeps the hit's bits alone: tests/xdiff/legit.json,
		 * the collision data's bn5_or) */
		or8(coll + BN6_COLL_HIT, LAVA_HIT_BITS);
	}
	return true;
}

/* the hooked instruction the stub went back to, on the stack it left: it
 * runs (one a core: the Arena's two run in turn, a run loop at a time) */
typedef struct { uint32_t at, sp; } Back;
static Back backs[HOOK_CORES];

static HookAct lava_site(HookRegs *r, void *user) {
	(void)user;
	Back *back = &backs[hook_slot()];
	if (back->at == r->pc && back->sp == r->sp) {
		back->at = 0;
		return HOOK_CONTINUE;
	}
	if (!lava_acts(r)) return HOOK_CONTINUE;
	back->at = r->pc;
	back->sp = r->sp;
	r->r[3] = r->pc | 1;
	r->r[12] = XP_STUB | 1;
	return HOOK_JUMP;
}

/* BN6's routines the stub calls (bn6_panels.h), each through a veneer */
enum { CALL_SET_TYPE, CALL_COORDS, CALL_JITTER, CALL_SPARK, CALLS };
static const uint32_t calls[CALLS] = { BN6_SET_PANEL_TYPE, BN6_PANEL_COORDS, BN6_PLACE_JITTER, BN6_SPAWN_SPARK };
#define STUB_CODE 0x30
#define STUB_SIZE (STUB_CODE + CALLS * THUMB_VENEER_SIZE)
_Static_assert(XP_STUB + STUB_SIZE <= XP_TILES, "the lava's cooling past its room (xpanels_look.h)");

static uint16_t imm(ThumbForm form, int rd, int rn, int value) {
	ThumbImm in = { form, (uint8_t)rd, (uint8_t)rn, (uint16_t)value };
	uint16_t insn = 0;
	thumb_imm_encode(&in, &insn);
	return insn;
}

/* The lava's cooling, as BN5's step ends (BN5_LAVA_STEP from 0x08016DE6):
 * the object's panel set plain, its place on the screen moved by a random
 * amount, a spark spawned there; entered with r5 the object, r3 the hooked
 * instruction (its Thumb bit set), lr as the code had it, and back there
 * with r3-r11, sp and lr as they were (r0-r2 and r12 not: BN5's step is a
 * call where BN6's code has none). Its veneers after it. */
static bool stub_make(uint8_t out[STUB_SIZE]) {
	const uint16_t code[STUB_CODE / 2] = {
		0xB598,                                            /* push {r3,r4,r7,lr} */
		imm(THUMB_LDR, 7, 5, BN6_T1_COLLISION),            /* ldr r7,[r5,#0x54]: its collision data */
		imm(THUMB_LDRB, 0, 7, BN6_COLL_PANEL_X),           /* ldrb r0,[r7,#0xa] */
		imm(THUMB_LDRB, 1, 7, BN6_COLL_PANEL_Y),           /* ldrb r1,[r7,#0xb] */
		imm(THUMB_MOVS, 2, 0, BN6_PANEL_PLAIN),            /* movs r2,#2 */
		0, 0,                                              /* bl set the panel's type */
		imm(THUMB_LDRB, 0, 7, BN6_COLL_PANEL_X),
		imm(THUMB_LDRB, 1, 7, BN6_COLL_PANEL_Y),
		0, 0,                                              /* bl its place on the screen */
		0x1C0A,                                            /* adds r2,r1,#0 */
		0x1C01,                                            /* adds r1,r0,#0 */
		imm(THUMB_MOVS, 3, 0, 0),                          /* movs r3,#0 */
		imm(THUMB_MOVS, 4, 0, 1),                          /* movs r4,#1 */
		imm(THUMB_MOVS, 0, 0, 0xF),                        /* movs r0,#0xf: the range */
		0, 0,                                              /* bl moved by a random amount */
		0, 0,                                              /* bl the spark */
		0xBC98,                                            /* pop {r3,r4,r7} */
		0xBC04,                                            /* pop {r2} */
		0x4696,                                            /* mov lr,r2 */
		0x4718,                                            /* bx r3 */
	};
	static const uint32_t bl_at[CALLS] = { 0x0A, 0x12, 0x20, 0x24 };
	memset(out, 0, STUB_SIZE);
	for (int i = 0; i < STUB_CODE / 2; ++i) put16(out + 2 * i, code[i]);
	for (int k = 0; k < CALLS; ++k) {
		uint32_t veneer = XP_STUB + STUB_CODE + (uint32_t)k * THUMB_VENEER_SIZE;
		if (!thumb_veneer(out + STUB_CODE + k * THUMB_VENEER_SIZE, veneer, calls[k]) || !thumb_bl_write(out, bl_at[k], XP_STUB, veneer)) return false;
	}
	return true;
}

/* ---- the sea underfoot (BN5_PANEL_STEP): a Fire object on it as on poison ---- */

static HookAct sea_step(HookRegs *r, void *user) {
	(void)user;
	/* (r0 the panel's type, r7 the object's collision data) */
	if (r->r[0] == XPANEL_SEA && hook_read8(r->r[7] + BN6_COLL_ELEMENT) == ELEMENT_FIRE) r->r[0] = BN6_PANEL_POISON;
	return HOOK_CONTINUE;
}

/* ---- lava and sea run out (BN5_PANEL_UPDATE): their timer counted down as
 * a conveyor's, blinking its last 60 frames, then plain. BN5 counts each
 * from 0x3C0, where its timer stands while the panel is another (a
 * field's lava too lasts 960 frames); BN6 keeps 0x708 there, which a
 * count from 0x3C0 never meets: a timer still at it is BN6's, made 0x3C0
 * as the count begins ---- */

static HookAct panel_ages(HookRegs *r, void *user) {
	(void)user;
	/* (r0 the panel's type, r7 the panel: the conveyors' count reads the type again from it) */
	if (r->r[0] != XPANEL_LAVA && r->r[0] != XPANEL_SEA) return HOOK_CONTINUE;
	if (hook_read16(r->r[7] + BN6_PANEL_TIMER) == BN6_PANEL_TIMER_IDLE) hook_write16(r->r[7] + BN6_PANEL_TIMER, MADE_FRAMES);
	r->r[0] = BN6_PANEL_CONVEYOR;
	return HOOK_CONTINUE;
}

/* ... made by a chip with BN5's frames (BN5_SET_PANEL_TYPE): r0 the column, r1 the row, r2 the type */
static HookAct panel_made(HookRegs *r, void *user) {
	(void)user;
	if (r->r[2] != XPANEL_LAVA && r->r[2] != XPANEL_SEA) return HOOK_CONTINUE;
	uint32_t p = panel_at(r->r[0], r->r[1]);
	if (p && hook_read8(p + BN6_PANEL_TYPE)) hook_write16(p + BN6_PANEL_TIMER, MADE_FRAMES);
	return HOOK_CONTINUE;
}

/* ---- an Aqua hit cools lava (BN5_PANEL_HIT) ----
 * BN6's tests of the volcano and the conveyors in BN6_PANEL_HIT made again
 * in their bytes, lava's beside the volcano's: r0 the panel's type, r4 the
 * attack's collision data, r6 and r7 the panel's column and row. Each of
 * BN6's types takes the steps and cycles it takes in BN6's own (a battle
 * without lava runs as it did), but for a Wood hit on a conveyor, which
 * goes on to the volcano's call of setPanelType in three cycles more (BN6
 * has a call for each): the room that frees holds the lava's test, which
 * the types past 12 reach where BN6's leave. */
static bool cool_make(uint8_t out[BN6_HIT_COOL_SPAN]) {
	const uint16_t code[BN6_HIT_COOL_SPAN / 2] = {
		imm(THUMB_CMP, 0, 0, BN6_PANEL_VOLCANO),   /* +00 cmp r0,#8 */
		0xD108,                                    /* +02 bne conveyors (+16) */
		imm(THUMB_LDRB, 0, 4, BN6_COLL_ELEMENT),   /* +04 aqua: ldrb r0,[r4,#2]: the attack's element */
		imm(THUMB_CMP, 0, 0, 2),                   /* +06 cmp r0,#2: Aqua cools a volcano and lava */
		0xD111,                                    /* +08 bne out (+2E) */
		0x1C30,                                    /* +0A plain: adds r0,r6,#0 */
		0x1C39,                                    /* +0C adds r1,r7,#0 */
		imm(THUMB_MOVS, 2, 0, BN6_PANEL_PLAIN),    /* +0E movs r2,#2 */
		0, 0,                                      /* +10 bl BN6_SET_PANEL_TYPE_IW */
		0xBDD0,                                    /* +14 pop {r4,r6,r7,pc} */
		imm(THUMB_CMP, 0, 0, BN6_PANEL_CONVEYOR),  /* +16 conveyors: cmp r0,#9 */
		0xDB09,                                    /* +18 blt out */
		imm(THUMB_CMP, 0, 0, BN6_PANEL_CONVEYOR_END), /* +1A cmp r0,#12 */
		0xDC03,                                    /* +1C bgt lava (+26) */
		imm(THUMB_LDRB, 0, 4, BN6_COLL_ELEMENT),   /* +1E ldrb r0,[r4,#2] */
		imm(THUMB_CMP, 0, 0, 4),                   /* +20 cmp r0,#4: Wood stops a conveyor */
		0xD104,                                    /* +22 bne out */
		0xE7F1,                                    /* +24 b plain (+0A) */
		imm(THUMB_CMP, 0, 0, XPANEL_LAVA),         /* +26 lava: cmp r0,#13 */
		0xD0EC,                                    /* +28 beq aqua (+04) */
		0x46C0, 0x46C0,                            /* +2A nop, nop */
		0xBDD0,                                    /* +2E out: pop {r4,r6,r7,pc}, BN6's own (its early outs come here) */
	};
	for (int i = 0; i < BN6_HIT_COOL_SPAN / 2; ++i) put16(out + 2 * i, code[i]);
	return thumb_bl_write(out, 0x10, BN6_PANEL_HIT_COOL, BN6_SET_PANEL_TYPE_IW);
}

/* ---- an Elec hit on sea counts twice (BN5_PANEL_BONUS) ----
 * BN6's tests in BN6_PANEL_BONUS made again in their bytes with BN5's
 * second, the sea's: Fire falls to the grass test as BN6's did, Elec to
 * the same test with the panel's type less the sea's distance from grass
 * (a type below the sea's wraps past every type), the rest out. Fire
 * takes a branch where BN6's did not (two cycles more), the rest one
 * cycle less. */
_Static_assert(XPANEL_SEA > BN6_PANEL_GRASS, "the sea's test reads its type as grass's");
static void bonus_make(uint8_t out[BN6_PANEL_BONUS_SPAN]) {
	const uint16_t code[BN6_PANEL_BONUS_SPAN / 2] = {
		imm(THUMB_CMP, 1, 0, ELEMENT_FIRE),        /* +00 cmp r1,#1: r1 the attack's element */
		0xD002,                                    /* +02 beq grass (+0A) */
		imm(THUMB_CMP, 1, 0, ELEMENT_ELEC),        /* +04 cmp r1,#3 */
		0xD104,                                    /* +06 bne out (+12) */
		imm(THUMB_SUBS, 0, 0, XPANEL_SEA - BN6_PANEL_GRASS), /* +08 subs r0,#8: r0 the panel's type, the sea's as grass's */
		imm(THUMB_CMP, 0, 0, BN6_PANEL_GRASS),     /* +0A grass: cmp r0,#6 */
		0xD101,                                    /* +0C bne out (+12) */
		imm(THUMB_LDRH, 0, 4, BN6_COLL_DAMAGE),    /* +0E ldrh r0,[r4,#0x2e]: the attack's damage, once more */
		0xBD10,                                    /* +10 pop {r4,pc} */
		imm(THUMB_MOVS, 0, 0, 0),                  /* +12 out: movs r0,#0 */
		0xBD10,                                    /* +14 pop {r4,pc} */
	};
	for (int i = 0; i < BN6_PANEL_BONUS_SPAN / 2; ++i) put16(out + 2 * i, code[i]);
}

/* ---- the install ---- */

typedef struct {
	uint32_t at;
	uint16_t op;     /* the instruction there, BN6's; 0 for a BL ... */
	uint32_t call;   /* ... to this */
	EmuHook fn;
} Site;

static const Site sites[] = {
	{ BN6_OBJECT_STATUS, 0xB550, 0, lava_site },           /* push {r4,r6,lr} */
	{ BN6_STEP_OBSTACLE_A, 0x7BFC, 0, lava_site },         /* ldrb r4,[r7,#0xf] */
	{ BN6_STEP_OBSTACLE_B, 0, 0x0800E420u, lava_site },
	{ BN6_STEP_OBSTACLE_C, 0x7BFC, 0, lava_site },
	{ BN6_STEP_OBSTACLE_D, 0, 0x0801AE56u, lava_site },
	{ BN6_STEP_OBJECT, 0, 0x0800E3BEu, lava_site },
	{ BN6_PANEL_STEP_POISON, 0x2804, 0, sea_step },        /* cmp r0,#4 */
	{ BN6_PANEL_UPDATE_CONVEYOR, 0x2809, 0, panel_ages },  /* cmp r0,#9 */
	{ BN6_SET_PANEL_TYPE, 0xB570, 0, panel_made },         /* push {r4,r5,r6,lr} */
};
#define NSITES (sizeof sites / sizeof *sites)

/* each site BN6's own instruction, in the player's ROM and the core's copy */
static bool sites_ok(void) {
	for (size_t i = 0; i < NSITES; ++i) {
		const Site *s = &sites[i];
		uint32_t to = 0;
		bool own = s->op ? rom16(s->at) == s->op : thumb_bl_decode(s->at, rom16(s->at), rom16(s->at + 2), &to) && to == s->call;
		if (!own || emu_read16(s->at) != rom16(s->at)) {
			fprintf(stderr, "xpanels: %08x is not the instruction expected\n", (unsigned)s->at);
			return false;
		}
	}
	return true;
}

static bool free_at(uint32_t at, uint32_t end) {
	for (uint32_t a = at; a < end; ++a)
		if (emu_read8(a) != 0xFF) return false;
	return true;
}

/* The flags table grown (BN6's 13 words, BN5's lava's and sea's, BN6's
 * type 0's for 15), and the IWRAM literal checked to be BN6's */
static bool flags_make(uint8_t out[16 * 4]) {
	const uint8_t *bn5 = XR[XROM_BN5_COLONEL_US].data;
	for (int t = 0; t < BN6_PANEL_TYPES; ++t) put32(out + 4 * t, rom32(BN6_PANEL_FLAGS_OF + 4u * (uint32_t)t));
	put32(out + 4 * XPANEL_LAVA, get32(bn5 + (BN5_PANEL_FLAGS_OF - BUS) + 4 * BN5_PANEL_LAVA));
	put32(out + 4 * XPANEL_SEA, get32(bn5 + (BN5_PANEL_FLAGS_OF - BUS) + 4 * BN5_PANEL_SEA));
	put32(out + 4 * 15, rom32(BN6_PANEL_FLAGS_OF));
	return rom32(FLAGS_IMAGE) == BN6_PANEL_FLAGS_IW && emu_read32(FLAGS_IMAGE) == BN6_PANEL_FLAGS_IW;
}

/* IWRAM's routines patched: in their image, and kept so in RAM */
static void iwram_patch(uint32_t at, const uint8_t *was, const uint8_t *now, uint32_t len) {
	emu_write(BN6_IWRAM_IMAGE + (at - BN6_IWRAM_CODE), now, len);
	if (!emu_keep(at, was, now, len)) fprintf(stderr, "xpanels: IWRAM %08x not kept patched (no room)\n", (unsigned)at);
}

static bool hooks_set(void) {
	size_t n = 0;
	while (n < NSITES && emu_hook(sites[n].at, sites[n].fn, NULL)) ++n;
	if (n == NSITES && xpanels_look_hook()) return true;
	fprintf(stderr, "xpanels: no hook at %08x (the table full?)\n", n < NSITES ? (unsigned)sites[n].at : (unsigned)BN6_PANEL_DRAW_ANIMATED);
	while (n) emu_unhook(sites[--n].at);
	return false;
}

/* an IWRAM routine's bytes in its image, BN6's own (their CRC32) in the ROM and the core's copy: into `was` */
static bool image_own(uint32_t image, uint8_t *was, uint32_t len, uint32_t crc) {
	memcpy(was, R.data + (image - BUS), len);
	bool own = xvirus_crc32(was, len) == crc;
	for (uint32_t k = 0; own && k < len; ++k) own = emu_read8(image + k) == was[k];
	return own;
}

void xpanels_install(int xrom) {
	hook_keep_slots(backs, sizeof *backs);   /* (a rollback's: docs/online/arena/) */
	static bool done;
	if (done || xrom != XROM_BN5_COLONEL_US || !XR[xrom].data || !R.data || !emu_ready()) return;
	done = true;
	uint8_t flags[16 * 4], stub[STUB_SIZE], cool[BN6_HIT_COOL_SPAN], cool_was[BN6_HIT_COOL_SPAN], ref_was[4];
	uint8_t bonus[BN6_PANEL_BONUS_SPAN], bonus_was[BN6_PANEL_BONUS_SPAN];
	put32(ref_was, BN6_PANEL_FLAGS_IW);
	bool cool_own = image_own(COOL_IMAGE, cool_was, sizeof cool_was, COOL_CRC);
	bool bonus_own = image_own(BONUS_IMAGE, bonus_was, sizeof bonus_was, BONUS_CRC);
	bonus_make(bonus);
	const char *why = !sites_ok() ? "a hook's site" : !flags_make(flags) ? "the flags' literal" : !cool_own ? "the element hit's tests"
		: !bonus_own ? "the panel bonus's tests" : !free_at(XP_AT, XP_END) ? "its free space" : !stub_make(stub) || !cool_make(cool) ? "its code"
		: !xpanels_look_stage() ? "their look" : NULL;
	if (why) {
		fprintf(stderr, "xpanels: BN5's lava and sea not in BN6's engine: %s is not as expected\n", why);
		return;
	}
	claims_begin("lava and sea (xpanels.c)");
	emu_write(XP_FLAGS, flags, sizeof flags);
	emu_write(XP_STUB, stub, sizeof stub);
	uint8_t ref_now[4];
	put32(ref_now, XP_FLAGS);
	iwram_patch(BN6_PANEL_FLAGS_REF, ref_was, ref_now, sizeof ref_now);
	iwram_patch(BN6_PANEL_HIT_COOL, cool_was, cool, sizeof cool);
	iwram_patch(BN6_PANEL_BONUS_TESTS, bonus_was, bonus, sizeof bonus);
	xpanels_look_write();
	ready = hooks_set();
	claims_end();
	printf("xpanels: BN5's lava and sea %s BN6's engine, as its panel types %d and %d\n", ready ? "in" : "NOT in", XPANEL_LAVA, XPANEL_SEA);
}
