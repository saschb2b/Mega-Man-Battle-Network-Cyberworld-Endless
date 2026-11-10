/* The run's Souls (souls.h). */
#include "souls.h"

#include <string.h>
#include <stddef.h>

#include "guardians.h"
#include "save_blob.h"

#define SOULS_MAGIC 0x534F5531u   /* "SOU1" */

/* (layer: the Souls held as the layer was made, in the byte after held,
 * which saves before it wrote as 0: none; team and team_layer the same of
 * Team ProtoMan's, BN5's Souls 1-6, in the two bytes past them, which
 * saves before wrote as 0 too: the struct cleared whole, never a field) */
typedef struct { uint32_t seed; uint8_t held, layer, team, team_layer; } SoulSave;
static SoulSave souls;
_Static_assert(sizeof(SoulSave) == 8 && offsetof(SoulSave, seed) == 0 && offsetof(SoulSave, held) == 4 &&
	offsetof(SoulSave, layer) == 5 && offsetof(SoulSave, team) == 6 && offsetof(SoulSave, team_layer) == 7, "Soul save layout changed");
uint8_t souls_dev_mask;

void souls_new_run(uint32_t seed) {
	memset(&souls, 0, sizeof souls);
	souls.seed = seed;
	souls.held = (uint8_t)(souls_dev_mask & ((1u << SOULS) - 1));
}

void souls_begin(uint32_t seed) {
	if (souls.seed != seed) souls_new_run(seed);
}

void souls_save(void) { save_write_blob("run.souls", SOULS_MAGIC, &souls, sizeof souls); }

void souls_load(uint32_t seed) {
	if (!save_read_blob("run.souls", SOULS_MAGIC, &souls, sizeof souls) || souls.seed != seed) souls_new_run(seed);
}

unsigned souls_held(void) { return souls.held; }

void souls_note_layer(void) {
	souls.layer = souls.held;
	souls.team_layer = souls.team;
}

unsigned souls_layer(void) { return souls.layer; }

bool soul_held(int navi) {
	if (guardian_team(navi)) return souls.team >> (navi - GUARDIAN_TEAM_FIRST) & 1;
	return guardian_older(navi) && souls.held >> (navi - GUARDIAN_OLDER_FIRST) & 1;
}

void soul_give(int navi) {
	if (guardian_older(navi)) souls.held |= (uint8_t)(1u << (navi - GUARDIAN_OLDER_FIRST));
	if (guardian_team(navi)) souls.team |= (uint8_t)(1u << (navi - GUARDIAN_TEAM_FIRST));
}
