/* A guardian's part of a layer (docs/BOSSES.md): its lines and music in the
 * layer's text archive, then its actors once the archive has an address. */
#include "guardian_objs.h"

#include <stdio.h>
#include <string.h>

#include "data.h"
#include "debug.h"
#include "flags.h"
#include "guardians.h"
#include "rivals.h"
#include "loot.h"
#include "meta.h"
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
		/* (its letter where the folder holds it, else its * where BN6 has
		 * one, the V1 chips: a playtester's Blade folder of S, L and * took
		 * a BlastMan B it could not play) */
		char c = ci.ncodes ? ci.codes[0] : '*';
		bool held = false;
		for (int k = 0; k < 3 && run.codes[k]; ++k) held |= c == 'A' + run.codes[k] - 1;
		if (!held && memchr(ci.codes, '*', (size_t)ci.ncodes)) c = '*';
		code = c == '*' ? 26 : c - 'A';
	}
	/* the NaviCust's part (docs/NAVICUST.md): an ExpMemry at the second
	 * and fourth acts' guardians, and a draft of three programs, or none
	 * for BugFrags (only a normal layer's guardian; a side layer's is its
	 * own reward) */
	ScriptsDraft draft = { 0 };
	/* (the run's last guardian: none, nothing more to run with) */
	bool last = run.side_kind == LAYER_NORMAL && run.mode == RUN_SHORT && run_short_last(run.depth);
	if (run.side_kind == LAYER_NORMAL && !last) {
		draft.expmemry = !navicust_expmemry(run.depth) ? 0 : run.depth <= 6 ? 1 : 2;
		/* (programs that fit beside those on MegaMan's board as the layer
		 * was made, on the board this Guardian Data leaves: its ExpMemry
		 * comes first) */
		NaviProgram pick[NAVICUST_DRAFT];
		int have = 0, w, h;
		while (have < (int)sizeof run.programs && run.programs[have]) ++have;
		navicust_board((run.depth >= 6) + (run.depth >= 12), &w, &h);
		int n = navicust_draft_fitting(run.depth, pick, run.programs, have, w, h);
		if (run.threat >= 8 && n > 2) n = 2;   /* (threat 8, docs/META.md) */
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
	 * act's area or another of its tier, each named with its guardian
	 * where MegaMan has battled him (else as one never battled, the way
	 * named by its area: docs/META.md, what MegaMan knows) */
	ScriptsRoute route = { 0 };
	static char question[400], then[3][96], area[3][32], who[3][48];
	const ScriptsRoute *way = NULL;
	int next = (run.depth % CYCLE_LAYERS) / 3, alt_navi = 0, alt = -1, dark_navi = 0, dark = -1;
	if (run.side_kind == LAYER_NORMAL && is_boss_depth(run.depth)) alt = run_route_alt(next, &alt_navi);
	/* (the short net's last act before the Nest: the dark way into the
	 * Undernet, open once the Secret Area has been cleared in any run, and
	 * said to be sealed till then: docs/META.md, branches) */
	if (alt >= 0) dark = run_route_dark(next, alt_navi, &dark_navi);
	bool dark_open = dark >= 0 && meta_dark_way_open();
	if (alt >= 0) {
		static const char *const elem[5] = { "", " (Fire)", " (Aqua)", " (Elec)", " (Wood)" };
		int nways = dark_open ? 3 : 2;
		int b[3] = { run.biome_order[next], alt, dark }, n[3] = { run.boss_order[run.biome_order[next]], alt_navi, dark_navi }, el[3];
		static char option[3][32];
		for (int k = 0; k < nways; ++k) {
			snprintf(area[k], sizeof area[k], "%s", guardian_area_in_text(b[k], LAYER_NORMAL));
			el[k] = enemy_element(enemy_id(1, n[k], 0));
			if (el[k] < 0 || el[k] > 4) el[k] = 0;
			if (guardian_known(n[k])) {
				snprintf(option[k], sizeof option[k], "%s%s", guardian(n[k])->name, elem[el[k]]);
				snprintf(who[k], sizeof who[k], "%s%s", guardian(n[k])->name, elem[el[k]]);
			} else {
				snprintf(option[k], sizeof option[k], "%s", guardian_area_name(b[k]));
				snprintf(who[k], sizeof who[k], "a Navi we've never battled");
			}
			route.option[k] = option[k];
			snprintf(then[k], sizeof then[k], "%c%s it is! The exit pad will take us there.", area[k][0] - ('a' <= area[k][0] ? 32 : 0), area[k] + 1);
			route.then[k] = then[k];
		}
		/* (the first names a sentence's start) */
		if ('a' <= who[0][0] && who[0][0] <= 'z') who[0][0] = (char)(who[0][0] - 32);
		/* (two boxes: five had named the ways) */
		if (dark_open)
			snprintf(question, sizeof question, "@M The net splits below us, Lan! %s guards %s,|@M %s guards %s, and a dark way "
				"leads down into the Undernet, where %s waits. Which way?", who[0], area[0], who[1], area[1], who[2]);
		else if (dark >= 0)
			snprintf(question, sizeof question, "@M The net splits below us, Lan! %s guards %s,|@M and %s guards %s.|@M A dark way "
				"leads down into the Undernet too, but it's sealed. Clearing the Secret Area would open it. Which way?",
				who[0], area[0], who[1], area[1]);
		else
			snprintf(question, sizeof question, "@M The net splits below us, Lan! %s guards %s,|@M and %s guards %s. Which way?",
				who[0], area[0], who[1], area[1]);
		route.question = question;
		route.n = nways;
		route.flag = LAYER_ROUTE_FLAG;
		route.dark_flag = LAYER_ROUTE_DARK_FLAG;
		way = &route;
	}
	/* (a first battle with this Navi, in any run: its battle data comes
	 * with the Guardian Data, and the next briefing reads it) */
	const char *power = powers_reward_text(g->navi, layer.biome, run.depth);
	static char with_data[640];
	if (!guardian_known(g->navi)) {
		snprintf(with_data, sizeof with_data, "%s%s@M And his battle data, Lan. Next time, we'll know how he fights!", power ? power : "", power ? "|" : "");
		power = with_data;
	}
	g->reward = ta_guardian_reward(text, gd->name, power, chip, ci.name, code, last,
		LAYER_REWARD_TAKEN_FLAG, run.threat >= 9 ? SCRIPTS_BOSS_HP_MEMORIES - 1 : SCRIPTS_BOSS_HP_MEMORIES, &draft, way);
	g->prelude = ta_music(text, SONG_BOSS_PRELUDE);
	g->hush = ta_music(text, SONG_STOP);
	g->theme = ta_music(text, SCRIPTS_AREA_MUSIC);
	for (int f = LAYER_BOSS_GONE_FLAG; f <= LAYER_EXIT_OPEN_FLAG; ++f) flag_clear(f);
	flag_clear(LAYER_ROUTE_FLAG);
	flag_clear(LAYER_ROUTE_DARK_FLAG);
}

void guardian_actors(NpcList *npcs, uint32_t archive, int sprite, const GuardianStage *g) {
	const Guardian *gd = guardian(g->navi);
	bool known = sprite != GUARDIAN_HEEL_SPRITE;
	if (npcs->n < 32)
		npcs->script[npcs->n++] = npc_guardian(sprite, g->x, g->y, g->z, g->face, known ? gd->pose : -1, known, &flags);
	if (npcs->n < 32)
		npcs->script[npcs->n++] = npc_guardian_data(g->x, g->y, g->z, MD_ANIM_GUARDIAN, archive, g->reward, &flags);
}

