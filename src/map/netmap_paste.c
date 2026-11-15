/* The original maps' scenery pasted onto a generated one (netmap.c):
 * stairs, props, ornaments, pads, emblems, arrows and bushes, where the
 * layer has them. */
#include "netmap_paste.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "net_shapes.h"
#include "netmap_learn.h"
#include "tilemap.h"
#include "voids.h"

/* ---- output ---- */

/* A stair block's corner with the lowest panel indices, in world units. */
void nm_stair_origin(const Stair *st, int *X0, int *Y0) {
	int A0 = -(st->y + stair_h(st) - 1 - nm_place.gy0), B0 = st->x - nm_place.gx0;
	*X0 = nm_place.ex + 32 * A0;
	*Y0 = nm_place.ey + 32 * B0;
}

/* The stairs' own tiles over whatever the classes put there. */
void nm_paste_stairs(const Learned *L, uint16_t *map, int tw, int th) {
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < nm_cur->nstairs; ++i) {
		const StairTemplate *t = &L->stairs[nm_cur->stairs[i].dir][stair_width(&nm_cur->stairs[i])];
		if (!t->ok) continue;
		int X0, Y0;
		nm_stair_origin(&nm_cur->stairs[i], &X0, &Y0);
		int px0 = X0 + Y0 + tw * 4, py0 = (Y0 - X0) / 2 + th * 4;
		if (emu_debug_on()) fprintf(stderr, "stair pasted area biome %d seed %u: dir %d wide %d len %d at %d,%d, world corner %d,%d\n", layer.biome, nm_cur->seed,
			nm_cur->stairs[i].dir, stair_wide(&nm_cur->stairs[i]), stair_len(&nm_cur->stairs[i]), px0, py0, X0, Y0);
		for (int k = 0; k < t->ntiles; ++k) {
			int px = px0 + t->tiles[k].px, py = py0 + t->tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
			/* (not a tile three quarters empty: over void it floated as a
			 * speck, over floor it cut a notch into it) */
			if (t->tiles[k].opaque < 16) continue;
			size_t at = (size_t)(py / 8) * tw + px / 8;
			map[at] = t->tiles[k].e0;
			map[cells + at] = t->tiles[k].e1;
			if (nm_last.pasted) nm_last.pasted[at] |= NETMAP_PASTED_STAIR;
		}
	}
}
/* A look's area cannot draw the layer's stairs where one's way and width
 * is not learned, or not at the layer's rise and the stair's length: its
 * ramps were void under MegaMan's feet and its raised floor islands on
 * short faces (Green's stairs in Central's look), so it draws them level
 * with the ground (netmap_look; docs/game/glitch-layer.md, Heights in a
 * flip). */
bool nm_stairs_drawn(const Learned *L) {
	for (int i = 0; i < nm_cur->nstairs; ++i) {
		const Stair *st = &nm_cur->stairs[i];
		const StairTemplate *t = &L->stairs[st->dir][stair_width(st)];
		if (!t->ok || t->rise != nm_cur->rise || t->len != stair_len(st)) return false;
	}
	return true;
}

PropAt nm_prop_at[MAX_PROPS];

void nm_props_place(const Learned *L) {
	memset(nm_prop_at, 0, sizeof nm_prop_at);
	for (int i = 0; i < nm_cur->nprops && i < MAX_PROPS; ++i) {
		const NetProp *p = &nm_cur->props[i];
		const PropStamp *st = &L->counter[p->faces];
		if (p->kind != PROP_COUNTER || !st->ok || st->len != p->len) continue;
		/* the corner of its panels with the lowest world X and Y: FACES_X
		 * runs along grid y, whose highest cell is the lowest X */
		int A = p->faces == FACES_X ? -(p->y + p->len - 1 - nm_place.gy0) : -(p->y - nm_place.gy0), B = p->x - nm_place.gx0;
		nm_prop_at[i] = (__typeof__(nm_prop_at[0])){ true, nm_place.ex + 32 * A + L->counter_dx[p->faces], nm_place.ey + 32 * B + L->counter_dy[p->faces], st };
		if (emu_debug_on()) fprintf(stderr, "counter faces %d at %d,%d (%d walls, %d layer priorities)\n", p->faces, nm_prop_at[i].X, nm_prop_at[i].Y, st->nwalls, st->nprio);
	}
}

/* Their tiles on the second layer, over the floor the classes drew. */
void nm_paste_props(uint16_t *map, int tw, int th) {
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < MAX_PROPS; ++i) {
		if (!nm_prop_at[i].ok) continue;
		const PropStamp *st = nm_prop_at[i].st;
		int px0 = area_px(tw, nm_prop_at[i].X, nm_prop_at[i].Y), py0 = area_py(th, nm_prop_at[i].X, nm_prop_at[i].Y);
		for (int k = 0; k < st->ntiles; ++k) {
			int px = px0 + st->tiles[k].px, py = py0 + st->tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
			map[cells + (size_t)(py / 8) * tw + px / 8] = st->tiles[k].e1;
		}
	}
}

/* The teleports' gems (issue #44). Every centrepiece of BN6's net maps
 * marks a warp (29 of them: the gem a teleport within the map, the cube a
 * homepage's link, the ring a comp's or another area's), so none is set
 * where no warp is: a pad that looks like a warp and does nothing lies. */
void nm_paste_ornaments(const Learned *L, uint16_t *map, int tw, int th) {
	if (!L->ornament[0].ok) return;
	size_t cells = (size_t)tw * th;
	int teleports = layer.nteleports ? 2 : 0;
	for (int i = 0; i < teleports + nm_cur->ngems; ++i) {
		int x = i < teleports ? layer.teleport_x[i] : nm_cur->gems[i - teleports][0];
		int y = i < teleports ? layer.teleport_y[i] : nm_cur->gems[i - teleports][1];
		const PropStamp *st = &L->ornament[0];
		int A, B;
		nm_grid_to_panel(x, y, &A, &B);
		int px0 = area_px(tw, nm_place.ex + 32 * A, nm_place.ey + 32 * B), py0 = area_py(th, nm_place.ex + 32 * A, nm_place.ey + 32 * B);
		for (int k = 0; k < st->ntiles; ++k) {
			int px = px0 + st->tiles[k].px, py = py0 + st->tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
			map[cells + (size_t)(py / 8) * tw + px / 8] = st->tiles[k].e1;
		}
	}
}

/* Whether grid panel (gx, gy) is floor: of the layer, or of the layout
 * being drawn (the Net Square's) */
static bool layer_floor(int gx, int gy) { return gx >= 0 && gy >= 0 && gx < MAP_W && gy < MAP_H && layer.cell[gy][gx] != C_VOID; }
static bool drawn_floor(int gx, int gy) {
	return gx >= 0 && gy >= 0 && gx < nm_cur->gw && gy < nm_cur->gh && nm_cur->cell[gy * nm_cur->gw + gx] != C_VOID;
}

/* Whether grid panel (gx, gy) of `floor` lies in a 2 x 2 block of it: a
 * platform's, not a walkway's (netmap.c, walkway). */
static bool in_block(bool (*floor)(int, int), int gx, int gy) {
	for (int dy = -1; dy <= 0; ++dy)
		for (int dx = -1; dx <= 0; ++dx)
			if (floor(gx + dx, gy + dy) && floor(gx + dx + 1, gy + dy) && floor(gx + dx, gy + dy + 1) && floor(gx + dx + 1, gy + dy + 1)) return true;
	return false;
}

/* What a tile of a pad's stamp lies over besides the pad's own panels and
 * the void (pad_tile): a walkway's panel; a platform's panel or faces
 * behind the pad (across its upper edges, or beside a corner); a
 * platform's panel in front of it (across its lower edges). */
enum { PAD_ON_WALKWAY = 1, PAD_BEHIND = 2, PAD_IN_FRONT = 4 };

/* What the tile at (pu, pv), map pixels from the map's middle, lies over
 * besides the 3 x 3 pad from (mx, my)'s own panels and the void, at the
 * floor's height (drawn dv below the panel edges), PAD_* bits. Each pixel
 * off the pad's top is looked up the screen as deep as the faces hang
 * till it meets a panel of other floor, past the pad's own: a slab's face
 * runs a few pixels past its panel's corner, under the pad's beside it
 * (Robot Control's, 4). */
static int pad_tile(const Learned *L, int mx, int my, int pu, int pv, bool (*floor)(int, int)) {
	int got = 0;
	for (int y = 1; y < 8; y += 2)
		for (int x = 1; x < 8; x += 2)
			for (int up = 0; up <= L->book[0].face; up += 2) {
				int u = pu + x, v = 2 * (pv + y - up - L->book[0].dv), X = (u - v) / 2, Y = (u + v) / 2;
				int A = nm_floordiv(X - nm_place.ex, 32), B = nm_floordiv(Y - nm_place.ey, 32);
				int gx = B + nm_place.gx0, gy = -A + nm_place.gy0;
				bool own = gx >= mx && gx < mx + 3 && gy >= my && gy < my + 3;
				if (own && !up) break;
				if (own || !floor(gx, gy)) continue;
				/* (a walkway's faces as before: the stamp over them) */
				bool block = in_block(floor, gx, gy);
				if (!up && !block) return PAD_ON_WALKWAY;
				if (block) got |= !up && gx >= mx && gy >= my ? PAD_IN_FRONT : PAD_BEHIND;
				break;
			}
	return got;
}

static bool drawn(uint16_t e) { return e & 0x3FF; }
static uint16_t front_of(uint16_t e0, uint16_t e1) { return drawn(e1) ? e1 : e0; }

/* A 3 x 3 the stamp goes on, and the floor it is read in. */
typedef struct { int x, y; bool (*floor)(int, int); } PadAt;

/* The area's pad stamped on the 3 x 3 `pad`; `beside`, both layers of the
 * floor round the pad picked with the pad's place taken for void
 * (nm_pick_beside_pads), or NULL */
static void paste_pad(const Learned *L, uint16_t *map, const uint16_t *beside, int tw, int th, const PadAt *pad) {
	int mx = pad->x, my = pad->y, A, B;
	nm_grid_to_panel(mx, my + 2, &A, &B);
	int X0 = nm_place.ex + 32 * A, Y0 = nm_place.ey + 32 * B;
	int px0 = area_px(tw, X0, Y0), py0 = area_py(th, X0, Y0);
	size_t cells = (size_t)tw * th;
	/* the stamp alone over the pad's own panels and the void; never over
	 * a walkway, whose last panel before the pad its edges and faces cut;
	 * with a platform it touches, on two layers (below) */
	for (int k = 0; k < L->pad.ntiles; ++k) {
		int px = px0 + L->pad.tiles[k].px, py = py0 + L->pad.tiles[k].py;
		if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
		int over = pad_tile(L, mx, my, px - tw * 4, py - th * 4, pad->floor);
		if (over & PAD_ON_WALKWAY) continue;
		size_t at = (size_t)(py / 8) * tw + px / 8;
		uint16_t stamp = L->pad.tiles[k].e0, mine = front_of(map[at], map[cells + at]);
		/* (a platform set flush with the pad, as no map sets one, at the
		 * pad's height: a platform in front of the pad on the second layer,
		 * picked as if no pad were there, so its top hides the pad's side
		 * faces, the stamp behind it; one behind the pad on the first, as
		 * the classes picked it, the stamp in front of it. Either way the
		 * pad is whole and the platform's top and faces show where the
		 * stamp draws nothing. Left out, the classes' pick there drew the
		 * pad's other look over its corner; stamped alone, it cut off the
		 * platform's side face beside it; in front of a platform, the pad
		 * stood raised above it) */
		if (over & PAD_IN_FRONT && beside && drawn(front_of(beside[at], beside[cells + at]))) {
			map[at] = stamp;
			map[cells + at] = front_of(beside[at], beside[cells + at]);
		} else if (over) {
			map[at] = mine;
			map[cells + at] = stamp;
		} else {
			map[at] = stamp;
			/* (and nothing over it: what the classes picked for the second
			 * layer there lay over the stamp, Seaside's pads a jumble of
			 * their own frame and their other pads'. No area's pad has art
			 * of its own on that layer; Central's maps have a warp's panel
			 * there) */
			map[cells + at] = 0;
		}
		if (nm_last.pasted) nm_last.pasted[at] |= NETMAP_PASTED_PAD;
	}
}

/* Whether the drawn layout's 3 x 3 from (mx, my) is flat floor, clear of
 * its stairs */
static bool drawn_flat(int mx, int my) {
	for (int y = my; y < my + 3; ++y)
		for (int x = mx; x < mx + 3; ++x) {
			if (x < 0 || y < 0 || x >= nm_cur->gw || y >= nm_cur->gh || nm_cur->cell[y * nm_cur->gw + x] != C_PATH) return false;
			if (nm_cur->level && nm_cur->level[y * nm_cur->gw + x]) return false;
		}
	for (int s = 0; s < nm_cur->nstairs; ++s) {
		const Stair *st = &nm_cur->stairs[s];
		if (!(st->x + stair_w(st) < mx - 1 || st->x > mx + 3 || st->y + stair_h(st) < my - 1 || st->y > my + 3)) return false;
	}
	return true;
}

/* The 3 x 3 pads the stamp goes on: every pad at ground level away from
 * the stairs, and the Net Square's gems' pads; how many (at most max). */
static int pads_stamped(const Learned *L, PadAt *at, int max) {
	int n = 0;
	if (!L->pad.ok) return 0;
	for (int i = 0; i < layer.nrooms && n < max; ++i) {
		const Room *m = &layer.rooms[i];
		if (m->w != 3 || m->h != 3 || !(m->kind == ROOM_PAD || (nm_cur->pad && nm_cur->pad[m->y * nm_cur->gw + m->x] && m->kind == ROOM_PLATFORM))) continue;
		bool flat = true;
		for (int y = m->y; y < m->y + 3; ++y)
			for (int x = m->x; x < m->x + 3; ++x) flat &= layer.cell[y][x] == C_PATH && !layer.level[y][x];
		for (int s = 0; s < layer.nstairs; ++s)
			flat &= layer.stair[s].x + stair_w(&layer.stair[s]) < m->x - 1 || layer.stair[s].x > m->x + 3 || layer.stair[s].y + stair_h(&layer.stair[s]) < m->y - 1 || layer.stair[s].y > m->y + 3;
		if (flat) at[n++] = (PadAt){ m->x, m->y, layer_floor };
	}
	for (int i = 0; i < nm_cur->ngems && n < max; ++i) {
		int mx = nm_cur->gems[i][0] - 1, my = nm_cur->gems[i][1] - 1;
		if (drawn_flat(mx, my)) at[n++] = (PadAt){ mx, my, drawn_floor };
	}
	return n;
}

#define MAX_STAMPED 64

static int pads_by_platforms(const Learned *L, int (*at)[2], int max) {
	static PadAt pads[MAX_STAMPED];
	int n = pads_stamped(L, pads, MAX_STAMPED), got = 0;
	for (int i = 0; i < n && got < max; ++i) {
		bool by = false;
		for (int y = pads[i].y - 1; y <= pads[i].y + 3 && !by; ++y)
			for (int x = pads[i].x - 1; x <= pads[i].x + 3 && !by; ++x)
				by = (x < pads[i].x || y < pads[i].y || x > pads[i].x + 2 || y > pads[i].y + 2) && pads[i].floor(x, y) && in_block(pads[i].floor, x, y);
		if (!by) continue;
		at[got][0] = pads[i].x;
		at[got][1] = pads[i].y;
		++got;
	}
	return got;
}

/* The pads the stamp goes on that touch a platform, while the floor round
 * them is picked alone, and the layer's floor as the classes read it. */
#define MAX_PADS_BY 32
static int by_n;
static int by_at[MAX_PADS_BY][2];
static TileFloor by_floor;

/* The floor within three panels of those pads, the pads themselves void:
 * a platform beside one drawn as if no pad were there, its rim and faces
 * along the pad's edge its own. */
static int floor_beside(int A, int B, const void *ctx) {
	int x = B + nm_place.gx0, y = -A + nm_place.gy0;
	bool near = false;
	for (int i = 0; i < by_n; ++i) {
		int px = by_at[i][0], py = by_at[i][1];
		if (x >= px && x < px + 3 && y >= py && y < py + 3) return TILE_VOID;
		near |= x >= px - 3 && x < px + 6 && y >= py - 3 && y < py + 6;
	}
	return near ? by_floor(A, B, ctx) : TILE_VOID;
}

uint16_t *nm_pick_beside_pads(const Learned *L, const TileGrid *grid, const TileBook *led, int nled, TileFloor floor) {
	by_n = pads_by_platforms(L, by_at, MAX_PADS_BY);
	if (!by_n) return NULL;
	by_floor = floor;
	size_t cells = (size_t)grid->tw * grid->th;
	uint16_t *beside = calloc(cells * 2, sizeof *beside);
	uint8_t *left = calloc(cells, 1);
	if (!beside || !left) {
		free(beside);
		free(left);
		return NULL;
	}
	TileStats was = tiles_stats;   /* (the layer's own pick is what is counted) */
	tilemap_pick(led, nled, &L->seams, grid, floor_beside, NULL, beside, left);
	tiles_stats = was;
	free(left);
	return beside;
}

/* The area's own pad, whole, on every pad the stamp goes on, in place of
 * the floor the classes drew there (the classes drew Central's pads
 * green, where its maps frame them); their centrepieces go on after. */
void nm_paste_pads(const Learned *L, uint16_t *map, const uint16_t *beside, int tw, int th) {
	static PadAt pads[MAX_STAMPED];
	int n = pads_stamped(L, pads, MAX_STAMPED);
	for (int i = 0; i < n; ++i) paste_pad(L, map, beside, tw, th, &pads[i]);
}

/* Whether grid panel (x, y) lies in a guardian's arena drawn in the
 * walkways' floor (netmap.c, arena_b): an emblem is cut from platform
 * floor, and the Underground's grey stone stood in its arena's magenta. */
static bool on_walkway_arena(const Learned *L, int x, int y) {
	if (L->def->styles & (TILES_BY_SHAPE | TILES_ARENA_FLOOR)) return false;
	return x >= nm_cur->ax && x < nm_cur->ax + nm_cur->aw && y >= nm_cur->ay && y < nm_cur->ay + nm_cur->ah;
}

/* The emblems the generator set in the floor (PROP_EMBLEM), in place of
 * the floor the classes drew there. */
void nm_paste_emblems(const Learned *L, uint16_t *map, int tw, int th) {
	if (!L->emblem.ok) return;
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < nm_cur->nprops && i < MAX_PROPS; ++i) {
		const NetProp *p = &nm_cur->props[i];
		if (p->kind != PROP_EMBLEM || on_walkway_arena(L, p->x, p->y)) continue;
		int A, B;
		nm_grid_to_panel(p->x, p->y, &A, &B);
		int px0 = area_px(tw, nm_place.ex + 32 * A, nm_place.ey + 32 * B), py0 = area_py(th, nm_place.ex + 32 * A, nm_place.ey + 32 * B);
		for (int k = 0; k < L->emblem.ntiles; ++k) {
			int px = px0 + L->emblem.tiles[k].px, py = py0 + L->emblem.tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
			size_t at = (size_t)(py / 8) * tw + px / 8;
			if (L->emblem.tiles[k].e0) map[at] = L->emblem.tiles[k].e0;
			if (L->emblem.tiles[k].e1) map[cells + at] = L->emblem.tiles[k].e1;
			if (nm_last.pasted) nm_last.pasted[at] |= NETMAP_PASTED_EMBLEM;
		}
	}
}

/* The arrow lanes' panels (issue #43), each its area's arrow panel for its
 * way, in place of the walkway the classes drew there. */
void nm_paste_arrows(const Learned *L, uint16_t *map, int tw, int th) {
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < layer.nlanes; ++i) {
		const NetLane *l = &layer.lane[i];
		const PropStamp *st = &L->arrow[(l->dir + 1) & 3];
		for (int k = 1; k <= l->len && st->ok; ++k) {
			int A, B;
			nm_grid_to_panel(l->x + dir_dx[l->dir] * k, l->y + dir_dy[l->dir] * k, &A, &B);
			int px0 = area_px(tw, nm_place.ex + 32 * A, nm_place.ey + 32 * B), py0 = area_py(th, nm_place.ex + 32 * A, nm_place.ey + 32 * B);
			for (int t = 0; t < st->ntiles; ++t) {
				int px = px0 + st->tiles[t].px, py = py0 + st->tiles[t].py;
				if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
				size_t at = (size_t)(py / 8) * tw + px / 8;
				map[at] = st->tiles[t].e0;
				map[cells + at] = st->tiles[t].e1;
				if (nm_last.pasted) nm_last.pasted[at] |= NETMAP_PASTED_ARROW;
			}
		}
	}
}

/* Green's potted bushes in the gaps between parallel planks, as its maps
 * set them: in a void panel with floor on both sides along one axis and
 * void on the other two, every second panel along the gap, the plain and
 * the flowering pot in turn. */
void nm_paste_bushes(const Learned *L, uint16_t *map, int tw, int th) {
	if (!L->bush[0].ok) return;
	size_t cells = (size_t)tw * th;
	for (int y = 1; y < MAP_H - 1; ++y)
		for (int x = 1; x < MAP_W - 1; ++x) {
			if (layer.cell[y][x] != C_VOID) continue;
			bool across = layer.cell[y][x - 1] == C_PATH && layer.cell[y][x + 1] == C_PATH && !layer.cell[y - 1][x] && !layer.cell[y + 1][x];
			bool down = layer.cell[y - 1][x] == C_PATH && layer.cell[y + 1][x] == C_PATH && !layer.cell[y][x - 1] && !layer.cell[y][x + 1];
			/* (every second one along the gap) */
			if (!(across && (y & 1) == 0) && !(down && (x & 1) == 0)) continue;
			const PropStamp *st = &L->bush[L->bush[1].ok && ((x + y) / 2 & 1)];
			int A, B;
			nm_grid_to_panel(x, y, &A, &B);
			int px0 = area_px(tw, nm_place.ex + 32 * A, nm_place.ey + 32 * B), py0 = area_py(th, nm_place.ex + 32 * A, nm_place.ey + 32 * B);
			for (int k = 0; k < st->ntiles; ++k) {
				int px = px0 + st->tiles[k].px, py = py0 + st->tiles[k].py;
				if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
				size_t at = (size_t)(py / 8) * tw + px / 8;
				if (!map[cells + at]) map[cells + at] = st->tiles[k].e1;   /* (never over the floor's own art) */
			}
		}
}

/* Whether grid panel (x, y) is ground floor nothing stands on or beside:
 * no raised floor, pad, lane or arena, no object within a panel. */
static bool side_free(int x, int y) {
	if (x < 1 || y < 1 || x >= MAP_W - 1 || y >= MAP_H - 1 || layer.cell[y][x] != C_PATH || layer.level[y][x]) return false;
	if (nm_cur->pad && nm_cur->pad[y * nm_cur->gw + x]) return false;
	if (x >= nm_cur->ax - 1 && x <= nm_cur->ax + nm_cur->aw && y >= nm_cur->ay - 1 && y <= nm_cur->ay + nm_cur->ah) return false;
	for (int i = 0; i < layer.nobj; ++i)
		if (abs((int)layer.obj[i].x - x) <= 1 && abs((int)layer.obj[i].y - y) <= 1) return false;
	for (int i = 0; i < layer.nlanes; ++i)
		for (int k = 0; k <= layer.lane[i].len + 1; ++k)
			if (abs(layer.lane[i].x + dir_dx[layer.lane[i].dir] * k - x) <= 1 && abs(layer.lane[i].y + dir_dy[layer.lane[i].dir] * k - y) <= 1) return false;
	return true;
}

/* Grid panel (x, y)'s neighbourhood as a side piece's (SidePiece.floor):
 * panel A runs against grid y, panel B along grid x. */
static unsigned side_floor(int x, int y) {
	unsigned f = 0;
	for (int k = 0; k < 9; ++k) {
		int gx = x + k / 3 - 1, gy = y - (k % 3 - 1);
		if (layer.cell[gy][gx] != C_VOID && !layer.level[gy][gx]) f |= 1u << k;
	}
	return f;
}

/* Whether the panels piece p draws over are at grid panel (x, y) what they
 * are in its map: ground floor, or none (beyond the grid is none). */
static bool side_under(const SidePiece *p, int x, int y) {
	for (int k = 0; k < p->nunder; ++k) {
		int gx = x + p->under[k][1], gy = y - p->under[k][0];
		bool in = gx >= 0 && gy >= 0 && gx < MAP_W && gy < MAP_H;
		if (p->under[k][2] ? !in || layer.cell[gy][gx] == C_VOID || layer.level[gy][gx] : in && layer.cell[gy][gx] != C_VOID) return false;
	}
	return true;
}

/* Whether grid panel (x, y) is a pad's on the ground: what stands under a
 * raised floor in its map (SidePiece.hang) stands under a layer's pads. */
static bool hang_free(int x, int y) {
	return x >= 1 && y >= 1 && x < MAP_W - 1 && y < MAP_H - 1 && layer.cell[y][x] == C_PATH && !layer.level[y][x]
		&& nm_cur->pad && nm_cur->pad[y * nm_cur->gw + x];
}

#define SIDES_A_LAYER 12   /* side pieces a layer sets at most */
#define SIDES_APART    3   /* panels between two */
#define GROWS_SKIP     3   /* what grows on a floor's edge: a panel that fits in this many takes none, */
#define GROWS_A_LAYER 64   /* ... and a layer sets this many at most */

/* The layer's tile map as the side pieces are set on it, the pixels its
 * tiles draw (voids_opaque; NULL unknown), and the tiles its area's maps
 * set side by side. */
typedef struct {
	uint16_t *map;
	int tw, th;
	const uint64_t *opaque;
	const TileSeams *seams;
} SideMap;

/* Whether the first layer shows the same round second-layer tile `e1`
 * over `was` (its map's) as over `is` (the layer's): the same entry, or
 * whatever the two draw hidden behind e1. */
static bool side_shows_same(const uint64_t *opaque, uint16_t e1, uint16_t was, uint16_t is) {
	if (was == is) return true;
	if (!opaque) return false;
	uint64_t open = ~voids_entry_px(opaque, e1);
	return !(voids_entry_px(opaque, was) & open) && !(voids_entry_px(opaque, is) & open);
}

/* A tile of the layer's map as its neighbours see it (seams.h): its first
 * layer's entry, or SEAM_VOID where neither layer draws one. */
static uint32_t seam_look(const SideMap *m, int tx, int ty) {
	size_t at = (size_t)ty * m->tw + tx;
	return m->map[at] & 0x3FF || m->map[(size_t)m->tw * m->th + at] & 0x3FF ? m->map[at] : SEAM_VOID;
}

/* Whether one of piece p's tiles, from (px0, py0), sits at tile (tx, ty) of the map. */
static bool piece_at(const SidePiece *p, int px0, int py0, int tx, int ty) {
	for (int k = 0; k < p->ntiles; ++k)
		if (px0 + p->tiles[k].px == tx * 8 && py0 + p->tiles[k].py == ty * 8) return true;
	return false;
}

/* Whether what grows on a floor's edge, its tiles from (px0, py0), meets
 * the layer's tiles round it as the area's maps set them side by side: a
 * planter's top beside a planter of the layer's wall would cut it in two. */
static bool grows_meets(const SidePiece *p, const SideMap *m, int px0, int py0) {
	static const int dir[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int k = 0; k < p->ntiles; ++k) {
		int tx = (px0 + p->tiles[k].px) / 8, ty = (py0 + p->tiles[k].py) / 8;
		for (int d = 0; d < 4; ++d) {
			int nx = tx + dir[d][0], ny = ty + dir[d][1];
			if (nx < 0 || ny < 0 || nx >= m->tw || ny >= m->th || piece_at(p, px0, py0, nx, ny)) continue;
			uint32_t near = seam_look(m, nx, ny), own = p->tiles[k].e0;
			bool after = dir[d][0] + dir[d][1] > 0;   /* (the neighbour to its right or below) */
			if (!seams_seen(m->seams, after ? own : near, after ? near : own, dir[d][1] != 0)) return false;
		}
	}
	return true;
}

/* Whether tile t of what stands behind a floor on both layers may go on
 * map cell `at`: over nothing but where its map draws its foot over the
 * floor (StairTile.foot), there over the floor's own art. */
static bool both_free(const StairTile *t, const SideMap *m, size_t at) {
	return !m->map[(size_t)m->tw * m->th + at] && (!m->map[at] || (!t->e0 && t->foot));
}

/* Piece p's tile t on map cell `at`; a foot over the floor's art marked so
 * (NETMAP_PASTED_FOOT). */
static void side_put(const SidePiece *p, const StairTile *t, uint16_t *map, size_t cells, size_t at) {
	if (!p->both) { (p->hang ? map : map + cells)[at] = p->hang ? t->e0 : t->e1; return; }
	if (nm_last.pasted && map[at] && t->foot) nm_last.pasted[at] |= NETMAP_PASTED_FOOT;
	if (t->e0) map[at] = t->e0;
	if (t->e1) map[cells + at] = t->e1;
}

/* Whether piece p goes down whole at grid panel (x, y): on the map, on its
 * tile lattice, over nothing its layer holds (the second; the first for
 * what stands under a floor; both for what stands on both). */
static bool side_fits(const SidePiece *p, const SideMap *m, int x, int y) {
	int tw = m->tw, th = m->th;
	const uint16_t *to = p->hang ? m->map : m->map + (size_t)tw * th;
	int A, B, over = 0, top = p->tiles[0].py, foot = top;
	nm_grid_to_panel(x, y, &A, &B);
	int px0 = area_px(tw, nm_place.ex + 32 * A, nm_place.ey + 32 * B), py0 = area_py(th, nm_place.ex + 32 * A, nm_place.ey + 32 * B);
	for (int k = 1; k < p->ntiles; ++k) {
		if (p->tiles[k].py < top) top = p->tiles[k].py;
		if (p->tiles[k].py > foot) foot = p->tiles[k].py;
	}
	for (int k = 0; k < p->ntiles; ++k) {
		int px = px0 + p->tiles[k].px, py = py0 + p->tiles[k].py;
		if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) return false;
		size_t at = (size_t)(py / 8) * tw + px / 8;
		/* (nor over a bridge's or a pad's art, set whole: wider than the panels under it) */
		if (nm_last.pasted && nm_last.pasted[at] & (NETMAP_PASTED_PAD | NETMAP_PASTED_STAIR)) return false;
		/* (what stands behind a floor on both layers: both_free) */
		if (p->both) {
			if (!both_free(&p->tiles[k], m, at)) return false;
			continue;
		}
		/* (what grows on a floor's edge goes over the floor's own art where
		 * its tile drew that too in its map, and nowhere else) */
		if (p->grows) {
			if (!to[at] != !(p->onto >> k & 1)) return false;
			continue;
		}
		/* (what stands under a pad begins in its rim's own tiles, what
		 * stands behind a floor ends in them: its two rows there, a third
		 * of it at most, set over them; anywhere else a tile it would
		 * cover is another floor's, a plateau's underside over the void) */
		bool meets = p->behind ? p->tiles[k].py >= foot - 8 : p->tiles[k].py <= top + 8;
		if (to[at] && (!p->hang || !meets || ++over * 3 > p->ntiles)) return false;
		/* (and where it keeps the first layer's tiles it stood over, a rim's
		 * tab or spikes, round it the picture its map showed: over anything
		 * else it breaks at the tile's edge) */
		if (p->over && !side_shows_same(m->opaque, p->tiles[k].e1, p->tiles[k].e0, m->map[at])) return false;
	}
	return !p->grows || grows_meets(p, m, px0, py0);
}

/* Piece p set at grid panel (x, y), whole or not at all (side_fits).
 * Whether it was. */
static bool side_set(const SidePiece *p, const SideMap *m, int x, int y) {
	if (!side_fits(p, m, x, y)) return false;
	int A, B;
	nm_grid_to_panel(x, y, &A, &B);
	int px0 = area_px(m->tw, nm_place.ex + 32 * A, nm_place.ey + 32 * B), py0 = area_py(m->th, nm_place.ex + 32 * A, nm_place.ey + 32 * B);
	for (int k = 0; k < p->ntiles; ++k) side_put(p, &p->tiles[k], m->map, (size_t)m->tw * m->th, (size_t)((py0 + p->tiles[k].py) / 8) * m->tw + (px0 + p->tiles[k].px) / 8);
	return true;
}

/* The piece for grid panel (x, y), by hash h: one of those cut at its
 * neighbourhood, of kind `kind` (0 on a floor's side, 1 under a pad, 2
 * behind a floor), NULL none; of the looks that keep the first layer's
 * tiles (SidePiece.over), only those that fit there. */
static const SidePiece *side_for(const SideBook *b, int kind, int x, int y, unsigned h, const SideMap *m) {
	unsigned f = side_floor(x, y);
	int match[SIDES_MAX], nmatch = 0;
	for (int k = 0; k < b->n; ++k) {
		const SidePiece *p = &b->piece[k];
		/* (the second layer's pieces behind a floor stand behind any
		 * stretch of its upper edge: the panels they draw over void, as
		 * in their maps, whatever floor lies before them; so do those on
		 * both layers, which reach past the nine, as a tank does past its
		 * landing's road) */
		bool any = p->behind && (!p->hang || p->both);
		if (!p->grows && (p->behind ? 2 : p->hang) == kind && (p->floor == f || any) && side_under(p, x, y) && (!p->over || side_fits(p, m, x, y))) match[nmatch++] = k;
	}
	return nmatch ? &b->piece[match[(h >> 4) % (unsigned)nmatch]] : NULL;
}

/* The panels a layer's pieces of what grows on a floor's edge stand on. */
typedef struct { int at[GROWS_A_LAYER][2], n; } Grown;

/* What grows on a floor's edge set at grid panel (x, y): on two panels
 * that fit in three, none a panel from another, as the maps scatter theirs
 * (on every one, a straight edge took one every other panel); the first of
 * those cut at its neighbourhood that fits, from the one the seed picks.
 * NULL none. */
static const SidePiece *grows_set(const SideBook *b, const SideMap *m, int x, int y, Grown *g) {
	unsigned h = (nm_cur->seed ^ (uint32_t)(x * 7919 + y * 104729)) * 2654435761u >> 8;
	if (g->n == GROWS_A_LAYER || (h >> 12) % GROWS_SKIP == 0) return NULL;
	for (int k = 0; k < g->n; ++k)
		if (abs(g->at[k][0] - x) < 2 && abs(g->at[k][1] - y) < 2) return NULL;
	unsigned f = side_floor(x, y);
	int match[SIDES_MAX], nmatch = 0;
	for (int k = 0; k < b->n; ++k)
		if (b->piece[k].grows && b->piece[k].floor == f && side_under(&b->piece[k], x, y)) match[nmatch++] = k;
	for (int i = 0; i < nmatch; ++i) {
		const SidePiece *p = &b->piece[match[((h >> 4) + (unsigned)i) % (unsigned)nmatch]];
		if (!side_set(p, m, x, y)) continue;
		g->at[g->n][0] = x;
		g->at[g->n++][1] = y;
		return p;
	}
	return NULL;
}

/* Piece p set at grid panel (x, y), or where there is none (and the panel
 * is no pad's), what grows on the floor's edge there; the piece set, NULL none. */
static const SidePiece *side_or_grows(const SidePiece *p, const SideBook *b, const SideMap *m, int x, int y, bool hang, Grown *g) {
	if (p) return side_set(p, m, x, y) ? p : NULL;
	return hang ? NULL : grows_set(b, m, x, y, g);
}

/* (debug output) what stands under a pad's panel or behind a floor, or
 * grows on its edge, set at grid panel (x, y) */
static void hang_said(const SidePiece *p, int x, int y) {
	if (!emu_debug_on()) return;
	int A, B;
	nm_grid_to_panel(x, y, &A, &B);
	fprintf(stderr, "  %s at world %d,%d: %d tiles\n", p->grows ? "on the floor's edge" : p->behind ? "behind the floor" : "under the pad's panel",
		nm_place.ex + 32 * A, nm_place.ey + 32 * B, p->ntiles);
}

/* (measuring) the breaks across the edges of a side piece's foot, set
 * whole over the floor's art, are none: its map stands it on a floor of
 * another look (the Aquarium's tanks behind their orange landings), so the
 * layer's floor going on under its foot shows strips no map does, as the
 * landing goes on under it there (docs/world/level-design/side-pieces.md) */
void nm_unfoot_breaks(int tw, int th) {
	uint8_t *b = nm_last.breaks;
	for (int i = 0; b && nm_last.pasted && i < tw * th; ++i) {
		if (!(nm_last.pasted[i] & NETMAP_PASTED_FOOT)) continue;
		/* (its right and lower edges, its left neighbour's right and its upper one's lower) */
		uint8_t *at[4] = { &b[i], &b[i], i % tw ? &b[i - 1] : NULL, i >= tw ? &b[i - tw] : NULL };
		const uint8_t edge[4] = { 1, 2, 1, 2 };
		for (int k = 0; k < 4; ++k) {
			if (!at[k]) continue;
			if (*at[k] & edge[k]) { *at[k] &= (uint8_t)~edge[k]; --tiles_stats.breaks; }
			if (*at[k] & edge[k] << 2) { *at[k] &= (uint8_t)~(edge[k] << 2); --tiles_stats.nicks; }
		}
	}
}

static struct { const SidePiece *p; int X, Y; } side_at[SIDES_A_LAYER + 1];   /* (and the piece in a hole) */
static int nside_at;

/* The area's piece in a walled hole where the generator stood it
 * (PROP_HOLE), whole on the second layer over the floor drawn round and
 * under it, kept with the side pieces for its layer priorities. */
static void paste_hole(const Learned *L, uint16_t *map, int tw, int th) {
	const SidePiece *h = &L->hole;
	if (!h->ntiles) return;
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < nm_cur->nprops && i < MAX_PROPS; ++i) {
		const NetProp *p = &nm_cur->props[i];
		if (p->kind != PROP_HOLE) continue;
		int A, B;
		nm_grid_to_panel(p->x, p->y + p->faces - 1, &A, &B);
		int X = nm_place.ex + 32 * A, Y = nm_place.ey + 32 * B, px0 = area_px(tw, X, Y), py0 = area_py(th, X, Y);
		for (int k = 0; k < h->ntiles; ++k) {
			int px = px0 + h->tiles[k].px, py = py0 + h->tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
			size_t at = (size_t)(py / 8) * tw + px / 8;
			map[cells + at] = h->tiles[k].e1;
			if (nm_last.pasted) nm_last.pasted[at] |= NETMAP_PASTED_EMBLEM;
		}
		side_at[nside_at].p = h;
		side_at[nside_at].X = X;
		side_at[nside_at++].Y = Y;
		if (emu_debug_on()) fprintf(stderr, "piece in a hole at world %d,%d: %d tiles, %d layer priorities\n", X, Y, h->ntiles, h->nprio);
		return;
	}
}

int nm_sides_set(int i, const SidePiece **p, int *X, int *Y) {
	if (i >= 0 && i < nside_at) { *p = side_at[i].p; *X = side_at[i].X; *Y = side_at[i].Y; }
	return nside_at;
}

int netmap_sides(void) { return nside_at; }

/* Whether grid panel (x, y) is ground floor (side_floor's). */
static bool side_ground(int x, int y) {
	return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] != C_VOID && !layer.level[y][x];
}

/* Whether grid panel (x, y) is a walkway's a panel wide running along grid
 * x (dx 1) or y (dy 1): floor with none on either side across it. */
static bool side_walk(int x, int y, int dx, int dy) {
	return side_ground(x, y) && !side_ground(x - dy, y - dx) && !side_ground(x + dy, y + dx);
}

/* The walkway run grid panel (x, y) lies in, two panels long or more, as a
 * hash of its first panel and its way (*d: 0 along grid x, 1 along y); 0
 * none. */
static unsigned side_run(int x, int y, int *d) {
	for (*d = 0; *d < 2; ++*d) {
		int dx = !*d, dy = *d, sx = x, sy = y, n = 1;
		if (!side_walk(x, y, dx, dy)) continue;
		while (side_walk(sx - dx, sy - dy, dx, dy)) sx -= dx, sy -= dy, ++n;
		while (side_walk(sx + n * dx, sy + n * dy, dx, dy)) ++n;
		return n < 2 ? 0 : ((nm_cur->seed ^ (uint32_t)(sx * 7919 + sy * 104729 + *d * 15485863)) * 2654435761u >> 8) | 1;
	}
	return 0;
}

/* (SIDES_RUNS) side_for on grid panel (x, y) of a walkway run of way d,
 * its neighbourhood compared along the run and on the side the piece
 * stands (`kind` 2 behind it, the upper; 0 in front, the lower), whatever
 * lies across the run on the other (a room's corner, a joint's): every look
 * the maps cut beside a catwalk differs there alone. Bit k of a
 * neighbourhood is panel (A + k % 3 - 1, B + k / 3 - 1); a run along grid x
 * runs along B, its upper side A + 1, one along grid y along A, its upper
 * side B - 1. */
static const SidePiece *side_for_run(const SideBook *b, int kind, int x, int y, unsigned h, const SideMap *m, int d) {
	unsigned other = d ? (kind ? 0x1C0u : 0x007u) : (kind ? 0x049u : 0x124u), f = side_floor(x, y) & ~other;
	int match[SIDES_MAX], nmatch = 0;
	for (int k = 0; k < b->n; ++k) {
		const SidePiece *p = &b->piece[k];
		if ((p->behind ? 2 : p->hang) == kind && (p->floor & ~other) == f && side_under(p, x, y) && side_fits(p, m, x, y)) match[nmatch++] = k;
	}
	return nmatch ? &b->piece[match[(h >> 4) % (unsigned)nmatch]] : NULL;
}

#define RUNS_EVERY 2   /* (SIDES_RUNS) a walkway run in this many takes a row behind it */
#define RUNS_LINED 3   /* ... and one of those in this many a row in front too */

/* (SIDES_RUNS) The piece behind free grid panel (x, y), by hash h, and in
 * `front` the one on its side (NULL none), as BN4's Undernet's maps line
 * their catwalks: every other walkway run a horn behind each panel it fits
 * on, a third of those runs one in front too; none elsewhere. */
static const SidePiece *side_pick_run(const SideBook *b, int x, int y, unsigned h, const SideMap *m, const SidePiece **front) {
	int d;
	unsigned r = side_run(x, y, &d);
	*front = NULL;
	if (!r || (r >> 4) % RUNS_EVERY) return NULL;
	if (!((r >> 12) % RUNS_LINED)) *front = side_for_run(b, 0, x, y, h, m, d);
	return side_for_run(b, 2, x, y, h, m, d);
}

/* The piece for free grid panel (x, y), by hash h, NULL none (`near`:
 * within SIDES_APART of one set): under a pad's panel whatever fits; on a
 * floor's side half, apart, or where its look keeps the first layer's
 * tiles, every one where it fits, apart; behind a floor's edge whatever
 * fits in one stretch of four panels square in three, as the maps set
 * theirs in rows. */
static const SidePiece *side_pick(const SideBook *b, int x, int y, unsigned h, bool near, const SideMap *m) {
	if (hang_free(x, y)) return side_for(b, 1, x, y, h, m);
	const SidePiece *p = near ? NULL : side_for(b, 0, x, y, h, m);
	if (p && !p->over && h % 2) p = NULL;
	unsigned stretch = (nm_cur->seed ^ (uint32_t)(x / 4 * 7919 + y / 4 * 104729)) * 2654435761u >> 8;
	return p || stretch % 3 ? p : side_for(b, 2, x, y, h, m);
}

void nm_paste_sides(const Learned *L, uint16_t *map, int tw, int th) {
	nside_at = 0;
	if (nm_cur->gw != MAP_W) return;
	paste_hole(L, map, tw, th);
	if (!L->sides.n) return;
	int set[SIDES_A_LAYER][2], nset = 0, nhang = 0;
	SideMap m = { map, tw, th, voids_opaque(L->def->xrom, L->def->group, L->def->number), &L->seams };
	Grown grown = { .n = 0 };
	for (int y = 1; y < MAP_H - 1 && nset < SIDES_A_LAYER; ++y)
		for (int x = 1; x < MAP_W - 1 && nset < SIDES_A_LAYER; ++x) {
			unsigned h = (nm_cur->seed ^ (uint32_t)(x * 7919 + y * 104729)) * 2654435761u >> 8;
			bool near = false;
			for (int k = 0; k < nset && !near; ++k) near = abs(set[k][0] - x) < SIDES_APART && abs(set[k][1] - y) < SIDES_APART;
			bool hang = hang_free(x, y);
			if (!hang && !side_free(x, y)) continue;
			/* (BN4's Undernet's horns in rows: the one behind, then the one in front) */
			const SidePiece *front = NULL, *p = L->def->sides & SIDES_RUNS ? side_pick_run(&L->sides, x, y, h, &m, &front) : side_pick(&L->sides, x, y, h, near, &m);
			if (p && front && side_set(p, &m, x, y)) ++nhang;
			if (front) p = front;
			if (!(p = side_or_grows(p, &L->sides, &m, x, y, hang, &grown))) continue;
			if (p->hang || p->behind) {
				++nhang;
				hang_said(p, x, y);
				continue;
			}
			int A, B;
			nm_grid_to_panel(x, y, &A, &B);
			side_at[nside_at].p = p;
			side_at[nside_at].X = nm_place.ex + 32 * A;
			side_at[nside_at++].Y = nm_place.ey + 32 * B;
			set[nset][0] = x; set[nset++][1] = y;
		}
	if (emu_debug_on()) {
		fprintf(stderr, "side pieces: %d set and %d under pads or behind floors, of the area's %d\n", nset, nhang, L->sides.n);
		for (int k = 0; k < nside_at; ++k) fprintf(stderr, "  side piece at world %d,%d: %d tiles, %d layer priorities\n", side_at[k].X, side_at[k].Y, side_at[k].p->ntiles, side_at[k].p->nprio);
	}
}

bool netmap_prop_navi(int i, int *wx, int *wy, int *tx, int *ty) {
	if (i < 0 || i >= MAX_PROPS || !nm_prop_at[i].ok) return false;
	*wx = nm_prop_at[i].X + nm_prop_at[i].st->navi_x;
	*wy = nm_prop_at[i].Y + nm_prop_at[i].st->navi_y;
	*tx = nm_prop_at[i].st->talk_x;
	*ty = nm_prop_at[i].st->talk_y;
	return true;
}

/* One shade of floor: tiles of the other bank in this one's colours, where
 * its maps draw the same tile in it. */
void nm_rebank_map(const Learned *L, uint16_t *map, size_t cells) {
	const uint8_t *rb = L->rebank;
	if (!rb[0] || !rb[1]) return;
	for (size_t i = 0; i < cells; ++i) {
		uint16_t e = map[i];
		if (e >> 12 == rb[0] && (e & 0x3FF) && L->rebank_to[(e & 0x3FF) >> 3] >> (e & 7) & 1) map[i] = (uint16_t)((e & 0x0FFF) | rb[1] << 12);
	}
}
