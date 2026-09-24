/* The title screen. The logo is the original's (docs/ROM_DATA.md), taken
 * from its 256-colour picture without the 6, the CYBEAST GREGAR plate and
 * the Cybeast; in the 6's place the engine draws an infinity mark in the
 * 6's colours, and under BATTLE NETWORK the subtitle CYBERWORLD ENDLESS in
 * the plate's. Behind it the battle backgrounds of the areas a run passes
 * through take turns, scrolling and animated as in battle. PRESS START,
 * NEW GAME / CONTINUE, the cursor and the copyright line are the game's. */
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "backdrop.h"
#include "game.h"
#include "gfx.h"
#include "platform.h"
#include "rom.h"
#include "run.h"
#include "save.h"

#define T (R.layout->title)
#define BLINK 32         /* PRESS START: 32 frames on, 32 off */
#define MENU_AFTER 40    /* frames from START to the menu */
#define LEAVE_WHITE 24   /* jacking in: the net rushes by into white... */
#define LEAVE_FRAMES 40  /* ...which fades to black */
#define SUMMARY_MIN 60   /* frames before the summary can be closed */
#define SHOW_FRAMES 600  /* each backdrop's turn */
#define SWAP_FRAMES 12   /* mosaic out and in between them */

#define LOGO_UP 8        /* the logo stands higher than in the original picture */
#define LOGO_BOTTOM 95   /* below: the CYBEAST GREGAR plate */
#define LOGO_END 0x50    /* banks 0-4 draw the logo, 5-6 the blue, 7-13 the Cybeast */

/* The areas of a run, in the order of their acts (the BIOME_* numbers):
 * Central, Seaside, Sky, Green, Graveyard, Undernet, Secret Area, Nest. */
static const uint8_t cycle[] = { 0x09, 0x0B, 0x04, 0x0D, 0x14, 0x0F, 0x13, 0x15 };
#define NCYCLE (int)(sizeof cycle / sizeof *cycle)

bool title_summary;

static struct {
	int t, pressed, menu, leaving, cursor, choice;
	bool has_save;
	int saved_depth;      /* the saved run's layer, 0 when unknown */
	bool summary;         /* the finished run's summary over its area */
	int next;             /* the cycle's next backdrop */
	int shown_at;         /* frame the backdrop came in */
	bool first;           /* the first backdrop fades in instead */
	int jack_v, jack_y;   /* jacking in: the backdrop's speed and offset, 1/16 pixel */
	SDL_Texture *tex;
} S;

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
 * crossing over the other one. */
static void build_infinity(void) {
	enum { W = 96, H = 52, N = 360 };
	const double A = 28, B = 48, RAD = 3.9, SLANT = 0.42;
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
	stamp(&m[top][0], W, bottom - top + 1, 204 - W / 2, 60 - (bottom - top + 1) / 2, fill, (int)sizeof fill,
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

static void show(int id) {
	backdrop_load(&bd, id);
	S.shown_at = S.t;
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

static void enter(void) {
	SDL_Texture *tex = S.tex;
	memset(&S, 0, sizeof S);
	S.tex = tex;
	if (!logo_ready) logo_ready = build_logo();
	S.has_save = save_exists();
	Run saved = { 0 };
	if (S.has_save && peek_run(&saved)) S.saved_depth = saved.depth;
	S.cursor = S.has_save ? 1 : 0; /* CONTINUE when there is one */
	S.summary = title_summary;
	title_summary = false;
	S.first = true;
	if (S.summary) show_area(run.biome);
	else if (S.saved_depth) show_area(saved.biome);
	else show_next();
	audio_music(MUS_TITLE);
}

static void update(void) {
	++S.t;
	if (S.leaving) {
		++S.leaving;
		if (S.jack_v < 96) S.jack_v += 4;
		S.jack_y += S.jack_v;
		if (S.leaving > LEAVE_FRAMES) {
			if (S.choice == 1 && load_run()) emu_resume_requested = true;
			else run_new(rng_next() ^ (uint32_t)SDL_GetTicks());
			scene_set(&scene_emu);
		}
		return;
	}
	if (!S.summary && S.t - S.shown_at >= SHOW_FRAMES) { S.first = false; show_next(); }
	if (S.summary) {
		/* then the net comes back for PRESS START */
		if (S.t > SUMMARY_MIN && (btn_pressed(BTN_A) || btn_pressed(BTN_START))) {
			S.summary = false;
			S.t = S.shown_at = 0;
		}
		return;
	}
	if (!S.pressed) {
		if (S.t > 20 && (btn_pressed(BTN_START) || btn_pressed(BTN_A))) { S.pressed = S.t; audio_sfx(SFX_SELECT); }
		if (btn_pressed(BTN_SELECT)) scene_set(&scene_gallery);
		return;
	}
	if (!S.menu) {
		if (S.t - S.pressed >= MENU_AFTER) S.menu = S.t;
		return;
	}
	int items = S.has_save ? 2 : 1;
	if (items > 1 && (btn_repeat(BTN_UP) || btn_repeat(BTN_DOWN))) { S.cursor ^= 1; audio_sfx(SFX_CURSOR); }
	if (btn_pressed(BTN_A) || btn_pressed(BTN_START)) {
		S.choice = S.cursor;
		/* the game plays 0x9D for NEW GAME and 0x9C for CONTINUE and stops the music */
		audio_play_song(S.choice == 1 ? 0x9C : 0x9D, false);
		audio_music(MUS_NONE);
		S.leaving = 1;
	}
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
	for (int y = 0; y < 160; ++y) dim[y] = (uint8_t)(1 + y * 9 / 160 + extra);
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

	/* the copyright line: 8 OBJs of 32x32 along the bottom */
	uint32_t copy = gfx_lz_ref(T.copy_tiles) + 4;
	for (int i = 0; i < 8; ++i) rom_tiles(copy + (uint32_t)i * 16 * 32, T.copy_pal, x0 + i * 32, y0 + 126, 4, 4, 0);

	if (S.summary) {
		int x = x0 + CORE_W / 2, y = y0 + 24;
		text_draw(x, y, "MegaMan was deleted", rgba(255, 120, 120, 255), TEXT_CENTER);
		text_drawf(x, y + 24, WHITE, TEXT_CENTER, "Reached B%d", run.depth);
		text_drawf(x, y + 40, WHITE, TEXT_CENTER, "Viruses deleted  %d", run.viruses_deleted);
		text_drawf(x, y + 56, WHITE, TEXT_CENTER, "Navis deleted  %d", run.bosses_beaten);
		text_drawf(x, y + 80, rgba(255, 230, 90, 255), TEXT_CENTER, "Best  B%d", profile.best_depth);
		return;
	}
	if (profile.best_depth > 0)
		text_drawf(x0 + CORE_W - 4, y0 + 2, rgba(255, 230, 90, 255), TEXT_RIGHT, "Best B%d", profile.best_depth);

	uint32_t text = gfx_lz_ref(T.text_tiles) + 4 - 32; /* OBJ tile 1 is the block's first */
	/* PRESS START blinks; once pressed it flickers until the menu */
	bool lit = S.pressed ? ((S.t - S.pressed) / 2) % 2 == 0 : ((S.t / BLINK) & 1) == 0;
	if (lit && !S.menu) {
		for (int i = 0; i < 4; ++i) rom_tiles(text + (uint32_t)(1 + i * 8) * 32, T.text_pal, x0 + 52 + i * 32, y0 + 120, 4, 2, 0);
		rom_tiles(text + 33u * 32, T.text_pal, x0 + 180, y0 + 120, 1, 2, 0);
	}
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
			text_drawf(x0 + 170, y0 + 130, WHITE, TEXT_LEFT, "B%d", S.saved_depth);
		/* the arrow cycles three frames, 6 frames each */
		int f = ((S.t - S.menu) / 6) % 3;
		rom_tiles(T.arrow + (uint32_t)f * 4 * 32, T.arrow_pal, x0 + 73, y0 + 113 + S.cursor * 16, 2, 2, 0);
	}
	if (S.first && S.t < 16 && !S.summary) { P.fx_fade = 16 - S.t; P.fx_fade_color = BLACK; }
	if (S.leaving) fill_rect(x0, y0, CORE_W, CORE_H, rgba(255, 255, 255, S.leaving * 255 / LEAVE_WHITE));
}

const Scene scene_title = { "title", enter, update, draw, NULL };
