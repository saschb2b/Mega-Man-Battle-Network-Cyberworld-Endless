/* xpanels_look.h: BN5's lava and sea and BN4's Hole drawn in BN6's battles.
 *
 * Their tiles are their games' own (each game's panel tile set), written
 * into VRAM after BN6's 498 panel tiles, at tiles 0x295 on (lava, the sea,
 * then the Hole), which no battle draws from (every battle background of
 * BN6's and BN5's copied ones takes its tiles below 0x51; docs/rom/), by
 * the panels' draw on a battle's first frame with them; a battle without
 * them loads and runs as it did, BN6's tile set its own (a larger set,
 * decompressed by the game, would cost every battle's start its cycles).
 * BN6's tile table grows to 16 types, each kind's entries its game's (its
 * tiles moved there), a type no ROM read gives BN6's type 0's. Their
 * colours: every BG palette is in use, so they stand where their games
 * give them, in the panel palettes those share with BN6 (lava with poison
 * in 2 and 6, the sea with ice in 3 and 7, the Hole with plain panels in
 * 1 and 5), each pixel's colour moved to BN6's room for it: lava's five
 * where BN6 keeps its volcano's, the sea's four where it keeps the
 * conveyors', the Hole's three dark ones where it keeps grass's; the
 * sea's glints are BN5's ice colours, there BN6's ice colours, the Hole's
 * rims BN4's plain panel's, there BN6's. Each frame a battle shows lava
 * (and no volcano), sea (and no conveyor) or a Hole (and no grass), those
 * colours are written over BN6's, by their games' animations, after BN6's
 * palettes have stepped. Their frames and colours come from the player's
 * ROMs as the core starts. */
#include "xpanels_look.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn4.h"
#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "lz.h"
#include "rom.h"
#include "xpanels.h"

#define BUS        0x08000000u
#define TILE       32                     /* a tile's bytes (4 bits a pixel) */
#define ENTRY      32                     /* a tile table entry's: 5x3 BG map entries, then two bytes */
#define ENTRIES    6                      /* a type's entries: the two sides' three rows */
#define BN6_TILES_N 498                   /* BN6's panel tiles (BN6_PANEL_TILESET) */
#define BN5_TILES_N 842                   /* BN5's (BN5_PANEL_TILESET) */
#define TYPES      16                     /* the grown table's types */
#define TILES_MAX  90                     /* a kind's tiles at most: 15 a panel, its three rows on its two sides */
#define LZ77_HEAD(n) (0x10u | (uint32_t)(n) << 8)   /* an LZ77 stream's first word: its type, its length */
#define FRAMES_MAX 16                     /* an animation's frames */
#define COLOURS    5                      /* a kind's own colours at most */

/* A kind of another game's panels as BN6 draws it */
typedef struct {
	int game;                  /* its game (XRomId) */
	int src, bn6;              /* its type in its game and in BN6 */
	uint32_t table, set;       /* its game's tile table and tile set */
	int set_n;                 /* ... the set's tiles */
	int pals[2];               /* its palettes, the player's side's and the enemies' (its game's numbers, BN6's the same) */
	int from, to, n;           /* its own colours: its game's [from, from + n) where BN6 keeps [to, to + n) */
	int other_lo, other_hi;    /* BN6's types those colours are theirs (shown where none is on the field) */
	int ntiles;                /* its tiles */
	uint8_t remap[16];         /* each pixel's colour as BN6's (0xFF: none of its tiles' own) */
	/* from its ROM */
	uint16_t first;            /* its first tile in its game's set */
	int frames;
	uint16_t delay[FRAMES_MAX];
	uint16_t colour[2][FRAMES_MAX][COLOURS];
	uint32_t period;
	bool staged;
} Kind;

enum { KIND_LAVA, KIND_SEA, KIND_HOLE, KINDS };

static Kind kinds[KINDS] = {
	{ XROM_BN5_COLONEL_US, BN5_PANEL_LAVA, XPANEL_LAVA, BN5_PANEL_TILES, BN5_PANEL_TILESET, BN5_TILES_N, { 2, 6 }, 6, 7, 5,
		BN6_PANEL_VOLCANO, BN6_PANEL_VOLCANO, 90, { 0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
		0, 0, { 0 }, { { { 0 } } }, 0, false },
	{ XROM_BN5_COLONEL_US, BN5_PANEL_SEA, XPANEL_SEA, BN5_PANEL_TILES, BN5_PANEL_TILESET, BN5_TILES_N, { 3, 7 }, 11, 12, 4,
		BN6_PANEL_CONVEYOR, BN6_PANEL_CONVEYOR_END, 90, { 0, 1, 2, 3, 4, 5, 0xFF, 7, 8, 9, 0xFF, 12, 13, 14, 15, 0xFF },
		0, 0, { 0 }, { { { 0 } } }, 0, false },
	/* (BN4's 1-5 its plain panel's rim, BN6's 1-5; its 6-10 the face, BN6's 7-11; its 13-15 the hole's dark) */
	{ XROM_BN4_RED_SUN_US, BN4_PANEL_HOLE, XPANEL_HOLE, BN4_PANEL_TILES, BN4_PANEL_TILESET, BN4_PANEL_TILES_N, { 1, 5 }, 13, 13, 3,
		BN6_PANEL_GRASS, BN6_PANEL_GRASS, 54, { 0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 0xFF, 0xFF, 13, 14, 15 },
		0, 0, { 0 }, { { { 0 } } }, 0, false },
};

_Static_assert(TYPES * ENTRIES * ENTRY == XP_TILES_SIZE && XP_TILES + XP_TILES_SIZE <= XP_END, "the grown tile table past its room");
_Static_assert(XPANEL_HOLE < TYPES, "the Hole past the grown table");

static struct {
	uint8_t table[TYPES * ENTRIES * ENTRY];
	uint8_t tiles[KINDS][TILES_MAX * TILE];   /* each kind's, their colours BN6's */
	bool based, written, hooked;
} L;

/* a kind's first tile in BN6's VRAM: after BN6's, each kind after the one before */
static unsigned kind_tile(int i) {
	unsigned t = BN6_TILE_PANELS + BN6_TILES_N;
	for (int k = 0; k < i; ++k) t += (unsigned)kinds[k].ntiles;
	return t;
}
static uint32_t kind_vram(int i) { return BN6_VRAM_PANELS + (uint32_t)(kind_tile(i) - BN6_TILE_PANELS) * TILE; }

/* ---- the tiles and the table ---- */

/* a kind's entries in its game's table, its tiles its game's [first, first + ntiles), no other: its first */
static bool kind_tiles(const uint8_t *rom, Kind *k) {
	const uint8_t *e = rom + (k->table - BUS) + (size_t)k->src * ENTRIES * ENTRY;
	unsigned lo = 0x3FF, hi = 0;
	for (int i = 0; i < ENTRIES * 15; ++i) {
		unsigned t = get16(e + (i / 15) * ENTRY + (i % 15) * 2) & 0x3FF;
		lo = t < lo ? t : lo;
		hi = t > hi ? t : hi;
	}
	k->first = (uint16_t)lo;
	return lo >= BN6_TILE_PANELS && hi - lo + 1 == (unsigned)k->ntiles && hi - BN6_TILE_PANELS < (unsigned)k->set_n;
}

/* its tiles out of its game's set, each pixel's colour moved to BN6's; false where one is none of the kind's */
static bool kind_pixels(const Kind *k, const uint8_t *set, uint8_t *out) {
	const uint8_t *in = set + (size_t)(k->first - BN6_TILE_PANELS) * TILE;
	for (int i = 0; i < k->ntiles * TILE; ++i) {
		uint8_t lo = k->remap[in[i] & 15], hi = k->remap[in[i] >> 4];
		if (lo == 0xFF || hi == 0xFF) return false;
		out[i] = (uint8_t)(lo | hi << 4);
	}
	return true;
}

/* its entries in BN6's grown table: its game's, each tile moved to `at` on */
static void kind_entries(const uint8_t *rom, const Kind *k, unsigned at) {
	const uint8_t *in = rom + (k->table - BUS) + (size_t)k->src * ENTRIES * ENTRY;
	uint8_t *out = L.table + (size_t)k->bn6 * ENTRIES * ENTRY;
	memcpy(out, in, ENTRIES * ENTRY);
	for (int i = 0; i < ENTRIES * 15; ++i) {
		uint8_t *p = out + (i / 15) * ENTRY + (i % 15) * 2;
		uint16_t v = get16(p);
		put16(p, (uint16_t)((v & ~0x3FFu) | (at + (v & 0x3FFu) - k->first)));
	}
}

/* BN6's own table, the types past its own given its type 0's, once */
static void table_base(void) {
	if (L.based) return;
	memcpy(L.table, R.data + (BN6_PANEL_TILES - BUS), BN6_PANEL_TYPES * ENTRIES * ENTRY);
	for (int t = BN6_PANEL_TYPES; t < TYPES; ++t) memcpy(L.table + (size_t)t * ENTRIES * ENTRY, L.table, ENTRIES * ENTRY);
	L.based = true;
}

/* a game's kinds' tiles out of its set, BN6's table grown to them; and
 * BN6's set checked to end where theirs begin */
static bool tiles_stage(int game, const uint8_t *rom) {
	uint32_t set_at = 0;
	int set_n = 0;
	for (int i = 0; i < KINDS; ++i)
		if (kinds[i].game == game) set_at = kinds[i].set, set_n = kinds[i].set_n;
	size_t n = 0;
	uint8_t *set = lz77_decompress(rom + (set_at - BUS), ROM_SIZE - (set_at - BUS), &n);
	bool ok = set && n == (size_t)set_n * TILE && rom_u32(BN6_PANEL_TILESET - BUS) == LZ77_HEAD(BN6_TILES_N * TILE);
	Kind *done[KINDS];
	int ndone = 0;
	for (int i = 0; ok && i < KINDS; ++i) {
		if (kinds[i].game != game) continue;
		ok = kind_tiles(rom, &kinds[i]) && kind_pixels(&kinds[i], set, L.tiles[i]);
		if (ok) done[ndone++] = &kinds[i];
	}
	free(set);
	if (!ok) return false;
	table_base();
	for (int i = 0; i < ndone; ++i) kind_entries(rom, done[i], kind_tile((int)(done[i] - kinds)));
	return true;
}

/* A kind's tiles in VRAM where they are not: a battle's first frame with
 * it (till then the tiles there are the map's), a word of each tile looked
 * at, then all written, at no cost in the game's cycles */
static void kind_load(int i) {
	uint32_t at = kind_vram(i);
	const uint8_t *t = L.tiles[i];
	int nt = kinds[i].ntiles, n = 0;
	while (n < nt) {
		uint32_t w = (uint32_t)n * TILE + (uint32_t)(n % 8) * 4;   /* a row of each, in turn */
		if (hook_read32(at + w) != get32(t + w)) break;
		++n;
	}
	if (n == nt) return;
	for (uint32_t b = 0; b < (uint32_t)nt * TILE; b += 4) hook_write32(at + b, get32(t + b));
}

/* ---- the colours ---- */

/* BN5's animation of panel palette `pal`: its frames' colours of the kind
 * and their delays; false where there is none or it is not as read */
static bool kind_anim_bn5(const uint8_t *bn5, Kind *k, int side) {
	for (uint32_t a = BN5_PANEL_ANIMS; a < BN5_PANEL_ANIMS + 4 * 8; a += 4) {
		uint32_t s = get32(bn5 + (a - BUS));
		if (!rom_is_ptr(s)) return false;
		const uint8_t *h = bn5 + (s - BUS);
		if (get32(h + 4) != BN5_BG_PALETTE + 0x20u * (uint32_t)k->pals[side]) continue;
		int n = h[1];
		if (n < 1 || n > FRAMES_MAX || (side && n != k->frames)) return false;
		k->frames = n;
		k->period = 0;
		for (int f = 0; f < n; ++f) {
			uint32_t src = get32(h + 8 + 8 * f), delay = get32(h + 12 + 8 * f);
			if (!rom_is_ptr(src) || !delay || delay > 0x400 || (side && delay != k->delay[f])) return false;
			k->delay[f] = (uint16_t)delay;
			k->period += delay;
			for (int c = 0; c < k->n; ++c) k->colour[side][f][c] = get16(bn5 + (src - BUS) + 2 * (k->from + c));
		}
		return true;
	}
	return false;
}

/* BN4's: a list of its frames' palettes, each shown BN4_PANEL_ANIM_DELAY
 * frames, after the two literals its step reads, the palette's place in
 * RAM and the list's address */
static bool kind_anim_bn4(const uint8_t *bn4, Kind *k, int side) {
	uint32_t list = side ? BN4_PANEL_ANIM_5 : BN4_PANEL_ANIM_1;
	if (get32(bn4 + (list - 8 - BUS)) != BN4_BG_PALETTE_RAM + 0x20u * (uint32_t)k->pals[side] || get32(bn4 + (list - 4 - BUS)) != list) return false;
	k->frames = BN4_PANEL_ANIM_FRAMES;
	k->period = BN4_PANEL_ANIM_FRAMES * BN4_PANEL_ANIM_DELAY;
	for (int f = 0; f < BN4_PANEL_ANIM_FRAMES; ++f) {
		uint32_t src = get32(bn4 + (list - BUS) + 4u * (uint32_t)f);
		if (!rom_is_ptr(src)) return false;
		k->delay[f] = BN4_PANEL_ANIM_DELAY;
		for (int c = 0; c < k->n; ++c) k->colour[side][f][c] = get16(bn4 + (src - BUS) + 2 * (k->from + c));
	}
	return true;
}

/* a kind's colours at `clock`, by its animation, over BN6's */
static void kind_colours(const Kind *k, uint32_t clock) {
	uint32_t t = clock % k->period;
	int f = 0;
	while (f < k->frames - 1 && t >= k->delay[f]) t -= k->delay[f++];
	for (int side = 0; side < 2; ++side)
		for (int c = 0; c < k->n; ++c)
			hook_write16(BN6_BG_PALETTE + 0x20u * (uint32_t)k->pals[side] + 2u * (uint32_t)(k->to + c), k->colour[side][f][c]);
}

/* After BN6's palettes have stepped, each frame of a battle: the kinds on
 * the field, their tiles where they are not, their colours where none of
 * BN6's types whose colours theirs take is */
static HookAct drawn(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	bool on[KINDS] = { false }, theirs[KINDS] = { false };
	for (uint32_t y = 1; y <= 3; ++y)
		for (uint32_t x = 1; x <= 6; ++x) {
			int t = hook_read8(BN6_FIELD_PANELS + (y * 8 + x) * BN6_PANEL_SIZE + BN6_PANEL_TYPE);
			for (int i = 0; i < KINDS; ++i) {
				on[i] |= kinds[i].staged && t == kinds[i].bn6;
				theirs[i] |= t >= kinds[i].other_lo && t <= kinds[i].other_hi;
			}
		}
	uint32_t clock = hook_read16(BN6_FRAME_COUNT);
	for (int i = 0; i < KINDS; ++i) {
		if (on[i]) kind_load(i);
		if (on[i] && !theirs[i]) kind_colours(&kinds[i], clock);
	}
	return HOOK_CONTINUE;
}

/* ---- staged, written, hooked ---- */

#define DRAWN_OP 0x4FB9   /* BN6_PANEL_DRAW_ANIMATED's ldr r7,[pc,#0x2e4] */

bool xpanels_look_stage(int xrom) {
	const uint8_t *rom = xrom >= 0 && xrom < XROM_COUNT ? XR[xrom].data : NULL;
	if (!rom || !R.data) return false;
	uint32_t ref = emu_read32(BN6_PANEL_TILES_REF);
	/* (the draw's instruction BN6's, or already hooked by an earlier game's kinds) */
	bool ok = rom_u32(BN6_PANEL_TILES_REF - BUS) == BN6_PANEL_TILES && (ref == BN6_PANEL_TILES || (L.written && ref == XP_TILES)) &&
		rom_u32(BN6_PANEL_TILESET_REF - BUS) == BN6_PANEL_TILESET && emu_read32(BN6_PANEL_TILESET_REF) == BN6_PANEL_TILESET &&
		rom_u16(BN6_PANEL_DRAW_ANIMATED - BUS) == DRAWN_OP && (L.hooked || emu_read16(BN6_PANEL_DRAW_ANIMATED) == DRAWN_OP);
	ok = ok && tiles_stage(xrom, rom);
	for (int i = 0; ok && i < KINDS; ++i) {
		if (kinds[i].game != xrom) continue;
		ok = xrom == XROM_BN4_RED_SUN_US ? kind_anim_bn4(rom, &kinds[i], 0) && kind_anim_bn4(rom, &kinds[i], 1)
			: kind_anim_bn5(rom, &kinds[i], 0) && kind_anim_bn5(rom, &kinds[i], 1);
	}
	for (int i = 0; i < KINDS; ++i)
		if (kinds[i].game == xrom) kinds[i].staged = ok;
	return ok;
}

void xpanels_look_write(void) {
	if (!L.based) return;
	emu_write(XP_TILES, L.table, sizeof L.table);
	emu_write32(BN6_PANEL_TILES_REF, XP_TILES);
	L.written = true;
}

bool xpanels_look_hook(void) {
	if (!L.hooked && L.written) L.hooked = emu_hook(BN6_PANEL_DRAW_ANIMATED, drawn, NULL);
	return L.hooked;
}
