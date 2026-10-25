/* A liberation's board across a CONTINUE (liberation.h; docs/SAVES.md):
 * what its course has changed since the layer's seed made it, saved beside
 * the run's checkpoint as `run.lib` and taken back once the checkpoint's
 * state has loaded on the layer the seed made again. The board's panels,
 * its units and their HP, the phase and the Order Points are the note's;
 * what was being shown (a banner's frames, words due, a unit's glide)
 * begins again. */
#include "liberation.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "liberation_state.h"
#include "netmap.h"
#include "run.h"
#include "save_blob.h"

#define LIB_NOTE_MAGIC 0x434C4233u   /* "CLB3": the units' HP on the board */

typedef struct {
	uint32_t seed;                 /* the layer's (run.layer_seed) */
	int32_t depth, step, after, phase, op, prize;
	int32_t fight, fight_unit, fight_cmd, fx, fy;
	int32_t enemy_unit, enemy_steps;
	uint8_t on, briefed, holes_told, prize_given, shield, enemy_banner, fight_seen, pad0;
	uint8_t nunits, pad[3];
	int8_t back_in[LIB_UNITS];
	struct { uint8_t x, y, gone, pad; } unit[LIB_UNITS];
	uint8_t kind[MAP_H][MAP_W];
	uint8_t item[MAP_H][MAP_W];    /* the item panels' and barriers' (keys taken, finds left) */
	uint8_t route_found, pad2[3];
	int16_t hp[LIB_UNITS];         /* each unit's HP on the board (the Darkloid's through his battles) */
} LibNote;
_Static_assert(sizeof(LibNote) == 68 + LIB_UNITS * 5 + 2 * MAP_H * MAP_W + 4 + LIB_UNITS * 2 && offsetof(LibNote, kind) == 68 + LIB_UNITS * 5,
	"Liberation save layout changed: bump LIB_NOTE_MAGIC");

void lib_save(void) {
	LibNote n;
	memset(&n, 0, sizeof n);
	n.seed = run.layer_seed;
	n.depth = run.depth;
	n.on = LB.on;
	if (LB.on) {
		n.step = LB.step; n.after = LB.after; n.phase = LB.phase; n.op = LB.op; n.prize = LB.prize;
		n.fight = LB.fight; n.fight_unit = LB.fight_unit; n.fight_cmd = LB.fight_cmd; n.fx = LB.fx; n.fy = LB.fy;
		n.enemy_unit = LB.enemy_unit; n.enemy_steps = LB.enemy_steps;
		n.briefed = LB.briefed; n.holes_told = LB.holes_told; n.prize_given = LB.prize_given; n.shield = LB.shield;
		n.enemy_banner = LB.enemy_banner; n.fight_seen = LB.fight_seen;
		n.nunits = (uint8_t)LB.b.nunits;
		for (int u = 0; u < LB.b.nunits && u < LIB_UNITS; ++u) {
			n.back_in[u] = (int8_t)LB.back_in[u];
			n.unit[u].x = LB.b.unit[u].x; n.unit[u].y = LB.b.unit[u].y; n.unit[u].gone = LB.b.unit[u].gone;
			n.hp[u] = (int16_t)LB.hp[u];
		}
		memcpy(n.kind, LB.b.kind, sizeof n.kind);
		memcpy(n.item, LB.b.item, sizeof n.item);
		n.route_found = LB.route_found;
	}
	save_write_blob("run.lib", LIB_NOTE_MAGIC, &n, sizeof n);
}

/* The step a saved one goes on as: a talk's words were not kept (no chat
 * box is open as a checkpoint is saved), so its step after; the menu
 * closed; a banner shown from its start */
static int resumed_step(int step, int after) {
	if (step == LIB_STEP_TALK) step = after;
	if (step == LIB_STEP_MENU || step == LIB_STEP_BONUS) step = LIB_STEP_MOVE;
	return step;
}

bool lib_resume(void) {
	LibNote n;
	if (!LB.on || !save_read_blob("run.lib", LIB_NOTE_MAGIC, &n, sizeof n) || !n.on || n.seed != run.layer_seed || n.depth != run.depth ||
	    n.nunits != LB.b.nunits)
		return false;
	memcpy(LB.b.kind, n.kind, sizeof LB.b.kind);
	memcpy(LB.b.item, n.item, sizeof LB.b.item);
	LB.route_found = n.route_found != 0;
	for (int u = 0; u < LB.b.nunits; ++u) {
		LB.b.unit[u].x = n.unit[u].x; LB.b.unit[u].y = n.unit[u].y; LB.b.unit[u].gone = n.unit[u].gone != 0;
		LB.back_in[u] = n.back_in[u];
		if (LB.hp_max[u]) LB.hp[u] = LB.hp_shown[u] = n.hp[u] < 1 ? 1 : n.hp[u] > LB.hp_max[u] ? LB.hp_max[u] : n.hp[u];
		netmap_world(LB.b.unit[u].x, LB.b.unit[u].y, &LB.ux[u], &LB.uy[u]);
	}
	LB.step = resumed_step(n.step, n.after);
	LB.after = n.after; LB.phase = n.phase; LB.op = n.op; LB.prize = n.prize;
	LB.fight = n.fight; LB.fight_unit = n.fight_unit; LB.fight_cmd = n.fight_cmd; LB.fx = n.fx; LB.fy = n.fy;
	LB.enemy_unit = n.enemy_unit; LB.enemy_steps = 0; LB.enemy_t = 0;   /* (the unit's turn shown from its start) */
	LB.briefed = n.briefed; LB.holes_told = n.holes_told; LB.prize_given = n.prize_given; LB.shield = n.shield;
	LB.enemy_banner = n.enemy_banner; LB.fight_seen = n.fight_seen;
	LB.banner_t = 0;
	LB.words[0] = 0;
	LB.cheer = 0;
	LB.npops = 0;
	LB.tour = LB.tour_t = 0;
	LB.effect.sprite = -1;
	LB.harm = 0;
	LB.reach_lit = false;
	memset(LB.hidden, 0, sizeof LB.hidden);
	lib_camera_mend();
	lib_look_begin();
	if (emu_debug_on()) fprintf(stderr, "liberation: board taken back, phase %d, step %d, %d Order Points\n", LB.phase, LB.step, LB.op);
	return true;
}
