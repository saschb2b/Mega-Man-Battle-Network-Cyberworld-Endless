/* Set pieces: which of BN6's own interactables a layer holds (epic #49,
 * docs/LEVEL_DESIGN.md, Set pieces). Each area repeats the few its
 * originals hold; a layer takes one or two by its place in the act, from
 * the run's seed and the depth alone, so a Net Dealer earlier in the act
 * knows which keys to stock. Where each stands is net_gen.c's. */
#include "net.h"
#include "run.h"

/* What each area's original maps hold, as weights (BN6's own placements:
 * the epic's Research 1): purple data in every area but the Secret Area,
 * the most in the Graveyard and the Undernet; Rush's bone gaps in
 * Central, Seaside, Green, Sky, the Undernet and ACDC's homepage, the
 * longest in Sky; teleport pairs where their gem marks them, Green's four
 * in Green Area 1, Sky's three, Central's one. */
static const struct { uint8_t purple, rush, rush_len, teleport; } area_pieces[BIOME_COUNT] = {
	[BIOME_CENTRAL] = { 2, 3, 2, 1 },
	[BIOME_SEASIDE] = { 2, 2, 1, 0 },
	[BIOME_SKY] = { 2, 3, 3, 3 },
	[BIOME_GREEN] = { 2, 3, 1, 4 },
	[BIOME_GRAVEYARD] = { 4, 0, 0 },
	[BIOME_UNDERNET] = { 4, 2, 2 },
	[BIOME_SECRET] = { 0, 0, 0 },
	[BIOME_NEST] = { 1, 0, 0 },
	[BIOME_COMP] = { 1, 0, 0 },
	[BIOME_HOMEPAGE] = { 2, 0, 0 },
	[BIOME_COMP_B] = { 1, 0, 0 },
	[BIOME_ROBOT_COMP] = { 1, 0, 0 },
	[BIOME_AQUARIUM_COMP] = { 1, 0, 0 },
	[BIOME_JUDGE_COMP] = { 1, 0, 0 },
	[BIOME_WEATHER_COMP] = { 1, 0, 0 },
	[BIOME_COPYBOT_COMP] = { 1, 0, 0 },
	[BIOME_ACDC_HP] = { 2, 2, 1 },
	[BIOME_GREEN_HP] = { 2, 0, 0 },
	[BIOME_SKY_HP] = { 2, 0, 0 },
};

static uint32_t piece_hash(int depth, uint32_t salt) {
	uint32_t h = (run.seed ^ (uint32_t)depth * 0x9E3779B9u ^ salt) * 2654435761u;
	h ^= h >> 15;
	return h * 2246822519u;
}

/* A layer's pieces (PIECE_* bits): none on the run's first layer or in the
 * Secret Area; else, as the act goes, one on its first layer (half of
 * them), two on its middle one (the dealer's: a choice of where to spend),
 * one on the guardian's (four in ten), one on a dark warp's layer (six in
 * ten); each of the area's, picked by its weight, never twice. */
unsigned layer_pieces(int depth, int biome, int kind) {
	if (depth <= 1 || kind == LAYER_SECRET || biome < 0 || biome >= BIOME_COUNT) return 0;
	int in_act = layer_in_act(depth);
	int chance = kind == LAYER_UNDERNET ? 60 : in_act == 0 ? 50 : in_act == 1 ? 90 : 40;
	int budget = kind == LAYER_NORMAL && in_act == 1 ? 2 : 1;
	uint32_t h = piece_hash(depth, 0x9A7B1E5Du);
	if ((int)(h % 100) >= chance) return 0;
	/* (and a weight of nothing, so an area with few of BN6's pieces keeps
	 * them rare, as its maps do: about one in two of BN6's maps holds a
	 * purple data) */
	/* (no Rush before a guardian: his cutscene restarts every NPC script on
	 * the map, the guardian's actors too) */
	int w[4] = { area_pieces[biome].purple, kind == LAYER_NORMAL && is_boss_depth(depth) ? 0 : area_pieces[biome].rush,
		area_pieces[biome].teleport, 2 };
	unsigned bits[4] = { PIECE_PURPLE, PIECE_RUSH, PIECE_TELEPORT, 0 }, got = 0;
	for (int k = 0; k < budget; ++k) {
		int total = 0;
		for (int i = 0; i < 4; ++i) total += got & bits[i] ? 0 : w[i];
		int roll = (int)((h >> (8 + 8 * k)) % (uint32_t)total);
		for (int i = 0; i < 4; ++i) {
			if (got & bits[i]) continue;
			if (roll < w[i]) { got |= bits[i]; break; }
			roll -= w[i];
		}
	}
	return got;
}

bool layer_purple(int depth, int biome, int kind) { return layer_pieces(depth, biome, kind) & PIECE_PURPLE; }

int layer_pieces_ahead(int depth, unsigned piece) {
	int n = 0;
	for (int d = depth; d <= depth + 2 && (d == depth || layer_in_act(d) > 0); ++d)
		n += (layer_pieces(d, biome_for_depth(d), LAYER_NORMAL) & piece) != 0;
	return n;
}

/* The longest gap Rush bridges in the area (Sky's run to three panels, as
 * Sky Area 1's do); a layer's gap is one to this long, from its seed. */
int layer_rush_len(int depth, int biome) {
	int most = biome >= 0 && biome < BIOME_COUNT ? area_pieces[biome].rush_len : 1;
	if (most < 1) most = 1;
	return 1 + (int)((piece_hash(depth, 0x0B0E5u) >> 4) % (uint32_t)most);
}
