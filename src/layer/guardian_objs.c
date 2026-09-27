/* A guardian's part of a layer (docs/BOSSES.md): its lines and music in the
 * layer's text archive, then its actors once the archive has an address. */
#include "guardian_objs.h"

#include "data.h"
#include "flags.h"
#include "guardians.h"
#include "loot.h"
#include "navicust.h"
#include "npc.h"
#include "powers.h"
#include "run.h"
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
		NaviProgram pick[NAVICUST_DRAFT];
		int n = navicust_draft(run.depth, pick);
		bool colored = n == NAVICUST_DRAFT;
		for (int k = 0; k < n && colored; ++k) {
			int c = navicust_color(pick[k].program);
			draft.program[k] = (uint8_t)(pick[k].program * 4);
			draft.color[k] = (uint8_t)c;
			draft.about[k] = navicust_about(pick[k].program);
			colored = c > 0;
		}
		if (colored) {
			draft.n = n;
			draft.skip_frags = navicust_skip_frags(run.depth);
			draft.teach = run.depth == 3;
		}
	}
	g->reward = ta_guardian_reward(text, gd->name, powers_reward_text(g->navi, layer.biome, run.depth), chip, ci.name, code,
		LAYER_REWARD_TAKEN_FLAG, &draft);
	g->prelude = ta_music(text, SONG_BOSS_PRELUDE);
	g->hush = ta_music(text, SONG_STOP);
	g->theme = ta_music(text, SCRIPTS_AREA_MUSIC);
	for (int f = LAYER_BOSS_GONE_FLAG; f <= LAYER_EXIT_OPEN_FLAG; ++f) flag_clear(f);
}

void guardian_actors(NpcList *npcs, uint32_t archive, int sprite, const GuardianStage *g) {
	const Guardian *gd = guardian(g->navi);
	bool known = sprite != GUARDIAN_HEEL_SPRITE;
	if (npcs->n < 32)
		npcs->script[npcs->n++] = npc_guardian(sprite, g->x, g->y, g->z, g->face, known ? gd->pose : -1, known, &flags);
	if (npcs->n < 32)
		npcs->script[npcs->n++] = npc_guardian_data(g->x, g->y, g->z, MD_ANIM_GUARDIAN, archive, g->reward, &flags);
}
