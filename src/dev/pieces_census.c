/* The set pieces a player meets (docs/world/level-design/set-pieces.md,
 * The act's plan): over runs of many seeds, each normal layer of the net's
 * cycle built as the game builds it, its area dressed by the run's seed
 * (BN5's and BN4's areas where their ROMs are beside BN6's) and its kit
 * that area's; what the act's plan gave it (layer_pieces) beside what
 * stands on it, as the unit tests' layers (one area each, a kit of no
 * area's) cannot say. A dark layer is the player's choice at a fork, and
 * counted as the plain layer it would be. */
#include "pieces_census.h"

#include <stdio.h>
#include <string.h>

#include "net.h"
#include "netmap.h"
#include "rom.h"
#include "run.h"

enum { K_PURPLE, K_RUSH, K_TELEPORT, K_OBSTACLE, K_CUBE, K_ARROW, K_HIDDEN, KINDS };
static const char *const kind_names[KINDS] = { "purple data", "Rush gap", "teleport", "Link Navi obstacle", "cube or door", "arrow lane",
	"invisible path" };
static const unsigned kind_bits[KINDS] = { PIECE_PURPLE, PIECE_RUSH, PIECE_TELEPORT, PIECE_OBSTACLE, PIECE_CUBE, PIECE_ARROW, PIECE_HIDDEN };

/* A tally: layers, those holding a lock MegaMan must open or find his way
 * past (a gap, an obstacle, a cube or door, an invisible path, a
 * teleport's island), and each kind planned and standing. */
typedef struct { int layers, locked, planned[KINDS], stood[KINDS]; } Tally;

#define AREAS (NET_AREAS + XAREAS_MAX)
static Tally by_place[3], by_dress[2], by_area[AREAS], all;
static int first_lock[4];   /* runs whose first lock stood in act 1, 2, later, never */

/* What stands on the layer just generated: a bit each (K_*) */
static unsigned stood(void) {
	unsigned got = 0;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_MYSTERY && layer.obj[i].param == MD_PURPLE) got |= 1u << K_PURPLE;
	if (layer.ngaps) got |= 1u << K_RUSH;
	if (layer.nteleports) got |= 1u << K_TELEPORT;
	for (int b = 0; b < layer.nblocks; ++b) got |= 1u << (layer.block[b].kind < BLOCK_KINDS ? K_OBSTACLE : K_CUBE);
	if (layer.nlanes) got |= 1u << K_ARROW;
	if (layer.npaths) got |= 1u << K_HIDDEN;
	return got;
}

static bool locked(unsigned got) {
	return got & (1u << K_RUSH | 1u << K_OBSTACLE | 1u << K_CUBE | 1u << K_HIDDEN) || layer.teleport_island;
}

static void count(Tally *t, unsigned planned, unsigned got) {
	++t->layers;
	t->locked += locked(got);
	for (int k = 0; k < KINDS; ++k) {
		t->planned[k] += (planned & kind_bits[k]) != 0;
		t->stood[k] += got >> k & 1;
	}
}

static void write_tally(FILE *out, const char *what, const Tally *t) {
	if (!t->layers) return;
	fprintf(out, "%-28s %5d layers, a lock on %5.1f%%:", what, t->layers, 100.0 * t->locked / t->layers);
	for (int k = 0; k < KINDS; ++k) fprintf(out, " %s %d/%d", kind_names[k], t->stood[k], t->planned[k]);
	fputc('\n', out);
}

/* Run `seed`'s cycle, layer by layer */
static void one_run(uint32_t seed) {
	int first = 3;
	for (int depth = 1; depth <= CYCLE_LAYERS; ++depth) {
		run_new(seed);
		int biome = biome_for_depth(depth), area = run_dress(biome);
		run.depth = depth;
		run.biome = biome;
		run.side_kind = LAYER_NORMAL;
		LayerKit kit;
		netmap_kit(area, &kit);
		layer_generate(run_layer_seed(depth, LAYER_NORMAL), depth, biome, LAYER_NORMAL, &kit);
		unsigned planned = layer_pieces(depth, biome, LAYER_NORMAL), got = stood();
		count(&by_place[layer_in_act(depth)], planned, got);
		count(&by_dress[area >= NET_AREAS], planned, got);
		if (area >= 0 && area < AREAS) count(&by_area[area], planned, got);
		count(&all, planned, got);
		int act = (depth - 1) / 3;
		if (locked(got) && first == 3) first = act < 2 ? act : 2;
	}
	++first_lock[first];
}

int pieces_census_run(const char *dir, int runs) {
	/* (the other games' ROMs as a run finds them: no research game's areas) */
	xrom_find_beside();
	char path[600];
	snprintf(path, sizeof path, "%s/pieces.txt", dir);
	FILE *out = fopen(path, "w");
	if (!out) { fprintf(stderr, "pieces: cannot write %s\n", path); return 1; }
	if (runs < 1) runs = 1;
	for (int s = 1; s <= runs; ++s) one_run((uint32_t)s * 2654435761u);
	fprintf(out, "set pieces over %d runs' normal layers, depths 1-%d (standing/planned layers)\n", runs, CYCLE_LAYERS);
	write_tally(out, "all", &all);
	static const char *const places[3] = { "an act's first layer", "an act's middle layer", "a guardian's layer" };
	for (int p = 0; p < 3; ++p) write_tally(out, places[p], &by_place[p]);
	write_tally(out, "BN6's areas", &by_dress[0]);
	write_tally(out, "another game's areas", &by_dress[1]);
	fprintf(out, "the run's first lock: in act 1 %d, act 2 %d, later %d, never %d\n", first_lock[0], first_lock[1], first_lock[2], first_lock[3]);
	for (int a = 0; a < AREAS; ++a) {
		const NetAreaDef *d = net_area_def(a);
		if (!by_area[a].layers || !d) continue;
		char what[64];
		snprintf(what, sizeof what, "%2d %s", a, d->name ? d->name : "?");
		write_tally(out, what, &by_area[a]);
	}
	fclose(out);
	printf("set pieces in %s\n", path);
	return 0;
}
