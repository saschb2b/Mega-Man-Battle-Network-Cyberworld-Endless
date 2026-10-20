/* xsea.h: BN5's sea's submerge as BN6's engine runs it. BN5's rules in C
 * at hooks on BN6's code, BN5's marker spawned and the hit gate's test
 * made by a few instructions of the project's own Thumb. */
#include "xsea.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "thumb_reloc.h"
#include "xnavi_bn5.h"
#include "xnavi_objects.h"
#include "xpanels.h"

#define BUS 0x08000000u

/* The code at XN_SEA: the marker's spawn (the hide's), the hit gate's test, the splash of an object held, their veneers */
#define STUB      (XN_SEA + 0x00u)
#define GATE      (XN_SEA + 0x10u)
#define GATE_SIZE 0x30u
#define GATE2     (GATE + GATE_SIZE)                /* ... the other way */
#define GATE2_SIZE 0x34u
#define SPLASH    (GATE2 + GATE2_SIZE)
#define VENEER    (SPLASH + 0x10u)                  /* ... on to the moved BN5_SEA_MARKER_SPAWN, */
#define VENEER2   (VENEER + THUMB_VENEER_SIZE)      /* ... on to BN6_SPAWN_EFFECT */
#define CODE_SIZE (VENEER2 + THUMB_VENEER_SIZE - XN_SEA)
#define ELEMENT_AQUA 2   /* an object's element (BN6_T1_ELEMENT): Aqua */
#define SPLASH_BN5   0x63   /* BN5's effect of the splash (BN5_PANEL_ARRIVED), BN6's as the moved objects number it */
_Static_assert(XN_SEA + CODE_SIZE <= XN_SEA_END, "the sea's code past its room (xnavi_recipe.h)");
#define HIDDEN_IMAGE (BN6_IWRAM_IMAGE + (BN6_HIT_HIDDEN - BN6_IWRAM_CODE))
#define HIDDEN2_IMAGE (BN6_IWRAM_IMAGE + (BN6_HIT_HIDDEN2 - BN6_IWRAM_CODE))
#define CLAIMS "the sea's submerge (xsea.c)"

static uint16_t rom16(uint32_t a) { return get16(R.data + (a - BUS)); }

/* its collision data's own bit (BN6_COLL_INDEX_BIT), 0 where it has none */
static uint32_t bit_of(uint32_t obj) {
	uint32_t coll = hook_read32(obj + BN6_T1_COLLISION);
	if (coll < BN6_EWRAM || coll >= BN6_EWRAM_END) return 0;
	uint32_t bit = hook_read32(coll + BN6_COLL_INDEX_BIT);
	return bit && !(bit & (bit - 1)) ? bit : 0;
}

/* where the marker of the collision data of bit `bit` is kept */
static uint32_t marker_at(uint32_t bit) {
	uint32_t i = 0;
	while (bit > 1) bit >>= 1, ++i;
	return XSEA_MARKERS + 4u * i;
}

static bool under(uint32_t obj) {
	uint32_t bit = bit_of(obj);
	return bit && (hook_read32(XSEA_MASK) & bit);
}

/* ---- the sea's test (BN5_SEA_TEST, BN5_SEA_SUBMERGE): under where it stands
 * on sea with its traits' bit, kept up by none of its flags ---- */

static HookAct sea_test(HookRegs *r, void *user) {
	(void)user;
	uint32_t obj = r->r[5], bit = bit_of(obj);
	if (!bit) return HOOK_CONTINUE;
	uint32_t coll = hook_read32(obj + BN6_T1_COLLISION), ai = hook_read32(obj + BN6_T1_AI_DATA);
	uint32_t x = hook_read8(coll + BN6_COLL_PANEL_X), y = hook_read8(coll + BN6_COLL_PANEL_Y);
	bool on_sea = x - 1 < 6 && y - 1 < 3 && hook_read8(BN6_FIELD_PANELS + (y * 8 + x) * BN6_PANEL_SIZE + BN6_PANEL_TYPE) == XPANEL_SEA;
	bool goes = on_sea && ai >= BN6_EWRAM && ai < BN6_EWRAM_END && (hook_read32(ai + BN6_AI_TRAITS) & BN6_TRAIT_SUBMERGES) &&
		!(hook_read32(coll + BN6_COLL_FLAGS) & BN6_SEA_KEPT_UP);
	uint32_t mask = hook_read32(XSEA_MASK);
	hook_write32(XSEA_MASK, goes ? mask | bit : mask & ~bit);
	return HOOK_CONTINUE;
}

/* ---- what is under not drawn, its marker following it (BN5_HIDE_STEP) ----
 * The marker is spawned through the stub (stub_make), which comes back to
 * the hooked instruction: that time it runs. */

typedef struct { uint32_t at, sp; } Back;
static Back backs[HOOK_CORES];   /* (one a core: the Arena's two run in turn, a run loop at a time) */

static HookAct sea_hide(HookRegs *r, void *user) {
	(void)user;
	Back *back = &backs[hook_slot()];
	if (back->at == r->pc && back->sp == r->sp) {
		back->at = 0;
		return HOOK_CONTINUE;
	}
	uint32_t obj = r->r[5], bit = bit_of(obj);
	if (!bit) return HOOK_CONTINUE;
	uint32_t at = marker_at(bit), marker = hook_read32(at);
	if (hook_read32(XSEA_MASK) & bit) {
		hook_write8(obj, (uint8_t)(hook_read8(obj) & ~BN6_OBJ_VISIBLE));
		if (marker) return HOOK_CONTINUE;
		back->at = r->pc;
		back->sp = r->sp;
		r->r[1] = hook_read32(obj + BN6_T1_COLLISION);
		r->r[2] = at;
		r->r[3] = r->pc | 1;
		r->r[12] = STUB | 1;
		return HOOK_JUMP;
	}
	/* (BN5's marker's end, 0x080E4D36: its +8, 8) */
	if (marker) {
		hook_write32(marker + 8, 8);
		hook_write32(at, 0);
	}
	return HOOK_CONTINUE;
}

/* ---- a collision data made: none of its last owner's submerge (BN5's
 * allocator clears the flags and the marker with the rest) ---- */

static HookAct sea_made(HookRegs *r, void *user) {
	(void)user;
	uint32_t bit = hook_read32(r->r[0] + BN6_COLL_INDEX_BIT);
	if (!bit || (bit & (bit - 1))) return HOOK_CONTINUE;
	hook_write32(XSEA_MASK, hook_read32(XSEA_MASK) & ~bit);
	hook_write32(marker_at(bit), 0);
	return HOOK_CONTINUE;
}

/* ---- BN6's tests of an invisible object that BN5 makes of the hidden: the
 * submerged read as invisible there (tst r0,r1: r1 the flag, r5 the object) ---- */

static HookAct sea_hidden(HookRegs *r, void *user) {
	(void)user;
	if (r->r[1] == BN6_OBJ_HIDDEN && under(r->r[5])) r->r[0] |= BN6_OBJ_HIDDEN;
	return HOOK_CONTINUE;
}

/* ---- a non-Aqua object stepping onto sea held, under a splash
 * (BN5_PANEL_ARRIVED, BN5_IMMOBILIZE): at BN6's ice test (BN6_ARRIVED_TYPE),
 * the panel's type in r1; the splash spawned by the stub, which ends the
 * routine as BN6's own end does ---- */

static HookAct sea_arrived(HookRegs *r, void *user) {
	(void)user;
	if (r->r[1] != XPANEL_SEA) return HOOK_CONTINUE;
	uint32_t obj = r->r[5], coll = r->r[7], ai = hook_read32(obj + BN6_T1_AI_DATA), flags = hook_read32(coll + BN6_COLL_FLAGS);
	bool submerges = ai >= BN6_EWRAM && ai < BN6_EWRAM_END && (hook_read32(ai + BN6_AI_TRAITS) & BN6_TRAIT_SUBMERGES);
	if ((flags & BN6_OBJ_FLOATS) || submerges || hook_read8(obj + BN6_T1_ELEMENT) == ELEMENT_AQUA) return HOOK_CONTINUE;
	hook_write16(coll + BN6_COLL_IMMOBILE, BN6_SEA_HOLD);
	hook_write32(coll + BN6_COLL_FLAGS, flags | BN6_OBJ_IMMOBILIZED);
	r->r[12] = SPLASH | 1;
	return HOOK_JUMP;
}

/* ---- the code ---- */

/* The marker spawned for the object r5 names (the moved BN5_SEA_MARKER_SPAWN, through the veneer) and kept at r2,
 * back to r3 (the hooked instruction, a BL's first half: r0-r3 and lr free there); r4 its collision data (r1) as BN5's
 * hide step has it, which the spawn takes for the marker's parameter (its +4) */
static const uint16_t stub_code[] = {
	0xB41C,           /* push {r2,r3,r4} */
	0x1C0C,           /* adds r4,r1,#0 */
	0, 0,             /* bl the veneer */
	0xBC1C,           /* pop {r2,r3,r4} */
	0x6010,           /* str r0,[r2] */
	0x4718,           /* bx r3 */
};

/* The hit gate's test of a hidden target, called from BN6_HIT_HIDDEN (r0 the target's flags, r3 the attack's +0x30,
 * r6 the attack's collision data, r7 the target's; r4 free): Z set where the hit goes on to the gate's next test, clear
 * where it stops. BN6's invisible stops all but flags 0x1008; the submerged, as BN5's hidden, Elec too goes through */
static const uint16_t gate_code[GATE_SIZE / 2 - 4] = {
	0xB406,           /* +00 push {r1,r2} */
	0x2404,           /* +02 movs r4,#4: BN6_OBJ_HIDDEN */
	0x4220,           /* +04 tst r0,r4 */
	0xD107,           /* +06 bne hidden (+18) */
	0x4C07,           /* +08 ldr r4,=XSEA_MASK */
	0x6824,           /* +0A ldr r4,[r4] */
	0x6C79,           /* +0C ldr r1,[r7,#0x44]: its collision data's bit */
	0x420C,           /* +0E tst r4,r1 */
	0xD007,           /* +10 beq goes (+22) */
	0x78B4,           /* +12 ldrb r4,[r6,#2]: the attack's element */
	0x2C03,           /* +14 cmp r4,#3: Elec */
	0xD004,           /* +16 beq goes */
	0x4C04,           /* +18 hidden: ldr r4,=0x1008 */
	0x4223,           /* +1A tst r3,r4 */
	0xD101,           /* +1C bne goes */
	0x2401,           /* +1E movs r4,#1: Z clear, it stops */
	0xE000,           /* +20 b out */
	0x2400,           /* +22 goes: movs r4,#0: Z set */
	0xBC06,           /* +24 out: pop {r1,r2} */
	0x46F7,           /* +26 mov pc,lr (Thumb kept) */
};
#define HIT_PASSES 0x1008u   /* an attack's +0x30 flags a hidden target takes */

/* ... the other way, from BN6_HIT_HIDDEN2: the first (r2 its flags, r6 its collision data) hidden from the second (r7: its
 * +0x30 into r3 where the first is hidden, as BN6's test leaves it) */
static const uint16_t gate2_code[GATE2_SIZE / 2 - 4] = {
	0xB402,           /* +00 push {r1} */
	0x2404,           /* +02 movs r4,#4 */
	0x4222,           /* +04 tst r2,r4 */
	0xD108,           /* +06 bne hidden (+1A) */
	0x4C08,           /* +08 ldr r4,=XSEA_MASK */
	0x6824,           /* +0A ldr r4,[r4] */
	0x6C71,           /* +0C ldr r1,[r6,#0x44] */
	0x420C,           /* +0E tst r4,r1 */
	0xD009,           /* +10 beq goes (+26) */
	0x6B3B,           /* +12 ldr r3,[r7,#0x30] */
	0x78BC,           /* +14 ldrb r4,[r7,#2] */
	0x2C03,           /* +16 cmp r4,#3: Elec */
	0xD005,           /* +18 beq goes */
	0x6B3B,           /* +1A hidden: ldr r3,[r7,#0x30] */
	0x4C04,           /* +1C ldr r4,=0x1008 */
	0x4223,           /* +1E tst r3,r4 */
	0xD101,           /* +20 bne goes */
	0x2401,           /* +22 movs r4,#1 */
	0xE000,           /* +24 b out */
	0x2400,           /* +26 goes: movs r4,#0 */
	0xBC02,           /* +28 out: pop {r1} */
	0x46F7,           /* +2A mov pc,lr */
};

/* The splash at the object r5 names (its place, +0x34 on), the effect BN6's number for BN5's (the moved objects'), then
 * the end of BN6_PANEL_ARRIVED as its own */
static const uint16_t splash_code[] = {
	0x2034,           /* movs r0,#0x34 */
	0x1940,           /* adds r0,r0,r5 */
	0xC80E,           /* ldmia r0!,{r1,r2,r3} */
	0x2400,           /* movs r4,#effect (code_make) */
	0, 0,             /* bl the veneer, on to BN6_SPAWN_EFFECT */
	0xBD90,           /* pop {r4,r7,pc}: BN6_ARRIVED_END's */
};

static bool code_make(uint8_t out[CODE_SIZE], uint32_t spawn, uint8_t splash) {
	memset(out, 0xFF, CODE_SIZE);
	for (size_t i = 0; i < sizeof stub_code / 2; ++i) put16(out + 2 * i, stub_code[i]);
	uint8_t *g = out + (GATE - XN_SEA), *h = out + (SPLASH - XN_SEA);
	for (size_t i = 0; i < sizeof gate_code / 2; ++i) put16(g + 2 * i, gate_code[i]);
	put32(g + 0x28, XSEA_MASK);
	put32(g + 0x2C, HIT_PASSES);
	uint8_t *g2 = out + (GATE2 - XN_SEA);
	for (size_t i = 0; i < sizeof gate2_code / 2; ++i) put16(g2 + 2 * i, gate2_code[i]);
	put32(g2 + 0x2C, XSEA_MASK);
	put32(g2 + 0x30, HIT_PASSES);
	for (size_t i = 0; i < sizeof splash_code / 2; ++i) put16(h + 2 * i, splash_code[i]);
	put16(h + 6, (uint16_t)(0x2400u | splash));
	return thumb_veneer(out + (VENEER - XN_SEA), VENEER, spawn) && thumb_bl_write(out, 4, XN_SEA, VENEER) &&
		thumb_veneer(out + (VENEER2 - XN_SEA), VENEER2, BN6_SPAWN_EFFECT) && thumb_bl_write(out, SPLASH - XN_SEA + 8, XN_SEA, VENEER2);
}

/* The hit gate's tests made calls to the gate's (14 bytes: ldr r4,[pc,#8]; mov lr,pc; bx r4; beq the next test; pop; the
 * gate's address; the other way 16: a nop before the address, to its word) */
static void hidden_make(uint8_t out[BN6_HIT_HIDDEN_SPAN], uint8_t out2[BN6_HIT_HIDDEN2_SPAN]) {
	static const uint16_t code[] = { 0x4C02, 0x46FE, 0x4720, 0xD002, 0xBDD0 }, code2[] = { 0x4C02, 0x46FE, 0x4720, 0xD003, 0xBDD0, 0x46C0 };
	for (size_t i = 0; i < sizeof code / 2; ++i) put16(out + 2 * i, code[i]);
	put32(out + 10, GATE | 1);
	for (size_t i = 0; i < sizeof code2 / 2; ++i) put16(out2 + 2 * i, code2[i]);
	put32(out2 + 12, GATE2 | 1);
}
static const uint8_t hidden_was[BN6_HIT_HIDDEN_SPAN] = { 0x04, 0x24, 0x20, 0x42, 0x03, 0xD0, 0xAF, 0x4C, 0x23, 0x42, 0x00, 0xD1, 0xD0, 0xBD };
static const uint8_t hidden2_was[BN6_HIT_HIDDEN2_SPAN] = { 0x04, 0x24, 0x22, 0x42, 0x04, 0xD0, 0x3B, 0x6B, 0x9E, 0x4C, 0x23, 0x42, 0x00, 0xD1,
	0xD0, 0xBD };

/* ---- the install ---- */

typedef struct { uint32_t at; uint16_t op; EmuHook fn; } Site;

static const Site sites[] = {
	{ BN6_STATUS_SEA, 0xF7F9, sea_test },        /* bl 0x08014326 (its first half) */
	{ BN6_STATUS_HIDE, 0xF7FE, sea_hide },       /* bl 0x0801A166 */
	{ BN6_COLL_MADE, 0x6568, sea_made },         /* str r0,[r5,#0x54] */
	{ BN6_SHOW_AFTER_HIDDEN, 0x4208, sea_hidden }, /* tst r0,r1 */
	{ BN6_CHILD_HIDDEN, 0x4208, sea_hidden },
	{ BN6_ARRIVED_TYPE, 0x2907, sea_arrived },   /* cmp r1,#7 */
};
#define NSITES (sizeof sites / sizeof *sites)

/* BN6's number for BN5's effect `e`, as the moved objects carry it; 0 none */
static uint8_t effect_of(int e) {
	for (int i = 0; i < xnavi_objects_bn5.neffects; ++i)
		if (xnavi_objects_bn5.effects[i].from == e) return (uint8_t)xnavi_objects_bn5.effects[i].to;
	return 0;
}

static const char *unready(uint32_t *spawn) {
	for (size_t i = 0; i < NSITES; ++i)
		if (rom16(sites[i].at) != sites[i].op || emu_read16(sites[i].at) != sites[i].op) return "a hook's site";
	if (memcmp(R.data + (HIDDEN_IMAGE - BUS), hidden_was, sizeof hidden_was) || memcmp(R.data + (HIDDEN2_IMAGE - BUS), hidden2_was, sizeof hidden2_was))
		return "the hit gate's tests";
	for (uint32_t k = 0; k < BN6_HIT_HIDDEN_SPAN; ++k)
		if (emu_read8(HIDDEN_IMAGE + k) != hidden_was[k]) return "the hit gate's test";
	for (uint32_t k = 0; k < BN6_HIT_HIDDEN2_SPAN; ++k)
		if (emu_read8(HIDDEN2_IMAGE + k) != hidden2_was[k]) return "the hit gate's other test";
	if (rom16(BN6_ARRIVED_END) != 0xBD90 || emu_read16(BN6_ARRIVED_END) != 0xBD90) return "the arrival's end";
	for (uint32_t a = XN_SEA; a < XN_SEA_END; ++a)
		if (emu_read8(a) != 0xFF) return "its room";
	if (!effect_of(SPLASH_BN5)) return "the splash's effect";
	const XvBlock *b = xvirus_block_of(&xnavi_objects_bn5, BN5_SEA_MARKER_SPAWN);
	if (!b || !xnavi_objects_install() || !xnavi_objects_have(4, 0x3E)) return "BN5's marker, moved";
	*spawn = (b->at + (BN5_SEA_MARKER_SPAWN - b->start)) | 1;
	return NULL;
}

void xsea_install(void) {
	hook_keep_slots(backs, sizeof *backs);   /* (a rollback's: docs/ONLINE_ARENA.md) */
	static bool done;
	if (done || !xpanels_ready() || !R.data || !emu_ready()) return;
	done = true;
	uint32_t spawn = 0;
	uint8_t code[CODE_SIZE], hidden[BN6_HIT_HIDDEN_SPAN], hidden2[BN6_HIT_HIDDEN2_SPAN];
	const char *why = unready(&spawn);
	if (!why && !code_make(code, spawn, effect_of(SPLASH_BN5))) why = "its code";
	if (why) {
		fprintf(stderr, "xsea: BN5's sea's submerge not in BN6's engine: %s is not as expected\n", why);
		return;
	}
	hidden_make(hidden, hidden2);
	claims_begin(CLAIMS);
	emu_write(XN_SEA, code, sizeof code);
	emu_write(HIDDEN_IMAGE, hidden, sizeof hidden);
	emu_write(HIDDEN2_IMAGE, hidden2, sizeof hidden2);
	bool kept = emu_keep(BN6_HIT_HIDDEN, hidden_was, hidden, sizeof hidden) && emu_keep(BN6_HIT_HIDDEN2, hidden2_was, hidden2, sizeof hidden2);
	size_t n = 0;
	while (kept && n < NSITES && emu_hook(sites[n].at, sites[n].fn, NULL)) ++n;
	claims_end();
	bool ready = kept && n == NSITES;
	if (!ready) {
		while (n) emu_unhook(sites[--n].at);
		fprintf(stderr, "xsea: %s\n", kept ? "a hook not set (the table full?)" : "the hit gate's test not kept patched (no room)");
	}
	printf("xsea: BN5's sea's submerge %s BN6's engine\n", ready ? "in" : "NOT in");
}
