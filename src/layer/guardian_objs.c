/* A guardian's part of a layer (docs/BOSSES.md): its lines and music in the
 * layer's text archive, then its actors once the archive has an address. */
#include "guardian_objs.h"

#include "data.h"
#include "flags.h"
#include "guardians.h"
#include "loot.h"
#include "npc.h"
#include "powers.h"
#include "run.h"
#include "scripts.h"
#include "stage_npc.h"

#define SONG_BOSS_PRELUDE 0x1C
#define SONG_STOP         0xFF
#define MD_ANIM_GUARDIAN  1     /* the Mystery Data sprite's blue crystal */
#define SPR_HEEL_NAVI     0x43  /* stands in for Navis Gregar has no sprite of */

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
	GuardianLine in = guardian_intro(g->navi, g->version), out = guardian_defeat(g->navi);
	int mugs[8];
	for (int i = 0; in.who[i] && i < 8; ++i) mugs[i] = in.who[i] == 'M' ? GUARDIAN_MEGAMAN_MUGSHOT : gd->mugshot;
	g->intro = ta_talk(text, in.boxes, mugs);
	mugs[0] = gd->mugshot;
	g->defeat = ta_talk(text, out.boxes, mugs);
	int chip = navi_chip(g->navi, g->version), code = 26;
	ChipInfo ci = { 0 };
	if (chip > 0) {
		chip_info(chip, &ci);
		if (ci.ncodes) code = ci.codes[0] == '*' ? 26 : ci.codes[0] - 'A';
	}
	g->reward = ta_guardian_reward(text, gd->name, powers_reward_text(g->navi, layer.biome), chip, ci.name, code,
		LAYER_REWARD_TAKEN_FLAG);
	g->prelude = ta_music(text, SONG_BOSS_PRELUDE);
	g->hush = ta_music(text, SONG_STOP);
	g->theme = ta_music(text, SCRIPTS_AREA_MUSIC);
	for (int f = LAYER_BOSS_GONE_FLAG; f <= LAYER_EXIT_OPEN_FLAG; ++f) flag_clear(f);
}

void guardian_actors(NpcList *npcs, uint32_t archive, int sprite, const GuardianStage *g) {
	const Guardian *gd = guardian(g->navi);
	bool known = sprite != SPR_HEEL_NAVI;
	if (npcs->n < 32)
		npcs->script[npcs->n++] = npc_guardian(sprite, g->x, g->y, g->z, g->face, known ? gd->pose : -1, known, &flags);
	if (npcs->n < 32)
		npcs->script[npcs->n++] = npc_guardian_data(g->x, g->y, g->z, MD_ANIM_GUARDIAN, archive, g->reward, &flags);
}
