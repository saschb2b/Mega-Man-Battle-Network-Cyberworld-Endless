/* A guardian's part of a layer (docs/BOSSES.md): its lines and music in the
 * layer's text archive, then its actors once the archive has an address. */
#include "guardian_objs.h"

#include <stdio.h>
#include <string.h>

#include "data.h"
#include "debug.h"
#include "flags.h"
#include "guardians.h"
#include "portrait.h"
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
#include "xguardian.h"

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

/* Guardian `navi`, a face made for him first where Gregar has none of
 * him (portrait.c), or BN5's own copied in with his sprite (xguardian.c),
 * before any of his words name it */
static const Guardian *guardian_faced(int navi) {
	guardian_set_face(navi, guardian_older(navi) ? xguardian_slot(navi) : portrait_face(navi));
	return guardian(navi);
}

/* The chip his Guardian Data gives: his Navi chip at the version fought,
 * or, for BN5's, whose Navi chips BN6 lacks, a chip of his Soul's kind
 * (docs/BOSSES.md, BN5's Navis); its code into *code, 0 none */
static int guardian_chip(const GuardianStage *g, int *code) {
	*code = 26;
	if (guardian_older(g->navi)) return xguardian_chip(g->navi, run.layer_seed * 2654435761u >> 8, code);
	int chip = navi_chip(g->navi, g->version);
	if (chip <= 0) return chip;
	ChipInfo ci;
	chip_info(chip, &ci);
	/* (its letter where the folder holds it, else its * where BN6 has one,
	 * the V1 chips: a playtester's Blade folder of S, L and * took a
	 * BlastMan B it could not play) */
	char c = ci.ncodes ? ci.codes[0] : '*';
	bool held = false;
	for (int k = 0; k < 3 && run.codes[k]; ++k) held |= c == 'A' + run.codes[k] - 1;
	if (!held && memchr(ci.codes, '*', (size_t)ci.ncodes)) c = '*';
	*code = c == '*' ? 26 : c - 'A';
	return chip;
}

/* What his Guardian Data says of the power it gives: a Cross's words
 * (powers.c), or BN5's guardian's Soul, for the run's BN5 battles
 * (docs/META.md, Souls in BN5 territory) */
static const char *reward_power(int navi) {
	return guardian_older(navi) ? xguardian_soul_words(navi) : powers_reward_text(navi, layer.biome, run.depth);
}

/* The NaviCust's draft after a normal layer's guardian (docs/NAVICUST.md):
 * an ExpMemry at the second and fourth acts' guardians, and three programs,
 * or none for BugFrags. */
static void draft_make(ScriptsDraft *draft, GuardianStage *g) {
	draft->expmemry = !navicust_expmemry(run.depth) ? 0 : run.depth <= 6 ? 1 : 2;
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
		for (int k = 0; k < NAVICUST_DRAFT && k < n; ++k) fprintf(stderr, " %d (colour %d)", pick[k].program, pick[k].color);
		fprintf(stderr, "\n");
	}
	bool colored = n > 0;
	for (int k = 0; k < NAVICUST_DRAFT && k < n && colored; ++k) {
		int c = pick[k].color ? pick[k].color : navicust_color(pick[k].program);
		draft->program[k] = (uint8_t)(pick[k].program * 4);
		draft->color[k] = (uint8_t)c;
		draft->about[k] = navicust_about(pick[k].program);
		colored = c > 0;
	}
	if (colored) {
		draft->n = n;
		draft->skip_frags = navicust_skip_frags(run.depth);
		/* (the player's first draft ever: a run from an older build met
		 * its first past act 1's, without it) */
		draft->teach = !profile.navicust_taught;
		/* (and whether each fits the board as it stands, said before the
		 * pick: two playtesters took a program MegaMan then said would not
		 * fit, session 63) */
		draft->fit_flag = LAYER_DRAFT_FIT_FLAG;
		for (int k = 0; k < NAVICUST_DRAFT && k < n; ++k) g->draft[k] = draft->program[k];
	}
}

void guardian_scripts(TextArchive *text, const NetObj *o, int wx, int wy, int wz, GuardianStage *g) {
	const Guardian *gd = guardian_faced(o->param);
	g->navi = o->param;
	/* (BN5's at the version his act's band takes: xguardian.c) */
	g->version = guardian_older(g->navi) ? xguardian_version(g->navi, run.depth) : make_boss(run.depth, run.biome, o->param).foes[0].version;
	g->x = wx; g->y = wy; g->z = wz;
	/* the guardian looks back down the bridge MegaMan comes by */
	g->face = facing(layer.arena >= 0 ? (layer.arena_dir + 2) & 3 : 1);
	g->intro = ta_talk(text, guardian_intro(g->navi, g->version, layer.biome), guardian_face(g->navi));
	g->defeat = ta_talk(text, guardian_defeat(g->navi), guardian_face(g->navi));
	int code, chip = guardian_chip(g, &code);
	ChipInfo ci = { 0 };
	if (chip > 0) chip_info(chip, &ci);
	/* the NaviCust's part (only a normal layer's guardian; a side layer's
	 * is its own reward) */
	ScriptsDraft draft = { 0 };
	memset(g->draft, 0, sizeof g->draft);
	/* (the run's last guardian: none, nothing more to run with) */
	bool last = run.side_kind == LAYER_NORMAL && run.mode == RUN_SHORT && run_short_last(run.depth);
	if (run.side_kind == LAYER_NORMAL && !last) draft_make(&draft, g);
	/* (a first battle with this Navi, in any run: its battle data comes
	 * with the Guardian Data, and the next briefing reads it) */
	const char *power = reward_power(g->navi);
	if (!guardian_known(g->navi)) power = guardian_data_words(power);
	ScriptsReward reward = { .name = gd->name, .power = power, .chip = chip, .code = code, .chip_name = ci.name, .last = last,
		.taken_flag = LAYER_REWARD_TAKEN_FLAG, .hp_memories = run.threat >= 9 ? SCRIPTS_BOSS_HP_MEMORIES - 1 : SCRIPTS_BOSS_HP_MEMORIES,
		.draft = &draft };
	g->reward = ta_guardian_reward(text, &reward);
	g->prelude = ta_music(text, SONG_BOSS_PRELUDE);
	g->hush = ta_music(text, SONG_STOP);
	g->theme = ta_music(text, SCRIPTS_AREA_MUSIC);
	for (int f = LAYER_BOSS_GONE_FLAG; f <= LAYER_EXIT_OPEN_FLAG; ++f) flag_clear(f);
}

void guardian_actors(NpcList *npcs, uint32_t archive, const GuardianStage *g) {
	NpcBody body = guardian_body(g->navi, g->face);
	if (npcs->n < 32) npcs->script[npcs->n++] = npc_guardian(&body, g->x, g->y, g->z, &flags);
	if (npcs->n < 32)
		npcs->script[npcs->n++] = npc_guardian_data(g->x, g->y, g->z, MD_ANIM_GUARDIAN, archive, g->reward, &flags);
}

