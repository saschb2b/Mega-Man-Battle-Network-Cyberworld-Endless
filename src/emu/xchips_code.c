/* BN5's chips' code in the core's copy (xchips_code.h). */
#include "xchips_code.h"

#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "song_table.h"
#include "xattack_grow.h"
#include "xchips_bn4_art.h"   /* (and xchips_bn4.h, which it is whole with) */
#include "xchips_bn5.h"
#include "xchips_flash.h"
#include "xchips_moved.h"
#include "xchips_pas.h"
#include "xchips_registry.h"
#include "xnavi_bn4_art.h"
#include "xnavi_sprites.h"
#include "xvirus.h"
#include "xvirus_grow.h"
#include "xsong.h"
#include "xvirus_tx.h"

#define BUS 0x08000000u

static bool installed, installed4, installed4n;
static bool tables_grown;   /* (the stubs and BN6's two routines' tables grown: by whichever game's units install first) */
static bool powers_grown;   /* (BN6's computed powers grown, XC_POWERS: likewise) */

/* ---- BN6's support and Navi chips' routines grown ---- */

/* One of the two tables: the words that name it (BN6's, and its place among the support routines' for the Navi part), its
 * entries in BN6 and its copy */
typedef struct { uint32_t at, refs[2], count, to, stub; } Routines;

/* BN6's misc family's routines by subfamily (family 0x1C: BN6_MISC_RUN's one literal), grown for BN4's units at
 * XC4_MISC, a routine that does nothing the rest */
static const Routines misc_table = { BN6_MISC_ROUTINES, { BN6_MISC_ROUTINES_REF, 0 }, BN6_MISC_ROUTINE_COUNT, XC4_MISC, XC_STUBS + 4 };

static const Routines tables[2] = {
	{ BN6_CHIP_ROUTINES, { BN6_CHIP_ROUTINES_REF, BN6_CHIP_ROUTINES_REF2 }, BN6_CHIP_ROUTINE_COUNT, XC_SUPPORT, XC_STUBS },
	{ BN6_NAVI_ROUTINES, { BN6_NAVI_ROUTINES_REF, BN6_NAVI_ROUTINES_REF2 }, BN6_NAVI_ROUTINE_COUNT, XC_NAVI, XC_STUBS + 4 },
};

/* the stubs (the project's own Thumb): a support routine that spawns nothing (movs r0,#0; bx lr: the support family takes
 * r0 as the object spawned; a unit's calls to BN5's panel claim spawners go there too, their gate clear in every battle a
 * run fights: tools/xchiprecipe.py, STUBS) and a Navi routine that does nothing (bx lr; nop) */
static void stubs_stage(void) {
	uint8_t *p = xv_put(XC_STUBS, XC_STUBS_END - XC_STUBS, true);
	if (!p) return;
	put16(p, 0x2000);        /* (movs r0,#0; mov pc,lr) */
	put16(p + 2, 0x46F7);
	put16(p + 4, 0x46F7);    /* (mov pc,lr) */
	put16(p + 6, 0x46C0);
	put16(p + 8, 0x2001);    /* (movs r0,#1; mov pc,lr) */
	put16(p + 10, 0x46F7);
	put16(p + 12, 0x46C0);
	put16(p + 14, 0x46C0);
	put16(p + 16, 0x2000);   /* (movs r0,#0; subs r0,#1; movs r1,#26; mov pc,lr: BN5's point 0x14 of MegaMan's, BN5 0x08015164's answer) */
	put16(p + 18, 0x3801);
	put16(p + 20, 0x211A);
	put16(p + 22, 0x46F7);
	/* (BN5's point 0x1D of MegaMan's, 0x08015140's answer: x 40 by his facing, r2 his side and r3 his flip as 0x0800C6CE has them
	 * (1 - 2 x (side ^ flip)), y 20) */
	put16(p + 24, 0x2028);   /* movs r0,#0x28 */
	put16(p + 26, 0x405A);   /* eors r2,r3 */
	put16(p + 28, 0xD000);   /* beq +2 */
	put16(p + 30, 0x4240);   /* negs r0,r0 */
	put16(p + 32, 0x2114);   /* movs r1,#0x14 */
	put16(p + 34, 0x46F7);   /* mov pc,lr */
	/* (DarkInvs's three, src/emu/darkinvs.c's hooks on them: mov pc,lr; nop each) */
	for (uint32_t k = XC_STUB_DARKINVS; k < XC_SP_TIMES - XC_STUBS; k += 4) {
		put16(p + k, 0x46F7);
		put16(p + k + 2, 0x46C0);
	}
	/* (the SP powers' deletion times, none: 0xFFFF each, both sides) */
	for (uint32_t k = XC_SP_TIMES - XC_STUBS; k < XC_STUBS_END - XC_STUBS; k += 2) put16(p + k, 0xFFFF);
}

/* a table copied, BN6's entries as the player's ROM has them (the core's copy the same), the rest its stub; its words
 * pointed at the copy */
static void routines_stage(const Routines *t) {
	uint8_t *p = xv_put(t->to, 4u * XC_ROUTINES, true);
	for (uint32_t i = 0; p && i < XC_ROUTINES; ++i) {
		uint32_t v = i < t->count ? rom_u32(t->at - BUS + 4 * i) : t->stub | 1u;
		if (i < t->count && emu_read32(t->at + 4 * i) != v) { xv_fail("BN6's routine %u at %08x was changed", (unsigned)i, (unsigned)t->at); return; }
		put32(p + 4 * i, v);
	}
	for (int k = 0; k < 2 && !xv_failed(); ++k)
		if (!t->refs[k]) continue;   /* (a table one word names: the misc family's) */
		else if (rom_u32(t->refs[k] - BUS) == t->at) xv_word(t->refs[k], t->at, t->to);
		else xv_fail("the word at %08x names %08x, not BN6's %08x", (unsigned)t->refs[k], (unsigned)rom_u32(t->refs[k] - BUS), (unsigned)t->at);
}

/* the stubs and the two tables staged in this step, where no earlier one grew them: whether it did (a unit's entry is then
 * put into the staged table, else changed in the grown one: entry_stage) */
static bool tables_stage(void) {
	if (tables_grown) return false;
	stubs_stage();
	for (int k = 0; k < 2 && !xv_failed(); ++k) routines_stage(&tables[k]);
	return true;
}

/* ---- a unit ---- */

/* (a unit's code past the homes, a helper's before the computed powers; the calls of either to the stubs or the helpers'
 * code, each into a helper before it: helper_calls_check) */
static const XvRegion region = { XC_CODE, XC_ART, XC_ART, XC_END, XC_STUBS, XC_POWERS };
static const XvRegion helpers = { XC_HELPERS, XC_POWERS, XC_ART, XC_END, XC_STUBS, XC_POWERS };

/* each call of a unit's into the helpers' code: to code of a helper installed before it (its copy of a BN5 routine) */
static void helper_calls_check(const XChipUnit *u, int at) {
	for (int i = 0; i < u->code.ncalls && !xv_failed(); ++i) {
		uint32_t to = u->code.calls[i].to;
		if (to < XC_HELPERS || to >= XC_POWERS) continue;
		bool ok = false;
		for (int k = 0; k < at && !ok; ++k) {
			const XChipUnit *h = &xchips_bn5[k];
			for (int b = 0; h->kind == XCHIP_HELPER && b < h->code.nblocks && !ok; ++b) {
				const XvBlock *x = &h->code.blocks[b];
				ok = to >= x->at && to < x->at + (x->end - x->start) && xvirus_is_code(&h->code, x->start + (to - x->at));
			}
		}
		if (!ok) xv_fail("its call %08x to %08x, no helper's code", (unsigned)u->code.calls[i].from, (unsigned)to);
	}
}

/* A waiting misc unit's wrapper (XChipUnit.wait), written at its home's end: the table's entry in its routine's place, its
 * address (Thumb), 0 failed. BN6's misc run calls a routine and ends the attack in the same frame (BN6_MISC_RUN: phase 1
 * stored, then mov lr,pc; bx r7 with the misc state pushed; only its own subfamily 0x14 waits, counting to below 0), BN4's
 * waits BN4's frames for the unit's subfamily, counting to 0 (BN4_MISC_WAIT). So the wrapper is the run's routine every
 * frame of the wait: the first time (the state's +1 is 0, cleared with the phase as an attack is set) it calls the routine
 * the same way, marks +1 and sets the count (+0x10) to BN4's; then, every time, it counts as BN4's run does and either
 * goes on into BN6's run where that ends the attack (BN6_MISC_END) or clears the phase, so the run calls it again the next
 * frame, and leaves by the run's way out (BN6_MISC_EXIT). The count holds BN4's numbers each frame.
 *   00 push {r0,r1}   02 ldr r1,[sp,#8]   04 ldrb r0,[r1,#1]   06 cmp r0,#0   08 pop {r0,r1}   0A bne 1C
 *   0C ldr r7,=routine   0E mov lr,pc   10 bx r7   12 pop {r7}   14 movs r0,#1   16 strb r0,[r7,#1]   18 movs r0,#wait   1A b 20
 *   1C pop {r7}   1E ldrh r0,[r7,#0x10]   20 subs r0,#1   22 strh r0,[r7,#0x10]   24 bgt 2A   26 ldr r0,=BN6_MISC_END   28 bx r0
 *   2A movs r0,#0   2C strb r0,[r7,#0]   2E ldr r0,=BN6_MISC_EXIT   30 bx r0   32 nop   34 the three words */
static uint32_t wait_stage(const XChipUnit *u, const XVirusFamily *code, uint32_t routine) {
	static const uint16_t thumb[26] = {
		0xB403, 0x9902, 0x7848, 0x2800, 0xBC03, 0xD107, 0x4F09, 0x46FE, 0x4738, 0xBC80, 0x2001, 0x7078, 0x2000, 0xE001,
		0xBC80, 0x8A38, 0x3801, 0x8238, 0xDC01, 0x4804, 0x4700, 0x2000, 0x7038, 0x4803, 0x4700, 0x46C0,
	};
	uint32_t at = u->home + XC4_HOME_WAIT;
	if (u->kind != XCHIP_MISC || XC4_HOME_VENEERS + xvirus_shims_room(code) > XC4_HOME_WAIT) {
		xv_fail("its wait %d: no misc unit's, or its shims reach its wrapper", u->wait);
		return 0;
	}
	uint8_t *p = xv_put(at, XC4_WAIT_SIZE, true);
	if (!p) return 0;
	for (int i = 0; i < 26; ++i) put16(p + 2 * i, i == 12 ? (uint32_t)(thumb[i] | u->wait) : thumb[i]);
	put32(p + 0x34, routine | 1u);
	put32(p + 0x38, BN6_MISC_END | 1u);
	put32(p + 0x3C, BN6_MISC_EXIT | 1u);
	return at | 1u;
}

/* A BN4 Navi chip's unit whose object plays a sound as it steps MegaMan in (XChipUnit.step_in: GutPnch's 0xB0, BN4
 * 0x080E91BC), which BN6's one Navi object does not: the word of BN6's object's states that names its last state
 * (BN6_CHIP_NAVI_IN_REF) aimed at a stub in the unit's home (XC4_HOME_WAIT). For this unit's number, on the state's first
 * frame, the stub steps MegaMan in as BN6's state does, plays the slot and goes on in BN6's state after its step's call;
 * for every other chip, and on the wait's frames, it is the routine the word named before (BN6's own, or another unit's
 * stub):
 *   00 ldrb r0,[r5,#0x19]  02 cmp r0,#number  04 bne 24  06 ldrb r0,[r5,#0xb]  08 cmp r0,#0  0A bne 24  0C push {lr}
 *   0E ldr r0,[r5,#0x4c]  10 movs r1,#0  12 ldr r3,=step  14 mov lr,pc  16 bx r3  18 ldr r0,=slot  1A ldr r3,=play
 *   1C mov lr,pc  1E bx r3  20 ldr r0,=the wait  22 bx r0  24 ldr r0,=before  26 bx r0  28 the five words */
static void step_in_stage(const XChipUnit *u) {
	static const uint16_t thumb[20] = {
		0x7E68, 0x2800, 0xD10E, 0x7AE8, 0x2800, 0xD10B, 0xB500, 0x6CE8, 0x2100, 0x4B06,
		0x46FE, 0x4718, 0x4805, 0x4B06, 0x46FE, 0x4718, 0x4805, 0x4700, 0x4800, 0x4700,
	};
	uint32_t at = u->home + XC4_HOME_WAIT, before = xv_read32(BN6_CHIP_NAVI_IN_REF);
	bool ours = (before & ~1u) >= XC4_HOMES && (before & ~1u) < XC4_CODE;
	if (u->kind != XCHIP_NAVI || u->wait || XC4_HOME_VENEERS + xvirus_shims_room(&u->code) > XC4_HOME_WAIT ||
	    (before != (BN6_CHIP_NAVI_IN | 1u) && !ours)) {
		xv_fail("its step-in sound %#x: no Navi chip's unit, its shims reach its stub, or BN6's Navi object's last state is not at %08x",
			u->step_in, (unsigned)BN6_CHIP_NAVI_IN);
		return;
	}
	uint8_t *p = xv_put(at, XC4_WAIT_SIZE, true);
	if (!p) return;
	for (int i = 0; i < 20; ++i) put16(p + 2 * i, i == 1 ? (uint32_t)(thumb[i] | u->number) : thumb[i]);
	put32(p + 0x28, before);
	put32(p + 0x2C, BN6_CHIP_NAVI_STEP | 1u);
	put32(p + 0x30, u->step_in);
	put32(p + 0x34, BN6_PLAY_SOUND | 1u);
	put32(p + 0x38, BN6_CHIP_NAVI_IN_WAIT | 1u);
	xv_word(BN6_CHIP_NAVI_IN_REF, emu_read32(BN6_CHIP_NAVI_IN_REF), at | 1u);
}

/* its entry in its table: the grown attack families' (its family), or the grown routines' (its subfamily), each holding
 * its stub first; `fresh` where the routines' tables are staged in this step (their stub there already), else grown by an
 * earlier one (the other game's units'); `code` the unit's own, or the unit's whose block holds its routine (XChipUnit.share) */
static void entry_stage(const XChipUnit *u, const XVirusFamily *code, bool fresh) {
	uint32_t at = 0, stub = 0, to = xvirus_code_at(code, u->entry | 1u);
	/* (BN5's attack families in the chips' range, BN4's in its own: unit4_stage holds each to its game's) */
	if (u->kind == XCHIP_ATTACK && ((u->number >= XATTACK_CHIPS && u->number < XATTACK_CHIPS_END) || u->number >= XATTACK_BN4)) {
		at = xattack_entry(u->number);
		stub = XATTACK_STUB | 1u;
	} else if (u->kind == XCHIP_MISC && u->number >= XC4_FIRST_SUPPORT) {
		/* (BN4's misc routines: their table staged in their own step, the entry put) */
		at = misc_table.to + 4u * u->number;
		stub = misc_table.stub | 1u;
		fresh = true;
	} else if (u->kind != XCHIP_ATTACK && u->kind != XCHIP_MISC && u->number >= (u->kind == XCHIP_SUPPORT ? XC_FIRST_SUPPORT : XC_FIRST_NAVI)) {
		const Routines *t = &tables[u->kind == XCHIP_NAVI];
		at = t->to + 4u * u->number;
		stub = t->stub | 1u;
	}
	if (!at || !to) {
		xv_fail("its number %#x or its entry %08x", u->number, (unsigned)u->entry);
		return;
	}
	/* (a unit BN4's misc run waits for: its wrapper the entry) */
	if (u->wait && !(to = wait_stage(u, code, to))) return;
	/* (a Navi chip's unit whose object sounds as it steps MegaMan in: BN6's object's last state through its stub) */
	if (u->step_in) step_in_stage(u);
	if (xv_failed()) return;
	/* (the grown routines are staged in this step: their stub there already; the attack families' grown before it) */
	if (u->kind == XCHIP_ATTACK || !fresh) xv_word(at, stub, to);
	else {
		uint8_t *p = xv_put(at, 4, false);
		if (p) put32(p, to);
	}
}

/* its sounds' songs copied into their slots (before the step: the songs' copies are xsong.c's own, made at once; a song not
 * copied fails the unit, its immediates naming the slot) */
static void songs_in(const XChipUnit *u) {
	for (int i = 0; i < u->nsongs && !xv_failed(); ++i)
		if (xsong_install_at(u->code.game, u->songs[i].song, u->songs[i].slot) != u->songs[i].slot)
			xv_fail("its sound %#x not copied to slot %#x", u->songs[i].song, u->songs[i].slot);
}

/* BN6's computed powers grown (XC_POWERS) in this step where no earlier one grew them: BN6's own entries as the player's ROM
 * has them, the rest the stub (power 0), its word pointed at the copy; the staged table, NULL where an earlier step grew it
 * (an entry is then changed in the grown one: power_put) or on a failure */
static uint8_t *powers_table_stage(void) {
	if (powers_grown) return NULL;
	uint8_t *p = xv_put(XC_POWERS, 4u * 256u, true);
	if (!p) return NULL;
	for (uint32_t i = 0; i < 256u; ++i) put32(p + 4 * i, i < BN6_CHIP_POWERS_COUNT ? rom_u32(BN6_CHIP_POWERS - BUS + 4 * i) : XC_STUBS | 1u);
	xv_word(BN6_CHIP_POWERS_REF, BN6_CHIP_POWERS, XC_POWERS);
	return p;
}

/* BN6's grown entry `i` pointed at the routine `to`: in the table staged in this step (`fresh`), or the grown one's stub changed */
static void power_put(uint8_t *fresh, uint32_t i, uint32_t to) {
	if (fresh) put32(fresh + 4u * i, to);
	else xv_word(XC_POWERS + 4u * i, XC_STUBS | 1u, to);
}

/* BN5's computed powers: the routines the helpers moved at their entries of the chips' (xchips_powers: XC_POWER_FIRST + BN5's) */
static void powers_stage(void) {
	const uint8_t *rom5 = XR[XROM_BN5_COLONEL_US].data;
	uint8_t *p = powers_table_stage();
	for (int k = 0; k < xchips_powers_count && !xv_failed(); ++k) {
		const XChipPower *w = &xchips_powers[k];
		uint32_t routine = w->bn5 < BN5_CHIP_POWERS_COUNT ? get32(rom5 + (BN5_CHIP_POWERS - BUS) + 4u * w->bn5) : 0, to = 0;
		for (int i = 0; i < xchips_bn5_count && !to; ++i)
			if (xchips_bn5[i].kind == XCHIP_HELPER && xvirus_is_code(&xchips_bn5[i].code, routine & ~1u)) to = xvirus_code_at(&xchips_bn5[i].code, routine | 1u);
		if (!to || w->bn6 < XC_POWER_FIRST) xv_fail("BN5's computed power %d (%08x) moved by no helper", 1000 + w->bn5, (unsigned)routine);
		else power_put(p, w->bn6, to);
	}
}

/* BN4's computed powers: each entry of BN4's table (BN4_CHIP_POWERS) whose routine its power unit moved (xchips_bn4_powers,
 * the routine of an XCHIP_HELPER of xchips_bn4: tools/xchiprecipe_bn4.py POWER4_ROUTINES) at BN6's XREG4_POWER_FIRST + the
 * entry, which a moved chip's record names (src/core/xchips_record.c); the rest the stub */
static void powers4_stage(void) {
	uint8_t *p = powers_table_stage();
	for (int e = 0; e < xchips_bn4_powers_count && !xv_failed(); ++e) {
		uint32_t routine = xchips_bn4_powers[e], to = 0;
		for (int i = 0; routine && i < xchips_bn4_count && !to; ++i)
			if (xchips_bn4[i].kind == XCHIP_HELPER && xvirus_is_code(&xchips_bn4[i].code, routine & ~1u)) to = xvirus_code_at(&xchips_bn4[i].code, routine | 1u);
		if (routine && !to) xv_fail("BN4's computed power %d (%08x) moved by no unit", 1000 + e, (unsigned)routine);
		else if (to) power_put(p, XREG4_POWER_FIRST + (uint32_t)e, to);
	}
}

/* BN6's after-effects of its own DarkChips (BN6_DARK_AFTERS): DarkPlus's (its HP bug) the Navi stub's return where BN5's
 * DarkPlus runs in BN6's record of its name (src/core/chips.c, xrec_same_moved), which BN5's dark rule follows (soulmood.c's
 * after-effect, BN5_DARK_AFTER's), not BN6's */
static void dark_after_stage(void) {
	if (!xchips_moved_of(BN6_DARKPLUS)) return;
	xv_word(BN6_DARK_AFTERS + 4u * (BN6_DARKPLUS - BN6_DARK_FIRST_ID), BN6_DARK_AFTER_HPBUG | 1u, (XC_STUBS + 4u) | 1u);
}

/* a game's recipes (`rows`) written into the recipes' room `e` from *at on, their addresses into the list `l` from entry *k
 * on; false where one does not fit */
static bool recipes_put(const XChipPa *rows, int n, uint8_t *e, uint32_t room, uint32_t *at, uint8_t *l, uint32_t *k) {
	for (int i = 0; i < n; ++i) {
		const XChipPa *r = &rows[i];
		uint32_t parts = r->kind == 4 ? r->n : 1;
		if (parts > XCHIP_PA_PARTS || *at + 4 + 2 * parts > room) return xv_fail("a Program Advance's recipe, %u", r->pa);
		e[*at] = r->n;
		e[*at + 1] = r->kind;
		put16(e + *at + 2, r->pa);
		for (uint32_t j = 0; j < parts; ++j) put16(e + *at + 4 + 2 * j, r->parts[j]);
		put32(l + 4u * (*k)++, FREE_CHIP_PAS + *at);
		*at += 4 + 2 * parts;
	}
	return true;
}

/* BN6's list of Program Advances grown (FREE_CHIP_PAS): BN6's 43 recipes, then BN5's of the registry's (src/core/xchips_pas.c,
 * BN6's first where two are the same) where BN5's chips' code is in (`bn5`), then BN4's (src/core/xchips_pas_bn4.c) where
 * BN4's is (`bn4`), then 0; its word pointed at it (the link battles' list, 0x0802BC60, BN6's own); the P.A. screen's code
 * letters drawn for every registered part (BN5's 0x08027BCE draws them below 0x160 as BN6's; BN6's own parts are all below
 * it, the registry's SpShake and CactBal past it). The list's place is one for both games' recipes, so each game's step
 * writes the whole anew: the second finds the word at it already */
static void pas_stage(bool bn5, bool bn4) {
	uint32_t list = FREE_CHIP_PAS_END - 4u * (BN6_PA_COUNT + (uint32_t)xchips_pas_count + (uint32_t)xchips_pas4_count + 1u), at = 0;
	bool grown = emu_read32(BN6_PA_LIST_REF) == list;
	if (rom_u32(BN6_PA_LIST_REF - BUS) != BN6_PA_LIST || rom_u32(BN6_PA_LIST - BUS + 4u * BN6_PA_COUNT) != 0) {
		xv_fail("BN6's Program Advances' list is not at %08x", (unsigned)BN6_PA_LIST);
		return;
	}
	uint8_t *e = xv_put(FREE_CHIP_PAS, list - FREE_CHIP_PAS, !grown), *l = xv_put(list, FREE_CHIP_PAS_END - list, !grown);
	if (!e || !l) return;
	uint32_t k = BN6_PA_COUNT;
	for (uint32_t i = 0; i < BN6_PA_COUNT; ++i) put32(l + 4 * i, rom_u32(BN6_PA_LIST - BUS + 4 * i));
	if (bn5 && !recipes_put(xchips_pas, xchips_pas_count, e, list - FREE_CHIP_PAS, &at, l, &k)) return;
	if (bn4 && !recipes_put(xchips_pas4, xchips_pas4_count, e, list - FREE_CHIP_PAS, &at, l, &k)) return;
	put32(l + 4u * k, 0);
	if (grown) return;
	xv_word(BN6_PA_LIST_REF, BN6_PA_LIST, list);
	xv_word(BN6_PA_LETTERS, 0x160, CHIP_ID_MASK + 1u);   /* (a part of any id its letter: a Navi chip past nine bits too) */
}

/* ---- the step's writes, each part under its own claims ---- */

static bool in_room(uint32_t at) { return at >= FREE_VIRUS_ROOM && at < FREE_FAMILIES; }
static bool in_attacks(uint32_t at) { return at >= FREE_ATTACK_FAMILIES && at < FREE_ATTACK_FAMILIES_END; }
static bool own(uint32_t at) { return !in_room(at) && !in_attacks(at); }

static void commit(void) {
	claims_begin(XC_CLAIMS);
	xv_commit_where(own);
	claims_end();
	claims_begin(XATTACK_CLAIMS);
	xv_commit_where(in_attacks);
	claims_end();
	claims_begin(XV_CLAIMS);
	xv_commit_where(in_room);
	claims_end();
}

/* the Navis' battle sprites the units draw, each the Navis' one copy at the number the units' loads name (before the
 * step, which they are not written in): false where one is not there */
static bool navi_sprites(void) {
	for (int i = 0; i < xchips_bn5_count; ++i)
		for (int n = BN5_NAVI_COLONEL; n <= BN5_NAVI_GRIDMAN; ++n)
			if ((xchips_bn5[i].navis >> XN_SEAT(n) & 1) &&
				(xnavi_sprite_number(n) != XNAVI_SPRITE_FIRST + XN_SEAT(n) || !xnavi_sprite_install(n))) {
				fprintf(stderr, "xchips_code: BN5's chips' code not installed (%s): BN5's Navi %d's battle sprite\n", xchips_bn5[i].code.name, n);
				return false;
			}
	return true;
}

bool xchips_code_install(void) {
	if (installed) return true;
	if (!XR[XROM_BN5_COLONEL_US].data || !emu_ready() || !R.data) return false;
	claims_begin(XV_CLAIMS);
	bool grown = xg_grow();
	claims_end();
	if (!grown || !xattack_grow()) {
		fprintf(stderr, "xchips_code: BN5's chips' code not installed: BN6's tables not grown\n");
		return false;
	}
	if (!navi_sprites()) return false;
	xv_tx_begin();
	bool fresh = tables_stage();
	const char *at = "BN6's routines' tables";
	for (int i = 0; i < xchips_bn5_count && !xv_failed(); ++i) {
		const XChipUnit *u = &xchips_bn5[i];
		at = u->code.name;
		if (u->home < XC_HOMES || u->home + XC_HOME_SIZE > XC_CODE || (u->home - XC_HOMES) % XC_HOME_SIZE) xv_fail("its home %08x", (unsigned)u->home);
		else if (u->kind == XCHIP_HELPER) {
			helper_calls_check(u, i);
			if (!xv_failed()) xvirus_code_stage(&u->code, &helpers, u->home, (int)(XC_HOME_SIZE / 12));
		} else {
			helper_calls_check(u, i);
			songs_in(u);
			if (!xv_failed() && xvirus_code_stage(&u->code, &region, u->home, (int)(XC_HOME_SIZE / 12))) entry_stage(u, &u->code, fresh);
		}
	}
	if (!xv_failed()) {
		at = "BN6's computed powers";
		powers_stage();
	}
	if (!xv_failed()) {
		at = "BN6's DarkPlus's after-effect";
		dark_after_stage();
	}
	if (!xv_failed()) {
		at = "BN6's Program Advances";
		pas_stage(true, installed4);
	}
	if (xv_failed()) {
		fprintf(stderr, "xchips_code: BN5's chips' code not installed (%s): %s\n", at, xv_why());
		xv_tx_end();
		return false;
	}
	uint32_t used = xv_used();
	commit();
	xv_tx_end();
	for (int i = 0; i < xchips_bn5_count; ++i)
		for (int k = 0; k < xchips_bn5[i].code.nblocks; ++k) {
			const XvBlock *b = &xchips_bn5[i].code.blocks[k];
			xv_moved_add(b->start, b->at, b->end - b->start);
		}
	installed = tables_grown = powers_grown = true;
	printf("xchips_code: %d units of BN5's chips' code (%u bytes written)\n", xchips_bn5_count, (unsigned)used);
	xchips_flash_install();
	return true;
}

/* A unit whose moved object the actors' runner runs (its code's `states`: TimeBomb's bomb): its state table, BN6's
 * layout, written at its home's XC4_HOME_STATES after its code is moved */
static void states_stage(const XChipUnit *u) {
	const XVirusFamily *f = &u->code;
	if (XC4_HOME_VENEERS + xvirus_shims_room(f) > XC4_HOME_STATES) {
		xv_fail("its shims reach its state table");
		return;
	}
	uint8_t *st = xv_put(u->home + XC4_HOME_STATES, 4u * (uint32_t)f->nstates, true);
	for (int i = 0; st && i < f->nstates; ++i) {
		uint32_t to = f->states[i].own ? xvirus_code_at(f, f->states[i].addr | 1u) : f->states[i].addr | 1u;
		if (!to) xv_fail("its state %d (%08x) is no code of its own", i, (unsigned)f->states[i].addr);
		put32(st + 4 * i, to);
	}
}

/* ---- BN4's units (xchips_bn4.h): support and misc routines and attack families (a state of BN4's MegaMan's at a number
 * of the grown attack families' own, XATTACK_BN4 on), each in its own region ---- */

static const XvRegion region4 = XC4_REGION;

/* BN4's unit `i` staged: its home and number checked, its sounds copied, its code moved and its entry put; or, for a row
 * that shares an earlier unit's code (XChipUnit.share), its entry alone */
static void unit4_stage(int i, bool fresh) {
	const XChipUnit *u = &xchips_bn4[i];
	if (u->home < XC4_HOMES || u->home + XC4_HOME_SIZE > XC4_CODE || (u->home - XC4_HOMES) % XC4_HOME_SIZE) xv_fail("its home %08x", (unsigned)u->home);
	else if (u->kind == XCHIP_ATTACK ? u->number < XATTACK_BN4 || u->share
	         : u->kind == XCHIP_HELPER ? u->number || u->share
	         : (u->kind != XCHIP_SUPPORT && u->kind != XCHIP_MISC && u->kind != XCHIP_NAVI) || u->number != XC4_FIRST_SUPPORT + u->bn5) xv_fail("its kind %d or its number %#x", u->kind, u->number);
	else if (u->kind == XCHIP_NAVI && (u->share || !u->navis || !xnavi_bn4_seat(u->navis - 1))) xv_fail("its Navi's seat %d, no installed BN4 Navi's", u->navis - 1);
	else if (u->share) {
		/* (a routine of an earlier unit's block: its entry alone) */
		if (u->share > i || xchips_bn4[u->share - 1].share || u->code.nblocks) xv_fail("its code's unit %d", u->share - 1);
		else entry_stage(u, &xchips_bn4[u->share - 1].code, fresh);
	} else if (u->code.home != XC4_SHIMS_HOME(u->home)) xv_fail("its shims' place %08x", (unsigned)u->code.home);
	else {
		songs_in(u);
		/* (a power unit's routines named by BN6's grown computed powers alone, powers4_stage: no entry of a table) */
		if (!xv_failed() && xvirus_code_stage(&u->code, &region4, u->home, (int)(XC4_HOME_VENEERS / 12)) && u->kind != XCHIP_HELPER) {
			entry_stage(u, &u->code, fresh);
			if (u->code.nstates) states_stage(u);
		}
	}
}

bool xchips_bn4_code_at(uint32_t to) { return to >= XC4_CODE && to < XC4_ART; }

bool xchips_bn4_moved(int id) {
	const XChipReg4 *r = xreg4_at(id);
	return r && r->how == XREG_MOVED;
}

/* XC4_COLL_PANEL_WORD (the project's own Thumb, xchips_bn4.h): r0 the word BN4's collision data keeps at +8, from BN6's +0xA
 * and +0xC; push and pop r1 around it */
static void coll_panel_word_stage(void) {
	static const uint16_t code[] = {
		0xB402,   /* push {r1} */
		0x6D68,   /* ldr r0, [r5, #0x54]: its collision data */
		0x8941,   /* ldrh r1, [r0, #0xa]: its panel's column and row */
		0x8980,   /* ldrh r0, [r0, #0xc]: its panel's byte and its counter time */
		0x0400,   /* lsls r0, r0, #16 */
		0x4308,   /* orrs r0, r1 */
		0xBC02,   /* pop {r1} */
		0x46F7,   /* mov pc, lr */
	};
	_Static_assert(XC4_COLL_PANEL_WORD + sizeof code <= FREE_BN4_CHIP_STUBS_END, "the panel word's room");   /* (its fields: tests/test_core.c) */
	uint8_t *p = xv_put(XC4_COLL_PANEL_WORD, sizeof code, true);
	for (size_t i = 0; p && i < sizeof code / sizeof *code; ++i) put16(p + 2 * i, code[i]);
}

/* XC4_FIRST_ACTOR (the project's own Thumb, xchips_bn4.h): r0 the side; its four actors' words from BN6's battle state
 * (the toolkit's +0x18) at BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP, the first that is one in r0, else 0;
 * mov pc,lr as the stubs */
static void first_actor_stage(void) {
	static const uint16_t code[] = {
		0x4651,   /* mov r1, r10 */
		0x6989,   /* ldr r1, [r1, #0x18]: the battle state */
		0x0100,   /* lsls r0, r0, #4: a side's four words */
		0x1809,   /* adds r1, r1, r0 */
		0x31D0,   /* adds r1, #0xd0: BN6_BATTLE_PLAYERS */
		0x2204,   /* movs r2, #4 */
		0x6808,   /* ldr r0, [r1] */
		0x2800,   /* cmp r0, #0 */
		0xD102,   /* bne past */
		0x3104,   /* adds r1, #4 */
		0x3A01,   /* subs r2, #1 */
		0xD1F9,   /* bne back to the load */
		0x46F7,   /* mov pc, lr */
		0x46C0,   /* nop */
	};
	_Static_assert(sizeof code <= FREE_BN4_CHIP_STUBS_END - FREE_BN4_CHIP_STUBS, "the first actor's room");   /* (its numbers: tests/test_core.c) */
	uint8_t *p = xv_put(XC4_FIRST_ACTOR, sizeof code, true);
	for (size_t i = 0; p && i < sizeof code / sizeof *code; ++i) put16(p + 2 * i, code[i]);
}

/* the chips' own routines (FREE_BN4_CHIP_STUBS) staged */
static void stubs4_stage(void) {
	first_actor_stage();
	if (!xv_failed()) coll_panel_word_stage();
}

/* BN4's units of one pass staged and committed: its Navi chips' (`navis`: after BN4's Navis), or the rest */
static bool units4_install(bool navis);

uint32_t xchips_bn4_moved_at(uint32_t bn4) {
	for (int i = 0; installed4 && i < xchips_bn4_count; ++i) {
		uint32_t at = xchips_bn4[i].share ? 0 : xvirus_code_at(&xchips_bn4[i].code, bn4);
		if (at) return at;
	}
	return 0;
}

bool xchips_bn4_code_install(void) {
	if (installed4) return true;
	installed4 = units4_install(false);
	return installed4;
}

bool xchips_bn4_navi_install(void) {
	if (installed4n) return true;
	installed4n = installed4 && units4_install(true);
	return installed4n;
}

static bool units4_install(bool navis) {
	if (!XR[XROM_BN4_RED_SUN_US].data || !emu_ready() || !R.data) return false;
	claims_begin(XV_CLAIMS);
	bool room = xg_grow();
	claims_end();
	if (!room || !xattack_grow()) {
		fprintf(stderr, "xchips_code: BN4's chips' code not installed: BN6's tables not grown\n");
		return false;
	}
	/* (their sounds BN6 has no song of go into the grown song table's slots, song_table.h: grown before the step) */
	if (!song_table_grow()) {
		fprintf(stderr, "xchips_code: BN4's chips' code not installed: BN6's song table not grown\n");
		return false;
	}
	xv_tx_begin();
	bool fresh = !navis && tables_stage();
	if (!navis && !xv_failed()) stubs4_stage();
	if (!navis && !xv_failed()) routines_stage(&misc_table);
	/* (the chips' own dispatch page staged once, by the first pass: staged again it would blank the types its units set) */
	if (!navis && !xv_failed()) xg_page4_stage();
	const char *at = "BN6's routines' tables";
	int units = 0;
	for (int i = 0; i < xchips_bn4_count && !xv_failed(); ++i) {
		if ((xchips_bn4[i].kind == XCHIP_NAVI) != navis) continue;
		at = xchips_bn4[i].code.name;
		unit4_stage(i, fresh);
		++units;
	}
	if (!navis && !xv_failed()) {
		at = "BN6's computed powers";
		powers4_stage();
	}
	if (!xv_failed()) {
		at = "BN6's Program Advances";
		pas_stage(installed, true);
	}
	if (xv_failed()) {
		fprintf(stderr, "xchips_code: BN4's chips' code not installed (%s): %s\n", at, xv_why());
		xv_tx_end();
		return false;
	}
	uint32_t used = xv_used();
	commit();
	xv_tx_end();
	for (int i = 0; i < xchips_bn4_count; ++i)
		for (int k = 0; (xchips_bn4[i].kind == XCHIP_NAVI) == navis && k < xchips_bn4[i].code.nblocks; ++k) {
			const XvBlock *b = &xchips_bn4[i].code.blocks[k];
			xv_moved_add(b->start, b->at, b->end - b->start);
		}
	tables_grown = true;
	if (!navis) powers_grown = true;
	printf("xchips_code: %d units of BN4's %s (%u bytes written)\n", units, navis ? "Navi chips' code" : "chips' code", (unsigned)used);
	/* (their art: none installed, a room filled as an object loads a sprite, xchips_bn4_art.h; a Navi chip's Navi draws
	 * the Navis' own copy, xnavi_bn4_chip.c) */
	if (!navis) xchips_bn4_art_install();
	return true;
}
