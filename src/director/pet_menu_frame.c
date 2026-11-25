/* The start menu's picture (pet_menu.h): BN6's own frame of its PET menu
 * laid out again, pixel for pixel BN6's (docs/game/pet-menu.md). BN6 draws
 * its eight rows down the left (a button and its icon, row k from y
 * 24 + 16k), its HP, ZENNY and BUGFRAGS panel, PLACE and EXIT; each is
 * copied to its place in one grid of two columns, EXIT its last place,
 * over the field as it stood on the map, darkened as BN6's PET darkens it;
 * the panel cut into one strip the grid's width, PLACE's name onto the
 * band's right stripe. The grid, the strip and the screen's foot stand an
 * equal gap apart. Forms' button is Save's with Forms' word's tiles, the
 * engine's own (pet_options.c), in palette 15, its icon BN6's Status icon
 * (pet_menu_install); Colours' the same way, its icon MegaMan's. What BN6 lights (its cursor's row, EXIT's pulse) and
 * fades it keeps, BN6's cursor set to ours (pet_menu.c).
 *
 * The picture reads nothing of the game: what it needs (where the game
 * stands, BN6's menu's state, its tiles, palettes and icons) is read after
 * each of the game's frames (pet_menu_see), since a read as a frame is drawn
 * waits, on the core's own thread, for the frame begun after it, and the
 * drawing then stands behind the GBA where it ran beside it: every frame of
 * the game, the menu up or not (issue #897). */
#include "pet_menu.h"

#include <string.h>

#include "bn6.h"
#include "emu.h"
#include "pet_options.h"
#include "platform.h"

#define W 240
#define H 160
#define BAND PET_MENU_BAND          /* the band's rows, left as BN6 draws them */
#define ROW_SRC_Y 24               /* BN6's row k from y 24 + 16k (its button at 25 + 16k) */
#define ROW_PITCH 16
#define ICON_X 1                   /* a cell's icon, from the cell's left, and its button (BN6's x 15 and 34) */
#define BUTTON_X 20
#define CELL_W 91                  /* a cell's pixels: the icon's left to the button's last */
#define COL_GAP 20                 /* between the columns' cells */
#define COL_X ((W - 2 * CELL_W - COL_GAP) / 2 - ICON_X)   /* the left column's cells: the grid in the middle */
#define COL_STEP (CELL_W + COL_GAP)
#define CELL_TOP 2                 /* a cell's first row drawn, from its y (the button's top edge), */
#define GRID_H (PM_ROWS * ROW_PITCH - CELL_TOP)   /* ... and the grid's rows drawn, its last the last button's bottom edge */
#define BAND_DRAWN 15              /* the band's rows drawn, its last a line */
#define STRIP_H 26
#define GAP ((H - BAND_DRAWN - GRID_H - STRIP_H + 1) / 3)   /* the band, the grid, the strip and the screen's foot apart (the band's gap a row less where they don't divide) */
#define STRIP_Y (H - GAP - STRIP_H)
#define ROWS_Y (STRIP_Y - GAP - GRID_H - CELL_TOP)
#define STRIP_X0 (COL_X + ICON_X)  /* the strip the grid's width */
#define STRIP_X1 (STRIP_X0 + 2 * CELL_W + COL_GAP)
#define EXIT_SRC_X 130             /* EXIT's capsule in BN6's frame: 80 x 13, its letters from x 155 */
#define EXIT_SRC_Y 144
#define EXIT_SRC_W 80
#define EXIT_H 13
#define EXIT_BODY_L 6              /* ... a column of its plain body before the arrow, and after the letters */
#define EXIT_BODY_R 70
#define EXIT_LETTERS 25            /* its letters from its left */
#define LABEL_LETTERS 8            /* a label's letters from its button's left */
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
#define ICON_TILES 848             /* the rows' icons in BN6's OBJ tiles: row k's four from 848 + 4k (the UI sprites' slots 1-9, the ninth the Status icon) */
#define ICONS BN6_PET_ICONS_GROWN
#define ICON_UNLIT 13              /* their OBJ palettes: unlit, and the cursor's row's */
#define ICON_LIT 12
#define ICON_POP 8                 /* the cursor's row's icon steps right */
#define DARKER 16                  /* BN6's PET darkens the field this much a 5-bit channel (measured) */

_Static_assert(COL_X >= 0 && ROWS_Y + CELL_TOP > BAND_DRAWN, "the grid off the screen");

/* the stats: each label's rows and its box's in BN6's panel, and its box's
 * width in the strip (HP's BN6's, room for 1000/1000; BUGFRAGS its label's;
 * ZENNY the rest), 4 pixels apart */
#define STAT_GAP 4
#define STAT_X0 (STRIP_X0 + 2)
#define STAT_X1 (STRIP_X1 - 3)
#define HP_W 78
#define FRAGS_W 54
static const struct { int label_y, box_y, x, w; } stats[3] = {
	{ 26, 34, STAT_X0, HP_W },
	{ 50, 58, STAT_X0 + HP_W + STAT_GAP, STAT_X1 - STAT_X0 - HP_W - FRAGS_W - 2 * STAT_GAP },
	{ 74, 82, STAT_X1 - FRAGS_W, FRAGS_W },
};

/* What the picture reads of BN6's frame: its panel, PLACE's name and EXIT,
 * all within columns SRC_X on and rows SRC_Y on. That part alone is kept
 * (61 KB), where the whole frame was copied (150 KB) each frame the menu
 * stood: with the frame's first copy and the field's that was three whole
 * frames moved a frame, 11 ms of drawing on a New 3DS and 45 frames shown
 * a second (issue #897). */
#define SRC_X 128
#define SRC_Y 24
#define SRC_W (W - SRC_X)
#define SRC_H (H - SRC_Y)
static uint32_t src[SRC_W * SRC_H];
static uint32_t teal_now;          /* BN6's teal in this frame (its pixel at 0, 100) */
static bool taken;                 /* src and teal_now taken from the core's frame this draw (pet_menu_under) */

static uint32_t get(int x, int y) { return src[(y - SRC_Y) * SRC_W + (x - SRC_X)]; }
static void put(uint32_t *px, int x, int y, uint32_t c) {
	if (x >= 0 && y >= 0 && x < W && y < H) px[y * W + x] = c;
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
	uint32_t colours[16];   /* (the bank's sixteen once a tile, not a pixel's each time) */
	for (int k = 1; k < 16; ++k) colours[k] = rgb(k == l->ink ? l->ink_to : pal16(l->pal, l->bank * 16 + k));
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			int sx = l->flip ? 7 - x : x, k = t[y * 4 + sx / 2] >> (sx & 1 ? 4 : 0) & 15;
			if (k) put(px, dx + x, dy + y, colours[k]);
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

/* row k's icon as BN6 has it on the screen: its tiles (16 x 16), its height
 * on its row; the ninth, the Status icon, resting under the screen, Forms'
 * (pet_menu_install) */
typedef struct { int tile, dy; bool found; } Icon;

/* What the picture is drawn from, the game's as its last frame left it
 * (pet_menu_see): where it stands, and while the menu is up its pieces,
 * each entry's button (its map, its 18 tiles), the icons' tiles and both
 * palettes */
static struct {
	bool on_map, up;                 /* MegaMan free on the map; BN6's menu open */
	int state;                       /* BN6's menu's (ePETMenuData +0), while up */
	bool kit;                        /* the pieces below read */
	uint8_t pal[0x400];              /* the BG palettes, then the OBJ's */
	bool has[PM_ENTRIES];
	uint16_t map[PM_ENTRIES][2 * 9];
	uint8_t button[PM_ENTRIES][2 * 9][32];
	Icon icons[ICONS];
	uint8_t icon[ICONS][4][32];
} seen;
static bool map_seen, map_left;      /* on the map at a frame since the last picture; or at the frame before one (the frame the PET opens in) */

/* a button from its 9 x 2 map and tiles, its letters (colour `ink`) in `ink_to` */
static void button(uint32_t *px, int e, int ink, uint16_t ink_to, int dx, int dy) {
	for (int i = 0; i < 2 * 9; ++i) {
		uint16_t m = seen.map[e][i];
		TileLook l = { seen.pal, m >> 12, (m & 0x400) != 0, ink, ink_to };
		tile(px, seen.button[e][i], &l, dx + i % 9 * 8, dy + i / 9 * 8);
	}
}

static void read_icons(const uint8_t *oam) {
	for (int i = 0; i < 128; ++i) {
		const uint8_t *o = oam + 8 * i;
		int tile_no = (o[4] | o[5] << 8) & 0x3FF, y = o[0];
		int k = (tile_no - ICON_TILES) / 4;
		if (tile_no < ICON_TILES || k >= ICONS || (tile_no - ICON_TILES) % 4) continue;
		seen.icons[k] = (Icon){ tile_no, y - (ROW_SRC_Y + ROW_PITCH * k), true };
	}
	/* (the Status icon at MegaMan's row's height: the row BN6 lights for Forms) */
	seen.icons[BN6_PET_ICON_STATUS].dy = seen.icons[pet_menu_bn6_row(PM_MEGAMAN)].dy;
}

static void icon(uint32_t *px, int k, bool lit, int dx, int dy) {
	if (!seen.icons[k].found) return;
	int x = dx + (lit ? ICON_POP : 0), y = dy + seen.icons[k].dy;
	TileLook l = { seen.pal + 0x200, lit ? ICON_LIT : ICON_UNLIT, false, -1, 0 };
	for (int q = 0; q < 4; ++q) tile(px, seen.icon[k][q], &l, x + q % 2 * 8, y + q / 2 * 8);
}

/* the menu's pieces out of the game's video memory, as its frame left them */
static void read_kit(void) {
	const uint8_t *bg = emu_memory(BG_TILES, 0x8000u), *obj = emu_memory(OBJ_TILES, 0x8000u);
	const uint8_t *pal = emu_memory(BG_PALETTES, 0x400u), *oam = emu_memory(OAM, 0x400u);
	seen.kit = bg && obj && pal && oam;
	if (!seen.kit) return;
	if (!have_maps) read_maps();
	memcpy(seen.pal, pal, sizeof seen.pal);
	read_icons(oam);
	for (int e = 0; e < PM_ENTRIES; ++e) {
		int k = pet_menu_bn6_row(e);
		if (k >= 0) memcpy(seen.map[e], maps[k], sizeof seen.map[e]);
		seen.has[e] = k >= 0 || (e == PM_LOOKS ? pet_options_looks_map(seen.map[e]) : pet_options_forms_map(seen.map[e]));
		for (int i = 0; seen.has[e] && i < 2 * 9; ++i) {
			int n = seen.map[e][i] & 0x3FF;
			const uint8_t *own = pet_options_forms_tile(n);   /* (Forms' word's: the engine's, not BN6's) */
			memcpy(seen.button[e][i], own ? own : bg + (uint32_t)n * 32u, 32);
		}
	}
	for (int k = 0; k < ICONS; ++k)
		for (int q = 0; seen.icons[k].found && q < 4; ++q) memcpy(seen.icon[k][q], obj + (uint32_t)(seen.icons[k].tile + q) * 32u, 32);
}

static void rows(uint32_t *px) {
	const uint8_t *pal = seen.pal;
	/* (BN6's inks: its lit row's, and an unlit one's) */
	int cur = pet_menu_cursor(), lit_row = cur == PM_EXIT ? -1 : pet_menu_bn6_row(cur) < 0 ? pet_menu_bn6_row(PM_MEGAMAN) : pet_menu_bn6_row(cur);
	int dim_row = lit_row == 0 ? 1 : 0;
	uint16_t lit_ink = pal16(pal, 15 * 16 + BN6_PET_LABEL_INK + (lit_row < 0 ? 0 : lit_row)), dim_ink = pal16(pal, 15 * 16 + BN6_PET_LABEL_INK + dim_row);
	for (int e = 0; e < PM_ENTRIES; ++e) {
		int col, row, k = pet_menu_bn6_row(e);
		bool lit = e == cur;
		pet_menu_place(e, &col, &row);
		int dx = COL_X + col * COL_STEP, dy = ROWS_Y + row * ROW_PITCH;
		if (seen.has[e]) button(px, e, k >= 0 ? BN6_PET_LABEL_INK + k : BN6_PET_LABEL_INK_NEXT, lit ? lit_ink : dim_ink, dx + BUTTON_X, dy + 1);
		/* (Forms' icon BN6's Status icon; Colours' MegaMan's own, his colours being what it opens) */
		int ic = k >= 0 ? k : e == PM_FORMS && seen.icons[BN6_PET_ICON_STATUS].found ? BN6_PET_ICON_STATUS : pet_menu_bn6_row(PM_MEGAMAN);
		icon(px, ic, lit, dx + ICON_X, dy);
	}
}

/* the stats in one strip: one frame of BN6's panel's colours, each label's
 * letters on its green, each box its left end and its right end with the
 * digits */
static void strip(uint32_t *px) {
	uint32_t hi = get(136, 40), green = get(137, 40), dark = get(223, 40);
	/* (BN6's panel in its place, not sliding or fading in: its frame's light edge at both ends) */
	if (hi != get(222, 40) || hi == green || (hi >> 8 & 0xFF) < 0xC0) return;
	for (int y = STRIP_Y; y < STRIP_Y + STRIP_H; ++y)
		for (int x = STRIP_X0; x < STRIP_X1; ++x) {
			bool edge = y == STRIP_Y || y == STRIP_Y + STRIP_H - 1 || x == STRIP_X0 || x == STRIP_X1 - 2;
			put(px, x, y, x == STRIP_X1 - 1 ? dark : edge ? hi : green);
		}
	for (int i = 0; i < 3; ++i) {
		for (int y = 0; y < 8; ++y)
			for (int x = 0; x < stats[i].w; ++x) {
				uint32_t c = get(140 + x, stats[i].label_y + y);   /* (each label at its box's left, as BN6 has it) */
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

/* EXIT, BN6's capsule as BN6 draws it (lit or not), in the grid's last
 * place: its plain body repeated after its rounded left end and after its
 * letters, so it fills the cell from the icons' column to the buttons' right
 * edge, its letters in the labels' column */
static void exit_cell(uint32_t *px) {
	int col, row;
	pet_menu_place(PM_EXIT, &col, &row);
	int x0 = COL_X + col * COL_STEP + ICON_X, y0 = ROWS_Y + row * ROW_PITCH + CELL_TOP;
	int left = BUTTON_X - ICON_X + LABEL_LETTERS - EXIT_LETTERS, right = CELL_W - EXIT_SRC_W - left;
	for (int x = 0; x < CELL_W; ++x) {
		int sx = x <= EXIT_BODY_L ? x : x <= EXIT_BODY_L + left ? EXIT_BODY_L : x - left;
		if (sx > EXIT_BODY_R) sx = sx <= EXIT_BODY_R + right ? EXIT_BODY_R : sx - right;
		for (int y = 0; y < EXIT_H; ++y) put(px, x0 + x, y0 + y, get(EXIT_SRC_X + sx, EXIT_SRC_Y + y));
	}
}

/* The field as it stood on the map, BN6's own picture of it covered by its
 * menu's pieces where ours stand: kept from the pictures drawn while START
 * is down on the map (in the player's hand, or among the keys the game was
 * given), the last of them the one BN6's PET opens over, and good for the
 * FIELD_FRESH pictures of the map after it (START let go before BN6 has
 * opened its PET). No other picture of the map is kept: copied every other
 * frame, as it was, its 150 KB were the map's every frame's on the slow
 * systems, for a menu opened once in a while. A menu no START of the map
 * opened stands on BN6's teal. */
#define FIELD_FRESH 4
static uint32_t field[W * H];
static bool have_field, field_dark;
static int field_age;

static void keep_field(const uint32_t *px) {
	bool map = map_seen, left = map_left;
	map_seen = map_left = false;
	if ((map || left) && (btn_held(BTN_START) || pet_menu_start())) {
		memcpy(field, px, sizeof field);
		have_field = true;
		field_dark = false;
		field_age = 0;
	} else if (map && have_field && ++field_age > FIELD_FRESH) have_field = false;
}

bool pet_menu_field_kept(void) { return have_field; }

/* a colour darkened as BN6's PET darkens the field: DARKER off each 5-bit channel */
static uint32_t darker(uint32_t c) {
	uint32_t out = c & 0xFF000000u;
	for (int sh = 0; sh < 24; sh += 8) {
		int v = (int)(c >> sh & 0xFF) >> 3;
		v = v > DARKER ? v - DARKER : 0;
		out |= (uint32_t)(v << 3 | v >> 2) << sh;
	}
	return out;
}

/* BN6's menu's states it stands over: taking input, closing, an entry
 * chosen (fading to its screen), a screen given back; not while BN6's own
 * slides in */
static bool over(int state) { return state == 4 || state == 8 || state == 0x10 || state == 0x14; }

void pet_menu_see(void) {
	int mode = emu_read8(emu_read32(BN6_TOOLKIT)), sub = emu_read8(BN6_GAMESTATE);
	bool was_map = seen.on_map;
	seen.on_map = mode == BN6_MODE_GAME && sub == BN6_SUB_MAP;
	seen.up = mode == BN6_MODE_GAME && sub == BN6_SUB_PET && (emu_read8(BN6_PET_MENU_OPEN) & 1);
	seen.state = seen.up ? emu_read8(BN6_PET_MENU) : 0;
	/* (the frame START opens BN6's PET in is still the map's picture: the
	 * game draws a frame from what the one before it left) */
	if (seen.on_map) map_seen = true;
	else if (was_map) map_left = true;
	if (seen.up && over(seen.state)) read_kit();
}

/* Whether the menu's picture stands over the frame now: every row under
 * the band is then the kept field's and the menu's pieces', nothing of the
 * frame's own. Not on a frame the field may still be kept from. */
bool pet_menu_covers(void) { return seen.up && over(seen.state) && seen.kit && !map_seen && !map_left; }

void pet_menu_under(const uint32_t *v, uint32_t (*shade)(uint32_t c, int y)) {
	for (int y = 0; y < SRC_H; ++y) {
		const uint32_t *from = v + (y + SRC_Y) * W + SRC_X;
		uint32_t *to = src + y * SRC_W;
		if (!shade) memcpy(to, from, SRC_W * sizeof *to);
		else for (int x = 0; x < SRC_W; ++x) to[x] = shade(from[x], y + SRC_Y);
	}
	teal_now = shade ? shade(v[100 * W], 100) : v[100 * W];
	taken = true;
}

void pet_menu_frame(uint32_t *px) {
	bool had = taken;
	taken = false;
	keep_field(px);
	if (!seen.up || !over(seen.state) || !seen.kit) return;
	/* (the frame whole in `px`, where pet_menu_under was not asked: its part read here) */
	if (!had) {
		for (int y = 0; y < SRC_H; ++y) memcpy(src + y * SRC_W, px + (y + SRC_Y) * W + SRC_X, SRC_W * sizeof *src);
		teal_now = px[100 * W];
	}
	/* (BN6's teal as it stands taking input, where no field was kept) */
	static uint32_t teal;
	if (seen.state == 4 || !teal) teal = teal_now;
	/* (the field darkened once a menu, not once a frame) */
	if (have_field && !field_dark) {
		for (int i = BAND * W; i < W * H; ++i) field[i] = darker(field[i]);
		field_dark = true;
	}
	if (have_field) memcpy(px + BAND * W, field + BAND * W, sizeof field - BAND * W * sizeof *field);
	else for (int i = BAND * W; i < W * H; ++i) px[i] = teal;
	rows(px);
	strip(px);
	place(px);
	exit_cell(px);
}
