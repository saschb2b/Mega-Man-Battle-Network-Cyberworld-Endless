/* The original maps' scenery pasted onto a generated one (netmap.c):
 * stairs, props, ornaments, pads, emblems, arrows and bushes, where the
 * layer has them. */
#include "netmap_paste.h"

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "net_shapes.h"
#include "netmap_learn.h"

/* ---- output ---- */

/* A stair block's corner with the lowest panel indices, in world units. */
void nm_stair_origin(const Stair *st, int *X0, int *Y0) {
	int A0 = -(st->y + 1 - nm_place.gy0), B0 = st->x - nm_place.gx0;
	*X0 = nm_place.ex + 32 * A0;
	*Y0 = nm_place.ey + 32 * B0;
}

/* The stairs' own tiles over whatever the classes put there. */
void nm_paste_stairs(const Learned *L, uint16_t *map, int tw, int th) {
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < nm_cur->nstairs; ++i) {
		const StairTemplate *t = &L->stairs[nm_cur->stairs[i].dir];
		if (!t->ok) continue;
		int X0, Y0;
		nm_stair_origin(&nm_cur->stairs[i], &X0, &Y0);
		int px0 = X0 + Y0 + tw * 4, py0 = (Y0 - X0) / 2 + th * 4;
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
	for (int i = 0; i < 2 && layer.nteleports; ++i) {
		int x = layer.teleport_x[i], y = layer.teleport_y[i];
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

/* The area's own pad, whole, on every 3 x 3 pad at ground level away from
 * the stairs, in place of the floor the classes drew there (the classes
 * drew Central's pads green, where its maps frame them); their
 * centrepieces go on after. */
/* Whether the tile at map pixel (px, py) lies over pad `m`'s own panels or
 * the void alone (at the floor's height, drawn dv below the panel edges). */
static bool pad_tile_ok(const Learned *L, const Room *m, int tw, int th, int px, int py) {
	for (int y = 1; y < 8; y += 2)
		for (int x = 1; x < 8; x += 2) {
			int u = px + x - tw * 4, v = 2 * (py + y - L->book[0].dv - th * 4), X = (u - v) / 2, Y = (u + v) / 2;
			int A = nm_floordiv(X - nm_place.ex, 32), B = nm_floordiv(Y - nm_place.ey, 32);
			int gx = B + nm_place.gx0, gy = -A + nm_place.gy0;
			if (gx >= m->x && gx < m->x + 3 && gy >= m->y && gy < m->y + 3) continue;
			if (gx >= 0 && gy >= 0 && gx < MAP_W && gy < MAP_H && layer.cell[gy][gx] != C_VOID) return false;
		}
	return true;
}

void nm_paste_pads(const Learned *L, uint16_t *map, int tw, int th) {
	if (!L->pad.ok) return;
	for (int i = 0; i < layer.nrooms; ++i) {
		const Room *m = &layer.rooms[i];
		if (m->kind != ROOM_PAD || m->w != 3 || m->h != 3) continue;
		bool flat = true;
		for (int y = m->y; y < m->y + 3; ++y)
			for (int x = m->x; x < m->x + 3; ++x) flat &= layer.cell[y][x] == C_PATH && !layer.level[y][x];
		for (int s = 0; s < layer.nstairs; ++s)
			flat &= layer.stair[s].x + 2 < m->x - 1 || layer.stair[s].x > m->x + 3 || layer.stair[s].y + 2 < m->y - 1 || layer.stair[s].y > m->y + 3;
		if (!flat) continue;
		int A, B;
		nm_grid_to_panel(m->x, m->y + 2, &A, &B);
		int X0 = nm_place.ex + 32 * A, Y0 = nm_place.ey + 32 * B;
		int px0 = area_px(tw, X0, Y0), py0 = area_py(th, X0, Y0);
		/* the stamp only over the pad's own panels or the void: its edges
		 * and faces would cut into a walkway or a platform it touches (a
		 * walkway's last panel before the pad came out cut) */
		for (int k = 0; k < L->pad.ntiles; ++k) {
			int px = px0 + L->pad.tiles[k].px, py = py0 + L->pad.tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th || !pad_tile_ok(L, m, tw, th, px, py)) continue;
			map[(size_t)(py / 8) * tw + px / 8] = L->pad.tiles[k].e0;
			if (nm_last.pasted) nm_last.pasted[(size_t)(py / 8) * tw + px / 8] |= NETMAP_PASTED_PAD;
		}
	}
}

/* The emblems the generator set in the floor (PROP_EMBLEM), in place of
 * the floor the classes drew there. */
void nm_paste_emblems(const Learned *L, uint16_t *map, int tw, int th) {
	if (!L->emblem.ok) return;
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < nm_cur->nprops && i < MAX_PROPS; ++i) {
		const NetProp *p = &nm_cur->props[i];
		if (p->kind != PROP_EMBLEM) continue;
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
