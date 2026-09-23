/* The original title screen, drawn from the ROM (docs/ROM_DATA.md): the
 * 256-colour picture with its glowing logo, the copyright line, PRESS START,
 * then NEW GAME / CONTINUE over the dimmed picture. Timings were recorded
 * from the game. */
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "game.h"
#include "gfx.h"
#include "platform.h"
#include "rom.h"
#include "run.h"
#include "save.h"

#define T (R.layout->title)
#define BLINK 32        /* PRESS START: 32 frames on, 32 off */
#define MENU_AFTER 84   /* frames from START to the menu */
#define LEAVE_FRAMES 16

typedef struct { uint32_t dest, count; int nsteps, total; uint32_t step[16]; int delay[16]; bool loop; } PalAnim;

static struct {
	int t, pressed, menu, leaving, cursor, choice;
	bool has_save;
	uint8_t *tiles;       /* 8bpp BG tiles */
	size_t tiles_len;
	uint8_t pal[512];     /* BG palette RAM */
	PalAnim anim[8];
	int nanim, key[8];
	SDL_Texture *tex;
} S;

static void load_anims(void) {
	S.nanim = 0;
	for (uint32_t a = T.bg_anims; S.nanim < 8 && a + 4 <= ROM_SIZE && rom_u32(a) != 0xFFFFFFFFu; a += 4) {
		uint32_t d = rom_u32(a);
		if (!rom_is_ptr(d)) break;
		d = rom_off(d);
		PalAnim *an = &S.anim[S.nanim];
		memset(an, 0, sizeof *an);
		if (R.data[d + 8] != 0) continue; /* palette copies only */
		an->dest = rom_u32(d) - 0x03001960u;
		an->count = rom_u32(d + 4);
		for (uint32_t q = d + 12; an->nsteps < 16; q += 8) {
			uint32_t nx = rom_u32(q);
			if (nx <= 1) { an->loop = nx == 1; break; }
			an->step[an->nsteps] = rom_off(nx);
			an->delay[an->nsteps] = (int)rom_u32(q + 4);
			an->total += an->delay[an->nsteps++];
		}
		if (an->nsteps && an->total && an->dest + an->count <= sizeof S.pal) S.nanim++;
	}
}

static int anim_step(const PalAnim *an, int frame) {
	int t = an->loop ? frame % an->total : (frame < an->total ? frame : an->total - 1);
	for (int i = 0; i < an->nsteps; ++i) {
		if (t < an->delay[i]) return i;
		t -= an->delay[i];
	}
	return an->nsteps - 1;
}

/* START dims the picture: every colour, the logo's animated ones included,
 * down 4 levels a channel, then 2 a frame to 16. */
static int dim_level(void) {
	if (!S.pressed) return 0;
	int k = S.t - S.pressed;
	return k < 0 ? 0 : k >= 6 ? 16 : 4 + 2 * k;
}

static void render_bg(void) {
	static uint32_t px[256 * 160];
	int dim = dim_level();
	uint32_t col[256];
	for (int i = 0; i < 256; ++i) {
		uint16_t c = (uint16_t)(S.pal[i * 2] | S.pal[i * 2 + 1] << 8);
		int r = (c & 31) - dim, g = ((c >> 5) & 31) - dim, b = ((c >> 10) & 31) - dim;
		col[i] = bgr555((uint16_t)((r < 0 ? 0 : r) | (g < 0 ? 0 : g) << 5 | (b < 0 ? 0 : b) << 10));
	}
	for (int ty = 0; ty < 20; ++ty)
		for (int tx = 0; tx < 32; ++tx) {
			uint16_t e = rom_u16(T.bg_map + (uint32_t)(ty * 32 + tx) * 2);
			size_t at = 4 + (size_t)(e & 0x3FF) * 64;
			const uint8_t *tile = at + 64 <= S.tiles_len ? S.tiles + at : NULL;
			for (int y = 0; y < 8; ++y)
				for (int x = 0; x < 8; ++x) {
					int X = (e & 0x400) ? 7 - x : x, Y = (e & 0x800) ? 7 - y : y;
					px[(ty * 8 + Y) * 256 + tx * 8 + X] = col[tile ? tile[y * 8 + x] : 0];
				}
		}
	if (!S.tex) {
		S.tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 160);
		SDL_SetTextureScaleMode(S.tex, SDL_ScaleModeNearest);
	}
	SDL_UpdateTexture(S.tex, NULL, px, 256 * 4);
}

static void enter(void) {
	uint8_t *keep = S.tiles;
	size_t keep_len = S.tiles_len;
	SDL_Texture *tex = S.tex;
	memset(&S, 0, sizeof S);
	S.tiles = keep; S.tiles_len = keep_len; S.tex = tex;
	if (!S.tiles) S.tiles = lz77_decompress(R.data + T.bg_tiles, ROM_SIZE - T.bg_tiles, &S.tiles_len);
	memcpy(S.pal, R.data + T.bg_pal, 0x1C0);
	memcpy(S.pal + 15 * 32, R.data + T.bg_pal15, 32);
	load_anims();
	for (int i = 0; i < 8; ++i) S.key[i] = -1;
	S.has_save = save_exists();
	S.cursor = S.has_save ? 1 : 0; /* CONTINUE when there is one */
	audio_music(MUS_TITLE);
}

static void update(void) {
	++S.t;
	if (S.leaving) {
		if (++S.leaving > LEAVE_FRAMES) {
			if (S.choice == 1 && load_run()) net_resume();
			else {
				run_new(rng_next() ^ (uint32_t)SDL_GetTicks());
				net_reset();
			}
			scene_set(&scene_net);
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

static void draw(void) {
	fill_rect(0, 0, P.w, P.h, BLACK);
	bool dirty = !S.tex || (S.pressed && S.t - S.pressed <= 6);
	for (int i = 0; i < S.nanim; ++i) {
		int st = anim_step(&S.anim[i], S.t);
		if (st != S.key[i]) {
			S.key[i] = st;
			memcpy(S.pal + S.anim[i].dest, R.data + S.anim[i].step[st], S.anim[i].count);
			dirty = true;
		}
	}
	if (dirty) render_bg();
	int x0 = P.core_x, y0 = P.core_y;
	SDL_Rect src = { 0, 0, CORE_W, CORE_H }, dst = { x0, y0, CORE_W, CORE_H };
	SDL_RenderCopy(P.renderer, S.tex, &src, &dst);

	/* the copyright line: 8 OBJs of 32x32 along the bottom */
	uint32_t copy = gfx_lz_ref(T.copy_tiles) + 4;
	for (int i = 0; i < 8; ++i) rom_tiles(copy + (uint32_t)i * 16 * 32, T.copy_pal, x0 + i * 32, y0 + 126, 4, 4, 0);

	uint32_t text = gfx_lz_ref(T.text_tiles) + 4 - 32; /* OBJ tile 1 is the block's first */
	/* PRESS START blinks from the start; after START it finishes its lit phase */
	int phase = (S.t / BLINK) & 1;
	bool lit = !phase && (!S.pressed || S.t / BLINK == S.pressed / BLINK) && !S.menu;
	if (lit) {
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
		/* the arrow cycles three frames, 6 frames each */
		int f = ((S.t - S.menu) / 6) % 3;
		rom_tiles(T.arrow + (uint32_t)f * 4 * 32, T.arrow_pal, x0 + 73, y0 + 113 + S.cursor * 16, 2, 2, 0);
	}
	if (S.leaving) { P.fx_fade = S.leaving; P.fx_fade_color = BLACK; }
}

const Scene scene_title = { "title", enter, update, draw, NULL };
