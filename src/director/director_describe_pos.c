/* The state's lines for a developer reproducing a playtest
 * (director_describe_pos.h; CYBERWORLD_STATE_POS): where Lan or MegaMan
 * stands and what holds him, the floor around him, the arrow's way and
 * heal, the layer drawn in letters (=map), the game's NPC objects near
 * him. Reads alone. */
#include "director_describe_pos.h"

#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "cinema.h"
#include "director_home.h"
#include "director_state.h"
#include "director_way.h"
#include "flags.h"
#include "net_route.h"
#include "net_shapes.h"
#include "net_signature.h"
#include "netmap.h"

/* An object's letter on the state's map (D a Navi gate, V a vault, Y the
 * rival, O an official gate; m a green Mystery Data, M a blue one, L a
 * purple one, locked), 0 none. */
static char state_mark(const NetObj *o) {
	static const char mark[] = "IXMSHTTBUGNCPRFDVYO";
	if (o->type == OBJ_MYSTERY && o->param != 1 && o->param != 2) return o->param ? 'L' : 'm';
	return o->type >= 0 && o->type < (int)sizeof mark - 1 ? mark[o->type] : 0;
}

static void box_add(int box[4], int x, int y) {
	box[0] = x < box[0] ? x : box[0];
	box[1] = y < box[1] ? y : box[1];
	box[2] = x > box[2] ? x : box[2];
	box[3] = y > box[3] ? y : box[3];
}

/* The layer drawn in letters for a state (CYBERWORLD_STATE_POS=map): . floor,
 * ^ raised floor, = a counter's aisle, # the counter, , the room before a
 * guardian's arena, % a sprite prop (in the void or its walled hole); the
 * box its floor and props take (x0, y0, x1, y1). */
static void map_cells(char g[MAP_H][MAP_W + 1], int box[4]) {
	box[0] = MAP_W; box[1] = MAP_H; box[2] = box[3] = 0;
	for (int y = 0; y < MAP_H; ++y) {
		for (int x = 0; x < MAP_W; ++x) {
			g[y][x] = layer.cell[y][x] == C_VOID ? ' ' : layer.cell[y][x] == C_SOLID ? '=' : layer.cell[y][x] == C_PROPPED ? '#'
				: layer.level[y][x] ? '^' : '.';
			if (layer.cell[y][x] != C_VOID) box_add(box, x, y);
		}
		g[y][MAP_W] = 0;
	}
	if (layer.ante >= 0) {
		const Room *r = &layer.rooms[layer.ante];
		for (int y = r->y; y < r->y + r->h; ++y)
			for (int x = r->x; x < r->x + r->w; ++x)
				if (g[y][x] == '.') g[y][x] = ',';
	}
	for (int i = 0; i < layer.nprops; ++i)
		if (layer.props[i].kind == PROP_SPRITE) {
			g[layer.props[i].y][layer.props[i].x] = '%';
			box_add(box, layer.props[i].x, layer.props[i].y);
		}
}

/* ... and over them: ~ a Rush gap's panels, W a teleport, K a Link Navi
 * obstacle or a cube, > an arrow lane's panels, ? an invisible path's, *
 * the arrow's walk, + where it aims, letters the objects, @ MegaMan at
 * panel (cx, cy); and each lane's and path's ends in the world. */
static void state_map(FILE *f, int cx, int cy) {
	goal_way();
	static char g[MAP_H][MAP_W + 1];
	int box[4];
	map_cells(g, box);
	for (int k = 0; k < layer.ngaps; ++k)
		for (int j = 1; j <= layer.gap[k].len; ++j) g[layer.gap[k].y + dir_dy[layer.gap[k].dir] * j][layer.gap[k].x + dir_dx[layer.gap[k].dir] * j] = '~';
	for (int k = 0; k < 2 && layer.nteleports; ++k) g[layer.teleport_y[k]][layer.teleport_x[k]] = 'W';
	for (int k = 0; k < layer.nblocks; ++k) g[layer.block[k].y][layer.block[k].x] = 'K';
	for (int k = 0; k < layer.nlanes; ++k)
		for (int j = 1; j <= layer.lane[k].len; ++j) g[layer.lane[k].y + dir_dy[layer.lane[k].dir] * j][layer.lane[k].x + dir_dx[layer.lane[k].dir] * j] = '>';
	for (int k = 0; k < layer.npaths; ++k)
		for (int j = 1; j <= layer.path[k].len; ++j) g[layer.path[k].y + dir_dy[layer.path[k].dir] * j][layer.path[k].x + dir_dx[layer.path[k].dir] * j] = '?';
	for (int i = 0; i < route_walk_len; ++i) g[route_walk[i] / MAP_W][route_walk[i] % MAP_W] = '*';
	if (route_walk_aim >= 0) g[route_walk_aim / MAP_W][route_walk_aim % MAP_W] = '+';
	for (int i = 0; i < layer.nobj; ++i) {
		int ox = (int)layer.obj[i].x, oy = (int)layer.obj[i].y;
		if (ox >= 0 && oy >= 0 && ox < MAP_W && oy < MAP_H && state_mark(&layer.obj[i])) g[oy][ox] = state_mark(&layer.obj[i]);
	}
	g[cy][cx] = '@';
	for (int y = box[1]; y <= box[3]; ++y) fprintf(f, "map %.*s\n", box[2] - box[0] + 1, &g[y][box[0]]);
	for (int k = 0; k < layer.nlanes; ++k) {
		const NetLane *l = &layer.lane[k];
		int ax, ay, bx, by;
		netmap_world(l->x, l->y, &ax, &ay);
		netmap_world(l->x + dir_dx[l->dir] * (l->len + 1), l->y + dir_dy[l->dir] * (l->len + 1), &bx, &by);
		fprintf(f, "lane %d panels from %d %d (world %d %d) to world %d %d\n", l->len, l->x, l->y, ax, ay, bx, by);
	}
	for (int k = 0; k < layer.npaths; ++k) {
		const NetGap *p = &layer.path[k];
		int ax, ay, bx, by;
		netmap_world(p->x, p->y, &ax, &ay);
		netmap_world(p->x + dir_dx[p->dir] * (p->len + 1), p->y + dir_dy[p->dir] * (p->len + 1), &bx, &by);
		fprintf(f, "path %d panels from %d %d (world %d %d) to world %d %d\n", p->len, p->x, p->y, ax, ay, bx, by);
	}
	for (int k = 0; k < layer.nblocks; ++k) {
		static const char *const kinds[] = { "geyser", "tree", "flames", "cyclone", "cloud", "pcode cube", "toll cube", "skull door", "number door" };
		int bx, by;
		netmap_world(layer.block[k].x, layer.block[k].y, &bx, &by);
		fprintf(f, "block %s at world %d %d, braziers %d\n", layer.block[k].kind >= 0 && layer.block[k].kind <= BLOCK_NUMBER ? kinds[layer.block[k].kind] : "?",
			bx, by, layer.braziers);
	}
	if (layer.hinter) {
		int hx, hy;
		netmap_world((int)layer.obj[layer.hinter - 1].x, (int)layer.obj[layer.hinter - 1].y, &hx, &hy);
		fprintf(f, "hinter at world %d %d\n", hx, hy);
	}
}

/* The arrow's heal first, the shortest walk there since L named it, or
 * MegaMan turned away from it (heal_turn_watch) */
static void describe_heal(FILE *f) {
	int hx, hy;
	if (heal_first(&hx, &hy)) fprintf(f, "heal first at %d %d, nearest %d\n", hx, hy, D.heal_near - 1);
	else if (D.heal_declined) fprintf(f, "heal turned away from\n");
}

/* For the developer reproducing a playtest (CYBERWORLD_STATE_POS): where
 * Lan or MegaMan is, the floor around him, the layer drawn in letters
 * (=map), and the game's NPC objects near him */
void director_describe_pos(FILE *f) {
	fprintf(f, "pos %d %d %d locked %d jt %02x ace0 %d canmove %d f1718 %d f1719 %d cinema %d\n", bn6_player_x(),
		bn6_player_y(), bn6_player_z(), emu_read8(BN6_PLAYER_LOCKED),
		emu_read8(BN6_PLAYER_STATE), emu_read8(BN6_DIALOGUE_LOCK), flag_get(BN6_FLAG_PLAYER_CAN_MOVE), flag_get(BN6_FLAG_DIALOGUE_1718),
		flag_get(BN6_FLAG_DIALOGUE_1719), cinema_input_mode());
	if (D.town) home_describe_pos(f);
	else {
		int ns = 0, nf = 0;
		for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) { ns += D.seen[y][x]; nf += D.seen[y][x] && layer.cell[y][x] == C_PATH; }
		fprintf(f, "seen %d floor %d\n", ns, nf);
		fprintf(f, "exit %d %d\nscripts shop %d heal %d gift %d programs %d\n", D.objs.exit_x, D.objs.exit_y, D.objs.script_of[OBJ_SHOP],
			D.objs.script_of[OBJ_HEAL], D.objs.script_of[OBJ_GIFT], D.objs.script_of[OBJ_PROGRAMS]);
		/* (the room the layer is remembered by, its anchor: docs/world/level-design/identity.md) */
		if (layer.sig_room >= 0 && layer.sig > SIG_NONE && layer.sig < SIG_COUNT) {
			const Room *r = &layer.rooms[layer.sig_room];
			int sx, sy;
			netmap_world(r->ax, r->ay, &sx, &sy);
			fprintf(f, "signature %s at panel %d %d (world %d %d), %d x %d\n", sig_names[layer.sig], r->ax, r->ay, sx, sy, r->w, r->h);
		}
		/* the floor around him, panels (x across, y down; @ he, # floor) */
		int px = bn6_player_x(), py = bn6_player_y(), cx, cy;
		if (netmap_panel(px, py, &cx, &cy)) {
			int wx, wy;
			netmap_world(cx, cy, &wx, &wy);
			fprintf(f, "panel %d %d (centre %d %d)\n", cx, cy, wx, wy);
			goal_way();
			fprintf(f, "way %d %s\n", way_last(), ways[way_last()]);
			describe_heal(f);
			for (int y = cy - 4; y <= cy + 4; ++y) {
				fprintf(f, "cells ");
				for (int x = cx - 4; x <= cx + 4; ++x)
					fputc(x == cx && y == cy ? '@' : x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH ? '#' : '.', f);
				fputc('\n', f);
			}
			/* (CYBERWORLD_STATE_POS=map: the whole layer, with its objects
			 * and the arrow's walk: * the walk, + where the arrow aims,
			 * letters the objects, ^ raised floor, , the floor of the
			 * room before a guardian's arena) */
			if (!strcmp(getenv("CYBERWORLD_STATE_POS"), "map")) state_map(f, cx, cy);
		}
	}
	/* the game's NPC objects near him: flags, state, radius, lock, text */
	int px = bn6_player_x(), py = bn6_player_y();
	for (int i = 0; i < 16; ++i) {
		uint32_t o = BN6_NPC_OBJECTS + (uint32_t)i * BN6_NPC_SIZE;
		int x = (int16_t)emu_read16(o + BN6_NPC_X16), y = (int16_t)emu_read16(o + BN6_NPC_Y16), z = (int16_t)emu_read16(o + BN6_NPC_Z16);
		if (!(emu_read8(o) & 1) || ((abs(x - px) > 64 || abs(y - py) > 64) && !getenv("CYBERWORLD_STATE_ALLNPC"))) continue;
		fprintf(f, "npc %d flags %02x state %02x radius %d zreach %d locked %d text %d at %d %d %d\n", i, emu_read8(o),
			emu_read8(o + BN6_NPC_STATE), emu_read8(o + BN6_NPC_RADIUS), emu_read8(o + BN6_NPC_Z_REACH), emu_read8(o + BN6_NPC_LOCKED), emu_read8(o + BN6_NPC_SCRIPT), x, y, z);
	}
}
