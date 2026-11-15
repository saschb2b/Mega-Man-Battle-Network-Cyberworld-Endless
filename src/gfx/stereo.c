#include "stereo.h"

#include <math.h>
#include <string.h>

#include "gba_tiles.h"

/* Where BN6 keeps what is no part of the world: its HP in the corner (as
 * voxel_lift.c's) */
#define HP_RIGHT  48
#define HP_BOTTOM 24
#define CELLS_X   (STEREO_W / 8)
#define CELLS_Y   (STEREO_H / 8)

static bool hud_box(const GbaBox *b, int hud_rows) {
	if (!b->w) return false;
	if (b->x + b->w <= HP_RIGHT && b->y + b->h <= HP_BOTTOM) return true;
	return hud_rows > 0 ? b->y >= hud_rows : hud_rows < 0 && b->y + b->h <= -hud_rows;
}

/* Whether a tile of a background's has a pixel (`seen`: each tile's answer
 * so far, 0 not asked, 1 empty, 2 not) */
static bool tile_drawn(const uint8_t *vram, const GbaBg *b, unsigned tile, uint8_t *seen) {
	tile &= 0x3FFu;
	if (!seen[tile]) {
		unsigned size = b->wide ? 64u : 32u, at = (b->chars + tile * size) & 0xFFFFu;
		seen[tile] = 1;
		for (unsigned i = 0; i < size && at + i < 0x10000u; ++i)
			if (vram[at + i]) { seen[tile] = 2; break; }
	}
	return seen[tile] == 2;
}

/* The screen entry a background's own map has at its pixel (px, py) */
static unsigned entry_at(const uint8_t *vram, const GbaBg *b, unsigned px, unsigned py) {
	px &= (b->size & 1u) ? 511u : 255u;
	py &= (b->size & 2u) ? 511u : 255u;
	unsigned block = (px >> 8) + (py >> 8) * ((b->size & 1u) ? 2u : 1u);
	return gba_rd16(vram, (b->screen + block * 0x800u + ((py >> 3 & 31u) * 32u + (px >> 3 & 31u)) * 2u) & 0xFFFEu);
}

/* The frame's cells a HUD background has a pixel in: each cell asks the
 * map's tiles under its corners (a scrolled background's cell lies over
 * up to four) */
static void hud_cells(const GbaVideo *v, const GbaBg *b, StereoFlat *out) {
	uint8_t seen[1024];
	memset(seen, 0, sizeof seen);
	/* (scrolled by whole tiles, as BG0 is: a cell over one tile) */
	int corners = (b->hofs | b->vofs) & 7u ? 4 : 1;
	for (int cy = 0; cy < CELLS_Y; ++cy)
		for (int cx = 0; cx < CELLS_X; ++cx)
			for (int k = 0; k < corners && !out->cell[cy][cx]; ++k) {
				unsigned px = (unsigned)(cx * 8 + (k & 1) * 7) + b->hofs, py = (unsigned)(cy * 8 + (k >> 1) * 7) + b->vofs;
				if (tile_drawn(v->vram, b, entry_at(v->vram, b, px, py), seen)) out->cell[cy][cx] = 1;
			}
}

void stereo_flat(const GbaVideo *v, int hud_rows, StereoFlat *out) {
	memset(out, 0, sizeof *out);
	if (gba_rd16(v->io, 0) & 0x1000u) {
		GbaBox box[GBA_OBJS];
		gba_obj_boxes(v, box);
		for (int n = 0; n < GBA_OBJS && out->nrects < STEREO_RECTS; ++n) {
			if (!hud_box(&box[n], hud_rows)) continue;
			int k = out->nrects++;
			out->rect[k].x0 = box[n].x;
			out->rect[k].y0 = box[n].y;
			out->rect[k].x1 = (int16_t)(box[n].x + box[n].w);
			out->rect[k].y1 = (int16_t)(box[n].y + box[n].h);
		}
	}
	GbaBg bg0 = gba_bg(v->io, 0), bg3 = gba_bg(v->io, 3);
	if (bg0.on) hud_cells(v, &bg0, out);
	if (bg3.on && gba_bg3_front(v->io)) hud_cells(v, &bg3, out);
	int cells = 0;
	for (int cy = 0; cy < CELLS_Y; ++cy)
		for (int cx = 0; cx < CELLS_X; ++cx) cells += out->cell[cy][cx];
	out->whole = cells * 4 >= CELLS_X * CELLS_Y;
}

void stereo_row(const StereoFlat *f, int y, StereoRow *out) {
	memset(out, 0, sizeof *out);
	if (f->whole) {
		out->n = 1;
		out->flat[0] = 1;
		return;
	}
	/* (a row with nothing flat: the world's whole, at once) */
	bool any = memchr(f->cell[y / 8], 1, CELLS_X) != NULL;
	for (int r = 0; !any && r < f->nrects; ++r) any = y >= f->rect[r].y0 && y < f->rect[r].y1;
	if (!any) {
		out->n = 1;
		return;
	}
	uint8_t flat[STEREO_W];
	for (int cx = 0; cx < CELLS_X; ++cx) memset(&flat[cx * 8], f->cell[y / 8][cx], 8);
	for (int r = 0; r < f->nrects; ++r) {
		if (y < f->rect[r].y0 || y >= f->rect[r].y1) continue;
		int x0 = f->rect[r].x0 < 0 ? 0 : f->rect[r].x0, x1 = f->rect[r].x1 > STEREO_W ? STEREO_W : f->rect[r].x1;
		if (x1 > x0) memset(&flat[x0], 1, (size_t)(x1 - x0));
	}
	for (int x = 0; x < STEREO_W; ++x) {
		if (x && flat[x] == flat[x - 1]) continue;
		/* (past the spans a row may have, the rest of it flat) */
		if (out->n == STEREO_SPANS) break;
		bool last = out->n == STEREO_SPANS - 1;
		if (last && !flat[x] && out->n && out->flat[out->n - 1]) continue;
		out->x[out->n] = (int16_t)x;
		out->flat[out->n++] = (uint8_t)(flat[x] || last);
	}
}

/* Two boxes that touch or overlap (a pixel apart counts) */
static bool touch(const StereoBox *a, const StereoBox *b) {
	return a->x0 <= b->x1 && b->x0 <= a->x1 && a->y0 <= b->y1 && b->y0 <= a->y1;
}

/* Bodies joined while any two touch */
static void bodies_join(StereoBodies *out) {
	for (int i = 0; i < out->n; ++i)
		for (int j = i + 1; j < out->n; ++j) {
			StereoBox *a = &out->box[i], *b = &out->box[j];
			if (!touch(a, b)) continue;
			if (b->x0 < a->x0) a->x0 = b->x0;
			if (b->y0 < a->y0) a->y0 = b->y0;
			if (b->x1 > a->x1) a->x1 = b->x1;
			if (b->y1 > a->y1) a->y1 = b->y1;
			out->box[j] = out->box[--out->n];
			j = i;   /* (the grown box asked of them all again) */
		}
}

void stereo_sprites(const GbaVideo *v, int hud_rows, uint8_t *mask, StereoBodies *out) {
	GbaBox box[GBA_OBJS];
	uint8_t want[GBA_OBJS], drew[GBA_OBJS];
	out->n = 0;
	if (!(gba_rd16(v->io, 0) & 0x1000u)) { memset(mask, 0, STEREO_N); return; }
	gba_obj_boxes(v, box);
	for (int n = 0; n < GBA_OBJS; ++n) want[n] = box[n].w && !hud_box(&box[n], hud_rows);
	gba_obj_pixels(v, want, mask, drew);
	for (int n = 0; n < GBA_OBJS; ++n) {
		if (!drew[n]) continue;
		StereoBox b = { box[n].x < 0 ? 0 : box[n].x, box[n].y < 0 ? 0 : box[n].y, (int16_t)(box[n].x + box[n].w), (int16_t)(box[n].y + box[n].h) };
		if (b.x1 > STEREO_W) b.x1 = STEREO_W;
		if (b.y1 > STEREO_H) b.y1 = STEREO_H;
		/* (past the most, the rest joined to the last) */
		if (out->n < STEREO_BODIES) out->box[out->n++] = b;
		else {
			StereoBox *l = &out->box[STEREO_BODIES - 1];
			if (b.x0 < l->x0) l->x0 = b.x0;
			if (b.y0 < l->y0) l->y0 = b.y0;
			if (b.x1 > l->x1) l->x1 = b.x1;
			if (b.y1 > l->y1) l->y1 = b.y1;
		}
	}
	bodies_join(out);
}

float stereo_depth(int row) {
	float d = (float)(STEREO_H - 1 - row) / (float)(STEREO_H - 1);
	return d < 0 ? 0 : d > 1 ? 1 : d;
}

/* (the floor tilted from the screen at the frame's last row to three
 * quarters of the depth at its first, and the moving background tilted
 * behind it, from half to all of it: a floor tilted from 0.15 to 0.6 and
 * the background a wall at 1, the owner saw it flat, 2026-11-15) */
#define MAP_NEAR 0.0f
#define MAP_FAR  0.75f
#define SKY_NEAR 0.5f
#define SKY_FAR  1.0f
float stereo_map_depth(int row) { return MAP_NEAR + (MAP_FAR - MAP_NEAR) * stereo_depth(row); }
float stereo_sky_depth(int row) { return SKY_NEAR + (SKY_FAR - SKY_NEAR) * stereo_depth(row); }

float stereo_body_depth(const StereoBox *b) { return stereo_map_depth(b->y1 - 1); }

float stereo_shift(int eye, float sep, float depth) { return 0.5f * (float)eye * sep * depth; }

/* An eye's picture being drawn: its row (in its pixels) and scale */
typedef struct { uint32_t *out; int w, oy, scale; } Eye;

/* A colour over the frame's columns `from` to `to` (fractions rounded to the eye's pixels) */
static void paint(const Eye *e, float from, float to, uint32_t c) {
	int o0 = (int)floorf(from * (float)e->scale + 0.5f), o1 = (int)floorf(to * (float)e->scale + 0.5f);
	for (int sy = 0; sy < e->scale; ++sy)
		for (int ox = o0 < 0 ? 0 : o0; ox < o1 && ox < e->w; ++ox) e->out[(e->oy + sy) * e->w + ox] = c;
}

/* The row's pixels of the parts `parts` (a bit each) from column `a` to
 * `b`, moved by `s` */
static void row_pass(const uint32_t *line, const uint8_t *part, int parts, int a, int b, float s, const Eye *e) {
	for (int x = a; x < b; ++x)
		if (parts >> part[x] & 1) paint(e, (float)x + s, (float)x + 1 + s, line[x]);
}

/* The gaps the map leaves beside the row's flat spans, filled from its edge */
static void gap_fills(const uint32_t *line, const StereoRow *r, float s, const Eye *e) {
	for (int i = 0; i < r->n; ++i) {
		int a = r->x[i], b = i + 1 < r->n ? r->x[i + 1] : STEREO_W;
		if (r->flat[i]) continue;
		if (i > 0 && s > 0) paint(e, (float)a, (float)a + s, line[a]);
		if (i + 1 < r->n && s < 0) paint(e, (float)b + s, (float)b, line[b - 1]);
	}
}

void stereo_compose(const StereoPicture *p, int eye, float sep, int scale, uint32_t *out) {
	int w = STEREO_W * scale, all = 1 << STEREO_MAP | 1 << STEREO_SKY | 1 << STEREO_SPRITE;
	memset(out, 0, sizeof *out * (size_t)w * (size_t)(STEREO_H * scale));
	for (int y = 0; y < STEREO_H; ++y) {
		float sky = stereo_shift(eye, sep, stereo_sky_depth(y));
		const uint32_t *line = &p->picture[y * STEREO_W];
		const uint8_t *part = &p->part[y * STEREO_W];
		Eye e = { out, w, y * scale, scale };
		float map = stereo_shift(eye, sep, stereo_map_depth(y));
		/* (the background at the map's depth first: where it is moved deep, the gap beside the map's edge is its own) */
		if (p->sky) {
			row_pass(line, part, 1 << STEREO_SKY, 0, STEREO_W, map, &e);
			row_pass(line, part, 1 << STEREO_SKY, 0, STEREO_W, sky, &e);
		}
		row_pass(line, part, p->sky ? all & ~(1 << STEREO_SKY) : all, 0, STEREO_W, map, &e);
		StereoRow r;
		stereo_row(p->flat, y, &r);
		gap_fills(line, &r, map, &e);
	}
	for (int k = 0; k < p->bodies->n; ++k) {
		const StereoBox *b = &p->bodies->box[k];
		float s = stereo_shift(eye, sep, stereo_body_depth(b));
		for (int y = b->y0; y < b->y1; ++y)
			row_pass(&p->picture[y * STEREO_W], &p->part[y * STEREO_W], 1 << STEREO_SPRITE, b->x0, b->x1, s, &(Eye){ out, w, y * scale, scale });
	}
	for (int y = 0; y < STEREO_H; ++y) {
		StereoRow r;
		stereo_row(p->flat, y, &r);
		for (int i = 0; i < r.n; ++i)
			if (r.flat[i]) row_pass(&p->picture[y * STEREO_W], &p->part[y * STEREO_W], all, r.x[i], i + 1 < r.n ? r.x[i + 1] : STEREO_W, 0.0f, &(Eye){ out, w, y * scale, scale });
	}
}
