/* A guardian's part of a layer (docs/BOSSES.md): its lines and music in the
 * layer's text archive, then its actors once the archive has an address. */
#include "guardian_objs.h"

#include <stdio.h>

#include "data.h"
#include "debug.h"
#include "flags.h"
#include "guardians.h"
#include "loot.h"
#include "navicust.h"
#include "net.h"
#include "npc.h"
#include "powers.h"
#include "run.h"
#include "save.h"
#include "scripts.h"
#include "stage_npc.h"

#define SONG_BOSS_PRELUDE 0x1C
#define SONG_STOP         0xFF
#define MD_ANIM_GUARDIAN  1     /* the Mystery Data sprite's blue crystal */

static const StageFlags flags = {
	LAYER_BOSS_APPEAR_FLAG, LAYER_BOSS_GONE_FLAG, LAYER_REWARD_FLAG, LAYER_REWARD_TAKEN_FLAG,
};

/* The overworld animation facing grid direction d (DIR_*, net_shapes.h). */
static int facing(int d) {
	static const int anim[4] = { 3, 5, 7, 1 };   /* down-right, down-left, up-left, up-right */
	return anim[d & 3];
}

void guardian_scripts(TextArchive *text, const NetObj *o, int wx, int wy, int wz, GuardianStage *g) {
	const Guardian *gd = guardian(o->param);
	Encounter e = make_boss(run.depth, run.biome, o->param);
	g->navi = o->param;
	g->version = e.foes[0].version;
	g->x = wx; g->y = wy; g->z = wz;
	/* the guardian looks back down the bridge MegaMan comes by */
	g->face = facing(layer.arena >= 0 ? (layer.arena_dir + 2) & 3 : 1);
	g->intro = ta_talk(text, guardian_intro(g->navi, g->version, layer.biome), guardian_face(g->navi));
	g->defeat = ta_talk(text, guardian_defeat(g->navi), guardian_face(g->navi));
	int chip = navi_chip(g->navi, g->version), code = 26;
	ChipInfo ci = { 0 };
	if (chip > 0) {
		chip_info(chip, &ci);
		if (ci.ncodes) code = ci.codes[0] == '*' ? 26 : ci.codes[0] - 'A';
	}
	/* the NaviCust's part (docs/NAVICUST.md): an ExpMemry at the second
	 * and fourth acts' guardians, and a draft of three programs, or none
	 * for BugFrags (only a normal layer's guardian; a side layer's is its
	 * own reward) */
	ScriptsDraft draft = { 0 };
	if (run.side_kind == LAYER_NORMAL) {
		draft.expmemry = !navicust_expmemry(run.depth) ? 0 : run.depth <= 6 ? 1 : 2;
		/* (programs that fit beside those on MegaMan's board as the layer
		 * was made, on the board this Guardian Data leaves: its ExpMemry
		 * comes first) */
		NaviProgram pick[NAVICUST_DRAFT];
		int have = 0, w, h;
		while (have < (int)sizeof run.programs && run.programs[have]) ++have;
		navicust_board((run.depth >= 6) + (run.depth >= 12), &w, &h);
		int n = navicust_draft_fitting(run.depth, pick, run.programs, have, w, h);
		if (emu_debug_on()) {
			fprintf(stderr, "draft: a %dx%d board beside", w, h);
			for (int i = 0; i < have; ++i) fprintf(stderr, " %d/%d", run.programs[i] / 4, run.programs[i] % 4);
			fprintf(stderr, ":");
			for (int k = 0; k < n; ++k) fprintf(stderr, " %d (colour %d)", pick[k].program, pick[k].color);
			fprintf(stderr, "\n");
		}
		bool colored = n > 0;
		for (int k = 0; k < n && colored; ++k) {
			int c = pick[k].color ? pick[k].color : navicust_color(pick[k].program);
			draft.program[k] = (uint8_t)(pick[k].program * 4);
			draft.color[k] = (uint8_t)c;
			draft.about[k] = navicust_about(pick[k].program);
			colored = c > 0;
		}
		if (colored) {
			draft.n = n;
			draft.skip_frags = navicust_skip_frags(run.depth);
			/* (the player's first draft ever: a run from an older build met
			 * its first past act 1's, without it) */
			draft.teach = !profile.navicust_taught;
		}
	}
	/* the way on (docs/META.md, routes): after an act's guardian, the next
	 * act's area or another of its tier, each named with its guardian */
	ScriptsRoute route = { 0 };
	static char question[240], then[2][96], area[2][32];
	const ScriptsRoute *way = NULL;
	int next = (run.depth % CYCLE_LAYERS) / 3, alt_navi = 0, alt = -1;
	if (run.side_kind == LAYER_NORMAL && is_boss_depth(run.depth)) alt = run_route_alt(next, &alt_navi);
	if (alt >= 0) {
		static const char *const elem[5] = { "", " (Fire)", " (Aqua)", " (Elec)", " (Wood)" };
		int b[2] = { run.biome_order[next], alt }, n[2] = { run.boss_order[run.biome_order[next]], alt_navi }, e[2];
		static char option[2][32];
		for (int k = 0; k < 2; ++k) {
			snprintf(area[k], sizeof area[k], "%s", guardian_area_in_text(b[k], LAYER_NORMAL));
			e[k] = enemy_element(enemy_id(1, n[k], 0));
			if (e[k] < 0 || e[k] > 4) e[k] = 0;
			snprintf(option[k], sizeof option[k], "%s%s", guardian(n[k])->name, elem[e[k]]);
			route.option[k] = option[k];
			snprintf(then[k], sizeof then[k], "%c%s it is! The exit pad will take us there.", area[k][0] - ('a' <= area[k][0] ? 32 : 0), area[k] + 1);
			route.then[k] = then[k];
		}
		/* (two boxes: five had named the ways) */
		snprintf(question, sizeof question, "@M The net splits below us, Lan! %s guards %s,|@M and %s guards %s. Which way?",
			option[0], area[0], option[1], area[1]);
		route.question = question;
		route.flag = LAYER_ROUTE_FLAG;
		way = &route;
	}
	g->reward = ta_guardian_reward(text, gd->name, powers_reward_text(g->navi, layer.biome, run.depth), chip, ci.name, code,
		LAYER_REWARD_TAKEN_FLAG, &draft, way);
	g->prelude = ta_music(text, SONG_BOSS_PRELUDE);
	g->hush = ta_music(text, SONG_STOP);
	g->theme = ta_music(text, SCRIPTS_AREA_MUSIC);
	for (int f = LAYER_BOSS_GONE_FLAG; f <= LAYER_EXIT_OPEN_FLAG; ++f) flag_clear(f);
	flag_clear(LAYER_ROUTE_FLAG);
}

void guardian_actors(NpcList *npcs, uint32_t archive, int sprite, const GuardianStage *g) {
	const Guardian *gd = guardian(g->navi);
	bool known = sprite != GUARDIAN_HEEL_SPRITE;
	if (npcs->n < 32)
		npcs->script[npcs->n++] = npc_guardian(sprite, g->x, g->y, g->z, g->face, known ? gd->pose : -1, known, &flags);
	if (npcs->n < 32)
		npcs->script[npcs->n++] = npc_guardian_data(g->x, g->y, g->z, MD_ANIM_GUARDIAN, archive, g->reward, &flags);
}
