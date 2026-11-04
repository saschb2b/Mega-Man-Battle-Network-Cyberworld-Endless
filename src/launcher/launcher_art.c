/* launcher_art.h. The cartridge is our own shape, after the Game Boy
 * Advance's in its proportions (wider than tall, a grip along its top, a
 * label in a recess, the connector's slot below), never traced: its shell
 * and its label's colours are drawn into pixels once and kept as a
 * texture; the game's face (its mugshot, BN6's MegaMan, BN5 Team
 * Colonel's Colonel, BN4 Red Sun's Duo), the name and the cursor are
 * drawn over it each frame. */
#include "launcher_art.h"

#include <stdint.h>
#include <string.h>

#include "gfx.h"
#include "launcher_text.h"
#include "pixfont.h"
#include "platform.h"
#include "rom.h"
#include "second_frame.h"
#include "text.h"

#define OUTLINE 0xFF182838u
#define METAL_HI 0xFFC6DEEFu
#define METAL 0xFFA5BDD6u
#define METAL_LO 0xFF638494u
#define GRIP 0xFF8CA5BDu
#define RECESS 0xFF3C5064u
#define SLOT_DARK 0xFF182838u
#define PIN 0xFFC69628u
#define FACE_BACK 0xFF10364Au

/* a label's colours: its top and its foot, and its light stripes (BN6's
 * green, BN5's blue, BN4's red) */
static const uint32_t label_top[SLOTS] = { 0xFF08BD73u, 0xFF319CDEu, 0xFFE74A31u };
static const uint32_t label_foot[SLOTS] = { 0xFF008C52u, 0xFF1063A5u, 0xFFA51810u };
static const uint32_t label_stripe[SLOTS] = { 0xFF4AE773u, 0xFF6BDEFFu, 0xFFFF8C6Bu };

int art_cart_w(int size) { return size == CART_LARGE ? CART_LARGE_W : size == CART_SMALL ? CART_W : CART_COMPACT_W; }
int art_cart_h(int size) { return size == CART_LARGE ? CART_LARGE_H : size == CART_SMALL ? CART_H : CART_COMPACT_H; }

/* where the label is in a cartridge of each size */
typedef struct { int x, y, w, h, face_w, face_h; } Label;
static Label label_of(int size) {
	if (size == CART_LARGE) return (Label){ 8, 10, 96, 52, 40, 48 };
	return size == CART_SMALL ? (Label){ 6, 9, 80, 38, 34, 34 } : (Label){ 5, 8, 60, 29, 26, 25 };
}

typedef struct { uint32_t *px; int w, h; } Img;

static void put(Img *im, int x, int y, uint32_t c) {
	if (x >= 0 && y >= 0 && x < im->w && y < im->h) im->px[y * im->w + x] = c;
}

static void box(Img *im, int x, int y, int w, int h, uint32_t c) {
	for (int j = 0; j < h; ++j)
		for (int i = 0; i < w; ++i) put(im, x + i, y + j, c);
}

/* the shell's outline: its top corners round, its foot's a pixel cut */
static bool inside(int x, int y, int w, int h) {
	if (x < 0 || y < 0 || x >= w || y >= h) return false;
	int dx = x < w / 2 ? x : w - 1 - x;
	if (y < 2 && dx + y < 2) return false;
	if (y == h - 1 && dx == 0) return false;
	return true;
}

static uint32_t mix(uint32_t a, uint32_t b, int k, int n) {
	uint32_t r = 0xFF000000u;
	for (int s = 0; s < 24; s += 8) {
		int ca = (int)(a >> s & 255), cb = (int)(b >> s & 255);
		r |= (uint32_t)(ca + (cb - ca) * k / (n ? n : 1)) << s;
	}
	return r;
}

static void shell(Img *im, int slot, int size) {
	int w = im->w, h = im->h;
	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x) {
			if (!inside(x, y, w, h)) continue;
			bool edge = !inside(x - 1, y, w, h) || !inside(x + 1, y, w, h) || !inside(x, y - 1, w, h) || !inside(x, y + 1, w, h);
			uint32_t c = edge ? OUTLINE : METAL;
			if (!edge && (y == 1 || x == 1)) c = METAL_HI;
			if (!edge && (y == h - 2 || x == w - 2)) c = METAL_LO;
			put(im, x, y, c);
		}
	/* the grip along the top, at the right: ridges a pixel apart */
	for (int x = w * 5 / 8; x < w - 5; ++x)
		for (int y = 2; y < 6; ++y) put(im, x, y, (x & 1) ? METAL_LO : GRIP);
	Label l = label_of(size);
	/* the label in its recess: its game's colours, top to foot, light stripes slanting up */
	box(im, l.x - 1, l.y - 1, l.w + 2, l.h + 2, RECESS);
	for (int y = 0; y < l.h; ++y)
		for (int x = 0; x < l.w; ++x)
			put(im, l.x + x, l.y + y, (x + y) % 12 < 2 ? label_stripe[slot] : mix(label_top[slot], label_foot[slot], y, l.h));
	box(im, l.x + 2, l.y + 2, l.face_w, l.face_h, FACE_BACK);
	/* the foot: a groove over it, the connector's slot with its pins, an arrow to the left */
	int foot = h - 8;
	box(im, 3, foot - 1, w - 6, 1, METAL_LO);
	box(im, w / 2 - 16, h - 5, 32, 4, SLOT_DARK);
	for (int x = w / 2 - 14; x < w / 2 + 14; x += 2) box(im, x, h - 4, 1, 3, PIN);
	for (int r = 0; r < 3; ++r) box(im, 7 + r, foot + 1 + r, 5 - 2 * r, 1, METAL_LO);
}

static SDL_Texture *shells[SLOTS][CART_SIZES];
static int shells_lost;

static SDL_Texture *shell_texture(int slot, int size) {
	if (shells_lost != P.textures_lost) {
		/* (the renderer reset: textures gone; their handles freed by SDL) */
		memset(shells, 0, sizeof shells);
		shells_lost = P.textures_lost;
	}
	SDL_Texture **t = &shells[slot][size];
	if (*t) return *t;
	static uint32_t px[CART_LARGE_W * CART_LARGE_H];
	Img im = { px, art_cart_w(size), art_cart_h(size) };
	memset(px, 0, sizeof px);
	shell(&im, slot, size);
	*t = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, im.w, im.h);
	if (!*t) return NULL;
	SDL_UpdateTexture(*t, NULL, px, im.w * 4);
	SDL_SetTextureBlendMode(*t, SDL_BLENDMODE_BLEND);
	return *t;
}

/* the game's face from its own ROM: BN6's MegaMan, an extra one's its
 * layout's (rom.h's XRomLayout.face) */
static Sprite *face_of(int slot) {
	if (slot == SLOT_BN6) return R.data ? sprite_get(SPR_MUGSHOT, FACE_MEGAMAN) : NULL;
	const XRom *x = slot > 0 && slot < SLOTS ? &XR[slot - 1] : NULL;
	return x && x->data ? sprite_get_rom(x->data, x->layout->sprite_lists, SPR_MUGSHOT, x->layout->face) : NULL;
}

void art_cartridge(int slot, SDL_Rect r, int size, int flash) {
	SDL_Texture *t = shell_texture(slot, size);
	if (t) SDL_RenderCopy(P.renderer, t, NULL, &r);
	Label l = label_of(size);
	bool large = size == CART_LARGE;
	SDL_Rect win = { r.x + l.x + 2, r.y + l.y + 2, l.face_w, l.face_h };
	Sprite *face = face_of(slot);
	if (face) {
		/* (a mugshot's origin is its middle; a small window shows its face, the shoulders cut) */
		SDL_RenderSetClipRect(P.renderer, &win);
		sprite_draw_frame(face, 0, 0, win.x + l.face_w / 2, win.y + 24 - (48 - l.face_h) / 2 + (large ? 0 : 2), false, 0, 0);
		SDL_RenderSetClipRect(P.renderer, NULL);
	}
	SDL_Color white = PET_WHITE, shade = rgba(0, 49, 74, 255);
	int tx = win.x + win.w + 3, tw = r.x + l.x + l.w - 2 - tx, cx = tx + tw / 2;
	if (large) {
		pixfont_draw(cx, r.y + l.y + 6, slot_short(slot), white, shade, PIXFONT_CENTER, 2);
		pixfont_draw(cx, r.y + l.y + 28, slot_label(slot, 0, true), white, shade, PIXFONT_CENTER, 1);
		pixfont_draw(cx, r.y + l.y + 39, slot_label(slot, 1, true), white, shade, PIXFONT_CENTER, 1);
	} else if (size == CART_SMALL) {
		pixfont_draw(cx, r.y + l.y + 4, slot_short(slot), white, shade, PIXFONT_CENTER, 2);
		pixfont_draw(cx, r.y + l.y + 24, slot_label(slot, 0, false), white, shade, PIXFONT_CENTER, 1);
	} else pixfont_draw(cx, r.y + l.y + 4, slot_short(slot), white, shade, PIXFONT_CENTER, 1);   /* (a compact label: its short name alone) */
	if (flash > 0) fill_rect(r.x + l.x, r.y + l.y, l.w, l.h, rgba(255, 255, 255, flash > 255 ? 255 : flash));
}

void art_open_spot(SDL_Rect r, int phase, bool lit) {
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, rgba(10, 58, 80, 255));
	SDL_Color c = lit ? PET_GOLD : PET_CYAN;
	/* (dashes of three, a gap of two, round the edge, marching) */
	int per = 2 * (r.w + r.h) - 4, k = 0;
	SDL_Rect dash[512];
	int n = 0;
	for (int i = 0; i < per && n < 512; ++i) {
		int x, y;
		if (i < r.w) { x = r.x + i; y = r.y; }
		else if (i < r.w + r.h - 1) { x = r.x + r.w - 1; y = r.y + i - r.w + 1; }
		else if (i < 2 * r.w + r.h - 2) { x = r.x + r.w - 2 - (i - r.w - r.h + 1); y = r.y + r.h - 1; }
		else { x = r.x; y = r.y + r.h - 2 - (i - 2 * r.w - r.h + 2); }
		k = (i + phase) % 5;
		if (k < 3) dash[n++] = (SDL_Rect){ x, y, 1, 1 };
	}
	fill_rects(dash, n, c);
}

void art_plus(int cx, int cy, SDL_Color c) {
	fill_rect(cx - 4, cy - 1, 9, 3, c);
	fill_rect(cx - 1, cy - 4, 3, 9, c);
}

void art_cursor(SDL_Rect r, int t) {
	int k = t / 16 % 2;
	SDL_Color c = k ? PET_GOLD : rgba(255, 247, 165, 255);
	SDL_Rect f[4] = { { r.x - 3, r.y - 3, r.w + 6, 1 }, { r.x - 3, r.y + r.h + 2, r.w + 6, 1 },
		{ r.x - 3, r.y - 2, 1, r.h + 4 }, { r.x + r.w + 2, r.y - 2, 1, r.h + 4 } };
	fill_rects(f, 4, c);
}

void art_button(SDL_Rect r, const char *label, bool gold, bool lit, bool off, bool icon) {
	SDL_Color edge, fill, top, foot, ink;
	if (off) { edge = PET_SLOT_EDGE; fill = rgba(40, 78, 98, 255); top = fill; foot = fill; ink = PET_DIM; }
	else if (gold) { edge = rgba(198, 90, 0, 255); fill = PET_GOLD; top = rgba(255, 247, 165, 255); foot = rgba(247, 165, 0, 255); ink = rgba(0, 49, 74, 255); }
	else { edge = lit ? PET_GOLD : PET_CYAN; fill = PET_SLOT; top = rgba(33, 123, 132, 255); foot = PET_SLOT_EDGE; ink = PET_WHITE; }
	if (lit) fill_rect(r.x - 2, r.y - 2, r.w + 4, r.h + 4, gold ? PET_WHITE : PET_GOLD);
	fill_rect(r.x, r.y, r.w, r.h, edge);
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, fill);
	fill_rect(r.x + 1, r.y + 1, r.w - 2, 1, top);
	fill_rect(r.x + 1, r.y + r.h - 2, r.w - 2, 1, foot);
	int w = pixfont_width(label, 1) + (icon ? 7 : 0), x = r.x + (r.w - w) / 2, y = r.y + (r.h - PIXFONT_CAP) / 2;
	SDL_Color none = { 0, 0, 0, 0 }, shade = gold && !off ? none : rgba(0, 49, 74, 255);
	pixfont_draw(x, y, label, ink, shade, PIXFONT_LEFT, 1);
	if (icon) pixfont_draw(x + w - 4, y, PIXFONT_PLAY, ink, shade, PIXFONT_LEFT, 1);
}

int art_chip_width(const char *word, bool check) { return pixfont_width(word, 1) + (check ? 9 : 0) + 6; }

void art_chip(int x, int y, const char *word, SDL_Color fill, SDL_Color ink, bool check) {
	int w = art_chip_width(word, check);
	fill_rect(x + 1, y, w - 2, CHIP_H, fill);
	fill_rect(x, y + 1, w, CHIP_H - 2, fill);
	SDL_Color none = { 0, 0, 0, 0 };
	if (check) pixfont_draw(x + 3, y + 2, PIXFONT_CHECK, ink, none, PIXFONT_LEFT, 1);
	pixfont_draw(x + 3 + (check ? 9 : 0), y + 2, word, ink, none, PIXFONT_LEFT, 1);
}
