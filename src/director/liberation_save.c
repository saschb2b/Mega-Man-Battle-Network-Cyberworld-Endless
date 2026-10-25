/* A liberation's board across a CONTINUE (liberation.h; docs/SAVES.md):
 * what its course has changed since the layer's seed made it, saved beside
 * the run's checkpoint as `run.lib` and taken back once the checkpoint's
 * state has loaded on the layer the seed made again. The board's panels,
 * its units, the phase and the Order Points are the note's; what was being
 * shown (a banner's frames, words due, a unit's glide) begins again. */
#include "liberation.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "liberation_state.h"
#include "liberation_team.h"
#include "netmap.h"
#include "run.h"
#include "save_blob.h"

#define LIB_NOTE_MAGIC 0x434C4233u   /* "CLB3": the team (docs/LIBERATION.md, The team) */

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
	/* the team: the Navi in play, each one's place, HP and turn; the units
	 * SneakAttack struck */
	int32_t in_play, nteam, switch_to, switch_after;
	struct { int16_t hp, wx, wy; uint8_t x, y, out, face, acted, melody, spoke, pad; } mate[LIB_TEAM];
	uint8_t weak[LIB_UNITS];
} LibNote;
_Static_assert(sizeof(LibNote) == 68 + LIB_UNITS * 5 + 2 * MAP_H * MAP_W + 4 + 16 + LIB_TEAM * 14 + 2 + LIB_UNITS &&
	offsetof(LibNote, kind) == 68 + LIB_UNITS * 5, "Liberation save layout changed: bump LIB_NOTE_MAGIC");

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
		}
		memcpy(n.kind, LB.b.kind, sizeof n.kind);
		memcpy(n.item, LB.b.item, sizeof n.item);
		n.route_found = LB.route_found;
		n.in_play = LB.in_play;
		n.nteam = LB.b.team.n;
		n.switch_to = LB.switch_to;
		n.switch_after = LB.switch_after;
		for (int k = 0; k < LB.b.team.n; ++k) {
			lib_mate_world(k, &LB.mate[k].wx, &LB.mate[k].wy);
			n.mate[k].hp = (int16_t)LB.mate[k].hp; n.mate[k].wx = (int16_t)LB.mate[k].wx; n.mate[k].wy = (int16_t)LB.mate[k].wy;
			n.mate[k].x = LB.b.mate[k].x; n.mate[k].y = LB.b.mate[k].y; n.mate[k].out = LB.b.mate[k].out;
			n.mate[k].face = (uint8_t)LB.mate[k].face; n.mate[k].acted = LB.mate[k].acted; n.mate[k].melody = LB.mate[k].melody;
			n.mate[k].spoke = LB.mate[k].spoke;
		}
		for (int u = 0; u < LIB_UNITS; ++u) n.weak[u] = LB.weak[u];
	}
	save_write_blob("run.lib", LIB_NOTE_MAGIC, &n, sizeof n);
}

/* The step a saved one goes on as: a talk's words were not kept (no chat
 * box is open as a checkpoint is saved), so its step after; the menu
 * closed; a banner shown from its start */
static int resumed_step(int step, int after) {
	if (step == LIB_STEP_TALK) step = after;
	if (step == LIB_STEP_MENU) step = LIB_STEP_MOVE;
	return step;
}

bool lib_resume(void) {
	LibNote n;
	if (!LB.on || !save_read_blob("run.lib", LIB_NOTE_MAGIC, &n, sizeof n) || !n.on || n.seed != run.layer_seed || n.depth != run.depth ||
	    n.nunits != LB.b.nunits || n.nteam != LB.b.team.n || n.in_play < 0 || n.in_play >= n.nteam)
		return false;
	memcpy(LB.b.kind, n.kind, sizeof LB.b.kind);
	memcpy(LB.b.item, n.item, sizeof LB.b.item);
	LB.route_found = n.route_found != 0;
	for (int k = 0; k < LB.b.team.n; ++k) {
		LB.mate[k].hp = n.mate[k].hp; LB.mate[k].wx = n.mate[k].wx; LB.mate[k].wy = n.mate[k].wy;
		LB.b.mate[k].x = n.mate[k].x; LB.b.mate[k].y = n.mate[k].y; LB.b.mate[k].out = n.mate[k].out != 0;
		LB.mate[k].face = n.mate[k].face; LB.mate[k].acted = n.mate[k].acted != 0; LB.mate[k].melody = n.mate[k].melody != 0;
		LB.mate[k].spoke = n.mate[k].spoke != 0;
		LB.anim[LIB_UNITS + k] = n.mate[k].face;
	}
	for (int u = 0; u < LIB_UNITS; ++u) LB.weak[u] = n.weak[u] != 0;
	/* (the state holds the player in the body of the Navi in play) */
	LB.in_play = n.in_play;
	LB.switch_to = n.switch_to >= 0 && n.switch_to < n.nteam ? n.switch_to : 0;
	LB.switch_after = n.switch_after;
	lib_team_resume();
	for (int u = 0; u < LB.b.nunits; ++u) {
		LB.b.unit[u].x = n.unit[u].x; LB.b.unit[u].y = n.unit[u].y; LB.b.unit[u].gone = n.unit[u].gone != 0;
		LB.back_in[u] = n.back_in[u];
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
