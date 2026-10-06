/* The title screen. The logo is the original's (docs/ROM_DATA.md), taken
 * from its 256-colour picture without the 6, the CYBEAST GREGAR plate and
 * the Cybeast; in the 6's place the engine draws an infinity mark in the
 * 6's colours, and under BATTLE NETWORK the subtitle CYBERWORLD ENDLESS in
 * the plate's. Behind it the battle backgrounds of the areas a run passes
 * through take turns, scrolling and animated as in battle. PRESS START,
 * NEW GAME / CONTINUE, the cursor and the copyright line are the game's,
 * and NEW GAME over a saved run asks in the game's chat box. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "title_lines.h"
#include "touch.h"
#include "audio.h"
#include "controls.h"
#include "backdrop.h"
#include "chatbox.h"
#include "cinema.h"
#include "game.h"
#include "gfx.h"
#include "minifont.h"
#include "platform.h"
#include "rom.h"
#include "run.h"
#include "guardians.h"
#include "meta.h"
#include "powers.h"
#include "save.h"
#include "second.h"
#include "text.h"

#include "version.h"   /* CW_VERSION, from the build (Makefile) */

#define T (R.layout->title)
#define BLINK 32         /* PRESS START: 32 frames on, 32 off */
#define MENU_AFTER 40    /* frames from START to the menu */
#define LEAVE_WHITE 24   /* jacking in: the net rushes by into white... */
#define LEAVE_FRAMES 40  /* ...which fades to black */
#define SUMMARY_MIN 60   /* frames before the summary can be closed */
#define SUMMARY_FACE_X 30   /* Lan's face on it */
#define SUMMARY_FACE_Y 66
#define SHOW_FRAMES 600  /* each backdrop's turn */
#define SWAP_FRAMES 12   /* mosaic out and in between them */
#define ASK_FIRST "Start over? We'd lose"   /* the question's first line */

#define LOGO_UP 8        /* the logo stands higher than in the original picture */
#define LOGO_BOTTOM 95   /* below: the CYBEAST GREGAR plate */
#define LOGO_END 0x50    /* banks 0-4 draw the logo, 5-6 the blue, 7-13 the Cybeast */

/* The areas of a run, in the order of their acts (the BIOME_* numbers):
 * Central, Seaside, Sky, Green, Graveyard, Undernet, Secret Area, Nest. */
static const uint8_t cycle[] = { 0x09, 0x0B, 0x04, 0x0D, 0x14, 0x0F, 0x13, 0x15 };
#define NCYCLE (int)(sizeof cycle / sizeof *cycle)

bool title_summary;
bool title_setup;
uint32_t title_seed;
char title_cause[48];
char title_learned[24];
bool title_new_best;
bool title_won;

static struct {
	int t, pressed, menu, leaving, cursor, choice;
	bool has_save;
	int saved_depth;      /* the saved run's layer, 0 when unknown */
	bool summary;         /* the finished run's summary over its area */
	int next;             /* the cycle's next backdrop */
	int shown_at;         /* frame the backdrop came in */
	bool first;           /* the first backdrop fades in instead */
	int jack_v, jack_y;   /* jacking in: the backdrop's speed and offset, 1/16 pixel */
	int lift;             /* the backdrop's own darkening: a bright one more (the Sky's pale clouds took the logo's white) */
	bool confirm, yes;    /* NEW GAME over a saved run asks first; Yes chosen */
	/* the setup after NEW GAME (docs/META.md): its row, the choices, the
	 * helper the cursor is on */
	bool setup;
	int row, net, folder, cross, threat, helpers, helper;
	int asked;            /* the frame it asked: its text types out from there */
	char ask[32];         /* its second line, the saved run's layer */
	uint16_t new_marks;   /* marks the run just over earned: they blink in after its summary */
	SDL_Texture *tex;
} S;

/* the "BN5 found" note (found_draw): from the title's frame FOUND_AT, on
 * its first entry only, so a run's end does not say it again */
#define FOUND_AT 30
#define FOUND_LEN 150
static int entries;

static Backdrop bd;
static uint8_t logo[160][256];   /* palette indices of the logo, 0 none */
static uint32_t logo_col[256];
static bool logo_ready;

/* ------------------------------------------------------------------ */
/* The logo */

/* Where the 6 starts on each row of the picture: all of it above MEGAMAN,
 * then along the italic N it leans against, then after BATTLE NETWORK's K. */
static int six_left(int y) {
	if (y < 44) return 160;
	if (y < 54) return 177 - (y - 44) / 2;
	if (y < 82) return 171 - (y - 54) / 3;
	return 175;
}

/* the dark rim the 6 has around its outline */
static bool rim(uint8_t v) { return v == 0x01 || v == 0x02 || (v >= 0x16 && v <= 0x1C); }

#define OUTLINE 0x13   /* black */

/* Puts a mask (1 fill, 2 outline) into the logo at (x0, y0): its fill in
 * graded rows (or a light top edge with `bevel`), a black outline and rings. */
static void stamp(const uint8_t *m, int w, int h, int x0, int y0, const uint8_t *fill, int nfill,
                  uint8_t bevel, const uint8_t *ring, int nring) {
#define AT(r, c) ((r) >= 0 && (r) < h && (c) >= 0 && (c) < w ? m[(r) * w + (c)] : 0)
	int reach = nring + 1;
	for (int r = -reach; r < h + reach; ++r)
		for (int c = -reach; c < w + reach; ++c) {
			int X = x0 + c, Y = y0 + r;
			if (X < 0 || X >= 256 || Y < 0 || Y >= 160) continue;
			uint8_t v = AT(r, c);
			if (v == 2) { logo[Y][X] = OUTLINE; continue; }
			if (v == 1) {
				logo[Y][X] = bevel && !AT(r - 1, c) ? bevel : fill[r * nfill / h];
				continue;
			}
			int d = reach + 1;
			for (int dr = -reach; dr <= reach; ++dr)
				for (int dc = -reach; dc <= reach; ++dc) {
					int k = abs(dr) > abs(dc) ? abs(dr) : abs(dc);
					if (k < d && AT(r + dr, c + dc)) d = k;
				}
			if (d == 1) logo[Y][X] = OUTLINE;
			else if (d - 2 < nring) logo[Y][X] = ring[d - 2];
		}
#undef AT
}

/* CYBERWORLD ENDLESS in bold capitals, leaning like the logo. */
static const struct { char ch; const char *rows[9]; } glyphs[] = {
	{ 'C', { ".#####.", "##...##", "##.....", "##.....", "##.....", "##.....", "##.....", "##...##", ".#####." } },
	{ 'Y', { "##...##", "##...##", "##...##", ".##.##.", "..###..", "..###..", "..###..", "..###..", "..###.." } },
	{ 'B', { "######.", "##...##", "##...##", "##...##", "######.", "##...##", "##...##", "##...##", "######." } },
	{ 'E', { "#######", "##.....", "##.....", "##.....", "######.", "##.....", "##.....", "##.....", "#######" } },
	{ 'R', { "######.", "##...##", "##...##", "##...##", "######.", "##.##..", "##..##.", "##...##", "##...##" } },
	{ 'W', { "##...##", "##...##", "##...##", "##.#.##", "##.#.##", "##.#.##", "##.#.##", "#######", ".##.##." } },
	{ 'O', { ".#####.", "##...##", "##...##", "##...##", "##...##", "##...##", "##...##", "##...##", ".#####." } },
	{ 'L', { "##.....", "##.....", "##.....", "##.....", "##.....", "##.....", "##.....", "##.....", "#######" } },
	{ 'D', { "######.", "##...##", "##...##", "##...##", "##...##", "##...##", "##...##", "##...##", "######." } },
	{ 'N', { "##...##", "###..##", "####.##", "##.####", "##..###", "##...##", "##...##", "##...##", "##...##" } },
	{ 'S', { ".######", "##.....", "##.....", "##.....", ".#####.", ".....##", ".....##", ".....##", "######." } },
};
#define GLYPH_H 9
#define GLYPH_ADV 8
#define SPACE_ADV 5
#define LEAN 3          /* rows per pixel of slant */

static void build_subtitle(void) {
	const char *s = "CYBERWORLD ENDLESS";
	int w = (GLYPH_H - 1) / LEAN + 2;
	for (const char *p = s; *p; ++p) w += *p == ' ' ? SPACE_ADV : GLYPH_ADV;
	uint8_t *m = calloc((size_t)w * GLYPH_H, 1);
	if (!m) return;
	int x = 0;
	for (const char *p = s; *p; ++p) {
		if (*p == ' ') { x += SPACE_ADV; continue; }
		for (size_t g = 0; g < sizeof glyphs / sizeof *glyphs; ++g) {
			if (glyphs[g].ch != *p) continue;
			for (int r = 0; r < GLYPH_H; ++r)
				for (int k = 0; glyphs[g].rows[r][k]; ++k)
					if (glyphs[g].rows[r][k] == '#') m[r * w + x + k + (GLYPH_H - 1 - r) / LEAN] = 1;
		}
		x += GLYPH_ADV;
	}
	/* the plate's colours: green, white in the middle, green (its rows'
	 * palette indices), a black outline, a dark and a grey rim */
	static const uint8_t fill[] = { 0x05, 0x04, 0x35, 0x2A, 0x2D, 0x2D, 0x2C, 0x24, 0x07 };
	static const uint8_t ring[] = { 0x18, 0x27 };
	stamp(m, w, GLYPH_H, 200 - w, 100, fill, (int)sizeof fill, 0, ring, 2);
	free(m);
}

/* The infinity mark: a lemniscate of Bernoulli drawn as a thick stroke,
 * slanted like the logo, the stroke from the upper right to the lower left
 * crossing over the other one; the size of MEGAMAN's letters beside it (at
 * the 6's size it stood over them and outweighed the name). */
static void build_infinity(void) {
	enum { W = 96, H = 52, N = 360 };
	const double A = 23.8, B = 40.8, RAD = 3.6, SLANT = 0.42;
	static double seg[2][N + 1][2];
	for (int s = 0; s < 2; ++s)
		for (int i = 0; i <= N; ++i) {
			double t = M_PI * (s + (double)i / N), k = 1 + sin(t) * sin(t);
			seg[s][i][0] = A * cos(t) / k;
			seg[s][i][1] = B * sin(t) * cos(t) / k;
		}
	static uint8_t m[H][W];
	memset(m, 0, sizeof m);
	for (int r = 0; r < H; ++r)
		for (int c = 0; c < W; ++c) {
			double py = r - H / 2, px = c - W / 2 + py * SLANT, d[2] = { 1e9, 1e9 };
			for (int s = 0; s < 2; ++s)
				for (int i = 0; i <= N; ++i) {
					double dx = px - seg[s][i][0], dy = py - seg[s][i][1], q = dx * dx + dy * dy;
					if (q < d[s]) d[s] = q;
				}
			if (d[0] <= RAD * RAD) m[r][c] = 1;
			else if (d[1] <= RAD * RAD) m[r][c] = d[0] <= (RAD + 1.2) * (RAD + 1.2) && fabs(px) < 2 * RAD + 2 ? 2 : 1;
		}
	/* rows of the 6's colours from top to bottom (palette indices) */
	static const uint8_t fill[] = { 0x3E, 0x3E, 0x3D, 0x3D, 0x3D, 0x3C, 0x41, 0x42, 0x45, 0x45,
	                                0x44, 0x44, 0x39, 0x33, 0x31, 0x30, 0x30, 0x2F, 0x2F, 0x2E };
	static const uint8_t ring[] = { 0x18, 0x02 };
	int top = H, bottom = 0;
	for (int r = 0; r < H; ++r)
		for (int c = 0; c < W; ++c)
			if (m[r][c]) { if (r < top) top = r; bottom = r; }
	if (top > bottom) return;
	stamp(&m[top][0], W, bottom - top + 1, 201 - W / 2, 62 - (bottom - top + 1) / 2, fill, (int)sizeof fill,
	      0x46, ring, 2);
}

static bool build_logo(void) {
	size_t n = 0;
	uint8_t *tiles = lz77_decompress(R.data + T.bg_tiles, ROM_SIZE - T.bg_tiles, &n);
	if (!tiles) return false;
	memset(logo, 0, sizeof logo);
	for (int ty = 0; ty < 20; ++ty)
		for (int tx = 0; tx < 32; ++tx) {
			uint16_t e = rom_u16(T.bg_map + (uint32_t)(ty * 32 + tx) * 2);
			size_t at = 4 + (size_t)(e & 0x3FF) * 64;
			if (at + 64 > n) continue;
			for (int y = 0; y < 8; ++y)
				for (int x = 0; x < 8; ++x) {
					int X = tx * 8 + ((e & 0x400) ? 7 - x : x), Y = ty * 8 + ((e & 0x800) ? 7 - y : y);
					uint8_t v = tiles[at + y * 8 + x];
					if (v < LOGO_END && Y <= LOGO_BOTTOM && X < six_left(Y)) logo[Y][X] = v;
				}
		}
	free(tiles);
	/* the 6's rim where it leant against the N */
	for (int y = 44; y < 82; ++y)
		for (int x = six_left(y) - 1; x > 0 && rim(logo[y][x]); --x) logo[y][x] = 0;
	build_subtitle();
	build_infinity();
	for (int i = 0; i < 256; ++i) logo_col[i] = bgr555(i < 0xE0 ? rom_u16(T.bg_pal + (uint32_t)i * 2) : 0);
	return true;
}

/* ------------------------------------------------------------------ */

/* How bright a backdrop is undimmed: the mean of its channels, 0-31. */
static int brightness(void) {
	static uint32_t px[240 * 160];
	backdrop_draw(&bd, 0, 0, 0, NULL, 1, px);
	long sum = 0;
	for (int i = 0; i < 240 * 160; ++i) sum += ((px[i] >> 16 & 255) + (px[i] >> 8 & 255) + (px[i] & 255)) / 3;
	return (int)(sum / (240 * 160) * 31 / 255);
}

static void show(int id) {
	backdrop_load(&bd, id);
	S.shown_at = S.t;
	int b = brightness();
	S.lift = b > 10 ? b - 10 : 0;
}

static void show_next(void) {
	show(cycle[S.next]);
	S.next = (S.next + 1) % NCYCLE;
}

/* Starts the cycle at an area: its own place in it, or its backdrop first. */
static void show_area(int b) {
	if (b >= 0 && b < NCYCLE) { S.next = b; show_next(); }
	else { show(biome_backdrop(b)); S.next = 0; }
}

static void setup_open(void);

static void enter(void) {
	SDL_Texture *tex = S.tex;
	memset(&S, 0, sizeof S);
	S.tex = tex;
	++entries;
	if (!logo_ready) logo_ready = build_logo();
	S.has_save = save_exists();
	Run saved = { 0 };
	if (S.has_save && peek_run(&saved)) S.saved_depth = saved.depth;
	S.cursor = S.has_save ? 1 : 0; /* CONTINUE when there is one */
	S.summary = title_summary;
	title_summary = false;
	S.first = true;
	/* (the second screen: the PET at rest, issue #79) */
	platform_second_screen(second_title_draw, second_title_changed);
	platform_second_screen_soon();
	if (S.summary) show_area(run.biome);
	else if (S.saved_depth) show_area(saved.biome);
	else show_next();
	/* (a won run's summary to BN6's staff roll theme, the title's after:
	 * the Nest's fall went from the exit pad to the title's own tune) */
	audio_music(S.summary && title_won ? MUS_CREDITS : MUS_TITLE);
	if (title_setup) {
		title_setup = false;
		S.menu = 1;
		setup_open();
	}
}

static void jack_in(void);

/* SELECT, on the title and its menu: the controls screen (issue #37; the
 * 3DS's buttons are the console's own); with L held, the developer's
 * sprite gallery, which SELECT alone opened before. R: the ROMs' screen,
 * the launcher, to add BN5 after the first start (issue #97), where the
 * build has one. True when it took the press. */
static bool select_pressed(void) {
	if (btn_pressed(BTN_R) && launcher_open()) {
		audio_sfx(SFX_SELECT);
		return true;
	}
	if (!btn_pressed(BTN_SELECT)) return false;
#ifndef __3DS__
	if (!btn_held(BTN_L)) {
		controls_open();
		audio_sfx(SFX_SELECT);
		return true;
	}
#endif
	scene_set(&scene_gallery);
	return true;
}

/* PRESS START: START or A (after a moment), or SELECT */
static void press_start(void) {
	if (S.t > 20 && (btn_pressed(BTN_START) || btn_pressed(BTN_A))) { S.pressed = S.t; audio_sfx(SFX_SELECT); }
	select_pressed();
}

/* (a first run starts as the net comes: nothing to choose yet. A run
 * given up for a NEW GAME counts as none, but what it opened is there: a
 * first run deleted ElecMan, and the setup with his Cross never came) */
static bool setup_wanted(void) { return profile.runs > 0 || meta_setup_has_choice(); }

/* ---- the title's marks (docs/META.md): BN6's own sprites, where its title
 * draws them, loaded with the copyright line (tiles from OBJ tile 0x180,
 * palettes from OBJ bank 5: docs/ROM_DATA.md) ---- */

static const struct { int bit, x, y, tile, w, h, bank; } mark_parts[] = {
	{ MARK_WIN, 4, 2, 0x280, 4, 2, 9 }, { MARK_WIN, 4, 18, 0x288, 2, 1, 9 },
	{ MARK_STD, 44, 4, 0x220, 4, 2, 8 }, { MARK_MEGA, 84, 4, 0x230, 4, 2, 8 }, { MARK_GIGA, 124, 4, 0x240, 4, 2, 8 },
	{ MARK_PA, 164, 4, 0x250, 4, 2, 8 }, { MARK_THREAT, 204, 4, 0x260, 4, 2, 8 },
	{ MARK_SECRET, 142, 20, 0x210, 4, 2, 7 }, { MARK_SECRET, 142, 36, 0x218, 2, 1, 7 },
	{ MARK_NEST, 64, 20, 0x200, 4, 2, 7 }, { MARK_NEST, 64, 36, 0x208, 2, 1, 7 },
};

#define MARK_ARRIVE 96   /* frames a new mark blinks, 4 on and 4 off */

static void marks_draw(int x0, int y0) {
	uint32_t copy = gfx_lz_ref(T.copy_tiles) + 4;
	for (unsigned i = 0; i < sizeof mark_parts / sizeof *mark_parts; ++i) {
		if (!(profile.marks & mark_parts[i].bit)) continue;
		if ((S.new_marks & mark_parts[i].bit) && S.t < MARK_ARRIVE && (S.t / 4) % 2) continue;
		rom_tiles(copy + (uint32_t)(mark_parts[i].tile - 0x180) * 32, T.copy_pal + (uint32_t)(mark_parts[i].bank - 5) * 32,
			x0 + mark_parts[i].x, y0 + mark_parts[i].y, mark_parts[i].w, mark_parts[i].h, 0);
	}
}

/* ---- the setup after NEW GAME (docs/META.md): the net's length, the
 * starting folder, the threat rung, the helpers; the last run's choices
 * to start from ---- */

enum { ROW_NET, ROW_FOLDER, ROW_CROSS, ROW_THREAT, ROW_HELPERS, ROW_GO, ROWS };

/* the rows the last summary's unlocks are on (profile.setup_new) */
static int row_new(int row) {
	int bit = row == ROW_NET ? SETUP_NEW_NET : row == ROW_FOLDER ? SETUP_NEW_FOLDER : row == ROW_CROSS ? SETUP_NEW_CROSS
		: row == ROW_THREAT ? SETUP_NEW_THREAT : 0;
	return profile.setup_new & bit;
}

static void setup_open(void) {
	S.setup = true;
	S.row = ROW_GO;
	meta_unlocks_unsaid();
	/* (on the row of what the last run opened, as a new unlock is spent
	 * here: a playtester's A went through a setup that showed none of his
	 * new SlashCross start, the cursor on JACK IN!; else JACK IN!, for the
	 * run again as before) */
	for (int r = ROW_GO - 1; r >= 0; --r) if (row_new(r)) S.row = r;
	S.net = profile.last_net == RUN_ENDLESS && meta_endless_open() ? RUN_ENDLESS : RUN_SHORT;
	S.folder = meta_folder_open(profile.last_folder) ? profile.last_folder : FOLDER_STANDARD;
	S.cross = meta_cross_open(profile.last_cross) ? profile.last_cross : 0;
	S.threat = profile.last_threat <= meta_threat_open() ? profile.last_threat : meta_threat_open();
	S.helpers = profile.last_helpers & ((1 << HELPERS) - 1);
	S.helper = 0;
}

static void setup_update(void) {
	if (btn_pressed(BTN_B)) { S.setup = false; audio_sfx(SFX_CANCEL); return; }
	if (btn_repeat(BTN_UP)) { S.row = (S.row + ROWS - 1) % ROWS; audio_sfx(SFX_CURSOR); }
	if (btn_repeat(BTN_DOWN)) { S.row = (S.row + 1) % ROWS; audio_sfx(SFX_CURSOR); }
	int d = btn_repeat(BTN_RIGHT) ? 1 : btn_repeat(BTN_LEFT) ? -1 : 0;
	if (d) {
		int was = S.row == ROW_NET ? S.net : S.row == ROW_FOLDER ? S.folder : S.row == ROW_CROSS ? S.cross : S.row == ROW_THREAT ? S.threat : S.helper;
		if (S.row == ROW_NET && meta_endless_open()) S.net = S.net == RUN_SHORT ? RUN_ENDLESS : RUN_SHORT;
		if (S.row == ROW_FOLDER)   /* (the open ones only) */
			for (int k = 1; k < FOLDER_COUNT; ++k) {
				int f = (S.folder + d * k + FOLDER_COUNT * k) % FOLDER_COUNT;
				if (meta_folder_open(f)) { S.folder = f; break; }
			}
		if (S.row == ROW_CROSS)   /* (none, or an open one) */
			for (int k = 1; k < 6; ++k) {
				int c = (S.cross + d * k + 6 * k) % 6;
				if (!c || meta_cross_open(c)) { S.cross = c; break; }
			}
		if (S.row == ROW_THREAT) S.threat = (S.threat + d + meta_threat_open() + 1) % (meta_threat_open() + 1);
		if (S.row == ROW_HELPERS) S.helper = (S.helper + d + HELPERS) % HELPERS;
		int now = S.row == ROW_NET ? S.net : S.row == ROW_FOLDER ? S.folder : S.row == ROW_CROSS ? S.cross : S.row == ROW_THREAT ? S.threat : S.helper;
		if (now != was) audio_sfx(SFX_CURSOR);
	}
	bool ok = btn_pressed(BTN_A) || btn_pressed(BTN_START);
	if (!ok) return;
	if (S.row == ROW_HELPERS && !btn_pressed(BTN_START)) { S.helpers ^= 1 << S.helper; audio_sfx(SFX_SELECT); return; }
	/* (A on another row, or START anywhere: jack in with these) */
	profile.last_net = (uint8_t)S.net;
	profile.last_folder = (uint8_t)S.folder;
	profile.last_cross = (uint8_t)S.cross;
	profile.last_threat = (uint8_t)S.threat;
	profile.last_helpers = (uint8_t)S.helpers;
	profile.setup_new = 0;
	profile_save();
	S.setup = false;
	jack_in();
}

/* the setup's note, at most two lines of 26 letters (the panel's width):
 * cut at a space, or where it says (\n) */
#define NOTE_CHARS 26
static int note_line(int x, int y, const char *s, SDL_Color c) {
	char a[40];
	size_t n = strlen(s);
	const char *nl = strchr(s, '\n');
	if (nl) {
		snprintf(a, sizeof a, "%.*s", (int)(nl - s), s);
		text_draw(x, y, a, c, TEXT_CENTER);
		text_draw(x, y + 12, nl + 1, c, TEXT_CENTER);
		return 2;
	}
	if (n <= NOTE_CHARS) { text_draw(x, y, s, c, TEXT_CENTER); return 1; }
	size_t cut = NOTE_CHARS;
	while (cut > 0 && s[cut] != ' ') --cut;
	if (!cut) cut = NOTE_CHARS;
	snprintf(a, sizeof a, "%.*s", (int)cut, s);
	text_draw(x, y, a, c, TEXT_CENTER);
	text_draw(x, y + 12, s + cut + (s[cut] == ' '), c, TEXT_CENTER);
	return 2;
}

/* a value the d-pad changes: the PET's small arrows either side of it */
static void choice_draw(int x, int y, const char *s, SDL_Color c, bool changes) {
	text_draw(x, y, s, c, TEXT_CENTER);
	if (!changes) return;
	int w = text_width(s) / 2, a = y + 2;
	SDL_Color arrow = rgba(255, 170, 40, 255);
	for (int i = 0; i < 4; ++i) {
		fill_rect(x - w - 10 + 3 - i, a + i, 1, 8 - 2 * i, arrow);   /* pointing left */
		fill_rect(x + w + 6 + i, a + i, 1, 8 - 2 * i, arrow);        /* pointing right */
	}
}

/* under the note: what is still shut, and how, dim */
static void note_more(int cx, int y, char lines[][48], int n) {
	for (int i = 0; i < n; ++i, y += 12) text_draw(cx, y, lines[i], rgba(120, 140, 170, 255), TEXT_CENTER);
}

/* the helpers, each on or off (green or dim) along the Help row, the one
 * the cursor is on underlined while the row is chosen */
static void helpers_draw(int x0, int y, bool chosen) {
	SDL_Color on = rgba(120, 255, 140, 255), dim = rgba(120, 140, 170, 255), orange = rgba(255, 170, 40, 255);
	/* (each name's middle: four in the row from "Help" to the frame's edge,
	 * All * beside Gentle, 8 pixels apart and 10 after "Help") */
	static const int hxs[HELPERS] = { 78, 117, 167, 214 };
	text_draw(x0 + 26, y, "Help", WHITE, TEXT_LEFT);
	for (int h = 0; h < HELPERS; ++h) {
		int hx = x0 + hxs[h];
		text_draw(hx, y, meta_helper(h)->name, S.helpers >> h & 1 ? on : dim, TEXT_CENTER);
		if (chosen && S.helper == h) fill_rect(hx - 16, y + 11, 32, 1, orange);
	}
}

static void setup_draw(int x0, int y0) {
	SDL_Color gold = rgba(255, 230, 90, 255), sky = rgba(170, 200, 255, 255), orange = rgba(255, 170, 40, 255);
	/* (four pixels wider each side than it was: "Storm: delete an Aqua
	 * guardian" ran from edge to edge, a playtester's) */
	fill_rect(x0 + 4, y0 + 6, CORE_W - 8, CORE_H - 12, rgba(66, 198, 231, 255));
	fill_rect(x0 + 6, y0 + 8, CORE_W - 12, CORE_H - 16, rgba(16, 60, 90, 245));
	int cx = x0 + CORE_W / 2, lx = x0 + 26, vx = x0 + 150;
	text_draw(cx, y0 + 12, "JACK-IN SETUP", gold, TEXT_CENTER);
	char v[48];
	static const int ry[ROWS] = { 30, 43, 56, 69, 82, 139 };
	/* the net */
	text_draw(lx, y0 + ry[ROW_NET], "Net", WHITE, TEXT_LEFT);
	choice_draw(vx, y0 + ry[ROW_NET], S.net == RUN_ENDLESS ? "Endless" : "Short", WHITE, meta_endless_open());
	/* the folder */
	text_draw(lx, y0 + ry[ROW_FOLDER], "Folder", WHITE, TEXT_LEFT);
	int open_folders = 0;
	for (int f = 0; f < FOLDER_COUNT; ++f) open_folders += meta_folder_open(f);
	choice_draw(vx, y0 + ry[ROW_FOLDER], meta_folder(S.folder)->name, WHITE, open_folders > 1);
	/* the Cross brought */
	text_draw(lx, y0 + ry[ROW_CROSS], "Cross", WHITE, TEXT_LEFT);
	int open_crosses = 0;
	for (int c = 1; c <= 5; ++c) open_crosses += meta_cross_open(c);
	choice_draw(vx, y0 + ry[ROW_CROSS], S.cross ? powers_cross_name(S.cross) : "None", S.cross ? orange : WHITE, open_crosses > 0);
	/* the threat */
	text_draw(lx, y0 + ry[ROW_THREAT], "Threat", WHITE, TEXT_LEFT);
	snprintf(v, sizeof v, "%d", S.threat);
	choice_draw(vx, y0 + ry[ROW_THREAT], v, S.threat ? orange : WHITE, meta_threat_open() > 0);
	/* what the last summary announced: NEW beside its row */
	for (int r = 0; r < ROW_HELPERS; ++r)
		if (row_new(r)) {
			static const char *const label[] = { "Net", "Folder", "Cross", "Threat" };
			text_draw(lx + text_width(label[r]) + 6, y0 + ry[r], "NEW", gold, TEXT_LEFT);
		}
	helpers_draw(x0, y0 + ry[ROW_HELPERS], S.row == ROW_HELPERS);
	/* jack in */
	text_draw(cx, y0 + ry[ROW_GO], "JACK IN!", S.row == ROW_GO ? gold : WHITE, TEXT_CENTER);
	/* the cursor: the PET's orange arrow */
	int ay = y0 + ry[S.row] + 2;
	int ax = S.row == ROW_GO ? cx - 44 : lx - 12;
	for (int i = 0; i < 4; ++i) fill_rect(ax + i, ay + i, 1, 8 - 2 * i, orange);
	/* what the chosen row means */
	const char *note = "";
	char buf[128], locked[FOLDER_COUNT][48];
	int nlocked = 0;
	switch (S.row) {
	case ROW_NET:
		note = S.net == RUN_ENDLESS ? "The net repeats, harder each time" : meta_endless_open() ? "Three acts, then the Nest" :
			"Three acts, then the Nest. Win it for the endless net";
		break;
	case ROW_FOLDER:
		note = meta_folder(S.folder)->about;
		/* (every folder still to open, and how: the telegraph comes first) */
		for (int f = 1; f < FOLDER_COUNT; ++f)
			if (!meta_folder_open(f) && meta_folder(f)->opens)
				snprintf(locked[nlocked++], sizeof locked[0], "%s: %s", meta_folder(f)->name, meta_folder(f)->opens);
		break;
	case ROW_CROSS: {
		/* (and its costs: BN6's own weakness, which a playtester was told
		 * wrong, and no other Cross) */
		const char *weak = S.cross ? powers_cross_weakness(S.cross) : NULL, *strong = S.cross ? powers_cross_strength(S.cross) : NULL;
		/* (and break it: a playtester knew from BN6, a newer player would not) */
		if (weak && strong) snprintf(buf, sizeof buf, "%s.\n%s hits do 2x and break it", strong, weak);
		else if (weak) snprintf(buf, sizeof buf, "From the first battle. %s hits do 2x and break it", weak);
		note = weak ? buf : S.cross ? "From the first battle"
			: open_crosses ? "Crosses from the guardians we delete" : "Delete a Cross Navi to start in his Cross";
		/* (the ones still shut, and how) */
		if (S.cross) snprintf(locked[nlocked++], sizeof locked[0], "%s", "No other Cross this run");
		else if (open_crosses && open_crosses < 5) snprintf(locked[nlocked++], sizeof locked[0], "%s", "Start in more: delete Navis");
		break;
	}
	case ROW_THREAT:
		if (!S.threat) note = meta_threat_open() ? "The net as it comes" : "The net as it comes. Win it for threat 1";
		else note = meta_threat_rule(S.threat);
		break;
	case ROW_HELPERS:
		snprintf(buf, sizeof buf, "%s. A: on or off", meta_helper(S.helper)->about);
		note = buf;
		/* (and what else it changes: All *'s Program Advances) */
		if (meta_helper(S.helper)->more) snprintf(locked[nlocked++], sizeof locked[0], "%s", meta_helper(S.helper)->more);
		break;
	default: note = "A: jack in. B: back"; break;
	}
	note_more(cx, y0 + 99 + note_line(cx, y0 + 97, note, sky) * 12, locked, nlocked);
}

static void update(void) {
	++S.t;
	if (S.leaving) {
		++S.leaving;
		if (S.jack_v < 96) S.jack_v += 4;
		S.jack_y += S.jack_v;
		if (S.leaving > LEAVE_FRAMES) {
			if (S.choice == 1 && load_run()) emu_resume_requested = true;
			else {
				run_new_varied(title_seed ? title_seed++ : rng_next() ^ (uint32_t)SDL_GetTicks());
				run_setup(S.net, S.folder, S.threat, S.helpers, S.cross);
				meta_run_begun();
				emu_start_in_town = true;
			}
			scene_set(&scene_emu);
		}
		return;
	}
	if (S.setup) { setup_update(); return; }
	if (!S.summary && S.t - S.shown_at >= SHOW_FRAMES) { S.first = false; show_next(); }
	if (S.summary) {
		/* then the net comes back for PRESS START */
		if (S.t > SUMMARY_MIN && (btn_pressed(BTN_A) || btn_pressed(BTN_START))) {
			S.summary = false;
			audio_music(MUS_TITLE);
			S.t = S.shown_at = 0;
			/* (the summary has room for two unlocks: a mark shows itself) */
			S.new_marks = meta_marks_new();
			if (S.new_marks) audio_sfx(SFX_REVEAL);
		}
		return;
	}
	if (!S.pressed) { press_start(); return; }
	if (!S.menu) {
		if (S.t - S.pressed >= MENU_AFTER) S.menu = S.t;
		return;
	}
	/* NEW GAME over a saved run: the run ends at its next checkpoint, so
	 * the one destructive choice here asks first, No chosen */
	bool ok = btn_pressed(BTN_A) || btn_pressed(BTN_START);
	if (S.confirm) {
		/* while it types, A or B shows all of it (as the game's chats) */
		int all = (int)(strlen(ASK_FIRST) + strlen(S.ask));
		if (S.t - S.asked < all) {
			if (ok || btn_pressed(BTN_B)) S.asked = S.t - all;
			return;
		}
		if (btn_pressed(BTN_LEFT) || btn_pressed(BTN_RIGHT) || btn_repeat(BTN_UP) || btn_repeat(BTN_DOWN)) {
			S.yes = !S.yes;
			audio_sfx(SFX_CURSOR);
		}
		if (btn_pressed(BTN_B) || (ok && !S.yes)) { S.confirm = false; audio_sfx(SFX_CANCEL); return; }
		if (!ok) return;
		S.confirm = false;
	} else {
		if (select_pressed()) return;
		int items = S.has_save ? 2 : 1;
		if (items > 1 && (btn_repeat(BTN_UP) || btn_repeat(BTN_DOWN))) { S.cursor ^= 1; audio_sfx(SFX_CURSOR); }
		if (!ok) return;
		if (S.cursor == 0 && S.has_save) {
			S.confirm = true;
			S.yes = false;
			S.asked = S.t;
			if (S.saved_depth) snprintf(S.ask, sizeof S.ask, "our Layer %d run!", S.saved_depth);
			else snprintf(S.ask, sizeof S.ask, "our saved run!");
			audio_sfx(SFX_SELECT);
			return;
		}
	}
	S.choice = S.cursor;
	S.net = RUN_SHORT;
	if (S.choice == 0 && setup_wanted()) {
		setup_open();
		audio_sfx(SFX_SELECT);
		return;
	}
	jack_in();
}

/* NEW GAME or CONTINUE chosen: the jack-in sound, and away */
static void jack_in(void) {
	/* the game plays 0x9D for NEW GAME and 0x9C for CONTINUE and stops the music */
	audio_play_song(S.choice == 1 ? 0x9C : 0x9D, false);
	audio_music(MUS_NONE);
	S.leaving = 1;
}

/* ------------------------------------------------------------------ */

static uint32_t lighten(uint32_t c) {
	uint32_t r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255;
	return 0xFF000000u | (r + (255 - r) / 2) << 16 | (g + (255 - g) / 2) << 8 | (b + (255 - b) / 2);
}

static void render(void) {
	static uint32_t px[240 * 160];
	/* the net darkens downwards; more behind the menu and the summary */
	int extra = S.summary ? 14 : 0;
	if (S.pressed) extra = S.t - S.pressed >= 3 ? 6 : 2 * (S.t - S.pressed);
	uint8_t dim[160];
	for (int y = 0; y < 160; ++y) dim[y] = (uint8_t)(1 + y * 9 / 160 + extra + S.lift);
	int k = S.t - S.shown_at, mosaic = 1;
	if (!S.summary && !S.leaving) {
		if (k < SWAP_FRAMES && !S.first) mosaic = SWAP_FRAMES - k;
		else if (k > SHOW_FRAMES - SWAP_FRAMES) mosaic = k - (SHOW_FRAMES - SWAP_FRAMES);
	}
	backdrop_draw(&bd, k, 0, S.jack_y >> 4, dim, mosaic, px);

	if (!S.summary) {
		/* a shine runs over the logo now and then */
		int shine = S.t % 300 < 40 ? (S.t % 300) * 8 - 40 : -1000;
		for (int y = 0; y < 160; ++y) {
			int ly = y + LOGO_UP;
			if (ly >= 160) break;
			for (int x = 0; x < 240; ++x) {
				uint8_t v = logo[ly][x];
				if (!v) continue;
				int s = x + ly / 2 - shine;
				px[y * 240 + x] = s >= 0 && s < 7 ? lighten(logo_col[v]) : logo_col[v];
			}
		}
	}
	if (!S.tex) {
		S.tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 240, 160);
		SDL_SetTextureScaleMode(S.tex, SDL_ScaleModeNearest);
	}
	SDL_UpdateTexture(S.tex, NULL, px, 240 * 4);
}

/* The build, for a report (v0.5.0 beta, v0.5.0+12 a dozen commits on; 0.1
 * to 0.4 were alphas) */
static const char *version_words(char *v, size_t n) {
	const char *cw = CW_VERSION;
	if (!strncmp(cw, "dev", 3) || !strncmp(cw, "0.0.1+git", 9)) snprintf(v, n, "dev");
	else {
		const char *g = strstr(cw, ".g");
		snprintf(v, n, "v%.*s%s", g ? (int)(g - cw) : (int)strlen(cw), cw, !strncmp(cw, "0.", 2) ? " beta" : "");
	}
	return v;
}

/* "BN5 found": the other games' ROMs read beside BN6's, said once a start
 * in the picture's top right corner a moment after the title comes up, in
 * the map's "Run saved" box, and gone (nothing had said whether Team
 * Colonel was found) */
static void found_draw(int x0, int y0) {
	int t = entries == 1 ? S.t - FOUND_AT : 0, n = 0;
	char s[64] = "";
	for (int i = 0; i < XROM_COUNT; ++i)
		if (XR[i].data) {
			size_t m = strlen(s);
			snprintf(s + m, sizeof s - m, "%s%s", m ? " + " : "", XR[i].layout->tag);
			++n;
		}
	if (t <= 0 || t >= FOUND_LEN || !s[0]) return;
	/* (and what it lends: "BN5 found" told a playtester nothing of his
	 * run, session 62; its areas, towns, songs and Navis join the net's,
	 * docs/MULTIROM.md) */
	size_t m = strlen(s);
	snprintf(s + m, sizeof s - m, n > 1 ? " found: their Nets join ours" : " found: its Net joins ours");
	cinema_note_box(x0, y0, s, t, FOUND_LEN);
}

/* The word under the picture: (a keyboard's Start, where no controller is:
 * a PC player had no word of which key it is) ENTER before the menu, and
 * with the menu where the controls are set, on the player's own SELECT
 * (controls_word: "R", "Minus", "SELECT"), and the ROMs, on R, where the
 * launcher is (issue #97: no way to add BN5 had been found) */
static void hint_draw(int x0, int y0) {
	SDL_Color grey = rgba(150, 160, 190, 255);
	if (!S.menu && !S.pressed && !platform_pad_present() && !touch_shown()) minifont_draw_centered(x0 + CORE_W / 2, y0 + 138, "ENTER", grey, 1);
#ifndef __3DS__
	if (!S.menu || S.confirm) return;
	char hint[64];
	snprintf(hint, sizeof hint, "%s: Controls", controls_word(BTN_SELECT));
	if (launcher_here()) {
		size_t m = strlen(hint);
		snprintf(hint + m, sizeof hint - m, "   %s: ROMs", controls_word(BTN_R));
	}
	minifont_draw_centered(x0 + CORE_W / 2, y0 + 138, hint, grey, 1);
#endif
}

/* the top: the marks and the note over them, neither over a run's summary
 * (a start whose title first showed a summary drew "BN5 found" over
 * "MegaMan was deleted", session 68) */
static void top_draw(int x0, int y0) {
	if (S.summary) return;
	marks_draw(x0, y0);
	found_draw(x0, y0);
}

/* a helped run's helpers, small over its summary's title (docs/META.md:
 * the summary names a helped run): "HELP: HP+ ALL *" */
static void helpers_named(int cx, int y) {
	if (!(run.helpers & ((1 << HELPERS) - 1))) return;
	char s[48] = "Help:";
	for (int h = 0; h < HELPERS; ++h)
		if (run.helpers >> h & 1) {
			size_t m = strlen(s);
			snprintf(s + m, sizeof s - m, " %s", meta_helper(h)->name);
		}
	minifont_draw_centered(cx, y, s, rgba(150, 160, 190, 255), 1);
}

/* The finished run's summary over its area: how it ended, how far, the
 * Library, the best, Dad's and Lan's words, what it opened or the
 * closest goal, and a helped run's helpers */
static void summary_draw(int x0, int y0) {
	SDL_Color gold = rgba(255, 230, 90, 255), sky = rgba(170, 200, 255, 255);
	int x = x0 + CORE_W / 2;
	helpers_named(x, y0 + 2);
	if (title_won) text_draw(x, y0 + 8, "The Nest has fallen!", gold, TEXT_CENTER);
	else text_draw(x, y0 + 8, "MegaMan was deleted", rgba(255, 120, 120, 255), TEXT_CENTER);
	if (title_cause[0]) text_draw(x, y0 + 22, title_cause, sky, TEXT_CENTER);
	/* Lan at his PET, and how far they got */
	Sprite *lan = sprite_get(SPR_MUGSHOT, 0x00);
	if (lan) sprite_draw_frame(lan, 0, 0, x0 + SUMMARY_FACE_X, y0 + SUMMARY_FACE_Y, false, 0, 0);
	int lx = x0 + 70, rx = x0 + CORE_W - 14;
	text_draw(lx, y0 + 40, "Reached", WHITE, TEXT_LEFT);
	text_drawf(rx, y0 + 40, WHITE, TEXT_RIGHT, "Layer %d", run_reached());
	text_draw(lx, y0 + 52, "Viruses deleted", WHITE, TEXT_LEFT);
	text_drawf(rx, y0 + 52, WHITE, TEXT_RIGHT, "%d", run.viruses_deleted);
	text_draw(lx, y0 + 64, "Navis deleted", WHITE, TEXT_LEFT);
	text_drawf(rx, y0 + 64, WHITE, TEXT_RIGHT, "%d", run.bosses_beaten);
	/* the Library, and what the run added to it (docs/META.md) */
	text_draw(lx, y0 + 76, "Library", WHITE, TEXT_LEFT);
	int lib = meta_library_count(-1), added = meta_library_new();
	if (added) text_drawf(rx, y0 + 76, WHITE, TEXT_RIGHT, "%d (+%d)", lib, added);
	else text_drawf(rx, y0 + 76, WHITE, TEXT_RIGHT, "%d", lib);
	if (title_new_best) text_draw(lx, y0 + 89, "New best!", gold, TEXT_LEFT);
	else {
		text_draw(lx, y0 + 89, "Best", gold, TEXT_LEFT);
		text_drawf(rx, y0 + 89, gold, TEXT_RIGHT, "Layer %d", profile.best_depth);
	}
	/* what the run opened for the next (docs/META.md): three lines
	 * where it opened three, the lines above drawn closer (a won run
	 * opened the endless net, a threat, a mark and a Cross start, and
	 * the summary named two) */
	const char *open[3];
	int n = meta_unlocked(open, 3), up = n >= 3 ? 2 : 0;
	/* Dad's backup, as his call promised, and Lan's word; a won run
	 * jacks out */
	if (title_won) {
		text_draw(x, y0 + 102 - up, summary_words(SUMMARY_WON_HOW), sky, TEXT_CENTER);
		text_draw(x, y0 + 114 - 2 * up, summary_words(SUMMARY_WON), WHITE, TEXT_CENTER);
	} else {
		/* (a line the game's width holds: "brought MegaMan home." ran off
		 * both sides where the screen shows no more than the game's 240
		 * pixels, a 480 x 320 handheld's or a 640 x 480 filled, issue #36) */
		text_draw(x, y0 + 102 - up, summary_words(SUMMARY_LOST_HOW), sky, TEXT_CENTER);
		/* (a first fight with the guardian who deleted him, named on
		 * the line above: what it gave, his battle data, which the
		 * next briefing reads, before a new best, which "New best!"
		 * says above; "We've got BlastMan's battle data now!" ran off
		 * the screen's sides) */
		int said = title_learned[0] ? SUMMARY_LEARNED : title_new_best ? SUMMARY_BEST : run_reached() <= 2 ? SUMMARY_ROUGH : SUMMARY_FURTHER;
		text_draw(x, y0 + 114 - 2 * up, summary_words(said), WHITE, TEXT_CENTER);
	}
	/* ... else the closest goal */
	int y = y0 + (n >= 3 ? 124 : 129);
	for (int i = 0; i < n; ++i, y += 12) text_drawf(x, y, gold, TEXT_CENTER, "Unlocked: %s", open[i]);
	const char *next = meta_next_goal();
	if (next && n < 2) text_draw(x, y, next, sky, TEXT_CENTER);
}

static void draw(void) {
	fill_rect(0, 0, P.w, P.h, BLACK);
	int x0 = P.core_x, y0 = P.core_y;
	if (S.leaving > LEAVE_WHITE) {
		/* into the net: the white fades to black */
		int a = 255 - (S.leaving - LEAVE_WHITE) * 255 / (LEAVE_FRAMES - LEAVE_WHITE);
		fill_rect(x0, y0, CORE_W, CORE_H, rgba(255, 255, 255, a < 0 ? 0 : a));
		return;
	}
	render();
	SDL_Rect dst = { x0, y0, 240, 160 };
	SDL_RenderCopy(P.renderer, S.tex, NULL, &dst);
	if (S.setup) { setup_draw(x0, y0); return; }

	/* the copyright line: 8 OBJs of 32x32 along the bottom */
	uint32_t copy = gfx_lz_ref(T.copy_tiles) + 4;
	if (!S.confirm && !S.summary)   /* (the question takes its place a moment; the summary runs to the bottom) */
		for (int i = 0; i < 8; ++i) rom_tiles(copy + (uint32_t)i * 16 * 32, T.copy_pal, x0 + i * 32, y0 + 126, 4, 4, 0);
	top_draw(x0, y0);

	if (S.summary) { summary_draw(x0, y0); return; }
	/* a line above the copyright, the marks holding the top: the build
	 * (version_words), and the best depth, a saved run deeper than the
	 * record counting too (the menu's CONTINUE names its own layer there;
	 * the question's box covers both) */
	if (!S.confirm) {
		char v[40];
		minifont_draw(x0 + 4, y0 + 138, version_words(v, sizeof v), rgba(150, 160, 190, 255), 1);
		int best = profile.best_depth > S.saved_depth ? profile.best_depth : S.saved_depth;
		if (best > 0 && !(S.menu && S.has_save && S.saved_depth)) {
			snprintf(v, sizeof v, "Best: Layer %d", best);
			minifont_draw(x0 + CORE_W - 4 - minifont_width(v, 1), y0 + 138, v, rgba(255, 230, 90, 255), 1);
		}
	}

	uint32_t text = gfx_lz_ref(T.text_tiles) + 4 - 32; /* OBJ tile 1 is the block's first */
	/* PRESS START blinks; once pressed it flickers until the menu */
	bool lit = S.pressed ? ((S.t - S.pressed) / 2) % 2 == 0 : ((S.t / BLINK) & 1) == 0;
	if (lit && !S.menu) {
		for (int i = 0; i < 4; ++i) rom_tiles(text + (uint32_t)(1 + i * 8) * 32, T.text_pal, x0 + 52 + i * 32, y0 + 120, 4, 2, 0);
		rom_tiles(text + 33u * 32, T.text_pal, x0 + 180, y0 + 120, 1, 2, 0);
	}
	hint_draw(x0, y0);
	if (S.menu) {
		/* NEW GAME (tiles 35-54) and CONTINUE (55-74): 32x16, 32x16, 16x16 */
		int n = S.has_save ? 2 : 1;
		for (int i = 0; i < n; ++i) {
			uint32_t first = i == 0 ? 35 : 55;
			int y = y0 + 112 + i * 16;
			rom_tiles(text + first * 32, T.menu_pal, x0 + 88, y, 4, 2, 0);
			rom_tiles(text + (first + 8) * 32, T.menu_pal, x0 + 120, y, 4, 2, 0);
			rom_tiles(text + (first + 16) * 32, T.menu_pal, x0 + 152, y, 2, 2, 0);
		}
		if (S.has_save && S.saved_depth)
			text_drawf(x0 + 170, y0 + 130, WHITE, TEXT_LEFT, "Layer %d", S.saved_depth);
		/* the arrow cycles three frames, 6 frames each */
		int f = ((S.t - S.menu) / 6) % 3;
		if (!S.confirm) rom_tiles(T.arrow + (uint32_t)f * 4 * 32, T.arrow_pal, x0 + 73, y0 + 113 + S.cursor * 16, 2, 2, 0);
	}
	if (S.confirm) {
		/* over the menu, MegaMan asks as in the game's chats: two lines,
		 * typed a character a frame, then Yes and No with the cursor (the
		 * places are the game's, from its own Yes/No) */
		chatbox_frame(x0 + CHATBOX_X, y0 + CHATBOX_Y, CHATBOX_W, CHATBOX_H);
		Sprite *face = sprite_get(SPR_MUGSHOT, FACE_MEGAMAN);
		if (face) sprite_draw_frame(face, 0, 0, x0 + CHATBOX_FACE_X, y0 + CHATBOX_FACE_Y, false, 0, 0);
		int typed = S.t - S.asked, first = (int)strlen(ASK_FIRST);
		int tx = x0 + CHATBOX_TEXT_X, ty = y0 + CHATBOX_TEXT_Y;
		chatbox_text(tx, ty, ASK_FIRST, typed);
		if (typed > first) chatbox_text(tx, ty + CHATBOX_LINE, S.ask, typed - first);
		if (typed >= first + (int)strlen(S.ask)) {
			int oy = ty + 2 * CHATBOX_LINE;
			chatbox_text(x0 + 91, oy, "Yes", -1);
			chatbox_text(x0 + 138, oy, "No", -1);
			int f = (S.t / 6) % 3;
			rom_tiles(T.arrow + (uint32_t)f * 4 * 32, T.arrow_pal, x0 + (S.yes ? 68 : 115), oy - 1, 2, 2, 0);
		}
	}
	if (S.first && S.t < 16 && !S.summary) { P.fx_fade = 16 - S.t; P.fx_fade_color = BLACK; }
	if (S.leaving) fill_rect(x0, y0, CORE_W, CORE_H, rgba(255, 255, 255, S.leaving * 255 / LEAVE_WHITE));
}

/* (the second screen's PET at its pace, then the title's frame) */
static void tick(void) {
	platform_second_screen_draw();
	update();
}

const Scene scene_title = { "title", enter, tick, draw, NULL };
