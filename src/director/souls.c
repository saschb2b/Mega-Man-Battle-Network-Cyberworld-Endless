/* The run's Souls (souls.h). */
#include "souls.h"

#include <string.h>
#include <stddef.h>

#include "bn6.h"
#include "bn6_forms.h"
#include "emu.h"
#include "forms.h"
#include "guardians.h"
#include "loot_souls.h"
#include "run.h"
#include "save_blob.h"

#define SOULS_MAGIC 0x534F5531u   /* "SOU1" */

/* (layer: the Souls held as the layer was made, in the byte after held,
 * which saves before it wrote as 0: none; team and team_layer the same of
 * Team ProtoMan's, BN5's Souls 1-6, in the two bytes past them, which
 * saves before wrote as 0 too: the struct cleared whole, never a field;
 * bn4 and bn4_layer BN4's twelve (issue #774), past the eight bytes older
 * saves wrote, read as none) */
typedef struct { uint32_t seed; uint8_t held, layer, team, team_layer; uint16_t bn4, bn4_layer; } SoulSave;
static SoulSave souls;
_Static_assert(sizeof(SoulSave) == 12 && offsetof(SoulSave, seed) == 0 && offsetof(SoulSave, held) == 4 &&
	offsetof(SoulSave, layer) == 5 && offsetof(SoulSave, team) == 6 && offsetof(SoulSave, team_layer) == 7 &&
	offsetof(SoulSave, bn4) == 8 && offsetof(SoulSave, bn4_layer) == 10, "Soul save layout changed");
uint8_t souls_dev_mask;

void souls_give_bn5(int soul) {
	if (soul >= 1 && soul <= SOULS_TEAM) souls.team |= (uint8_t)(1u << (soul - 1));
	if (soul > SOULS_TEAM && soul <= SOULS_OF_BN5) souls.held |= (uint8_t)(1u << (soul - SOULS_TEAM - 1));
}

void souls_give_bn4(int soul) {
	if (soul >= 1 && soul <= SOULS_OF_BN4) souls.bn4 |= (uint16_t)(1u << (soul - 1));
}

void souls_new_run(uint32_t seed) {
	memset(&souls, 0, sizeof souls);
	souls.seed = seed;
	souls.held = (uint8_t)(souls_dev_mask & ((1u << SOULS) - 1));
	/* (the Soul a Soul form brought from the start, run.soul_start) */
	if (run.seed == seed && run.soul_start && run.form == RUN_FORM_SOUL_BN5) souls_give_bn5(run.soul_start);
	if (run.seed == seed && run.soul_start && run.form == RUN_FORM_SOUL_BN4) souls_give_bn4(run.soul_start);
	souls_note_layer();
}

void souls_begin(uint32_t seed) {
	if (souls.seed != seed) souls_new_run(seed);
}

void souls_save(void) { save_write_blob("run.souls", SOULS_MAGIC, &souls, sizeof souls); }

void souls_load(uint32_t seed) {
	if (!save_read_blob_upto("run.souls", SOULS_MAGIC, &souls, sizeof souls) || souls.seed != seed) souls_new_run(seed);
}

unsigned souls_held(void) { return souls.held; }

void souls_note_layer(void) {
	souls.layer = souls.held;
	souls.team_layer = souls.team;
	souls.bn4_layer = souls.bn4;
}

unsigned souls_layer(void) { return souls.layer; }

void souls_lean(void) {
	loot_soul_souls(souls.layer);
	unsigned kinds = 0;
	for (int p = 1; p <= SOULS_TEAM; ++p)
		if (souls.team_layer >> (p - 1) & 1) kinds |= 1u << (forms_power_kind(RUN_FORM_SOUL_BN5, p) - 1);
	for (int p = 1; p <= SOULS_OF_BN4; ++p)
		if (souls.bn4_layer >> (p - 1) & 1) kinds |= 1u << (forms_power_kind(RUN_FORM_SOUL_BN4, p) - 1);
	loot_soul_kinds(kinds);
}

bool soul_held(int navi) {
	if (guardian_team(navi)) return souls.team >> (navi - GUARDIAN_TEAM_FIRST) & 1;
	return guardian_older(navi) && souls.held >> (navi - GUARDIAN_OLDER_FIRST) & 1;
}

void soul_give(int navi) {
	if (guardian_older(navi)) souls.held |= (uint8_t)(1u << (navi - GUARDIAN_OLDER_FIRST));
	if (guardian_team(navi)) souls.team |= (uint8_t)(1u << (navi - GUARDIAN_TEAM_FIRST));
}

void souls_battle_begins(void) {
	if (run_form_crosses() || !run.soul_turns) return;
	uint32_t at = BN6_NAVI_STATS + BN6_NAVI_TURN_EXTRA;   /* (MegaMan's own: the battle's copy is made from it after StartBattle) */
	if ((int8_t)emu_read8(at) < 1) emu_write8(at, 1);
}

unsigned souls_held_bn5(void) { return (unsigned)souls.team | (unsigned)souls.held << SOULS_TEAM; }
unsigned souls_held_bn4(void) { return souls.bn4; }
unsigned souls_layer_bn5(void) { return (unsigned)souls.team_layer | (unsigned)souls.layer << SOULS_TEAM; }
unsigned souls_layer_bn4(void) { return souls.bn4_layer; }
