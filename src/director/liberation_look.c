/* A dark layer's board in BN5's own look (liberation.h; docs/LIBERATION.md,
 * How the board looks and moves). Where the layer is drawn in BN5's
 * Liberation map (lib_layer_tiles: its floor learned from map 0x86:00, its
 * tile set and colours copied whole, so BN5's tile numbers and palette
 * banks hold), each panel's kind lies over it on BG2 as BN5 lays it: an
 * isometric diamond of 8 x 4 tile-map entries, one of BN5's twelve looks
 * (BN5_LIB_LOOKS), its tiles copied from BN5's ROM into VRAM where BN5's
 * board copies them (the look record, BN5_LIB_PTR_LOOK). Nothing of BN5's
 * is kept: all of it is read from the player's ROM as the layer begins.
 *
 * BN6 reads a map's tile map in place (0x02013A00 + 12, layer 0 then
 * layer 1) and streams it into a 32 x 32 screen map per BG in IWRAM as the
 * camera moves, copied to VRAM each frame (bn6_map.h): a look is written
 * into the RAM map, which what scrolls in later shows, and into BG2's
 * screen map where its tiles show now. A map entered again (after
 * a battle) is the ROM's tile map afresh: every panel's look is written
 * again. A panel freed dissolves as BN5's do: dark, paled, clean. */
#include "liberation.h"

#include <stdio.h>
#include <string.h>

#include "bn5_liberation.h"
#include "bn6_fields.h"
#include "bn6.h"
#include "bn6_map.h"
#include "debug.h"
#include "director_state.h"
#include "emu.h"
#include "liberation_state.h"
#include "netmap.h"
#include "rom.h"

#define GBA_BG2HOFS    0x04000018u
#define GBA_BG2VOFS    0x0400001Au
#define TILEMAP_RAM    (BN6_MAP_TILEMAP + BN6_MAP_TILEMAP_HEAD)
#define LOOKS          12
#define BLANK_TILE     864           /* the looks' empty tile, the overlay set's first (all 0: transparent) */
#define LIB_LOOK_DOWN  8             /* pixels a diamond lies below its panel's centre on the map (calibrated on captures) */
#define REFRESH_FRAMES 30            /* frames BG2's screen map is made again after the looks change */
#define HOLE_FRAMES    (BN5_LIB_PAL_HOLE - 0x20u)   /* palette 10's four frames, the capture's the second */
#define FADE_IN        27            /* frames a freed panel keeps its look as the board fades back in (BN5's, traced) */
#define DISSOLVE       (FADE_IN + BN5_LIB_BLINK * BN5_LIB_BLINKS)
enum { LOOK_CLEAN, LOOK_DARK, LOOK_PALED, LOOK_ITEM, LOOK_PLUS5, LOOK_BAR1, LOOK_BONUS = 9, LOOK_HOLE, LOOK_GRID };

static struct {
	bool on;
	const uint8_t *rom;
	uint32_t src, vram, size;      /* the overlay's tiles: BN5's ROM, VRAM, bytes */
	uint16_t look[LOOKS][32];
	int tw, th;
	uint8_t shown[MAP_H][MAP_W];   /* the look written at each panel, + 1 (0: none) */
	uint8_t dissolve[MAP_H][MAP_W];/* frames left of a freed panel's blink */
	uint8_t was[MAP_H][MAP_W];     /* ... and the look it had */
	uint16_t mark;                 /* the start panel's first entry as written: gone, the map was entered again */
	uint32_t mark_at;
	int refresh;                   /* frames BG2's screen map is still made from the RAM map */
} K;

static uint32_t rom32(uint32_t a) {
	uint32_t o = a - 0x08000000u;
	return o + 4 <= ROM_SIZE ? (uint32_t)K.rom[o] | (uint32_t)K.rom[o + 1] << 8 | (uint32_t)K.rom[o + 2] << 16 | (uint32_t)K.rom[o + 3] << 24 : 0;
}
static uint16_t rom16(uint32_t a) {
	uint32_t o = a - 0x08000000u;
	return o + 2 <= ROM_SIZE ? (uint16_t)(K.rom[o] | K.rom[o + 1] << 8) : 0;
}

bool lib_look_on(void) { return K.on && LB.on; }


/* The looks and their tiles, from mission 1's record (the same in all
 * nine): false where BN5's ROM is not there or not as mapped */
static bool read_looks(void) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	if (!x->data) return false;
	K.rom = x->data;
	uint32_t rec = rom32(BN5_LIB_MISSIONS + 0x0C + 4 * BN5_LIB_PTR_LOOK);
	K.src = rom32(rec);
	K.vram = rom32(rec + 4);
	K.size = rom32(rec + 8);
	if (K.vram != 0x06000000u + BLANK_TILE * 32 || K.size == 0 || K.size > 0x2000 || K.src < 0x08000000u || K.src - 0x08000000u + K.size > ROM_SIZE) return false;
	for (int l = 0; l < LOOKS; ++l) {
		uint32_t at = rom32(rec + 0x10 + 4 * (uint32_t)l);
		if (at < 0x08000000u) return false;
		for (int e = 0; e < 32; ++e) K.look[l][e] = rom16(at + 2 * (uint32_t)e);
	}
	return true;
}

void lib_look_begin(void) {
	memset(K.shown, 0, sizeof K.shown);
	memset(K.dissolve, 0, sizeof K.dissolve);
	K.mark_at = 0;
	K.on = layer.dark && LB.on && D.layer_tiles == lib_layer_tiles(-1) && read_looks();
	if (!K.on) return;
	netmap_last_tiles(&K.tw, &K.th);
	if (emu_debug_on()) fprintf(stderr, "liberation look: BN5's, tiles %08x -> %08x (%u bytes), map %dx%d\n", K.src, K.vram, K.size, K.tw, K.th);
}

/* The look panel (x, y) shows as the board stands, as BN5_LIB_LOOK_OF
 * picks it by the kind (-1 none: the start, its map's floor alone; the
 * Darkloid's ring a grey grid until the DarkHoles fall) */
static int look_of(int x, int y) {
	int k = LB.b.kind[y][x];
	/* (a freed panel: its old look while the board fades back in, then
	 * clean and old by turns, BN5's blink) */
	if (K.dissolve[y][x]) {
		int t = DISSOLVE - K.dissolve[y][x];
		return t < FADE_IN || (t - FADE_IN) / BN5_LIB_BLINK % 2 ? K.was[y][x] : LOOK_CLEAN;
	}
	if (x == LB.b.start_x && y == LB.b.start_y) return -1;
	switch (k) {
	case LIB_CLEAN: return LOOK_CLEAN;
	case LIB_DARK: return LOOK_DARK;
	case LIB_RING: case LIB_BOSS: return lib_holes_left(&LB.b) ? LOOK_GRID : LOOK_DARK;
	case LIB_ITEM: return LOOK_ITEM;
	case LIB_BONUS: return LOOK_BONUS;
	case LIB_HOLE: return LOOK_HOLE;
	default: return -1;
	}
}

/* Panel (x, y)'s diamond's first tile in the tile map: its box's top left,
 * 32 by 16 pixels off its centre (netmap's tile grid: tiles.c's tile_class) */
static void box_at(int x, int y, int *tx, int *ty) {
	int wx, wy;
	netmap_world(x, y, &wx, &wy);
	int px = wx + wy + 4 * K.tw - 32, py = (wy - wx) / 2 + 4 * K.th - 16 + LIB_LOOK_DOWN;
	*tx = px >> 3;
	*ty = py >> 3;
}

/* The map tiles BG2's ring holds now: from the picture's corner (the
 * scroll, which is the map's pixel there plus a constant: x + y + 256 and
 * (y - x) / 2 - 256 of the world at the picture's middle, measured; taken
 * near MegaMan's place, as it is mod 256), those in the picture and a tile
 * round it; and where the map's tile t sits in the ring, t + the offset */
static void ring_window(int *tx0, int *ty0, int *ox, int *oy) {
	int px = bn6_player_x(), py = bn6_player_y();
	int u = px + py + 4 * K.tw - 120, v = (py - px) / 2 + 4 * K.th - 80;
	int cu = (emu_read16(GBA_BG2HOFS) & 0x1FF) - 376 + 4 * K.tw, cv = (emu_read16(GBA_BG2VOFS) & 0x1FF) + 176 + 4 * K.th;
	u += ((cu - u) % 256 + 256 + 128) % 256 - 128;
	v += ((cv - v) % 256 + 256 + 128) % 256 - 128;
	*tx0 = (u >> 3) - 1;
	*ty0 = (v >> 3) - 1;
	*ox = ((376 - 4 * K.tw) >> 3) & 31;
	*oy = ((-176 - 4 * K.th) >> 3) & 31;
}

/* Look `l` at panel (x, y): its entries into the RAM map's layer 1 (BG2's
 * screen map takes them in refresh_screen) */
static void put_look(int x, int y, int l) {
	int bx, by;
	box_at(x, y, &bx, &by);
	for (int e = 0; e < 32; ++e) {
		uint16_t v = K.look[l][e];
		if ((v & 0x3FF) == BLANK_TILE) continue;
		int tx = bx + e % 8, ty = by + e / 8;
		if (tx < 0 || ty < 0 || tx >= K.tw || ty >= K.th) continue;
		emu_write(TILEMAP_RAM + 2u * (uint32_t)(K.tw * K.th + ty * K.tw + tx), &v, 2);
		if (!K.mark_at) { K.mark_at = TILEMAP_RAM + 2u * (uint32_t)(K.tw * K.th + ty * K.tw + tx); K.mark = v; }
	}
	K.shown[y][x] = (uint8_t)(l + 1);
	K.refresh = REFRESH_FRAMES;
}

/* BG2's screen map made the RAM map's layer 1 where the picture shows it
 * (and a tile round it), a while after the looks change: the camera read
 * afresh each frame, as BN6 may still be setting it after a map's entry */
static void refresh_screen(void) {
	if (!K.refresh) return;
	--K.refresh;
	int tx0, ty0, ox, oy;
	ring_window(&tx0, &ty0, &ox, &oy);
	static uint16_t row[32];
	for (int ty = ty0; ty < ty0 + 22; ++ty) {
		if (ty < 0 || ty >= K.th) continue;
		for (int k = 0; k < 32; ++k) {
			int tx = tx0 + k;
			row[(tx + ox) & 31] = tx >= 0 && tx < K.tw ? emu_read16(TILEMAP_RAM + 2u * (uint32_t)(K.tw * K.th + ty * K.tw + tx)) : 0;
		}
		emu_write(BN6_BG2_SCREEN + 64u * (uint32_t)((ty + oy) & 31), row, sizeof row);
	}
}

/* The overlay's tiles into VRAM, a word at a time (VRAM takes no bytes:
 * a byte written there lands in both halves of its halfword) */
static void copy_tiles(void) {
	for (uint32_t k = 0; k < K.size; k += 4) emu_write32(K.vram + k, rom32(K.src + k));
}

/* ... still there: a few of its words (a map entered again loads its own) */
static bool tiles_there(void) {
	for (uint32_t k = 32; k < K.size; k += 0x1F4) if (emu_read32(K.vram + (k & ~3u)) != rom32(K.src + (k & ~3u))) return false;
	return true;
}

static void bank_from(int bank, uint32_t from) { emu_write(BN6_BG_PALETTE + 0x20u * (uint32_t)bank, K.rom + (from - 0x08000000u), 0x20); }

/* Which of `n` frames shows at time `t` of a cycle they are held for
 * `held` frames each, in that order */
static int frame_at(int t, const int *held, int n) {
	for (int i = 0; i < n; ++i) { if (t < held[i]) return i; t -= held[i]; }
	return 0;
}

/* The board's palettes animated as BN5's (traced: docs/ROM_DATA.md), in
 * BN6's BG palette buffer, copied to the screen each frame: the dark
 * panels' (9) at rest 120 frames, then three frames down and up; the
 * DarkHoles' (10) four frames there and back; the barriers' glint (11)
 * every 132 frames; the BonusPanel's breath (12) there and back */
static void pulse(void) {
	static const int dark_held[6] = { 120, 16, 16, 32, 16, 16 }, dark_at[6] = { 3, 2, 1, 0, 1, 2 };
	bank_from(9, BN5_LIB_DARK_FRAMES + 0x20u * (uint32_t)dark_at[frame_at(D.frame % 216, dark_held, 6)]);
	static const int hole_held[6] = { BN5_LIB_HOLE_STEP, BN5_LIB_HOLE_STEP, BN5_LIB_HOLE_STEP, BN5_LIB_HOLE_STEP, BN5_LIB_HOLE_STEP, BN5_LIB_HOLE_STEP },
		hole_at[6] = { 0, 1, 2, 3, 2, 1 };
	bank_from(10, HOLE_FRAMES + 0x20u * (uint32_t)hole_at[frame_at(D.frame % (6 * BN5_LIB_HOLE_STEP), hole_held, 6)]);
	static const int glint_held[6] = { 120, 6, 2, 2, 1, 1 };
	bank_from(11, BN5_LIB_PULSE + 0x20u * (uint32_t)frame_at(D.frame % 132, glint_held, 6));
	static const int breath_held[10] = { 12, 12, 12, 12, 12, 8, 12, 12, 12, 12 }, breath_at[10] = { 0, 1, 2, 3, 4, 5, 4, 3, 2, 1 };
	bank_from(12, BN5_LIB_BREATH + 0x20u * (uint32_t)breath_at[frame_at(D.frame % 116, breath_held, 10)]);
}

/* The map's layer 1 and BG2 cleared, every look to be written again: on
 * BN5's board BG2 holds the looks alone (the Liberation map's own second
 * layer, learned with its floor, drew pieces of other colours over it) */
static void clear_layer(void) {
	static const uint32_t zero[64];
	uint32_t n = (uint32_t)(K.tw * K.th) * 2;
	for (uint32_t k = 0; k < n; k += sizeof zero) emu_write(TILEMAP_RAM + n + k, zero, n - k < sizeof zero ? n - k : sizeof zero);
	for (uint32_t k = 0; k < BN6_BG_SCREEN_SIZE; k += sizeof zero) emu_write(BN6_BG2_SCREEN + k, zero, sizeof zero);
	memset(K.shown, 0, sizeof K.shown);
	K.mark_at = 0;
}

void lib_look_freed(int x, int y) {
	if (!K.on || !K.shown[y][x]) return;
	K.was[y][x] = (uint8_t)(K.shown[y][x] - 1);
	K.dissolve[y][x] = DISSOLVE;
}

void lib_look_update(void) {
	if (!lib_look_on() || !on_map()) return;
	/* (a map entered again: its tile map and tiles as the ROM has them) */
	if (!tiles_there()) { copy_tiles(); K.mark_at = 0; }
	if (!K.mark_at || emu_read16(K.mark_at) != K.mark) clear_layer();
	pulse();
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (K.dissolve[y][x]) --K.dissolve[y][x];
			int l = look_of(x, y);
			if (l >= 0 && K.shown[y][x] != l + 1) put_look(x, y, l);
		}
	refresh_screen();
}
