/* Set pieces: which of BN6's own interactables a layer holds (epic #49,
 * docs/LEVEL_DESIGN.md, Set pieces). Each area repeats the few its
 * originals hold; a layer takes one or two by its place in the act, from
 * the run's seed and the depth alone, so a Net Dealer earlier in the act
 * knows which keys to stock. Where each stands is net_gen.c's. */
#include "net.h"
#include "run.h"

/* (Seaside's toll, as its Area 2 takes 100 zenny a pass; elsewhere a
 * P-Code, as BN6's comps and homepages ask) */
int layer_cube_kind(int biome) { return biome == BIOME_SEASIDE ? BLOCK_TOLL : BLOCK_PCODE; }

/* What each area's original maps hold, as weights (BN6's own placements:
 * the epic's Research 1): purple data in every area but the Secret Area,
 * the most in the Graveyard and the Undernet; Rush's bone gaps in
 * Central, Seaside, Green, Sky, the Undernet and ACDC's homepage, the
 * longest in Sky; teleport pairs where their gem marks them, Green's four
 * in Green Area 1, Sky's three, Central's one; security cubes in every
 * comp and homepage, Central and Seaside; the Link Navis' obstacles
 * of the area's own kinds (Seaside's water and cyclone, Green's trees,
 * Sky's clouds, flames and water, the Underground's under the Nest, all
 * five in the Graveyard, which keeps nearly all its data behind them;
 * none in Central); arrow lanes where BN6's maps lay arrow panels, the
 * most in Seaside, whose second area is a field of them, few in Sky, whose
 * layers seldom leave a short gap from a far floor back to the way (one
 * in ten found room), none in Robot Control's (two ways drawn, its
 * layers too close). */
static const struct { uint8_t purple, rush, rush_len, teleport, obstacle, kinds, cube, arrow; } area_pieces[BIOME_COUNT] = {
	[BIOME_CENTRAL] = { 2, 3, 2, 1, 0, 0, 2 },
	[BIOME_SEASIDE] = { 2, 2, 1, 0, 2, 1 << BLOCK_WATER | 1 << BLOCK_CYCLONE, 1, 3 },
	[BIOME_SKY] = { 2, 3, 3, 3, 3, 1 << BLOCK_CLOUD | 1 << BLOCK_FLAMES | 1 << BLOCK_WATER, 0, 1 },
	[BIOME_GREEN] = { 2, 3, 1, 4, 2, 1 << BLOCK_TREE, 0, 2 },
	[BIOME_GRAVEYARD] = { 4, 0, 0, 0, 4, 0x1F },
	[BIOME_UNDERNET] = { 4, 2, 2, 0, 0, 0, 0, 2 },
	[BIOME_SECRET] = { 0, 0, 0 },
	[BIOME_NEST] = { 1, 0, 0, 0, 3, 1 << BLOCK_CLOUD | 1 << BLOCK_CYCLONE | 1 << BLOCK_FLAMES, 0, 2 },
	[BIOME_COMP] = { 1, 0, 0, 0, 0, 0, 3 },
	[BIOME_HOMEPAGE] = { 2, 0, 0, 0, 0, 0, 3 },
	[BIOME_COMP_B] = { 1, 0, 0, 0, 0, 0, 3 },
	[BIOME_ROBOT_COMP] = { 1, 0, 0, 0, 0, 0, 3 },
	[BIOME_AQUARIUM_COMP] = { 1, 0, 0, 0, 0, 0, 3 },
	[BIOME_JUDGE_COMP] = { 1, 0, 0, 0, 0, 0, 3, 2 },
	[BIOME_WEATHER_COMP] = { 1, 0, 0, 0, 0, 0, 3 },
	[BIOME_COPYBOT_COMP] = { 1, 0, 0, 0, 0, 0, 3 },
	[BIOME_ACDC_HP] = { 2, 2, 1, 0, 0, 0, 3 },
	[BIOME_GREEN_HP] = { 2, 0, 0, 0, 0, 0, 3 },
	[BIOME_SKY_HP] = { 2, 0, 0, 0, 0, 0, 3 },
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
unsigned layer_pieces_forced;

/* The pieces an area has at all (a weight of its own). */
static unsigned area_has(int biome) {
	return (area_pieces[biome].purple ? PIECE_PURPLE : 0) | (area_pieces[biome].rush ? PIECE_RUSH : 0) |
		(area_pieces[biome].teleport ? PIECE_TELEPORT : 0) | (area_pieces[biome].obstacle ? PIECE_OBSTACLE : 0) |
		(area_pieces[biome].cube ? PIECE_CUBE : 0) | (area_pieces[biome].arrow ? PIECE_ARROW : 0);
}

static unsigned rolled_pieces(int depth, int biome, int kind);

unsigned layer_pieces(int depth, int biome, int kind) {
	if (depth <= 1 || kind == LAYER_SECRET || biome < 0 || biome >= BIOME_COUNT) return 0;
	/* (--dev pieces=MASK: those of the area's every layer, for captures;
	 * no Rush before a guardian still) */
	unsigned forced = layer_pieces_forced & area_has(biome) & (kind == LAYER_NORMAL && is_boss_depth(depth) ? ~(unsigned)PIECE_RUSH : ~0u);
	return rolled_pieces(depth, biome, kind) | forced;
}

static unsigned rolled_pieces(int depth, int biome, int kind) {
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
	int w[7] = { area_pieces[biome].purple, kind == LAYER_NORMAL && is_boss_depth(depth) ? 0 : area_pieces[biome].rush,
		area_pieces[biome].teleport, area_pieces[biome].obstacle, area_pieces[biome].cube, area_pieces[biome].arrow, 2 };
	unsigned bits[7] = { PIECE_PURPLE, PIECE_RUSH, PIECE_TELEPORT, PIECE_OBSTACLE, PIECE_CUBE, PIECE_ARROW, 0 }, got = 0;
	for (int k = 0; k < budget; ++k) {
		int total = 0;
		for (int i = 0; i < 7; ++i) total += got & bits[i] ? 0 : w[i];
		int roll = (int)((h >> (8 + 8 * k)) % (uint32_t)total);
		for (int i = 0; i < 7; ++i) {
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

/* (as powers.c gives them: the Cross brought keeps alone, else each
 * guardian of navis 1-5 deleted before `depth` gives his) */
unsigned layer_crosses(int depth) {
	if (run.cross) return 1u << run.cross;
	unsigned held = 0;
	for (int d = 1; d < depth; ++d)
		if (is_boss_depth(d)) {
			int navi = run.boss_order[biome_for_depth(d)];
			if (navi >= 1 && navi <= 5) held |= 1u << navi;
		}
	return held;
}

/* Gregar's pairs: the cyberwater and the cloud ElecMan or EraseMan, the
 * cybertree HeatMan or SlashMan, the flames HeatMan or ChargeMan, the
 * cyclone SlashMan or ChargeMan (navis 1 Heat, 2 Elec, 3 Slash, 4 Erase,
 * 5 Charge) */
unsigned block_openers(int kind) {
	static const uint8_t openers[BLOCK_KINDS] = { 1 << 2 | 1 << 4, 1 << 1 | 1 << 3, 1 << 1 | 1 << 5, 1 << 3 | 1 << 5, 1 << 2 | 1 << 4 };
	return kind >= 0 && kind < BLOCK_KINDS ? openers[kind] : 0;
}

/* The pick of n kinds among `set`, by r. */
static int nth_kind(unsigned set, uint32_t r) {
	int n = 0;
	for (int k = 0; k < BLOCK_KINDS; ++k) n += set >> k & 1;
	if (!n) return -1;
	int pick = (int)(r % (uint32_t)n);
	for (int k = 0; k < BLOCK_KINDS; ++k)
		if (set >> k & 1 && pick-- == 0) return k;
	return -1;
}

/* Mostly one the run can clear (four in five), else one it can't, a hint
 * for the next run's Cross; with no Cross that clears any of the area's,
 * one of them two layers in five, else none (a locked pocket a run can't
 * open is dead weight, one now and then a lesson). */
int layer_block_kind(int depth, int biome) {
	if (biome < 0 || biome >= BIOME_COUNT) return -1;
	unsigned kinds = area_pieces[biome].kinds, held = layer_crosses(depth), open = 0;
	for (int k = 0; k < BLOCK_KINDS; ++k)
		if (kinds >> k & 1 && block_openers(k) & held) open |= 1u << k;
	uint32_t h = piece_hash(depth, 0x0B57AC1Eu);
	if (open) return (h % 100 < 80 || open == kinds) ? nth_kind(open, h >> 8) : nth_kind(kinds & ~open, h >> 8);
	return h % 100 < 40 ? nth_kind(kinds, h >> 8) : -1;
}
