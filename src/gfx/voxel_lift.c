/* voxel.h's lift: the map's pixels sorted by what they show and placed in
 * the world, the frame's sprites joined into bodies on their feet
 * (docs/engine/voxel.md, The lift). */
#include "voxel.h"

#include <string.h>

/* Where BN6 keeps what is no part of the world: its HP in the corner, and
 * with a chat box open the box's rows (its frame, face and arrow) */
#define HP_RIGHT   48
#define HP_BOTTOM  16
#define BOX_TOP    96

/* How far under an edge a slab's side and what holds it up are drawn */
#define WALL_REACH  40
/* ... how far a raised slab's side hangs over the floor behind it */
#define SLAB_THICK  8
/* ... and how far under a sprite in the air its floor is looked for */
#define HOVER_REACH 28
/* ... and how deep inside a floor a tile lies to be taken for its art, in rows */
#define INSIDE      6

/* The highest level whose floor's top shows at (x, y), -1 none */
static int top_at(const VoxGround *g, int x, int y) {
	for (int l = VOX_LEVELS - 1; l >= 0; --l)
		if (vox_floor(g, x, y + g->z[l]) >> l & 1u) return l;
	return -1;
}

/* A map pixel by the layer's own floor: a level's top, or a raised
 * slab's side over the floor behind it; false where the floor explains
 * neither */
static bool lift_floor(const VoxGround *g, int x, int y, uint8_t *kind, int16_t *gy) {
	int raised = VOX_LEVELS - 1;
	int t = top_at(g, x, y);
	if (t != raised && g->z[raised] > SLAB_THICK)
		for (int k = 1; k <= SLAB_THICK; ++k)
			if (vox_floor(g, x, y - k + g->z[raised]) >> raised & 1u) { *kind = VOX_WALL; *gy = (int16_t)(y - k + g->z[raised] + 1); return true; }
	if (t < 0) return false;
	*kind = VOX_TOP;
	*gy = (int16_t)(y + g->z[t]);
	return true;
}

/* The map's pixels sorted. The layer's floor says where its tops are; the
 * tiles seen well inside them are the area's floor art, and the same
 * tiles elsewhere are floor too, lying flat: the scenery's platforms
 * beside the way, which the layer's floor knows nothing of. What is left
 * under a top, down its column, is that top's side. */
static void lift_window(const VoxGround *g, VoxScene *s) {
	static bool known[VOX_WN], floor_art[0x402];
	memset(floor_art, 0, sizeof floor_art);
	for (int j = 0; j < VOX_WH; ++j)
		for (int i = 0; i < VOX_WW; ++i) {
			int at = j * VOX_WW + i;
			s->kind[at] = VOX_SKY;
			s->gy[at] = (int16_t)(j - VOX_UP);
			known[at] = s->map[at] && lift_floor(g, i - VOX_LEFT, j - VOX_UP, &s->kind[at], &s->gy[at]);
		}
	for (int j = INSIDE; j < VOX_WH - INSIDE; ++j)
		for (int i = 0; i < VOX_WW; ++i) {
			int at = j * VOX_WW + i;
			if (known[at] && s->kind[at] == VOX_TOP && known[at - INSIDE * VOX_WW] && s->kind[at - INSIDE * VOX_WW] == VOX_TOP &&
			    known[at + INSIDE * VOX_WW] && s->kind[at + INSIDE * VOX_WW] == VOX_TOP) floor_art[s->tile[at]] = true;
		}
	for (int i = 0; i < VOX_WW; ++i) {
		int edge = -WALL_REACH - 1, edge_gy = 0;
		for (int j = 0; j < VOX_WH; ++j) {
			int at = j * VOX_WW + i;
			if (!s->map[at]) continue;
			if (!known[at] && floor_art[s->tile[at]]) s->kind[at] = VOX_TOP;
			else if (!known[at] && j - edge <= WALL_REACH) { s->kind[at] = VOX_WALL; s->gy[at] = (int16_t)(edge_gy + 1); }
			if (s->kind[at] == VOX_TOP) { edge = j; edge_gy = s->gy[at]; }
		}
	}
}

/* One body: the sprite pixels joined to (x, y), 8-connected */
static void body_fill(VoxScene *s, int x, int y, int id) {
	static int16_t stack[VOX_N * 2];
	VoxBody *b = &s->bodies[id];
	int n = 0;
	stack[n++] = (int16_t)x;
	stack[n++] = (int16_t)y;
	s->body[y * VOX_W + x] = (uint8_t)id;
	while (n) {
		y = stack[--n];
		x = stack[--n];
		if (x < b->x0) b->x0 = x;
		if (x > b->x1) b->x1 = x;
		if (y < b->y0) b->y0 = y;
		if (y > b->y1) b->y1 = y;
		for (int k = 0; k < 9; ++k) {
			int nx = x + k % 3 - 1, ny = y + k / 3 - 1, j = ny * VOX_W + nx;
			if (nx < 0 || ny < 0 || nx >= VOX_W || ny >= VOX_H || !s->sprite[j] || s->body[j]) continue;
			s->body[j] = (uint8_t)id;
			stack[n++] = (int16_t)nx;
			stack[n++] = (int16_t)ny;
		}
	}
}

static void lift_bodies(const VoxGround *g, VoxScene *s) {
	s->nbodies = 0;
	memset(s->body, 0, sizeof s->body);
	for (int i = 0; i < VOX_N; ++i) {
		if (!s->sprite[i] || s->body[i]) continue;
		/* (past the most, the rest join the last) */
		int id = VOX_BODIES;
		if (s->nbodies < VOX_BODIES) {
			id = ++s->nbodies;
			s->bodies[id] = (VoxBody){ VOX_W, VOX_H, -1, -1, 0, 0 };
		}
		body_fill(s, i % VOX_W, i / VOX_W, id);
	}
	for (int id = 1; id <= s->nbodies; ++id) {
		VoxBody *b = &s->bodies[id];
		/* (its feet: the floor under the middle of its lowest row; one in
		 * the air stands over the floor a little way under it) */
		int mid = (b->x0 + b->x1) / 2, t = -1, down = 0;
		for (; down <= HOVER_REACH && (t = top_at(g, mid, b->y1 + down)) < 0; ++down) {}
		b->z = t < 0 ? 0 : g->z[t];
		b->gy = b->y1 + 1 + b->z + (t < 0 ? 0 : down);
	}
}

/* A sprite of the HUD: all of it in the HP's corner, or in an open chat
 * box's rows */
static bool hud_obj(const GbaLayers *l, int n, bool chat) {
	if (!l->w[n]) return false;
	if (l->x[n] + l->w[n] <= HP_RIGHT && l->y[n] + l->h[n] <= HP_BOTTOM) return true;
	return chat && l->y[n] >= BOX_TOP;
}

void voxel_lift(const GbaLayers *l, bool chat, const uint32_t *frame, const VoxGround *ground, VoxScene *s) {
	lift_window(ground, s);
	bool hud[GBA_OBJS];
	for (int n = 0; n < GBA_OBJS; ++n) hud[n] = hud_obj(l, n, chat);
	for (int i = 0; i < VOX_N; ++i) {
		bool obj = l->top[i] == GBA_LAYER_OBJ;
		s->hud[i] = obj ? hud[l->obj[i] - 1] : l->top[i] == 0;
		/* (a blended sprite, a shadow or a light, is left out: the sun casts its own) */
		s->sprite[i] = obj && !s->hud[i] && !l->sheer[i] ? frame[i] | 0xFF000000u : 0;
	}
	lift_bodies(ground, s);
}
