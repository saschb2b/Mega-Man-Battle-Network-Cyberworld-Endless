/* The start menu's picture (pet_menu.h): BN6's own frame of its PET menu
 * laid out again, pixel for pixel BN6's (docs/game/pet-menu.md). BN6 draws
 * its eight rows down the left (a button and its icon, row k from y
 * 24 + 16k), its HP, ZENNY and BUGFRAGS panel, PLACE and EXIT; each is
 * copied to its place in two columns of six over BN6's teal, the panel cut
 * into one strip, PLACE's name onto the band's right stripe, EXIT under
 * one line. Forms' button is Save's with Forms' label tiles of the labels'
 * copy (pet_options.c) in palette 15, its icon MegaMan's. What BN6 lights
 * (its cursor's row, EXIT's pulse) and fades it keeps, BN6's cursor set to
 * ours (pet_menu.c); a row BN6 lights for Forms' sake is taken from its
 * last unlit frame. */
#include "pet_menu.h"

#include <string.h>

#include "bn6.h"
#include "emu.h"
#include "pet_options.h"

#define W 240
#define H 160
#define BAND 16                    /* the band's rows, left as BN6 draws them */
#define RIGHT 224                  /* the field shows past here, as past BN6's panel */
#define ROW_SRC_Y 24               /* BN6's row k from y 24 + 16k (its button at 25 + 16k) */
#define ROW_PITCH 16
#define COL_X 14                   /* the left column's cells at x 14, the right's 112 on */
#define COL_STEP 112
#define ROWS_Y 16                  /* row 0's cell at y 16: its button at 17 */
#define BUTTON_X 20                /* the button in a cell, from its left (BN6's x 34 to the icon's 15) */
#define STRIP_Y 113
#define STRIP_X0 4
#define STRIP_X1 224
#define FOOT_Y 140
#define EXIT_SRC_X 128             /* EXIT's capsule in BN6's frame */
#define EXIT_SRC_Y 143
#define EXIT_W 84
#define EXIT_H 14
#define EXIT_X 140
#define EXIT_Y 144
#define NAME_SRC_X 142             /* PLACE's name in BN6's frame */
#define NAME_SRC_Y 119
#define NAME_W 82
#define NAME_H 15
#define NAME_X 134                 /* ... on the band's right stripe */
#define NAME_Y 1
#define BG_TILES 0x06008000u       /* BG character block 2: the labels' tiles (BN6_PET_LABEL_FIRST on) */
#define BG_PALETTES 0x05000000u
#define OBJ_TILES 0x06010000u
#define OAM 0x07000000u
#define ICON_TILES 848             /* the rows' icons in BN6's OBJ tiles: row k's four from 848 + 4k (the UI sprites' slots 1-8) */
#define ICON_UNLIT 13              /* their OBJ palettes: unlit, and the cursor's row's */
#define ICON_LIT 12
#define ICON_POP 8                 /* the cursor's row's icon steps right */

/* the stats: each label's rows, its box's, and its cell in the strip */
static const struct { int label_y, box_y, x, w; } stats[3] = {
	{ 26, 34, 10, 70 }, { 50, 58, 88, 60 }, { 74, 82, 156, 62 },
};

static uint32_t src[W * H];

static uint32_t get(int x, int y) { return src[y * W + x]; }
static void put(uint32_t *px, int x, int y, uint32_t c) {
	if (x >= 0 && y >= 0 && x < W && y < H) px[y * W + x] = c;
}
static void copy(uint32_t *px, int sx, int sy, int w, int h, int dx, int dy) {
	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x) put(px, dx + x, dy + y, get(sx + x, sy + y));
}

/* a BGR555 colour as the core's picture holds it (R in the low byte) */
static uint32_t rgb(uint16_t c) {
	uint32_t r = c & 31, g = c >> 5 & 31, b = c >> 10 & 31;
	return 0xFF000000u | (b << 3 | b >> 2) << 16 | (g << 3 | g >> 2) << 8 | (r << 3 | r >> 2);
}

static uint16_t pal16(const uint8_t *pal, int i) { return (uint16_t)(pal[2 * i] | pal[2 * i + 1] << 8); }

/* how a tile is drawn: its palette (bank of `pal`), flipped across or not, colour `ink` (-1 none) in `ink_to` */
typedef struct { const uint8_t *pal; int bank; bool flip; int ink; uint16_t ink_to; } TileLook;

/* a 4bpp tile at (dx, dy), colour 0 clear */
static void tile(uint32_t *px, const uint8_t *t, const TileLook *l, int dx, int dy) {
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			int sx = l->flip ? 7 - x : x, k = t[y * 4 + sx / 2] >> (sx & 1 ? 4 : 0) & 15;
			if (k) put(px, dx + x, dy + y, rgb(k == l->ink ? l->ink_to : pal16(l->pal, l->bank * 16 + k)));
		}
}

/* the rows' maps, BN6's labels' table read once from the core's copy (Comm's made Options by pet_options.c) */
static uint16_t maps[8][2 * 9];
static bool have_maps;

static void read_maps(void) {
	for (int k = 0; k < 8; ++k) {
		uint32_t m = emu_read32(BN6_PET_LABEL_MAPS + 4u * (uint32_t)k);
		for (int i = 0; i < 2 * 9; ++i) maps[k][i] = emu_read16(m + 2u * (uint32_t)i);
	}
	have_maps = true;
}

/* a button from its 9 x 2 map, its letters (colour `ink`) in `ink_to` */
static void button(uint32_t *px, const uint16_t *map, int ink, uint16_t ink_to, int dx, int dy) {
	const uint8_t *vram = emu_memory(BG_TILES, 0x8000u), *pal = emu_memory(BG_PALETTES, 0x200u);
	if (!vram || !pal) return;
	for (int i = 0; i < 2 * 9; ++i) {
		uint16_t e = map[i];
		TileLook l = { pal, e >> 12, (e & 0x400) != 0, ink, ink_to };
		tile(px, vram + (e & 0x3FF) * 32u, &l, dx + i % 9 * 8, dy + i / 9 * 8);
	}
}

/* row k's icon as BN6 has it on the screen: its tiles (16 x 16), its height on its row */
typedef struct { int tile, dy; bool found; } Icon;
static Icon icons[8];

static void read_icons(void) {
	const uint8_t *oam = emu_memory(OAM, 0x400u);
	if (!oam) return;
	for (int i = 0; i < 128; ++i) {
		const uint8_t *o = oam + 8 * i;
		int tile_no = (o[4] | o[5] << 8) & 0x3FF, y = o[0];
		int k = (tile_no - ICON_TILES) / 4;
		if (tile_no < ICON_TILES || k >= 8 || (tile_no - ICON_TILES) % 4) continue;
		icons[k] = (Icon){ tile_no, y - (ROW_SRC_Y + ROW_PITCH * k), true };
	}
}

static void icon(uint32_t *px, int k, bool lit, int dx, int dy) {
	const uint8_t *vram = emu_memory(OBJ_TILES, 0x8000u), *pal = emu_memory(BG_PALETTES + 0x200u, 0x200u);
	if (!vram || !pal || !icons[k].found) return;
	int x = dx + (lit ? ICON_POP : 0), y = dy + icons[k].dy;
	TileLook l = { pal, lit ? ICON_LIT : ICON_UNLIT, false, -1, 0 };
	for (int q = 0; q < 4; ++q) tile(px, vram + (uint32_t)(icons[k].tile + q) * 32u, &l, x + q % 2 * 8, y + q / 2 * 8);
}

static void rows(uint32_t *px) {
	if (!have_maps) read_maps();
	read_icons();
	const uint8_t *pal = emu_memory(BG_PALETTES, 0x200u);
	uint16_t forms[2 * 9];
	if (!pal) return;
	/* (BN6's inks: its lit row's, and an unlit one's) */
	int cur = pet_menu_cursor(), lit_row = cur == PM_EXIT ? -1 : pet_menu_bn6_row(cur) < 0 ? pet_menu_bn6_row(PM_MEGAMAN) : pet_menu_bn6_row(cur);
	int dim_row = lit_row == 0 ? 1 : 0;
	uint16_t lit_ink = pal16(pal, 15 * 16 + BN6_PET_LABEL_INK + (lit_row < 0 ? 0 : lit_row)), dim_ink = pal16(pal, 15 * 16 + BN6_PET_LABEL_INK + dim_row);
	for (int e = 0; e < PM_ENTRIES; ++e) {
		int col, row, k = pet_menu_bn6_row(e);
		bool lit = e == cur;
		pet_menu_place(e, &col, &row);
		int dx = COL_X + col * COL_STEP, dy = ROWS_Y + row * ROW_PITCH;
		if (k >= 0) button(px, maps[k], BN6_PET_LABEL_INK + k, lit ? lit_ink : dim_ink, dx + BUTTON_X, dy + 1);
		else if (pet_options_forms_map(forms)) button(px, forms, BN6_PET_LABEL_INK_NEXT, lit ? lit_ink : dim_ink, dx + BUTTON_X, dy + 1);
		icon(px, k >= 0 ? k : pet_menu_bn6_row(PM_MEGAMAN), lit, dx + 1, dy);
	}
}

/* the stats in one strip: one frame of BN6's panel's colours, each label's
 * letters on its green, each box its left end and its right end with the
 * digits */
static void strip(uint32_t *px) {
	uint32_t hi = get(136, 40), green = get(137, 40), dark = get(223, 40);
	/* (BN6's panel in its place, not sliding or fading in: its frame's light edge at both ends) */
	if (hi != get(222, 40) || hi == green || (hi >> 8 & 0xFF) < 0xC0) return;
	for (int y = STRIP_Y; y < STRIP_Y + 26; ++y)
		for (int x = STRIP_X0; x < STRIP_X1; ++x) {
			bool edge = y == STRIP_Y || y == STRIP_Y + 25 || x == STRIP_X0 || x == STRIP_X1 - 2;
			put(px, x, y, x == STRIP_X1 - 1 ? dark : edge ? hi : green);
		}
	for (int i = 0; i < 3; ++i) {
		for (int y = 0; y < 8; ++y)
			for (int x = 0; x < stats[i].w; ++x) {
				uint32_t c = get(137 + x, stats[i].label_y + y);
				if (c != green && c != hi) put(px, stats[i].x + x, STRIP_Y + 2 + y, c);
			}
		for (int y = 0; y < 16; ++y)
			for (int x = 0; x < stats[i].w; ++x) put(px, stats[i].x + x, STRIP_Y + 9 + y, get(x < 3 ? 140 + x : 218 - (stats[i].w - 1 - x), stats[i].box_y + y));
	}
}

/* PLACE's name on the band's stripe: its letters in the band's navy (the
 * letters' shade in BN6's frame), its background and shade left out */
static void place(uint32_t *px) {
	uint32_t bg = get(NAME_SRC_X, NAME_SRC_Y), white = 0, navy = 0;
	for (int y = 0; y < NAME_H; ++y)
		for (int x = 0; x < NAME_W; ++x) {
			uint32_t c = get(NAME_SRC_X + x, NAME_SRC_Y + y);
			if (c == bg) continue;
			if (!white || (c & 0xFF) > (white & 0xFF)) white = c;
		}
	for (int y = 0; y < NAME_H && !navy; ++y)
		for (int x = 0; x < NAME_W; ++x) {
			uint32_t c = get(NAME_SRC_X + x, NAME_SRC_Y + y);
			if (c != bg && c != white) { navy = c; break; }
		}
	if (!white || !navy) return;
	for (int y = 0; y < NAME_H; ++y)
		for (int x = 0; x < NAME_W; ++x)
			if (get(NAME_SRC_X + x, NAME_SRC_Y + y) == white) put(px, NAME_X + x, NAME_Y + y, navy);
}

static void foot(uint32_t *px) {
	uint32_t line[3] = { get(60, 155), get(60, 156), get(60, 157) };
	for (int x = 8; x < RIGHT; ++x)
		for (int k = 0; k < 3; ++k) put(px, x, FOOT_Y + k, line[k]);
	copy(px, EXIT_SRC_X, EXIT_SRC_Y, EXIT_W, EXIT_H, EXIT_X, EXIT_Y);
}

/* BN6's menu's states it stands over: taking input, closing, an entry
 * chosen (fading to its screen), a screen given back; not while BN6's own
 * slides in */
static bool over(int state) { return state == 4 || state == 8 || state == 0x10 || state == 0x14; }

void pet_menu_frame(uint32_t *px) {
	if (!pet_menu_up() || !over(emu_read8(BN6_PET_MENU))) return;
	memcpy(src, px, sizeof src);
	/* (BN6's teal as it stands taking input: its column slides away as it closes) */
	static uint32_t teal;
	if (emu_read8(BN6_PET_MENU) == 4 || !teal) teal = get(0, 100);
	for (int y = BAND; y < H; ++y)
		for (int x = 0; x < RIGHT; ++x) px[y * W + x] = teal;
	rows(px);
	strip(px);
	place(px);
	foot(px);
}
