/* The towns' styles (docs/world/overworld.md): each the original it is cut
 * from or copied whole, and where (town_style.h); town.c plans and installs
 * them. Oran Isle is another game's (docs/world/bn5-town.md): its map read
 * from BN5's ROM, its tiles and colours copied in (xtown.c), its people
 * the stop's (oran.c), its song BN5's (xsong.c). */
#include "town_style.h"

#include <stdlib.h>

#include "area_src.h"
#include "bn5_world.h"
#include "oran.h"
#include "rom.h"
#include "town_lines.h"
#include "xsong.h"
#include "xtown.h"

_Static_assert(TOWN_STYLES == TOWN_LINES && TOWN_STYLES == TOWN_ORAN + 1, "each style its people and words (town_lines.c)");

int town_env_or(const char *name, int value) {
	const char *v = getenv(name);
	return v ? atoi(v) : value;
}




/* -- Central Town (0x01:0) -- */

static int central_mat(int i) {
	if (i == 0) return TM_VOID;
	if (i == 10) return TM_ROAD;
	if (i >= 1 && i <= 3) return TM_MARK;
	if (i == 4) return TM_SIDE;
	if (i >= 5 && i <= 8) return TM_COBB;
	if (i >= 80 && i <= 84) return TM_BRICK;
	if ((i >= 32 && i <= 39) || i == 15 || i == 17 || i == 18 || i == 20 || i == 97 || i == 98)
		return TM_GRASS;   /* (the pinks 106-109 are the houses' walls) */
	return TM_EDGE;
}

/* The original's cells, generously: the slab spans x -25..20, y -25..40;
 * past it hang its faces and the backs of the tall pieces. */
#define SX0 (-31)
#define SX1 24
#define SY0 (-33)
#define SY1 45

/* The plan: the original, widened by w cells (0 or 8): the Expo gates
 * stand further from the Academy, the plaza, the main road and the houses'
 * cobbled court grow wider. The cuts run through ground that is the same
 * all along them: the north-west road's middle (row -18|-17) splits the
 * gates' band, stretched in the gap between the Expo gates and the Academy
 * (x -5|-4), from the middle, stretched through the plaza's brick and the
 * main road (x 5|6); the main road's near curb (row 10|11) splits that from
 * the houses', stretched through their court (x -22|-21). */
static void design_central(const TownKit *k) {
	int w = town_env_or("CYBERWORLD_TOWN_VARIANT", 0) ? 8 : 0;   /* (home: the same each run) */
	/* the gates' band */
	k->copy(SX0, SY0, -5, -18, 0, 0, 0);
	k->tile(-4, SY0, -5 + w, -18, -5, SY0, 1, -18 - SY0 + 1);
	k->copy(-4, SY0, SX1, -18, w, 0, 0);
	/* Aster Land, the plaza, the main road */
	k->copy(SX0, -17, 5, 10, 0, 0, F_JACK_IN);
	k->tile(6, -17, 5 + w, 10, 5, -17, 1, 28);
	k->copy(6, -17, SX1, 10, w, 0, F_JACK_IN);
	/* the houses and the bus stop */
	k->copy(SX0, 11, -22, SY1, 0, 0, 0);
	k->tile(-21, 11, -22 + w, SY1, -22, 11, 1, SY1 - 11 + 1);
	k->copy(-21, 11, SX1, SY1, w, 0, 0);
	/* the trees on the verge between the gates, a row 4 cells apart (where
	 * in their cells the original's stand) */
	for (int x = -3; x < -3 + w; x += 4) k->tree(x * 8 + 4, -24 * 8 + 4);
}

/* -- ACDC Town (0x00:0) -- */

static int acdc_mat(int i) {
	if (i == 0) return TM_VOID;
	if (i == 6 || (i >= 182 && i <= 186)) return TM_ROAD;   /* (with its joints) */
	if (i == 10) return TM_MARK;                             /* the crossings */
	if (i >= 1 && i <= 3) return TM_SIDE;
	if (i >= 11 && i <= 15) return TM_GRASS;                 /* lawn, walkable in its rims */
	if (i >= 89 && i <= 92) return TM_GRASS;                 /* bushes */
	if (i >= 121 && i <= 123) return TM_COBB;                /* the park's dirt */
	return TM_EDGE;
}

/* The original's cells: the slab spans x -39..39, y -23..22. */
#define AX0 (-45)
#define AX1 46
#define AY0 (-31)
#define AY1 29

/* The plan: ACDC Town's blocks in one row between its roads: the park and
 * Higsby's (x -33..-14), the L of houses round the Metroline with Dex's
 * block in its notch (x -9..22), the mansion at the end. The first two
 * trade places half the time, and the road between them (4 cells) goes
 * with them, stretched from the original's. The cuts run down the roads
 * from the back road to the avenue's near curb (row 10|11); below it they
 * run down the avenue and the promenade clear of each block's crossing, so
 * a crossing and its landing go with the block at whose corner they lie. */
static void design_acdc(const TownKit *k) {
	if (!town_env_or("CYBERWORLD_TOWN_VARIANT", k->coin())) {
		k->copy(AX0, AY0, AX1, AY1, 0, 0, F_JACK_IN);
		return;
	}
	/* the blocks and the road between them */
	k->copy(AX0, AY0, -34, 10, 0, 0, 0);
	k->copy(-9, AY0, 22, 10, -24, 0, F_JACK_IN);
	k->tile(-1, AY0, 2, 10, -12, AY0, 1, 10 - AY0 + 1);
	k->copy(-33, AY0, -14, 10, 36, 0, F_JACK_IN);
	k->copy(23, AY0, AX1, 10, 0, 0, 0);
	/* the avenue, the promenade and the slab's face below them */
	k->copy(AX0, 11, -36, AY1, 0, 0, 0);
	k->copy(-11, 11, 22, AY1, -24, 0, 0);
	k->tile(-1, 11, 0, AY1, -12, 11, 1, AY1 - 11 + 1);
	k->copy(-35, 11, -14, AY1, 36, 0, 0);
	k->copy(23, 11, AX1, AY1, 0, 0, 0);
}

/* -- Seaside Town (0x03:0) -- */

/* (its ground is copied whole: the materials only tell sky from the rest) */
static int seaside_mat(int i) { return i == 0 ? TM_VOID : TM_EDGE; }

/* The plan: the original as it stands, with its own walls and heights (the
 * plaza at 0, the walkway over the whale and the station at 64, the pier
 * at -32), which the other towns' flat plans do not model; the whole of
 * it, since the roofs' and the whale's art stands on cells far up the
 * picture. */
static void design_seaside(const TownKit *k) { k->copy(-62, -62, 62, 62, 0, 0, F_JACK_IN); }

/* -- Green Town (0x04:0) -- */

/* (copied whole, as Seaside Town: its planks, ponds and stumps stand at 16,
 * its plaza, paths and the pond by the Judge Tree at 0) */
static int green_mat(int i) { return i == 0 ? TM_VOID : TM_EDGE; }
static void design_green(const TownKit *k) { k->copy(-62, -62, 62, 62, 0, 0, F_JACK_IN); }
/* (Oran Isle, BN5's, copied whole in Seaside Town's map: docs/world/bn5-town.md) */

const TownStyle town_styles[TOWN_STYLES] = {
	{ 0x01, 0x00, 0x03, 0, { 12, -9, 16, -2 }, { 14, -9, 16, -6 }, central_mat, design_central, -40, 266, FACE_SW,
	  { { -40, 266 }, { 100, -30 }, { -150, -16 }, { 90, -150 }, { 110, 110 }, { -150, -160 } }, false,
	  "Central Town", "bird statue", "bird statue on the plaza", { 0, 0 }, 1, 1u << 2 | 1u << 4 },
	{ 0x00, 0x00, 0x24, 1 << 0, { -23, -10, -17, -3 }, { -20, -9, -16, -3 }, acdc_mat, design_acdc, -60, -108, FACE_SW,
	  { { -60, -108 }, { -190, -30 }, { -190, -120 }, { 110, -120 }, { 260, -60 }, { 60, 120 } }, false,
	  "ACDC Town", "squirrel statue", "squirrel statue in the park", { 0x41, 0xF7 } },
	/* (its own two ports the act's other ways: the fountain is the landmark) */
	{ 0x03, 0x00, 0x06, 0, { -8, -15, -6, -8 }, { -18, -16, -9, -7 }, seaside_mat, design_seaside, 4, -100, FACE_SW,
	  { { 4, -100 }, { -100, -100 }, { -120, -170 }, { 40, -70 }, { 150, 40 }, { 300, 0 } }, true,
	  "Seaside Town", "mermaid fountain", "mermaid fountain on the plaza", { 0x40, 0x41 } },
	/* (its port the original's own, round the knight statue) */
	{ 0x04, 0x00, 0x08, 1 << 0, { -7, -22, -1, -16 }, { -5, -22, -1, -18 }, green_mat, design_green, 40, -100, FACE_SW,
	  { { 40, -100 }, { -60, -204 }, { -20, -268 }, { -196, -172 }, { -32, 24 }, { -64, 312 } }, true,
	  "Green Town", "knight statue", "knight statue on the flower plaza", { 0xF1, 0xF7 } },
	{ 0x03, 0x00, BN5_SONG_BEACH_DAY, 1 << 0, { 999, 999, 0, 0 }, { 999, 999, 0, 0 }, green_mat, design_green, BN5_ORAN_DOCK_X, BN5_ORAN_DOCK_Y, FACE_NW,
	  { { BN5_ORAN_DOCK_X, BN5_ORAN_DOCK_Y }, { 40, 40 }, { 104, -160 }, { -120, 120 }, { 160, 0 }, { -40, -200 } }, true,
	  "Oran Isle", "signal post", "signal post on the plaza", { 0, 0 }, 0, 0, 1 + XROM_BN5_COLONEL_US, BN5_ORAN_GROUP, BN5_ORAN_MAP },
};

TownBook *town_style_learn(const TownStyle *s) {
	return townsrc_learn(s->rom, s->rom ? s->src_group : s->group, s->rom ? s->src_number : s->number, s->mat_of);
}

bool town_style_take(const TownStyle *s, const TownBook *book, uint32_t *desc, uint32_t *coord_slot) {
	townsrc_slots(book, desc, coord_slot);
	/* (another game's town: in its BN6 map, its tiles and colours copied in;
	 * a BN6 town's own put back) */
	if (s->rom && !area_src_slots(s->group, s->number, desc, coord_slot)) return false;
	return xtown_gfx(*desc, s->rom, s->src_group, s->src_number);
}

int town_style_song(const TownStyle *s) { return s->rom ? xsong_install(s->rom - 1, s->song, 0x06) : s->song; }

int town_style_folk(const TownStyle *s, void *text, int i, int face) { return s->rom ? oran_script(text, i, face) : -1; }
