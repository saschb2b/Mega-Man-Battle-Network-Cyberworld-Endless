/* turned_aim.h: MegaMan's side's aim between a Surrounded field's two sides. */
#include "turned_aim.h"

#include <stdio.h>

#include "bn4_chips.h"
#include "bn6.h"
#include "bn6_forms.h"
#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "xchips_code.h"
#include "xnavi_bn4.h"
#include "xnavi_room.h"
#include "xvirus.h"

#define BUS 0x08000000u   /* (the cartridge on the bus: a ROM address's offset in the file) */

/* Whether the object at `o` is one of MegaMan's side's on a field none of BN6's plain ones */
static bool mends(uint32_t o) {
	uint32_t settings = hook_read32(BN6_BATTLE_SETTINGS);
	return settings && turned_aim_mends(hook_read8(settings + BN6_SETTINGS_SIDES), hook_read8(o + BN6_T1_ALLIANCE));
}

/* Whether MegaMan (side 0's player) stands turned round */
static bool turned(void) {
	uint32_t megaman = hook_read32(BN6_BATTLE_PLAYERS);
	return megaman && (hook_read8(megaman + BN6_T1_FLIP) & 1);
}

/* Whether `a` lies in BN4's moved code, its chips' units' or its Navis' (BN4 never turned MegaMan: its way toward the enemy
 * is the way he faces) */
static bool bn4_code(uint32_t a) {
	return xchips_bn4_code_at(a) || (a >= XN_BN4_CODE && a < XN_BN4_ART);
}

/* (BN6's search of a row, at its start's adds r0,#6: the object pushed with r4 and lr at the routine's entry, at [sp+4]) */
static HookAct row_start(HookRegs *r, void *user) {
	(void)user;
	uint32_t o = hook_read32(r->sp + 4);
	if (!mends(o)) return HOOK_CONTINUE;
	r->r[0] = hook_read8(o + BN6_T1_PANEL_X);
	if (emu_debug_on()) fprintf(stderr, "row search: from column %u, the object's own, step %d\n", (unsigned)r->r[0], (int)(int32_t)r->r[5]);
	r->r[12] = BN6_ROW_SEARCH_LOOP | 1;
	return HOOK_JUMP;
}

/* (object_getEnemyDirection's first instruction, r5 the object: the way it faces, as object_getFrontDirection gives it,
 * while MegaMan stands turned round; unturned, his battle runs as BN6's, his side's objects that face away by their own
 * flip (DblBeast's) aimed by side as BN6 aims them. Called from BN4's moved code, the way MegaMan faces: BN4 left its
 * objects' flip 0, its side alone their way, issue #846) */
static HookAct enemy_direction(HookRegs *r, void *user) {
	(void)user;
	uint32_t o = r->r[5];
	if (!mends(o) || !turned()) return HOOK_CONTINUE;
	r->r[0] = bn4_code(r->lr) || (hook_read8(o + BN6_T1_FLIP) & 1) ? (uint32_t)-1 : 1u;
	return HOOK_RETURN;
}

/* (BN6_ROW_ACROSS_WAY's eors r0,r1, r5 the object: its edge behind it by the way it faces, r1 1 ^ its flip) */
static HookAct row_across(HookRegs *r, void *user) {
	(void)user;
	if (mends(r->r[5])) r->r[1] = 1u ^ (hook_read8(r->r[5] + BN6_T1_FLIP) & 1);
	return HOOK_CONTINUE;
}

/* (a side read by the alliance alone, BN6's or BN4's, user: the register it went into (the alliance, or 1 where it is the
 * flip forced): turned round with MegaMan) */
static HookAct side_read(HookRegs *r, void *user) {
	int reg = (int)(intptr_t)user;
	if (mends(r->r[5]) && turned()) r->r[reg] ^= 1;
	return HOOK_CONTINUE;
}

/* (BN6_BLASTMAN_START's lsls r4,#8, r5 the object: his flames from his own column, the way he faces, as the row's search,
 * turned or not: from the field's edge behind him they found the enemy's behind first, as GolmHit's fist did) */
static HookAct blastman_start(HookRegs *r, void *user) {
	(void)user;
	if (!mends(r->r[5])) return HOOK_CONTINUE;
	r->r[0] = hook_read8(r->r[5] + BN6_T1_PANEL_X);
	r->r[4] = hook_read8(r->r[5] + BN6_T1_FLIP) & 1 ? 0u : 1u;
	return HOOK_CONTINUE;
}

/* Where BN4's instruction at `bn4` was moved, its bytes there BN4's (0 none): its chips' units', or its Navis' code (`navi`) */
static uint32_t bn4_moved(uint32_t bn4, bool *navi) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	uint32_t at = b4 ? xchips_bn4_moved_at(bn4) & ~1u : 0;
	*navi = false;
	for (int i = 0; b4 && !at && i < xnavi_bn4_count; ++i) *navi = (at = xvirus_code_at(&xnavi_bn4[i]->code, bn4) & ~1u) != 0;
	return at && emu_read16(at) == (b4[bn4 - BUS] | b4[bn4 - BUS + 1] << 8) ? at : 0;
}

void turned_aim_install(void) {
	if (!emu_hook(BN6_ENEMY_DIRECTION, enemy_direction, NULL))
		fprintf(stderr, "turned aim: no hook at %08x: chips aim by side alone on a Surrounded field\n", (unsigned)BN6_ENEMY_DIRECTION);
	if (!emu_hook(BN6_ROW_SEARCH_START, row_start, NULL))
		fprintf(stderr, "turned aim: no hook at %08x: GolmHit's fist strikes behind MegaMan on a Surrounded field\n", (unsigned)BN6_ROW_SEARCH_START);
	if (!emu_hook(BN6_ROW_ACROSS_WAY, row_across, NULL) || !emu_hook(BN6_ROW_ACROSS_WAY2, row_across, NULL))
		fprintf(stderr, "turned aim: no hook at %08x: BN4's Meteors fall behind MegaMan on a Surrounded field\n", (unsigned)BN6_ROW_ACROSS_WAY);
	if (!emu_hook(BN6_BLASTMAN_START, blastman_start, NULL))
		fprintf(stderr, "turned aim: no hook at %08x: BlastMan's flames burn behind MegaMan on a Surrounded field\n", (unsigned)BN6_BLASTMAN_START);
	static const struct { uint32_t at; int reg; } own[] = { { BN6_SANDWORM_FLIP, 0 }, { BN6_SANDWORM_FLIP2, 0 }, { BN6_BURNSQR_START, 0 },
		{ BN6_TENGU_DASH, 0 }, { BN6_TENGU_DIVE, 0 }, { BN6_AIRRAID_EDGE, 0 }, { BN6_MAGNUM_START, 0 } };
	for (size_t i = 0; i < sizeof own / sizeof *own; ++i)
		if (!emu_hook(own[i].at, side_read, (void *)(intptr_t)own[i].reg))
			fprintf(stderr, "turned aim: no hook at %08x: a chip strikes behind MegaMan on a Surrounded field\n", (unsigned)own[i].at);
	/* (BN4's own: where its ROM is read and the code moved) */
	static const struct { uint32_t bn4; int reg; } reads[] = { { BN4_SEEKBOM_AHEAD, 1 }, { BN4_PITRNG_EDGE, 0 }, { BN4_GUTSMAN_EDGE, 0 } };
	for (size_t i = 0; i < sizeof reads / sizeof *reads; ++i) {
		bool navi;
		uint32_t at = bn4_moved(reads[i].bn4, &navi);
		/* (a hook on the moved code's bytes claimed as theirs, as darkinvs.c's are) */
		claims_begin(navi ? XN_CLAIMS : XC_CLAIMS);
		bool ok = !at || emu_hook(at, side_read, (void *)(intptr_t)reads[i].reg);
		claims_end();
		if (!ok)
			fprintf(stderr, "turned aim: no hook at %08x (BN4's %08x): a chip strikes behind MegaMan on a Surrounded field\n", (unsigned)at,
				(unsigned)reads[i].bn4);
	}
}

