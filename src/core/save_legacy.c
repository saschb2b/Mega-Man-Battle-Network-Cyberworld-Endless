/* Saves from earlier versions. The first run format ("CWE1") kept the pure-C
 * engine's MegaMan (HP, folder, perks) beside the run; only the run's own
 * fields carry over, since the game's state holds MegaMan. The second
 * ("CWE2") had guardians for eight areas, the third ("CWE3") for sixteen;
 * the areas added since get theirs drawn from the run's seed. The fifth,
 * sixth and seventh ("CWE5" to "CWE7") are the current run without the
 * fields added at its end since. */
#include <stdio.h>

#include "compat.h"
#include <string.h>

#include "game.h"
#include "run.h"
#include "save_blob.h"

#define RUN_MAGIC_V1 0x43574531u /* "CWE1" */
#define RUN_MAGIC_V2 0x43574532u /* "CWE2" */
#define RUN_MAGIC_V3 0x43574533u /* "CWE3" */
#define RUN_MAGIC_V5 0x43574535u /* "CWE5" */
#define RUN_MAGIC_V6 0x43574536u /* "CWE6" */
#define RUN_MAGIC_V7 0x43574537u /* "CWE7" */

typedef struct {
	bool active;
	uint32_t seed;
	int depth;
	int biome;
	int side_kind;
	uint32_t layer_seed;
	uint8_t biome_order[6];
	uint8_t boss_order[16];
	int bosses_beaten;
	int viruses_deleted;
	int fragments;
	bool secret_cleared;
} RunV3;

typedef struct {
	bool active;
	uint32_t seed;
	int depth;
	int biome;
	int side_kind;
	uint32_t layer_seed;
	uint8_t biome_order[6];
	uint8_t boss_order[8];
	int bosses_beaten;
	int viruses_deleted;
	int fragments;
	bool secret_cleared;
} RunV2;

typedef struct {
	bool active;
	uint32_t seed;
	int hp, max_hp;
	int zenny, bugfrags;
	int atk, rapid, charge;
	int custom_size;
	struct { uint16_t id; char code; } folder[30];
	int folder_n;
	uint32_t perks;
	uint32_t crosses;
	int depth;
	int biome;
	int bosses_beaten;
	int viruses_deleted;
	uint32_t frames;
	int score;
	bool beast_out;
	uint8_t biome_order[6];
	uint8_t boss_order[8];
	int fragments;
	int unlockers;
	int side_kind;
	uint32_t layer_seed;
	bool secret_cleared;
	uint64_t layer_used;
	bool layer_boss_beaten;
} RunV1;

/* (no Cross brought, and the folder's codes and the board's programs read
 * again as the next layer is made) */
static bool load_v5(void) {
	Run v;
	if (!save_read_blob_upto("run.sav", RUN_MAGIC_V5, &v, sizeof v) || !v.active) return false;
	v.cross = 0;
	memset(v.codes, 0, sizeof v.codes);
	memset(v.programs, 0, sizeof v.programs);
	v.clock = v.back_spent = 0;
	v.home_depth = 0;
	run = v;
	return true;
}

static bool load_v6(void) {
	Run v;
	if (!save_read_blob_upto("run.sav", RUN_MAGIC_V6, &v, sizeof v) || !v.active) return false;
	memset(v.programs, 0, sizeof v.programs);
	v.clock = v.back_spent = 0;
	v.home_depth = 0;
	run = v;
	return true;
}

/* (the Net's clock at none, and no trip back) */
static bool load_v7(void) {
	Run v;
	if (!save_read_blob_upto("run.sav", RUN_MAGIC_V7, &v, sizeof v) || !v.active) return false;
	v.clock = v.back_spent = 0;
	v.home_depth = 0;
	run = v;
	return true;
}

static bool load_v3(void) {
	RunV3 v;
	if (!save_read_blob("run.sav", RUN_MAGIC_V3, &v, sizeof v) || !v.active) return false;
	uint32_t keep = rng_state();
	run_new(v.seed);   /* the new areas' guardians */
	rng_restore(keep);
	run.depth = v.depth;
	run.biome = v.biome;
	run.side_kind = v.side_kind;
	run.layer_seed = v.layer_seed;
	memcpy(run.biome_order, v.biome_order, sizeof run.biome_order);
	memcpy(run.boss_order, v.boss_order, sizeof v.boss_order);
	run.bosses_beaten = v.bosses_beaten;
	run.viruses_deleted = v.viruses_deleted;
	run.fragments = v.fragments;
	run.secret_cleared = v.secret_cleared;
	return true;
}

static bool load_v2(void) {
	RunV2 v;
	if (!save_read_blob("run.sav", RUN_MAGIC_V2, &v, sizeof v) || !v.active) return false;
	uint32_t keep = rng_state();
	run_new(v.seed);   /* the new areas' guardians */
	rng_restore(keep);
	run.depth = v.depth;
	run.biome = v.biome;
	run.side_kind = v.side_kind;
	run.layer_seed = v.layer_seed;
	memcpy(run.biome_order, v.biome_order, sizeof run.biome_order);
	memcpy(run.boss_order, v.boss_order, sizeof v.boss_order);
	run.bosses_beaten = v.bosses_beaten;
	run.viruses_deleted = v.viruses_deleted;
	run.fragments = v.fragments;
	run.secret_cleared = v.secret_cleared;
	return true;
}

bool legacy_load_run(void) {
	if (load_v7() || load_v6() || load_v5() || load_v3() || load_v2()) return true;
	RunV1 v;
	if (!save_read_blob("run.sav", RUN_MAGIC_V1, &v, sizeof v) || !v.active) return false;
	memset(&run, 0, sizeof run);
	run.active = true;
	run.seed = v.seed;
	run.depth = v.depth;
	run.biome = v.biome;
	run.side_kind = v.side_kind;
	run.layer_seed = v.layer_seed;
	memcpy(run.biome_order, v.biome_order, sizeof run.biome_order);
	memcpy(run.boss_order, v.boss_order, sizeof run.boss_order);
	run.bosses_beaten = v.bosses_beaten;
	run.viruses_deleted = v.viruses_deleted;
	run.fragments = v.fragments;
	run.secret_cleared = v.secret_cleared;
	return true;
}

/* The game's state for the run's checkpoint lived beside the game. */
void legacy_move_state(void) {
	char old[600], cur[600];
	snprintf(old, sizeof old, "%s/run.state", g_data_dir);
	save_path(cur, sizeof cur, "run.state");
	FILE *f = fopen(cur, "rb");
	if (f) { fclose(f); return; }
	if (!(f = fopen(old, "rb"))) return;
	fclose(f);
	char dir[600];
	snprintf(dir, sizeof dir, "%s/savedata", g_data_dir);
	cw_mkdir(dir);
	cw_rename(old, cur);
}
