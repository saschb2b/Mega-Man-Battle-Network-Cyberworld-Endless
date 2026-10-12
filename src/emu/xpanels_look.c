/* xpanels_look.h: BN5's lava and sea drawn in BN6's battles.
 *
 * Their tiles are BN5's own (its panel tile set, BN5_PANEL_TILESET),
 * written into VRAM after BN6's 498 panel tiles, at tiles 0x295 on, which
 * no battle draws from (every battle background of BN6's and BN5's copied
 * ones takes its tiles below 0x51; docs/ROM_DATA.md), by the panels' draw
 * on a battle's first frame with them; a battle without them loads and
 * runs as it did, BN6's tile set its own (a larger set, decompressed by
 * the game, would cost every battle's start its cycles). BN6's tile table
 * grows to 15 types, lava's and the sea's entries BN5's (its tiles moved
 * there). Their colours: every BG palette is in use, so they stand where
 * BN5 gives them, in the panel palettes it shares with BN6 (lava with
 * poison in 2 and 6, the sea with ice in 3 and 7), each pixel's colour
 * moved to BN6's room for it: lava's five where BN6 keeps its volcano's,
 * the sea's four where it keeps the conveyors'; the sea's glints are
 * BN5's ice colours, there BN6's ice colours. Each frame a battle shows
 * lava (and no volcano), or sea (and no conveyor), those colours are
 * written over BN6's, by BN5's animations, after BN6's palettes have
 * stepped. Their frames and colours come from the player's BN5 ROM as the
 * core starts. */
#include "xpanels_look.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define KIND_TILES 90                     /* lava's, and the sea's: 15 a panel, its three rows on its two sides */
#define TYPES      (XPANEL_SEA + 1)       /* the grown table's types */
#define KIND_TILE(i) (BN6_TILE_PANELS + BN6_TILES_N + (unsigned)(i) * KIND_TILES)   /* lava's first tile, the sea's */
#define KIND_VRAM(i) (BN6_VRAM_PANELS + (uint32_t)(BN6_TILES_N + (i) * KIND_TILES) * TILE)
#define LZ77_HEAD(n) (0x10u | (uint32_t)(n) << 8)   /* an LZ77 stream's first word: its type, its length */
#define FRAMES_MAX 16                     /* an animation's frames */
#define COLOURS    5                      /* lava's colours, then the sea's four */

/* A kind of BN5's panels as BN6 draws it */
typedef struct {
	int bn5, bn6;              /* its type in each game */
	int pals[2];               /* its palettes, the player's side's and the enemies' (BN5's numbers, BN6's the same) */
	int from, to, n;           /* its own colours: BN5's [from, from + n) where BN6 keeps [to, to + n) */
	int other_lo, other_hi;    /* BN6's types those colours are theirs (shown where none is on the field) */
	uint8_t remap[16];         /* each pixel's colour as BN6's (0xFF: none of its tiles' own) */
	/* from BN5's ROM */
	uint16_t first;            /* its first tile in BN5's set */
	int frames;
	uint16_t delay[FRAMES_MAX];
	uint16_t colour[2][FRAMES_MAX][COLOURS];
	uint32_t period;
} Kind;

static Kind kinds[2] = {
	{ BN5_PANEL_LAVA, XPANEL_LAVA, { 2, 6 }, 6, 7, 5, BN6_PANEL_VOLCANO, BN6_PANEL_VOLCANO,
		{ 0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 0, 0, { 0 }, { { { 0 } } }, 0 },
	{ BN5_PANEL_SEA, XPANEL_SEA, { 3, 7 }, 11, 12, 4, BN6_PANEL_CONVEYOR, BN6_PANEL_CONVEYOR_END,
		{ 0, 1, 2, 3, 4, 5, 0xFF, 7, 8, 9, 0xFF, 12, 13, 14, 15, 0xFF }, 0, 0, { 0 }, { { { 0 } } }, 0 },
};

static struct {
	uint8_t table[TYPES * ENTRIES * ENTRY];
	uint8_t tiles[2][KIND_TILES * TILE];   /* lava's and the sea's, their colours BN6's */
	bool staged;
} L;

/* ---- the tiles and the table ---- */

/* a kind's entries in BN5's table, its tiles BN5's [first, first + KIND_TILES), no other: its first */
static bool kind_tiles(const uint8_t *bn5, Kind *k) {
	const uint8_t *e = bn5 + (BN5_PANEL_TILES - BUS) + (size_t)k->bn5 * ENTRIES * ENTRY;
	unsigned lo = 0x3FF, hi = 0;
	for (int i = 0; i < ENTRIES * 15; ++i) {
		unsigned t = get16(e + (i / 15) * ENTRY + (i % 15) * 2) & 0x3FF;
		lo = t < lo ? t : lo;
		hi = t > hi ? t : hi;
	}
	k->first = (uint16_t)lo;
	return lo >= BN6_TILE_PANELS && hi - lo + 1 == KIND_TILES && hi - BN6_TILE_PANELS < BN5_TILES_N;
}

/* its tiles out of BN5's set, each pixel's colour moved to BN6's; false where one is none of the kind's */
static bool kind_pixels(const Kind *k, const uint8_t *bn5_set, uint8_t *out) {
	const uint8_t *in = bn5_set + (size_t)(k->first - BN6_TILE_PANELS) * TILE;
	for (int i = 0; i < KIND_TILES * TILE; ++i) {
		uint8_t lo = k->remap[in[i] & 15], hi = k->remap[in[i] >> 4];
		if (lo == 0xFF || hi == 0xFF) return false;
		out[i] = (uint8_t)(lo | hi << 4);
	}
	return true;
}

/* its entries in BN6's grown table: BN5's, each tile moved to `at` on */
static void kind_entries(const uint8_t *bn5, const Kind *k, unsigned at) {
	const uint8_t *in = bn5 + (BN5_PANEL_TILES - BUS) + (size_t)k->bn5 * ENTRIES * ENTRY;
	uint8_t *out = L.table + (size_t)k->bn6 * ENTRIES * ENTRY;
	memcpy(out, in, ENTRIES * ENTRY);
	for (int i = 0; i < ENTRIES * 15; ++i) {
		uint8_t *p = out + (i / 15) * ENTRY + (i % 15) * 2;
		uint16_t v = get16(p);
		put16(p, (uint16_t)((v & ~0x3FFu) | (at + (v & 0x3FFu) - k->first)));
	}
}

/* the two kinds' tiles out of BN5's set, BN6's table grown to them; and
 * BN6's set checked to end where theirs begin */
static bool tiles_stage(const uint8_t *bn5) {
	size_t n5 = 0;
	uint8_t *set5 = lz77_decompress(bn5 + (BN5_PANEL_TILESET - BUS), ROM_SIZE - (BN5_PANEL_TILESET - BUS), &n5);
	bool ok = set5 && n5 == BN5_TILES_N * TILE && rom_u32(BN6_PANEL_TILESET - BUS) == LZ77_HEAD(BN6_TILES_N * TILE);
	memcpy(L.table, R.data + (BN6_PANEL_TILES - BUS), BN6_PANEL_TYPES * ENTRIES * ENTRY);
	for (int i = 0; ok && i < 2; ++i) {
		ok = kind_tiles(bn5, &kinds[i]) && kind_pixels(&kinds[i], set5, L.tiles[i]);
		if (ok) kind_entries(bn5, &kinds[i], KIND_TILE(i));
	}
	free(set5);
	return ok;
}

/* A kind's tiles in VRAM where they are not: a battle's first frame with
 * it (till then the tiles there are the map's), a word of each tile looked
 * at, then all written, at no cost in the game's cycles */
static void kind_load(int i) {
	uint32_t at = KIND_VRAM(i);
	const uint8_t *t = L.tiles[i];
	int n = 0;
	while (n < KIND_TILES) {
		uint32_t w = (uint32_t)n * TILE + (uint32_t)(n % 8) * 4;   /* a row of each, in turn */
		if (hook_read32(at + w) != get32(t + w)) break;
		++n;
	}
	if (n == KIND_TILES) return;
	for (uint32_t b = 0; b < KIND_TILES * TILE; b += 4) hook_write32(at + b, get32(t + b));
}

/* ---- the colours ---- */

/* BN5's animation of panel palette `pal`: its frames' colours of the kind
 * and their delays; false where there is none or it is not as read */
static bool kind_anim(const uint8_t *bn5, Kind *k, int side) {
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
	bool on[2] = { false, false }, theirs[2] = { false, false };
	for (uint32_t y = 1; y <= 3; ++y)
		for (uint32_t x = 1; x <= 6; ++x) {
			int t = hook_read8(BN6_FIELD_PANELS + (y * 8 + x) * BN6_PANEL_SIZE + BN6_PANEL_TYPE);
			for (int i = 0; i < 2; ++i) {
				on[i] |= t == kinds[i].bn6;
				theirs[i] |= t >= kinds[i].other_lo && t <= kinds[i].other_hi;
			}
		}
	uint32_t clock = hook_read16(BN6_FRAME_COUNT);
	for (int i = 0; i < 2; ++i) {
		if (on[i]) kind_load(i);
		if (on[i] && !theirs[i]) kind_colours(&kinds[i], clock);
	}
	return HOOK_CONTINUE;
}

/* ---- staged, written, hooked ---- */

#define DRAWN_OP 0x4FB9   /* BN6_PANEL_DRAW_ANIMATED's ldr r7,[pc,#0x2e4] */

bool xpanels_look_stage(void) {
	const uint8_t *bn5 = XR[XROM_BN5_COLONEL_US].data;
	if (!bn5 || !R.data) return false;
	bool ok = rom_u32(BN6_PANEL_TILES_REF - BUS) == BN6_PANEL_TILES && emu_read32(BN6_PANEL_TILES_REF) == BN6_PANEL_TILES &&
		rom_u32(BN6_PANEL_TILESET_REF - BUS) == BN6_PANEL_TILESET && emu_read32(BN6_PANEL_TILESET_REF) == BN6_PANEL_TILESET &&
		rom_u16(BN6_PANEL_DRAW_ANIMATED - BUS) == DRAWN_OP && emu_read16(BN6_PANEL_DRAW_ANIMATED) == DRAWN_OP;
	ok = ok && tiles_stage(bn5);
	for (int i = 0; ok && i < 2; ++i) ok = kind_anim(bn5, &kinds[i], 0) && kind_anim(bn5, &kinds[i], 1);
	L.staged = ok;
	return ok;
}

void xpanels_look_write(void) {
	if (!L.staged) return;
	emu_write(XP_TILES, L.table, sizeof L.table);
	emu_write32(BN6_PANEL_TILES_REF, XP_TILES);
}

bool xpanels_look_hook(void) { return L.staged && emu_hook(BN6_PANEL_DRAW_ANIMATED, drawn, NULL); }
