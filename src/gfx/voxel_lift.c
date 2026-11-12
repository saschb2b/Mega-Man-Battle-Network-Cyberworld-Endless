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

/* How long a run of art under an edge is still its side (the rim and what
 * holds the slab up), in rows */
#define SIDE_MOST   28
/* ... and how near beside it, in columns, the scenery makes such a run its own */
#define BESIDE      8
/* ... how far a raised slab's side hangs over the floor behind it */
#define SLAB_THICK  8
/* ... and how far under a sprite in the air its floor is looked for */
#define HOVER_REACH 28
/* The floor whose top shows at (x, y): its ground row, -1 none. The
 * raised level's first, then a stair's ramp where it stands as high as
 * it shows, then the ground's. */
static int top_at(const VoxGround *g, int x, int y) {
	int raised = VOX_LEVELS - 1;
	if (g->z[raised] > 0 && (vox_floor(g, x, y + g->z[raised]) >> raised & 1u)) return y + g->z[raised];
	/* (of the heights a ramp could show at, the one it stands nearest to) */
	int best = -1, off = 2;
	for (int k = g->z[raised]; k >= 0; --k) {
		int r = vox_ramp(g, x, y + k) - 1, d = r < 0 ? 2 : r > k ? r - k : k - r;
		if (d < off) { best = y + k; off = d; }
	}
	if (best >= 0) return best;
	return vox_floor(g, x, y) & 1u ? y : -1;
}

/* A map pixel by the layer's own floor: a top, or a raised slab's side
 * over the floor behind it; false where the floor explains neither */
static bool lift_floor(const VoxGround *g, int x, int y, uint8_t *kind, int16_t *gy) {
	int raised = VOX_LEVELS - 1, t = top_at(g, x, y);
	if (t != y + g->z[raised] && g->z[raised] > SLAB_THICK)
		for (int k = 1; k <= SLAB_THICK; ++k)
			if (vox_floor(g, x, y - k + g->z[raised]) >> raised & 1u) { *kind = VOX_WALL; *gy = (int16_t)(y - k + g->z[raised] + 1); return true; }
	if (t < 0) return false;
	*kind = VOX_TOP;
	*gy = (int16_t)t;
	return true;
}

/* The map's pixels sorted. The layer's floor says where its tops are.
 * Art it does not explain is, down each column, either a top's side (a
 * short run right under a top or under a raised slab's side, the void
 * under it) or the scenery's: the platforms pasted beside the way, which the layer's floor
 * knows nothing of and which lie flat on the ground. */
static void lift_window(const VoxGround *g, VoxScene *s) {
	static bool known[VOX_WN];
	for (int j = 0; j < VOX_WH; ++j)
		for (int i = 0; i < VOX_WW; ++i) {
			int at = j * VOX_WW + i;
			s->kind[at] = VOX_SKY;
			s->gy[at] = (int16_t)(j - VOX_UP);
			known[at] = s->map[at] && lift_floor(g, i - VOX_LEFT, j - VOX_UP, &s->kind[at], &s->gy[at]);
		}
	/* (first each run by itself: 1 a side, 2 the scenery's) */
	static uint8_t run[VOX_WN];
	memset(run, 0, sizeof run);
	for (int i = 0; i < VOX_WW; ++i)
		for (int j = 0; j < VOX_WH; ++j) {
			int from = j;
			if (!s->map[j * VOX_WW + i] || known[j * VOX_WW + i]) continue;
			while (j + 1 < VOX_WH && s->map[(j + 1) * VOX_WW + i] && !known[(j + 1) * VOX_WW + i]) ++j;
			/* (a side ends in the void: a run between two floors is floor) */
			bool side = from > 0 && known[(from - 1) * VOX_WW + i] && j - from < SIDE_MOST && (j + 1 >= VOX_WH || !s->map[(j + 1) * VOX_WW + i]);
			for (int y = from; y <= j; ++y) run[y * VOX_WW + i] = side ? 1 : 2;
		}
	/* (then a side with the scenery beside it in its rows is the
	 * scenery's too: a strip of it under the floor's edge) */
	for (int i = 0; i < VOX_WW; ++i)
		for (int j = 0; j < VOX_WH; ++j) {
			int from = j, over = (j - 1) * VOX_WW + i;
			if (!run[j * VOX_WW + i]) continue;
			while (j + 1 < VOX_WH && run[(j + 1) * VOX_WW + i]) ++j;
			bool side = run[from * VOX_WW + i] == 1;
			for (int y = from; side && y <= j; ++y)
				for (int d = -BESIDE; d <= BESIDE; ++d)
					if (i + d >= 0 && i + d < VOX_WW && run[y * VOX_WW + i + d] == 2) side = false;
			for (int y = from; y <= j; ++y) {
				s->kind[y * VOX_WW + i] = side ? VOX_WALL : VOX_TOP;
				if (side) s->gy[y * VOX_WW + i] = (int16_t)(s->gy[over] + (s->kind[over] == VOX_TOP ? 1 : 0));
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
		b->z = t < 0 ? 0 : t - (b->y1 + down);
		b->gy = t < 0 ? b->y1 + 1 : t + 1;
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
	/* (what lies on the floor is the floor's: its colour there, and floor where the map has none) */
	for (int i = 0; i < VOX_N; ++i) {
		int at = (i / VOX_W + VOX_UP) * VOX_WW + i % VOX_W + VOX_LEFT;
		if (!l->lying[i]) continue;
		if (!s->map[at] || s->kind[at] == VOX_SKY) { s->kind[at] = VOX_TOP; s->gy[at] = (int16_t)(i / VOX_W); }
		s->map[at] = l->lying[i];
	}
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
