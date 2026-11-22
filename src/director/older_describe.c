/* older_describe.h: the older nets in the state's words, read only. */
#include "older_describe.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bn6_forms.h"
#include "boss.h"
#include "chip_entry.h"
#include "chip_targets.h"
#include "data.h"
#include "director_state.h"
#include "forms.h"
#include "emu.h"
#include "guardians.h"
#include "net.h"
#include "netmap.h"
#include "npc_sprite.h"
#include "run.h"
#include "soulunite.h"
#include "souls.h"
#include "xchips_registry.h"
#include "xguardian_bn4.h"

/* The game an area is of, as a word: BN6's own, or the ROM its def names */
static const char *game_of_area(int area) {
	const NetAreaDef *d = area >= NET_AREAS ? net_area_def(area) : NULL;
	if (!d || d->xrom <= 0) return "bn6";
	return d->xrom - 1 == XROM_BN4_RED_SUN_US ? "bn4" : "bn5";
}

/* ... and a guardian's */
static const char *game_of_guardian(int navi) { return guardian_bn4(navi) ? "bn4" : guardian_moved(navi) ? "bn5" : "bn6"; }

/* The run's acts in order, each by the game whose area draws it (run_dress),
 * how many are an older net's, how many end at one of its Navis
 * (run_xguardian_at), and the Souls the run holds */
static void run_lines(FILE *f) {
	int acts = run.mode == RUN_SHORT ? 3 : 6, older = 0, guards = 0;
	fprintf(f, "acts");
	for (int a = 0; a < acts; ++a) {
		int biome = run.biome_order[a], area = run_dress(biome);
		fprintf(f, " %s", game_of_area(area));
		older += area >= NET_AREAS;
		guards += run_xguardian_at(a, biome) != 0;
	}
	/* (and BN5's Souls it holds, a bit each from Colonel's: souls.h; every
	 * Soul by its game's numbers, BN5's and BN4's, held and as the layer was
	 * made; MegaMan's form, issue #774) */
	fprintf(f, "\nolderacts %d\nolderguards %d\nsoulsheld %02x\n", older, guards, souls_held());
	fprintf(f, "soulsbn5 %03x %03x\nsoulsbn4 %03x %03x\n", souls_held_bn5(), souls_layer_bn5(), souls_held_bn4(), souls_layer_bn4());
	forms_describe(f);
}

/* The NPC object the map holds at world (x, y): its address, 0 none */
static uint32_t object_at(int x, int y) {
	for (int i = 0; i < BN6_NPC_COUNT; ++i) {
		uint32_t o = BN6_NPC_OBJECTS + (uint32_t)i * BN6_NPC_SIZE;
		if (!(emu_read8(o) & 1)) continue;
		if ((int16_t)emu_read16(o + BN6_NPC_X16) == x && (int16_t)emu_read16(o + BN6_NPC_Y16) == y) return o;
	}
	return 0;
}

/* A Glitch layer's copy on the map (docs/game/glitch-layer.md, The copy at
 * its heart): the Navi whose body it stands in first, and its NPC's
 * animation (a Falzar Navi's body is his battle sprite, whose animation 0
 * stands and 4 warps out: the copy stood in 4, 0.22.0) */
static void copy_lines(FILE *f) {
	for (int i = 0; i < layer.nobj; ++i) {
		if (layer.obj[i].type != OBJ_COPY) continue;
		int wx, wy, forms[GLITCH_FORMS], n = run_glitch_forms(run.depth, forms);
		netmap_world(layer.obj[i].x, layer.obj[i].y, &wx, &wy);
		uint32_t o = object_at(wx, wy);
		if (o) fprintf(f, "copyform %s\ncopystands %d\n", n ? guardian(forms[0])->name : "none", emu_read8(o + BN6_NPC_ANIM));
		return;
	}
}

/* The layer: whose area it is drawn as, whose Navi guards it, and the
 * animation he stands in on the map with whether his sprite draws anything
 * there, and for one of BN4's the view it draws (BN4's Navis draw four
 * diagonals, not BN6's eight facings: one facing left showed his back or
 * nothing, 15 November 2026) */
static void layer_lines(FILE *f) {
	fprintf(f, "areafrom %s\n", game_of_area(D.layer_tiles));
	copy_lines(f);
	const GuardianStage *g = &D.objs.guardian;
	if (!g->navi) return;
	fprintf(f, "guardianfrom %s\n", game_of_guardian(g->navi));
	uint32_t o = object_at(g->x, g->y);
	if (!o) return;
	static const char *const views[] = { "back-right", "front-right", "front-left", "back-left" };
	NpcBody b = guardian_body(g->navi, g->face);
	int anim = emu_read8(o + BN6_NPC_ANIM), view = guardian_bn4(g->navi) ? xguardian_bn4_view(g->navi, anim) : 0;
	bool shown = npc_sprite_shows(b.list, b.index, anim) && view >= 0;
	fprintf(f, "guardianstands %d %s\n", anim, !shown ? "unseen" : guardian_bn4(g->navi) ? views[view] : "shown");
}

/* A battle's enemies as BN6's engine holds them: their HP together, their
 * names, and how many are an older net's, moved in (their ids past BN6's
 * own, chip_targets.h); how many stand and are down is the tally's `foes`
 * and `down` (director_tally.c) */
static void foes_lines(FILE *f) {
	int n = 0, hp = 0, older = 0;
	char names[160] = "";
	for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		if (!(emu_read8(o) & 1) || emu_read8(o + BN6_T1_ALLIANCE) != 1 || !emu_read8(o + BN6_T1_PANEL_X)) continue;
		int id = emu_read16(o + BN6_T1_NAME_ID);
		char name[24];
		enemy_name(id, name, sizeof name);
		size_t m = strlen(names);
		snprintf(names + m, sizeof names - m, "%s%s", m ? "+" : "", name);
		++n;
		hp += emu_read16(o + BN6_T1_HP);
		older += chip_targets_name((uint16_t)id) != id;
	}
	fprintf(f, "enemyhp %d\nenemynames %s\nolderfoes %d\n", hp, n ? names : "none", older);
	/* (a random battle from an older net's own record, its address there:
	 * the territory's own, xbattles.h) */
	if (!boss_fighting() && D.next.xrecord) fprintf(f, "olderrecord %07X\n", (unsigned)(D.next.xrecord & 0xFFFFFFFu));
	else fprintf(f, "olderrecord none\n");
}

/* The game a chip's record is of: one of the registry's own ids (BN5's, BN4's: xchips_registry.h), else BN6's */
static const char *game_of_chip(int id) {
	const XChipReg *r = xreg_at(id);
	if (r && xreg_own(r)) return "bn5";
	return xreg4_at(id) ? "bn4" : "bn6";
}

/* One chip of a hand as a word: its id and its code's letter ("204L", "1*") */
static void chip_word(FILE *f, uint16_t entry) {
	unsigned code = chip_entry_code(entry);
	fprintf(f, " %u%c", chip_entry_id(entry), code < 26 ? (char)('A' + code) : '*');
}

/* The chips: those the Custom screen offers while it is open, how many are
 * picked, and after OK the hand's left to use, the next one's power */
static void chips_lines(FILE *f) {
	if (emu_read8(BN6_BATTLE_PHASE) == BN6_PHASE_CUSTOM) {
		int n = emu_read8(BN6_CUSTOM_HAND);
		fprintf(f, "drawn");
		for (int i = 0; i < n && i < 10; ++i) chip_word(f, emu_read16(BN6_BATTLE_DECK + (uint32_t)i * 2));
		fprintf(f, "\nhandpicks %d\n", emu_read8(BN6_CUSTOM_PICKED));
		return;
	}
	int at = emu_read8(BN6_BATTLE_HAND + BN6_HAND_AT), left = 0;
	for (int i = at; i < BN6_HAND_MAX && emu_read16(BN6_BATTLE_HAND + BN6_HAND_CHIPS + (uint32_t)i * 2) != 0xFFFF; ++i) ++left;
	fprintf(f, "hand %d\n", left);
	if (!left) return;
	int chip = emu_read16(BN6_BATTLE_HAND + BN6_HAND_CHIPS + (uint32_t)at * 2);
	fprintf(f, "nextchip %d\nnextchipfrom %s\nnextpower %d\n", chip, game_of_chip(chip),
		emu_read16(BN6_BATTLE_HAND + BN6_HAND_POWERS + (uint32_t)at * 2) + emu_read16(BN6_BATTLE_HAND + BN6_HAND_BONUS + (uint32_t)at * 2));
}

/* BN5's Souls in BN6's battles (soulunite.h): UNITE
 * under OK on this Custom screen (none: BN6's emblem or nothing; grey;
 * lit, with the Soul the last chip picked lights; chosen), the Soul
 * MegaMan is united with (BN5's number, 0 none), its turns left and the
 * Souls used this battle */
static void souls_lines(FILE *f) {
	static const char *const states[] = { "lit", "grey", "chosen" };
	if (!soulunite_ready()) return;
	uint32_t s = BN6_SOULS;
	int state = emu_read8(BN6_CUSTOM_UNITE + BN6_ELEMENT_STATE);
	bool unite = emu_read8(BN6_BATTLE_PHASE) == BN6_PHASE_CUSTOM && emu_read8(s + BN6_SOUL_SLOT) == BN6_SLOT_UNITE && state < 3;
	fprintf(f, "unite %s\nunitesoul %d\n", unite ? states[state] : "none", unite ? emu_read8(BN6_CUSTOM_UNITE + BN6_ELEMENT_SOUL) : 0);
	fprintf(f, "united %d\nunitedturns %d\nsoulsused %x\n", emu_read8(s + BN6_SOUL_ID), emu_read8(s + BN6_SOUL_TURNS),
		(unsigned)emu_read32(s + BN6_SOUL_USED));
}

void older_describe(FILE *f) {
	run_lines(f);
	if (D.town) return;
	layer_lines(f);
	if (emu_read8(BN6_GAMESTATE) != BN6_SUB_BATTLE) return;
	foes_lines(f);
	chips_lines(f);
	souls_lines(f);
}
