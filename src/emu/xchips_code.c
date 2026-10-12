/* BN5's chips' code in the core's copy (xchips_code.h). */
#include "xchips_code.h"

#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "xattack_grow.h"
#include "xchips_bn5.h"
#include "xchips_moved.h"
#include "xvirus.h"
#include "xvirus_grow.h"
#include "xsong.h"
#include "xvirus_tx.h"

#define BUS 0x08000000u

static bool installed;

/* ---- BN6's support and Navi chips' routines grown ---- */

/* One of the two tables: the words that name it (BN6's, and its place among the support routines' for the Navi part), its
 * entries in BN6 and its copy */
typedef struct { uint32_t at, refs[2], count, to, stub; } Routines;

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
	for (int k = 24; k < 32; k += 2) put16(p + k, 0x46C0);
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
		if (rom_u32(t->refs[k] - BUS) == t->at) xv_word(t->refs[k], t->at, t->to);
		else xv_fail("the word at %08x names %08x, not BN6's %08x", (unsigned)t->refs[k], (unsigned)rom_u32(t->refs[k] - BUS), (unsigned)t->at);
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

/* its entry in its table: the grown attack families' (its family), or the grown routines' (its subfamily), each holding
 * its stub first */
static void entry_stage(const XChipUnit *u) {
	uint32_t at = 0, stub = 0, to = xvirus_code_at(&u->code, u->entry | 1u);
	if (u->kind == XCHIP_ATTACK && u->number >= XATTACK_CHIPS && u->number < XATTACK_CHIPS_END) {
		at = xattack_entry(u->number);
		stub = XATTACK_STUB | 1u;
	} else if (u->kind != XCHIP_ATTACK && u->number >= (u->kind == XCHIP_SUPPORT ? XC_FIRST_SUPPORT : XC_FIRST_NAVI)) {
		const Routines *t = &tables[u->kind == XCHIP_NAVI];
		at = t->to + 4u * u->number;
		stub = t->stub | 1u;
	}
	if (!at || !to) {
		xv_fail("its number %#x or its entry %08x", u->number, (unsigned)u->entry);
		return;
	}
	/* (the grown routines are staged in this step: their stub there already; the attack families' grown before it) */
	if (u->kind == XCHIP_ATTACK) xv_word(at, stub, to);
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

/* BN6's computed powers grown (XC_POWERS): BN6's own entries as the player's ROM has them, BN5's routines the helpers moved at
 * their entries of the chips' (xchips_powers: XC_POWER_FIRST + BN5's), the rest the stub; its word pointed at the copy */
static void powers_stage(void) {
	const uint8_t *rom5 = XR[XROM_BN5_COLONEL_US].data;
	uint8_t *p = xv_put(XC_POWERS, 4u * 256u, true);
	if (!p) return;
	for (uint32_t i = 0; i < 256u; ++i) put32(p + 4 * i, i < BN6_CHIP_POWERS_COUNT ? rom_u32(BN6_CHIP_POWERS - BUS + 4 * i) : XC_STUBS | 1u);
	for (int k = 0; k < xchips_powers_count && !xv_failed(); ++k) {
		const XChipPower *w = &xchips_powers[k];
		uint32_t routine = w->bn5 < BN5_CHIP_POWERS_COUNT ? get32(rom5 + (BN5_CHIP_POWERS - BUS) + 4u * w->bn5) : 0, to = 0;
		for (int i = 0; i < xchips_bn5_count && !to; ++i)
			if (xchips_bn5[i].kind == XCHIP_HELPER && xvirus_is_code(&xchips_bn5[i].code, routine & ~1u)) to = xvirus_code_at(&xchips_bn5[i].code, routine | 1u);
		if (!to || w->bn6 < XC_POWER_FIRST) xv_fail("BN5's computed power %d (%08x) moved by no helper", 1000 + w->bn5, (unsigned)routine);
		else put32(p + 4u * w->bn6, to);
	}
	if (!xv_failed()) xv_word(BN6_CHIP_POWERS_REF, BN6_CHIP_POWERS, XC_POWERS);
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
	xv_tx_begin();
	stubs_stage();
	for (int k = 0; k < 2 && !xv_failed(); ++k) routines_stage(&tables[k]);
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
			if (!xv_failed() && xvirus_code_stage(&u->code, &region, u->home, (int)(XC_HOME_SIZE / 12))) entry_stage(u);
		}
	}
	if (!xv_failed()) {
		at = "BN6's computed powers";
		powers_stage();
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
	installed = true;
	printf("xchips_code: %d units of BN5's chips' code (%u bytes written)\n", xchips_bn5_count, (unsigned)used);
	return true;
}
