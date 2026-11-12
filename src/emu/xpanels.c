/* xpanels.h: BN5's lava and sea, and BN4's Hole, as BN6's engine runs them. Each of BN5's
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

#include "bn4.h"
#include "bn5.h"
#include "bn5_team_protoman.h"
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
static bool hole_in;   /* BN4's Hole is in too */

bool xpanels_ready(void) { return ready; }

bool xpanels_has(int type) { return type == XPANEL_HOLE ? hole_in : type >= XPANEL_LAVA && type <= XPANEL_MAGNET && ready; }

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

/* ---- a magnet panel pushes what comes onto it (BN5_MAGNET_ARRIVED) ----
 * BN5's rules at BN6's twin of its arrival test (BN6's own twin of the routine,
 * 0x0801A400, runs for its conveyors alone), its fields at the same places in
 * both games: none where the AI data's countdown (+0x38, which
 * the arrival counts down before) is not 0 or the object floats or flies,
 * else a push along the panel (BN6_STRUCK_SLIDE, its kind 3 at +0x0F).
 * BN5 spares the player in its Soul form 5 (NaviStats byte 0x2C), MagnetSoul,
 * which BN6's engine has none of yet (docs/game/team-protoman.md, unit 8). */
#define MAGNET_SPARED  0x24u     /* the object's flags (BN6_COLL_FLAGS) that spare it: BN5's 0x0801721C */
#define AI_PANEL_WAIT  0x38      /* u16 an AI data's frames before a panel acts on it again (BN5 0x08017164, BN6 0x0801A37A) */
#define PUSH_KIND      0x0F      /* the object's push's kind, */
#define PUSH_MAGNET    3         /* ... a magnet's (BN5 0x08017238) */

void xpanels_magnet_arrived(uint32_t obj, uint32_t coll) {
	uint32_t ai = hook_read32(obj + BN6_T1_AI_DATA);
	if (ai < BN6_EWRAM || ai >= BN6_EWRAM_END || hook_read16(ai + AI_PANEL_WAIT)) return;
	if (hook_read32(coll + BN6_COLL_FLAGS) & MAGNET_SPARED) return;
	hook_write32(coll + BN6_COLL_STRUCK, hook_read32(coll + BN6_COLL_STRUCK) | BN6_STRUCK_SLIDE);
	hook_write8(obj + PUSH_KIND, PUSH_MAGNET);
}

/* ---- the magnet's pull (BN5_STEP_PULL) ----
 * BN6's movement step lacks BN5's branch for a push of kind 3, which the
 * magnet panel asks for: at its read of the kind (BN6_STEP_KIND_READ), that
 * kind jumps to BN5's branch made again in Thumb, its two calls BN6's twins
 * through veneers (which keep the flags the second's answer is), its table of
 * directions copied from the player's BN5 ROM; it ends the step as both
 * games' do (add sp,#16; pop {r4,pc}) */
#define PULL_KIND   3
#define DIRECTIONS  4             /* a direction's numbers, 1-4 (0 none) */
#define PULL_CODE   0x44
#define PULL_LIT    PULL_CODE
#define PULL_VENEER (PULL_LIT + 4)
#define PULL_TABLE  (PULL_VENEER + 2 * THUMB_VENEER_SIZE)
#define PULL_ROWS   6             /* BN5_PULL_DIRECTIONS' rows, by the collision data's +0xC (0-5; its code from 0x0800C930) */
#define PULL_SIZE   (PULL_TABLE + 4 * PULL_ROWS)
static uint32_t pull_rows;   /* (the kind-3 slide's step a frame down a row, BN5's: BN5_PULL_ROWS, read as the core starts) */
_Static_assert(XP_TILES + XP_TILES_SIZE + 0x100u <= XP_PULL && XP_PULL + PULL_SIZE <= XP_END, "the pull past its room (xpanels_look.h)");

static bool pull_make(uint8_t out[PULL_SIZE]) {
	const uint16_t code[PULL_CODE / 2] = {
		0x6D68,         /* +00 ldr r0,[r5,#0x54]: its collision data */
		0x7B00,         /* +02 ldrb r0,[r0,#0xc] */
		0x0080,         /* +04 lsls r0,r0,#2 */
		0x490F,         /* +06 ldr r1,[pc,#0x3c]: the directions' table (PULL_LIT) */
		0x5808,         /* +08 ldr r0,[r1,r0]: its row, four directions */
		0x9000,         /* +0A str r0,[sp,#0] */
		0x2400,         /* +0C movs r4,#0 */
		0xA800,         /* +0E loop: add r0,sp,#0 */
		0x5D00,         /* +10 ldrb r0,[r0,r4] */
		0, 0,           /* +12 bl the direction's step */
		0x9001,         /* +16 str r0,[sp,#4] */
		0x9102,         /* +18 str r1,[sp,#8] */
		0x9203,         /* +1A str r2,[sp,#0xc] */
		0x7CAB,         /* +1C ldrb r3,[r5,#0x12] */
		0x18C0,         /* +1E adds r0,r0,r3 */
		0x7CEB,         /* +20 ldrb r3,[r5,#0x13] */
		0x18C9,         /* +22 adds r1,r1,r3 */
		0, 0,           /* +24 bl whether the panel takes it */
		0xD106,         /* +28 bne found (+38) */
		0x3401,         /* +2A adds r4,#1 */
		0x2C04,         /* +2C cmp r4,#4 */
		0xDBEE,         /* +2E blt loop (+0E) */
		0x2000,         /* +30 movs r0,#0: none */
		0x2100,         /* +32 movs r1,#0 */
		0x2200,         /* +34 movs r2,#0 */
		0xE002,         /* +36 b end (+3E) */
		0x9801,         /* +38 found: ldr r0,[sp,#4] */
		0x9902,         /* +3A ldr r1,[sp,#8] */
		0x9A03,         /* +3C ldr r2,[sp,#0xc] */
		0xB004,         /* +3E end: add sp,#16 */
		0xBD10,         /* +40 pop {r4,pc} */
		0x46C0,         /* +42 nop */
	};
	const uint8_t *bn5 = XR[XROM_BN5_COLONEL_US].data;
	memset(out, 0, PULL_SIZE);
	for (int i = 0; i < PULL_CODE / 2; ++i) put16(out + 2 * i, code[i]);
	put32(out + PULL_LIT, XP_PULL + PULL_TABLE);
	/* (its directions in BN5's numbers, each made BN6's of the same step: the two games' tables of a direction's column and
	 * row matched, BN5_DIRECTION_STEPS and BN6_DIRECTION_STEPS) */
	for (int i = 0; i < 4 * PULL_ROWS; ++i) {
		int d5 = bn5[(BN5_PULL_DIRECTIONS - BUS) + (uint32_t)i], d6 = -1;
		for (int k = 0; d5 <= DIRECTIONS && k <= DIRECTIONS && d6 < 0; ++k)
			if (get16(bn5 + (BN5_DIRECTION_STEPS - BUS) + 4u * (uint32_t)d5) == rom16(BN6_DIRECTION_STEPS + 4u * (uint32_t)k)) d6 = k;
		if (d6 < 0) return false;
		out[PULL_TABLE + i] = (uint8_t)d6;
	}
	return thumb_veneer(out + PULL_VENEER, XP_PULL + PULL_VENEER, BN6_STEP_DIRECTION) &&
		thumb_veneer(out + PULL_VENEER + THUMB_VENEER_SIZE, XP_PULL + PULL_VENEER + THUMB_VENEER_SIZE, BN6_STEP_PANEL_TAKES) &&
		thumb_bl_write(out, 0x12, XP_PULL, XP_PULL + PULL_VENEER) && thumb_bl_write(out, 0x24, XP_PULL, XP_PULL + PULL_VENEER + THUMB_VENEER_SIZE);
}

/* BN5's branch as this makes it again: its kind's test and its first and last instructions */
static bool pull_bn5_ok(void) {
	const uint8_t *bn5 = XR[XROM_BN5_COLONEL_US].data;
	pull_rows = get32(bn5 + (BN5_PULL_ROWS - BUS));
	return get16(bn5 + (BN5_STEP_PULL - BUS)) == 0x6D68 && get16(bn5 + (BN5_STEP_PULL - BUS) + 0x40) == 0xBD10 &&
		get16(bn5 + (BN5_STEP_PULL - BUS) - 0x2C) == 0x2803 && pull_rows && pull_rows < 0x00100000u &&
		rom32(BN6_SLIDE_ROWS) == 0x00060000u;
}

/* (BN6's kind 3 is its conveyors' push: its own magnet-like routine, 0x0801A400, the twin of BN5_MAGNET_ARRIVED, asks for
 * it on its conveyors, 9-12, and its step's table moves it by the conveyor's way. BN5 has no conveyors: its kind 3 is the
 * magnet's alone. So the pull is BN5's where the object stands on a magnet panel, BN6's own elsewhere.) */
static HookAct step_pull(HookRegs *r, void *user) {
	(void)user;
	if (hook_read8(r->r[5] + PUSH_KIND) != PULL_KIND) return HOOK_CONTINUE;
	uint32_t coll = hook_read32(r->r[5] + BN6_T1_COLLISION);
	if (coll < BN6_EWRAM || coll >= BN6_EWRAM_END) return HOOK_CONTINUE;
	uint32_t p = panel_at(hook_read8(coll + BN6_COLL_PANEL_X), hook_read8(coll + BN6_COLL_PANEL_Y));
	if (!p || hook_read8(p + BN6_PANEL_TYPE) != XPANEL_MAGNET) return HOOK_CONTINUE;
	r->r[12] = XP_PULL | 1;
	return HOOK_JUMP;
}

/* ... and its frames' move down a row BN5's (BN5_PULL_ROWS, 8 pixels; BN6's slide moves its conveyors 6), where the slide
 * began on a magnet panel: the panel it goes to (+0x14, +0x15) less its step (+0x1C, +0x1D) */
#define SLIDE_TO_X   0x14
#define SLIDE_STEP_X 0x1C
static HookAct slide_rows(HookRegs *r, void *user) {
	(void)user;
	uint32_t obj = r->r[5];
	if (hook_read8(obj + PUSH_KIND) != PULL_KIND) return HOOK_CONTINUE;
	uint32_t x = (uint32_t)(hook_read8(obj + SLIDE_TO_X) - (int8_t)hook_read8(obj + SLIDE_STEP_X));
	uint32_t y = (uint32_t)(hook_read8(obj + SLIDE_TO_X + 1) - (int8_t)hook_read8(obj + SLIDE_STEP_X + 1));
	uint32_t p = panel_at(x & 0xFF, y & 0xFF);
	if (p && hook_read8(p + BN6_PANEL_TYPE) == XPANEL_MAGNET) r->r[2] = pull_rows;
	return HOOK_CONTINUE;
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
	{ BN6_STEP_KIND_READ, 0x7BE8, 0, step_pull },          /* ldrb r0,[r5,#0xf] */
	{ BN6_SLIDE_ROW_MUL, 0x4342, 0, slide_rows },          /* muls r2,r0 */
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

/* The flags table grown: BN6's 13 words, then BN5's lava's, sea's and magnet
 * panel's and BN4's Hole's where their games have come in, BN6's type 0's
 * where not */
static uint8_t flags[XPANEL_TYPES * 4];
_Static_assert(sizeof flags <= XP_STUB - XP_FLAGS, "the flags past their room (xpanels_look.h)");
static bool flags_based, flags_pointed;

static void flags_base(void) {
	if (flags_based) return;
	for (int t = 0; t < XPANEL_TYPES; ++t) put32(flags + 4 * t, rom32(BN6_PANEL_FLAGS_OF + 4u * (uint32_t)(t < BN6_PANEL_TYPES ? t : 0)));
	flags_based = true;
}

/* the IWRAM literal BN6's, or already the grown table */
static bool flags_ref_ok(void) {
	if (rom32(FLAGS_IMAGE) != BN6_PANEL_FLAGS_IW) return false;
	return flags_pointed ? emu_read32(FLAGS_IMAGE) == XP_FLAGS : emu_read32(FLAGS_IMAGE) == BN6_PANEL_FLAGS_IW;
}

/* BN5's lava's, sea's and magnet panel's words */
static void flags_bn5(void) {
	const uint8_t *bn5 = XR[XROM_BN5_COLONEL_US].data;
	flags_base();
	put32(flags + 4 * XPANEL_LAVA, get32(bn5 + (BN5_PANEL_FLAGS_OF - BUS) + 4 * BN5_PANEL_LAVA));
	put32(flags + 4 * XPANEL_SEA, get32(bn5 + (BN5_PANEL_FLAGS_OF - BUS) + 4 * BN5_PANEL_SEA));
	put32(flags + 4 * XPANEL_MAGNET, get32(bn5 + (BN5_PANEL_FLAGS_OF - BUS) + 4 * BN5_PANEL_MAGNET));
}

/* BN4's Hole's, plain's word in both games (checked) */
static bool flags_bn4(void) {
	const uint8_t *bn4 = XR[XROM_BN4_RED_SUN_US].data;
	uint32_t hole = get32(bn4 + (BN4_PANEL_FLAGS_OF - BUS) + 4 * BN4_PANEL_HOLE);
	if (hole != get32(bn4 + (BN4_PANEL_FLAGS_OF - BUS) + 4 * BN6_PANEL_PLAIN) || hole != rom32(BN6_PANEL_FLAGS_OF + 4u * BN6_PANEL_PLAIN)) return false;
	flags_base();
	put32(flags + 4 * XPANEL_HOLE, hole);
	return true;
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

/* the grown tables' room free, or theirs already */
static bool tables_free(void) { return flags_pointed || (free_at(XP_FLAGS, XP_FLAGS + sizeof flags) && free_at(XP_TILES, XP_TILES + XP_TILES_SIZE)); }

/* The grown tables written, with every game's kinds staged so far: the
 * flags (the IWRAM literal pointed at them once) and the look; one
 * claim for every game's write of them, each writing the whole again */
static void tables_write(void) {
	claims_begin("the panels' grown tables (xpanels.c)");
	emu_write(XP_FLAGS, flags, sizeof flags);
	if (!flags_pointed) {
		uint8_t ref_was[4], ref_now[4];
		put32(ref_was, BN6_PANEL_FLAGS_IW);
		put32(ref_now, XP_FLAGS);
		iwram_patch(BN6_PANEL_FLAGS_REF, ref_was, ref_now, sizeof ref_now);
		flags_pointed = true;
	}
	xpanels_look_write();
	claims_end();
}

/* an IWRAM routine's bytes in its image, BN6's own (their CRC32) in the ROM and the core's copy: into `was` */
static bool image_own(uint32_t image, uint8_t *was, uint32_t len, uint32_t crc) {
	memcpy(was, R.data + (image - BUS), len);
	bool own = xvirus_crc32(was, len) == crc;
	for (uint32_t k = 0; own && k < len; ++k) own = emu_read8(image + k) == was[k];
	return own;
}

/* BN5's lava and sea: their flags, their rules (the hooks, the cooling's
 * stub, the element hit's and the bonus's tests) and their look */
static void install_bn5(void) {
	uint8_t stub[STUB_SIZE], cool[BN6_HIT_COOL_SPAN], cool_was[BN6_HIT_COOL_SPAN];
	uint8_t bonus[BN6_PANEL_BONUS_SPAN], bonus_was[BN6_PANEL_BONUS_SPAN], pull[PULL_SIZE];
	bool cool_own = image_own(COOL_IMAGE, cool_was, sizeof cool_was, COOL_CRC);
	bool bonus_own = image_own(BONUS_IMAGE, bonus_was, sizeof bonus_was, BONUS_CRC);
	bonus_make(bonus);
	const char *why = !sites_ok() ? "a hook's site" : !flags_ref_ok() ? "the flags' literal" : !cool_own ? "the element hit's tests"
		: !bonus_own ? "the panel bonus's tests" : !tables_free() || !free_at(XP_STUB, XP_STUB + STUB_SIZE) || !free_at(XP_PULL, XP_PULL + PULL_SIZE)
		? "its free space" : !pull_bn5_ok() ? "BN5's pull" : !stub_make(stub) || !cool_make(cool) || !pull_make(pull) ? "its code"
		: !xpanels_look_stage(XROM_BN5_COLONEL_US) ? "their look" : NULL;
	if (why) {
		fprintf(stderr, "xpanels: BN5's lava, sea and magnet panel not in BN6's engine: %s is not as expected\n", why);
		return;
	}
	claims_begin("lava and sea (xpanels.c)");
	flags_bn5();
	tables_write();
	emu_write(XP_STUB, stub, sizeof stub);
	emu_write(XP_PULL, pull, sizeof pull);
	iwram_patch(BN6_PANEL_HIT_COOL, cool_was, cool, sizeof cool);
	iwram_patch(BN6_PANEL_BONUS_TESTS, bonus_was, bonus, sizeof bonus);
	ready = hooks_set();
	claims_end();
	printf("xpanels: BN5's lava, sea and magnet panel %s BN6's engine, as its panel types %d, %d and %d\n", ready ? "in" : "NOT in", XPANEL_LAVA,
		XPANEL_SEA, XPANEL_MAGNET);
}

/* BN4's Hole: its flags and its look, nothing it does (xpanels.h) */
static void install_bn4(void) {
	const char *why = !flags_ref_ok() ? "the flags' literal" : !flags_bn4() ? "its flags" : !tables_free() ? "its free space" : !xpanels_look_stage(XROM_BN4_RED_SUN_US) ? "its look" : NULL;
	if (why) {
		fprintf(stderr, "xpanels: BN4's Hole not in BN6's engine: %s is not as expected\n", why);
		return;
	}
	claims_begin("the Hole (xpanels.c)");
	tables_write();
	hole_in = xpanels_look_hook();
	claims_end();
	printf("xpanels: BN4's Hole %s BN6's engine, as its panel type %d\n", hole_in ? "in" : "NOT in", XPANEL_HOLE);
}

void xpanels_install(int xrom) {
	hook_keep_slots(backs, sizeof *backs);   /* (a rollback's: docs/online/arena/) */
	static bool done[XROM_COUNT];
	if (xrom < 0 || xrom >= XROM_COUNT || done[xrom] || !XR[xrom].data || !R.data || !emu_ready()) return;
	done[xrom] = true;
	if (xrom == XROM_BN5_COLONEL_US) install_bn5();
	else if (xrom == XROM_BN4_RED_SUN_US) install_bn4();
}
