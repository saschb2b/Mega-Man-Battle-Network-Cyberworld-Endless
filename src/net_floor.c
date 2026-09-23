/* Net floors built from the original areas' own panels.
 *
 * The internet maps are drawn as 64x32 isometric panels whose rims and
 * slabs depend on the neighbouring panels. At load the engine renders one
 * original map per biome from the player's ROM, splits every panel into four
 * corner pieces (the pixels nearer to that panel than to any neighbour,
 * nearest to that corner) and keeps, per corner and per arrangement of the
 * three neighbours that corner touches, the piece the map uses most often.
 * A generated layer is then drawn from those pieces. */
#include "net_floor.h"

#include <stdlib.h>
#include <string.h>

#include "gfx.h"
#include "platform.h"
#include "rom.h"

#define WIN_W 144   /* piece window: 72 either side of the panel centre */
#define WIN_H 112   /* 48 above, 64 below (slab) */
#define WIN_X 72
#define WIN_Y 48
#define MAX_CANDS 48

typedef struct {
	SDL_Texture *tex;
	int x, y, w, h;   /* piece bounds relative to the panel centre */
} Piece;

typedef struct {
	bool tried, ok;
	Piece piece[4][8];
	int use[4][8];    /* key -> learned key (nearest when missing) */
} Floor;

static Floor floors[FLOOR_AREAS];

/* Neighbour offsets in the rendered map, in NB_* order. */
static const int nb_off[8][2] = { { -32, -16 }, { 0, -32 }, { 32, -16 }, { 64, 0 }, { 32, 16 }, { 0, 32 }, { -32, 16 }, { -64, 0 } };
/* The three neighbours each corner (N, E, S, W) touches. */
static const int corner_nb[4][3] = { { NB_NW, NB_N, NB_NE }, { NB_NE, NB_E, NB_SE }, { NB_SE, NB_S, NB_SW }, { NB_SW, NB_W, NB_NW } };

typedef struct {
	uint32_t *px;     /* ARGB, alpha 0 where nothing is drawn */
	int w, h;
} MapImage;

/* Render map (group, number) from its descriptor: two tile sets, 16
 * palettes, then the LZ77 tile map with its layers drawn back to front. */
static bool render_map(int group, int number, MapImage *out) {
	uint32_t table = rom_u32(R.layout->net_maps + (uint32_t)(group - 0x80) * 4);
	if (!rom_is_ptr(table)) return false;
	uint32_t d = rom_off(table) + (uint32_t)number * 12;
	uint32_t ts = rom_u32(d), pal = rom_u32(d + 4), tm = rom_u32(d + 8);
	if (!rom_is_ptr(ts) || !rom_is_ptr(pal) || !rom_is_ptr(tm)) return false;
	ts = rom_off(ts); pal = rom_off(pal) + 4; tm = rom_off(tm);
	uint8_t *vram = calloc(0x10000, 1);
	if (!vram) return false;
	for (int k = 0; k < 2; ++k) {
		uint32_t wc = rom_u32(ts + (uint32_t)k * 12), off = rom_u32(ts + (uint32_t)k * 12 + 4), vo = rom_u32(ts + (uint32_t)k * 12 + 8);
		if (!wc || ts + off >= ROM_SIZE) continue;
		size_t n = 0;
		uint8_t *t = lz77_decompress(R.data + ts + off, ROM_SIZE - (ts + off), &n);
		if (!t) continue;
		size_t want = (size_t)wc * 4;
		if (want > n) want = n;
		if (vo < 0x10000) memcpy(vram + vo, t, want < 0x10000 - vo ? want : 0x10000 - vo);
		free(t);
	}
	uint32_t colors[256];
	for (int i = 0; i < 256; ++i) colors[i] = bgr555(rom_u16(pal + (uint32_t)i * 2));
	int w = R.data[tm], h = R.data[tm + 1];
	size_t n = 0;
	uint8_t *m = lz77_decompress(R.data + tm + 12, ROM_SIZE - (tm + 12), &n);
	if (!m || w <= 0 || h <= 0) { free(m); free(vram); return false; }
	int layers = (int)(n / ((size_t)w * h * 2));
	out->w = w * 8; out->h = h * 8;
	out->px = calloc((size_t)out->w * out->h, 4);
	if (!out->px) { free(m); free(vram); return false; }
	for (int layer = layers - 1; layer >= 0; --layer)
		for (int ty = 0; ty < h; ++ty)
			for (int tx = 0; tx < w; ++tx) {
				size_t at = ((size_t)layer * w * h + (size_t)ty * w + tx) * 2;
				uint16_t e = (uint16_t)(m[at] | m[at + 1] << 8);
				int t = e & 0x3FF;
				if (!t) continue;
				const uint8_t *tile = vram + t * 32;
				for (int y = 0; y < 8; ++y)
					for (int x = 0; x < 8; ++x) {
						uint8_t v = tile[y * 4 + x / 2];
						int ci = (x & 1) ? v >> 4 : v & 15;
						if (!ci) continue;
						int X = (e & 0x400) ? 7 - x : x, Y = (e & 0x800) ? 7 - y : y;
						out->px[(size_t)(ty * 8 + Y) * out->w + tx * 8 + X] = colors[(e >> 12) * 16 + ci];
					}
			}
	free(m);
	free(vram);
	return true;
}

static bool occupied(const MapImage *m, int x, int y) {
	return x >= 0 && y >= 0 && x < m->w && y < m->h && (m->px[(size_t)y * m->w + x] >> 24);
}

/* Hue bucket (0-11) of the panel's middle, 12 for greys. */
static int style_of(const MapImage *m, int cx, int cy) {
	long r = 0, g = 0, b = 0, n = 0;
	for (int y = cy - 4; y < cy + 4; ++y)
		for (int x = cx - 8; x < cx + 8; ++x) {
			if (x < 0 || y < 0 || x >= m->w || y >= m->h) continue;
			uint32_t c = m->px[(size_t)y * m->w + x];
			r += (c >> 16) & 255; g += (c >> 8) & 255; b += c & 255; ++n;
		}
	if (!n) return 12;
	float fr = (float)r / n, fg = (float)g / n, fb = (float)b / n;
	float mx = fr > fg ? (fr > fb ? fr : fb) : (fg > fb ? fg : fb);
	float mn = fr < fg ? (fr < fb ? fr : fb) : (fg < fb ? fg : fb);
	if (mx <= 0 || (mx - mn) / mx <= 0.25f) return 12;
	float d = mx - mn, hue;
	if (mx == fr) hue = (fg - fb) / d;
	else if (mx == fg) hue = 2 + (fb - fr) / d;
	else hue = 4 + (fr - fg) / d;
	hue /= 6;
	if (hue < 0) hue += 1;
	int k = (int)(hue * 12);
	return k > 11 ? 11 : k;
}

/* The pixels of panel (cx, cy) nearest to it (|dx| + 2|dy|, up to 1.6 panel
 * radii) and nearest to its corner `corner`, in a WIN_W x WIN_H window. */
static void cut_piece(const MapImage *m, int cx, int cy, int corner, uint32_t *win) {
	memset(win, 0, WIN_W * WIN_H * 4);
	bool nb[8];
	for (int k = 0; k < 8; ++k) nb[k] = occupied(m, cx + nb_off[k][0], cy + nb_off[k][1]);
	for (int wy = 0; wy < WIN_H; ++wy) {
		int y = cy - WIN_Y + wy, dy = wy - WIN_Y;
		if (y < 0 || y >= m->h) continue;
		for (int wx = 0; wx < WIN_W; ++wx) {
			int x = cx - WIN_X + wx, dx = wx - WIN_X;
			if (x < 0 || x >= m->w) continue;
			int adx = abs(dx), ady = abs(dy);
			int dist = adx + 2 * ady;
			if (dist > 51) continue;
			int c = 2 * ady >= adx ? (dy < 0 ? 0 : 2) : (dx > 0 ? 1 : 3);
			if (c != corner) continue;
			bool own = true;
			for (int k = 0; k < 8 && own; ++k) {
				if (!nb[k]) continue;
				int ox = nb_off[k][0], oy = nb_off[k][1];
				int d2 = abs(dx - ox) + 2 * abs(dy - oy);
				if (d2 < dist || (d2 == dist && !(oy > 0 || (oy == 0 && ox > 0)))) own = false;
			}
			if (own) win[wy * WIN_W + wx] = m->px[(size_t)y * m->w + x];
		}
	}
}

static uint64_t hash_win(const uint32_t *win) {
	uint64_t h = 1469598103934665603ull;
	for (int i = 0; i < WIN_W * WIN_H; ++i) { h ^= win[i]; h *= 1099511628211ull; }
	return h;
}

typedef struct { uint64_t hash; int count, cx, cy; } Cand;

static bool learn(int area, Floor *f) {
	const __typeof__(R.layout->net_area[0]) *a = &R.layout->net_area[area];
	MapImage m;
	if (!render_map(a->group, a->number, &m)) return false;
	/* panels of the chosen styles */
	uint8_t *good = calloc((size_t)m.w * m.h, 1);
	static Cand cand[4][8][MAX_CANDS];
	static int ncand[4][8];
	memset(ncand, 0, sizeof ncand);
	uint32_t *win = malloc(WIN_W * WIN_H * 4);
	if (!good || !win) { free(good); free(win); free(m.px); return false; }
	for (int y = a->oy % 16; y < m.h; y += 16) {
		int row = (y - a->oy) / 16;
		for (int x = (a->ox + ((row & 1) ? 32 : 0)) % 64; x < m.w; x += 64)
			if (occupied(&m, x, y) && (a->styles >> style_of(&m, x, y) & 1)) good[(size_t)y * m.w + x] = 1;
	}
	for (int cy = 0; cy < m.h; ++cy)
		for (int cx = 0; cx < m.w; ++cx) {
			if (!good[(size_t)cy * m.w + cx]) continue;
			bool nb[8], ok[8];
			for (int k = 0; k < 8; ++k) {
				int x = cx + nb_off[k][0], y = cy + nb_off[k][1];
				nb[k] = occupied(&m, x, y);
				ok[k] = !nb[k] || good[(size_t)y * m.w + x];
			}
			for (int c = 0; c < 4; ++c) {
				const int *q = corner_nb[c];
				if (!ok[q[0]] || !ok[q[1]] || !ok[q[2]]) continue;
				int cfg = nb[q[0]] << 2 | nb[q[1]] << 1 | nb[q[2]];
				cut_piece(&m, cx, cy, c, win);
				uint64_t h = hash_win(win);
				int i = 0;
				while (i < ncand[c][cfg] && cand[c][cfg][i].hash != h) ++i;
				if (i < ncand[c][cfg]) cand[c][cfg][i].count++;
				else if (i < MAX_CANDS) cand[c][cfg][ncand[c][cfg]++] = (Cand){ h, 1, cx, cy };
			}
		}
	bool any = false;
	for (int c = 0; c < 4; ++c)
		for (int cfg = 0; cfg < 8; ++cfg) {
			if (!ncand[c][cfg]) continue;
			const Cand *best = &cand[c][cfg][0];
			for (int i = 1; i < ncand[c][cfg]; ++i)
				if (cand[c][cfg][i].count > best->count) best = &cand[c][cfg][i];
			cut_piece(&m, best->cx, best->cy, c, win);
			int x0 = WIN_W, y0 = WIN_H, x1 = -1, y1 = -1;
			for (int y = 0; y < WIN_H; ++y)
				for (int x = 0; x < WIN_W; ++x)
					if (win[y * WIN_W + x] >> 24) {
						if (x < x0) x0 = x;
						if (x > x1) x1 = x;
						if (y < y0) y0 = y;
						if (y > y1) y1 = y;
					}
			if (x1 < 0) continue;
			Piece *p = &f->piece[c][cfg];
			p->x = x0 - WIN_X; p->y = y0 - WIN_Y; p->w = x1 - x0 + 1; p->h = y1 - y0 + 1;
			p->tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, p->w, p->h);
			if (!p->tex) continue;
			SDL_SetTextureBlendMode(p->tex, SDL_BLENDMODE_BLEND);
			SDL_UpdateTexture(p->tex, NULL, win + y0 * WIN_W + x0, WIN_W * 4);
			any = true;
		}
	/* a missing arrangement borrows the nearest learned one, the outer
	 * neighbours (the ones along the rims) counting most */
	for (int c = 0; c < 4; ++c)
		for (int cfg = 0; cfg < 8; ++cfg) {
			int best = -1, bc = 99;
			for (int k = 0; k < 8; ++k) {
				if (!f->piece[c][k].tex) continue;
				int d = cfg ^ k;
				int cost = (d & 4 ? 3 : 0) + (d & 2 ? 1 : 0) + (d & 1 ? 3 : 0);
				if (cost < bc) { bc = cost; best = k; }
			}
			f->use[c][cfg] = best;
		}
	free(win);
	free(good);
	free(m.px);
	return any;
}

bool floor_ready(int area) {
	if (area < 0 || area >= FLOOR_AREAS) return false;
	Floor *f = &floors[area];
	if (!f->tried) {
		f->tried = true;
		f->ok = learn(area, f);
	}
	return f->ok;
}

void floor_draw(int area, int sx, int sy, unsigned nb, SDL_Color mod) {
	if (!floor_ready(area)) return;
	Floor *f = &floors[area];
	for (int c = 0; c < 4; ++c) {
		const int *q = corner_nb[c];
		int cfg = (nb >> q[0] & 1) << 2 | (nb >> q[1] & 1) << 1 | (nb >> q[2] & 1);
		int k = f->use[c][cfg];
		if (k < 0) continue;
		Piece *p = &f->piece[c][k];
		SDL_SetTextureColorMod(p->tex, mod.r, mod.g, mod.b);
		SDL_Rect d = { sx + p->x, sy + p->y, p->w, p->h };
		SDL_RenderCopy(P.renderer, p->tex, NULL, &d);
	}
}
