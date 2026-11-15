/* Scenery: what an original map draws away from its floor (the Aquarium's
 * tanks, coral and shells, the Judge Tree's stumps, Robot Control's
 * capsules), cut out whole and set beside a generated layer's floor.
 *
 * A source tile is scenery when none of its pixels lies where floor, its
 * side faces or the legs under it are drawn: the floor is every panel the
 * walls say is floor (any style) with something drawn at its middle, at its
 * height. Tiles of scenery that touch (8 ways) make one piece. A piece is
 * set only on empty tiles with empty tiles around it, near the floor, so it
 * never covers floor and nothing walks into it: it stands in the void. */
#include "decor.h"

#include <stdlib.h>
#include <string.h>

#define FACE_REACH   44   /* pixels under a floor's bottom edge that belong to it */
#define PAD           3   /* pixels around the floor that belong to it too */
#define MIN_TILES     4
#define MIN_PIXELS  100
#define MAX_SIDE     20   /* tiles */
#define NEAR_FLOOR    3   /* tiles from the floor a piece stands at most */
#define PIECES_MIN    4
#define PIECES_MAX   10
#define TRIES       500

/* Pixels of floor, faces and legs, over the whole map; `reach` pixels
 * under its lower edges are the floor's (0: FACE_REACH). */
static uint8_t *footprint(const AreaSrc *a, int reach) {
	int W = a->tw * 8, H = a->th * 8;
	if (!reach) reach = FACE_REACH;
	uint8_t *fp = calloc((size_t)W * H, 1);
	for (int A = -64; A < 64; ++A)
		for (int B = -64; B < 64; ++B) {
			int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
			if (area_src_walled_floor(a, X, Y) != 1) continue;
			int z = area_src_height(a, X, Y);
			if (z == HEIGHT_UNEVEN) z = 0;
			int cx = area_px(a->tw, X, Y), cy = area_py(a->th, X, Y) - z;
			if (cx < 0 || cy < 0 || cx >= W || cy >= H || !a->idx[(size_t)cy * W + cx]) continue;
			/* the diamond (64 x 32), and under it its faces */
			for (int dx = -32 - PAD; dx <= 32 + PAD; ++dx) {
				int half = (32 - abs(dx)) / 2;
				if (half < 0) half = 0;
				int top = cy - half - PAD, bottom = cy + half + reach + PAD;
				for (int y = top; y <= bottom; ++y) {
					int x = cx + dx;
					if (x >= 0 && y >= 0 && x < W && y < H) fp[(size_t)y * W + x] = 1;
				}
			}
		}
	return fp;
}

/* Whether palette bank `bank` of map a is stored blank, one colour
 * throughout: a bank the map's palette script fills as it runs (BN5's
 * Undernet's bank 12, its skull totems'), whose tiles show one colour
 * here and their art in the game. */
static bool blank_bank(const AreaSrc *a, int bank) {
	for (int k = 2; k < 16; ++k)
		if (a->colors[bank * 16 + k] != a->colors[bank * 16 + 1]) return false;
	return true;
}

/* An opaque tile of one colour: filler, not scenery; but not one in a
 * bank stored blank (its art, coloured as the map runs). */
static bool flat(const AreaSrc *a, int tx, int ty) {
	int W = a->tw * 8;
	size_t at = (size_t)ty * a->tw + tx;
	uint16_t e = a->idx0[(size_t)(ty * 8) * W + tx * 8] || a->layers < 2 ? a->tile[0][at] : a->tile[1][at];
	uint32_t c = area_src_floor_px(a, (size_t)(ty * 8) * W + tx * 8);
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x)
			if (area_src_floor_px(a, (size_t)(ty * 8 + y) * W + tx * 8 + x) != c) return false;
	return !blank_bank(a, e >> 12);
}

static bool drawn_at(const AreaSrc *a, bool bg_in_map, int x, int y) {
	int W = a->tw * 8, H = a->th * 8;
	if (x < 0 || y < 0 || x >= W || y >= H) return false;
	return (bg_in_map ? a->idx0 : a->idx)[(size_t)y * W + x] != 0;
}

/* Whether tile (tx, ty)'s art runs on into its neighbour (dx, dy): a
 * drawn pixel on that side next to a drawn one across it. */
static bool touches(const AreaSrc *a, bool bg_in_map, int tx, int ty, int dx, int dy) {
	for (int k = 0; k < 8; ++k)
		for (int j = -1; j <= 1; ++j) {
			/* the pixels along the shared side (or corner), and across it */
			int x = dx < 0 ? 0 : dx > 0 ? 7 : k, y = dy < 0 ? 0 : dy > 0 ? 7 : k;
			if (dx && dy && k) return false;   /* a corner: one pixel */
			int px = tx * 8 + x, py = ty * 8 + y;
			int qx = px + (dx ? dx : (dy ? j : 0)), qy = py + (dy ? dy : (dx ? j : 0));
			if (drawn_at(a, bg_in_map, px, py) && drawn_at(a, bg_in_map, qx, qy)) return true;
		}
	return false;
}

static bool same_piece(const DecorPiece *p, const DecorPiece *q) {
	return p->w == q->w && p->h == q->h &&
		!memcmp(p->e0, q->e0, (size_t)p->w * p->h * 2) && !memcmp(p->e1, q->e1, (size_t)p->w * p->h * 2);
}

void decor_learn(const AreaSrc *a, bool bg_in_map, int reach, DecorBook *out) {
	if (!a->rings) return;
	int W = a->tw * 8, tw = a->tw, th = a->th;
	uint8_t *fp = footprint(a, reach);
	/* 1: scenery, 2: taken into a piece, 3: floor and whatever touches it */
	uint8_t *kind = calloc((size_t)tw * th, 1), *pixels = calloc((size_t)tw * th, 1);
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) {
			int drawn = 0, on_floor = 0;
			for (int y = 0; y < 8; ++y)
				for (int x = 0; x < 8; ++x) {
					size_t i = (size_t)(ty * 8 + y) * W + tx * 8 + x;
					if (!(bg_in_map ? a->idx0 : a->idx)[i]) continue;
					++drawn;
					on_floor += fp[i];
				}
			if (drawn && !on_floor && !flat(a, tx, ty)) kind[ty * tw + tx] = 1;
			else if (drawn) kind[ty * tw + tx] = 3;   /* other art */
			pixels[ty * tw + tx] = (uint8_t)drawn;
		}
	static int qx[256 * 256], qy[256 * 256];
	for (int sy = 0; sy < th; ++sy)
		for (int sx = 0; sx < tw; ++sx) {
			if (kind[sy * tw + sx] != 1) continue;
			/* the piece: every scenery tile it touches */
			int n = 0, h = 0, x0 = sx, x1 = sx, y0 = sy, y1 = sy, drawn = 0;
			bool edge = false, attached = false;
			qx[n] = sx; qy[n++] = sy;
			kind[sy * tw + sx] = 2;
			while (h < n) {
				int x = qx[h], y = qy[h++];
				if (x < x0) x0 = x;
				if (x > x1) x1 = x;
				if (y < y0) y0 = y;
				if (y > y1) y1 = y;
				if (x == 0 || y == 0 || x == tw - 1 || y == th - 1) edge = true;
				drawn += pixels[y * tw + x];
				for (int dy = -1; dy <= 1; ++dy)
					for (int dx = -1; dx <= 1; ++dx) {
						int nx = x + dx, ny = y + dy;
						if (nx < 0 || ny < 0 || nx >= tw || ny >= th) continue;
						if (kind[ny * tw + nx] == 3 && touches(a, bg_in_map, x, y, dx, dy)) attached = true;
						if (kind[ny * tw + nx] != 1) continue;
						kind[ny * tw + nx] = 2;
						qx[n] = nx; qy[n++] = ny;
					}
			}
			int w = x1 - x0 + 1, hh = y1 - y0 + 1;
			/* whole pieces only: none cut by the map's edge or off what they
			 * stand on (a tank's top without its foot), none too small to be
			 * more than a scrap, none too big to set */
			if (edge || attached || n < MIN_TILES || drawn < MIN_PIXELS || w > MAX_SIDE || hh > MAX_SIDE || out->n >= DECOR_MAX) continue;
			DecorPiece *p = &out->piece[out->n];
			p->w = (uint8_t)w; p->h = (uint8_t)hh;
			p->e0 = calloc((size_t)w * hh, 2);
			p->e1 = calloc((size_t)w * hh, 2);
			for (int k = 0; k < n; ++k) {
				size_t src = (size_t)qy[k] * tw + qx[k], dst = (size_t)(qy[k] - y0) * w + (qx[k] - x0);
				p->e0[dst] = a->tile[0][src];
				if (!bg_in_map && a->layers > 1) p->e1[dst] = a->tile[1][src];
			}
			/* each look once */
			bool dup = false;
			for (int i = 0; i < out->n && !dup; ++i) dup = same_piece(&out->piece[i], p);
			if (dup) { free(p->e0); free(p->e1); continue; }
			out->n++;
		}
	free(kind);
	free(pixels);
	free(fp);
}

/* ---- what a map draws over its floor (decor_learn_floating) ---- */

#define FLOAT_SIDE   32   /* tiles a side at most */
#define FLOAT_OVER    9   /* tenths of its pixels over ground floor at least */
#define FLOAT_ON      8   /* pixels on raised floor at most: a walkable cloud's, its yellow pad's */
#define FLOAT_PX   2048   /* pixels at least, two panels' tops: what is smaller marks the floor (Undernet 1's warp hole in its frame) */

static int div_floor(int a, int b) { return a / b - (a % b < 0); }

/* Whether pixel (x, y) of map a lies on the top of floor at height z:
 * the panel its world point at that height falls in is walled floor of
 * that height. */
static bool floor_at(const AreaSrc *a, int x, int y, int z) {
	int s = x - a->tw * 4, d = 2 * (y + z - a->th * 4);   /* X + Y, Y - X (area_px, area_py) */
	int A = div_floor(div_floor(s - d, 2) - a->ex, 32), B = div_floor(div_floor(s + d, 2) - a->ey, 32);
	int PX = a->ex + 16 + 32 * A, PY = a->ey + 16 + 32 * B;
	return area_src_walled_floor(a, PX, PY) == 1 && area_src_height(a, PX, PY) == z;
}

/* The group of second-layer tiles joined 8 ways to tile `at`, marked in
 * `seen`, into q; its box in box[] (x0, y0, x1, y1), whether the map's
 * edge cuts it in *edge. */
static int float_group(const AreaSrc *a, int at, uint8_t *seen, int *q, int box[4], bool *edge) {
	int tw = a->tw, th = a->th, n = 0;
	box[0] = box[2] = at % tw; box[1] = box[3] = at / tw;
	*edge = false;
	q[n++] = at;
	seen[at] = 1;
	for (int h = 0; h < n; ++h) {
		int x = q[h] % tw, y = q[h] / tw;
		if (x < box[0]) box[0] = x;
		if (x > box[2]) box[2] = x;
		if (y < box[1]) box[1] = y;
		if (y > box[3]) box[3] = y;
		*edge |= x == 0 || y == 0 || x == tw - 1 || y == th - 1;
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx) {
				int nx = x + dx, ny = y + dy;
				if (nx < 0 || ny < 0 || nx >= tw || ny >= th) continue;
				int j = ny * tw + nx;
				if (seen[j] || !(a->tile[1][j] & 0x3FF)) continue;
				seen[j] = 1;
				q[n++] = j;
			}
	}
	return n;
}

/* Whether the group lies over the ground floor and is none itself:
 * FLOAT_PX pixels or more, nine tenths of them or more over ground panels,
 * hardly any on the top of floor at any of the map's raised heights
 * (`levels`). */
static bool over_floor(const AreaSrc *a, const int *q, int n, const bool *levels) {
	int W = a->tw * 8, drawn = 0, over = 0, on = 0;
	for (int k = 0; k < n; ++k)
		for (int y = q[k] / a->tw * 8; y < q[k] / a->tw * 8 + 8; ++y)
			for (int x = q[k] % a->tw * 8; x < q[k] % a->tw * 8 + 8; ++x) {
				if (!a->front[(size_t)y * W + x]) continue;
				++drawn;
				over += floor_at(a, x, y, 0);
				for (int z = 8; z < HEIGHT_UNEVEN && on <= FLOAT_ON; z += 8) on += levels[z] && floor_at(a, x, y, z);
			}
	return drawn >= FLOAT_PX && on <= FLOAT_ON && over * 10 >= FLOAT_OVER * drawn;
}

void decor_learn_floating(const AreaSrc *a, DecorBook *out) {
	if (a->layers < 2 || !a->front || !a->rings) return;
	int tw = a->tw, th = a->th;
	bool levels[256] = { false };
	for (int i = 0; a->hz && i < a->hw * a->hh; ++i) levels[a->hz[i]] = true;
	uint8_t *seen = calloc((size_t)tw * th, 1);
	int *q = malloc((size_t)tw * th * sizeof *q);
	for (int i = 0; seen && q && i < tw * th && out->n < DECOR_MAX; ++i) {
		if (seen[i] || !(a->tile[1][i] & 0x3FF)) continue;
		int box[4];
		bool edge;
		int n = float_group(a, i, seen, q, box, &edge), w = box[2] - box[0] + 1, h = box[3] - box[1] + 1;
		if (edge || n < MIN_TILES || w > FLOAT_SIDE || h > FLOAT_SIDE || !over_floor(a, q, n, levels)) continue;
		DecorPiece *p = &out->piece[out->n];
		p->w = (uint8_t)w; p->h = (uint8_t)h;
		p->e0 = calloc((size_t)w * h, 2);
		p->e1 = calloc((size_t)w * h, 2);
		if (!p->e0 || !p->e1) { free(p->e0); free(p->e1); break; }
		for (int k = 0; k < n; ++k) p->e1[(q[k] / tw - box[1]) * w + q[k] % tw - box[0]] = a->tile[1][q[k]];
		bool dup = false;
		for (int k = 0; k < out->n && !dup; ++k) dup = same_piece(&out->piece[k], p);
		if (dup) { free(p->e0); free(p->e1); continue; }
		out->n++;
	}
	free(seen);
	free(q);
}

void decor_free(DecorBook *b) {
	for (int i = 0; i < b->n; ++i) { free(b->piece[i].e0); free(b->piece[i].e1); }
	b->n = 0;
}

static uint32_t next(uint32_t *s) { *s = *s * 1103515245u + 12345u; return *s >> 16; }

/* Whether piece p fits at (px, py): its tiles and their neighbours empty,
 * the floor within NEAR_FLOOR tiles. */
static bool fits(const DecorPiece *p, const uint16_t *map, int tw, int th, int px, int py) {
	size_t cells = (size_t)tw * th;
	for (int y = 0; y < p->h; ++y)
		for (int x = 0; x < p->w; ++x) {
			if (!p->e0[y * p->w + x] && !p->e1[y * p->w + x]) continue;
			for (int dy = -1; dy <= 1; ++dy)
				for (int dx = -1; dx <= 1; ++dx) {
					int mx = px + x + dx, my = py + y + dy;
					if (mx < 0 || my < 0 || mx >= tw || my >= th) return false;
					size_t i = (size_t)my * tw + mx;
					if (map[i] || map[cells + i]) return false;
				}
		}
	for (int y = py - NEAR_FLOOR; y < py + p->h + NEAR_FLOOR; ++y)
		for (int x = px - NEAR_FLOOR; x < px + p->w + NEAR_FLOOR; ++x) {
			if (x < 0 || y < 0 || x >= tw || y >= th) continue;
			size_t i = (size_t)y * tw + x;
			if (map[i] || map[cells + i]) return true;
		}
	return false;
}

int decor_place(const DecorBook *b, uint16_t *map, int tw, int th, uint32_t seed) {
	if (!b->n) return 0;
	size_t cells = (size_t)tw * th;
	uint32_t s = seed ^ 0xDEC0u;
	int want = PIECES_MIN + (int)(next(&s) % (PIECES_MAX - PIECES_MIN + 1)), placed = 0;
	uint8_t used[DECOR_MAX] = { 0 };
	for (int t = 0; t < TRIES && placed < want; ++t) {
		int k = (int)(next(&s) % (uint32_t)b->n);
		if (used[k] >= 2) continue;   /* a piece twice at most */
		const DecorPiece *p = &b->piece[k];
		int px = (int)(next(&s) % (uint32_t)(tw - p->w + 1)), py = (int)(next(&s) % (uint32_t)(th - p->h + 1));
		if (!fits(p, map, tw, th, px, py)) continue;
		for (int y = 0; y < p->h; ++y)
			for (int x = 0; x < p->w; ++x) {
				size_t i = (size_t)(py + y) * tw + px + x;
				if (p->e0[y * p->w + x]) map[i] = p->e0[y * p->w + x];
				if (p->e1[y * p->w + x]) map[cells + i] = p->e1[y * p->w + x];
			}
		used[k]++;
		++placed;
	}
	return placed;
}
