/* What stands on the layer, in the state's words (layer_describe,
 * director_layer.h): the exit pad, the guardian heard of, the Mystery Data
 * taken, the locks, a Navi gate, and what MegaMan stands on. Read by
 * tests/scenarios (docs/devtools/scenarios.md); nothing here changes the game. */
#include <stdio.h>

#include "blockers.h"
#include "bn6_fields.h"
#include "director_layer.h"
#include "director_state.h"
#include "flags.h"
#include "guardians.h"
#include "mapslot.h"
#include "net_shapes.h"
#include "netmap.h"
#include "rivals.h"

/* whether panel (px, py) is one of the `len` panels that lead from (x, y) along `dir` */
static bool on_run(int px, int py, int x, int y, int dir, int len) {
	for (int j = 1; j <= len; ++j)
		if (px == x + dir_dx[dir] * j && py == y + dir_dy[dir] * j) return true;
	return false;
}

void layer_describe(FILE *f) {
	int taken[3] = { 0, 0, 0 };
	for (int k = 0; k < D.objs.nmd; ++k)
		taken[D.objs.md_colour[k] == MYSTERY_GREEN ? 0 : D.objs.md_colour[k] == MYSTERY_BLUE ? 1 : 2] += flag_get(MAPSLOT_MD_FLAG + k);
	fprintf(f, "exitpad %s\nheard %s\ngreentaken %d\nbluetaken %d\npurpletaken %d\n", D.exit_shut ? "shut" : "open", guardian_heard() ? "yes" : "no",
		taken[0], taken[1], taken[2]);
	/* (the locks still standing, and a Security Cube's P-Code told) */
	int locks = 0;
	for (int k = 0; k < layer.nblocks; ++k) locks += flag_get(BLOCK_PRESENT_FLAG + k);
	fprintf(f, "locks %d\npcode %s\n", locks, flag_get(LAYER_PCODE_FLAG) ? "told" : "untold");
	/* (a Navi gate's Navi, and whether his code is earned) */
	if (D.objs.gate_navi)
		fprintf(f, "gate %s %s\n", guardian(D.objs.gate_navi)->name, rival(D.objs.gate_navi)->megaman_won >= GATE_CODE ? "earned" : "sealed");
	/* (the teleport pad MegaMan stands on) */
	int px = -1, py = -1;
	const char *on = "floor";
	if (!netmap_panel(bn6_player_x(), bn6_player_y(), &px, &py)) on = "none";
	for (int k = 0; k < 2 && layer.nteleports; ++k)
		if (px == layer.teleport_x[k] && py == layer.teleport_y[k]) on = k ? "teleport2" : "teleport1";
	/* (... or the set piece's panels: Rush's gap, an arrow lane, an invisible path) */
	for (int k = 0; k < layer.ngaps; ++k) if (on_run(px, py, layer.gap[k].x, layer.gap[k].y, layer.gap[k].dir, layer.gap[k].len)) on = "gap";
	for (int k = 0; k < layer.nlanes; ++k) if (on_run(px, py, layer.lane[k].x, layer.lane[k].y, layer.lane[k].dir, layer.lane[k].len)) on = "lane";
	for (int k = 0; k < layer.npaths; ++k) if (on_run(px, py, layer.path[k].x, layer.path[k].y, layer.path[k].dir, layer.path[k].len)) on = "path";
	fprintf(f, "on %s\n", on);
}
