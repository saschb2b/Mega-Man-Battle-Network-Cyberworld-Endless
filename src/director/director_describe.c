/* What a player sees, in words, one fact a line (director_describe): the
 * state play.py prints after each batch, and with CYBERWORLD_STATE_POS what a
 * developer reproducing a playtest needs (the layer drawn in letters). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "boss.h"
#include "cinema.h"
#include "director.h"
#include "director_folder.h"
#include "director_hold.h"
#include "director_layer.h"
#include "director_state.h"
#include "director_way.h"
#include "guardians.h"
#include "home_places.h"
#include "job_words.h"
#include "net_route.h"
#include "net_shapes.h"
#include "net_signature.h"
#include "netmap.h"
#include "save.h"
#include "talk.h"
#include "town.h"

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

static void print_near(int id, int x, int y, void *ctx) {
	int px = bn6_player_x(), py = bn6_player_y();
	if (abs(x - px) < 48 && abs(y - py) < 48) fprintf(ctx, "near %s %d %d\n", id < 0 ? "folk" : "object", x, y);
}

/* What the game is doing, for a state, and MegaMan's HP there: in a
 * battle the HUD's is his battle object's, and in an older net's battle
 * the guest's (BN6 stands on the map meanwhile, at the battle's start's) */
bool director_megaman_hp(int *hp, int *max) {
	if (guest_fight_hp(hp, max)) return true;
	*hp = emu_read16(BN6_NAVI_HP);
	*max = emu_read16(BN6_NAVI_MAX_HP);
	/* (an older net's battle beginning or ending: the Navi's still) */
	if (guest_active() || on_map()) return false;
	for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		if ((emu_read8(o) & 1) && emu_read8(o + BN6_T1_ALLIANCE) == 0) { *hp = emu_read16(o + BN6_T1_HP); *max = emu_read16(o + BN6_T1_MAX_HP); break; }
	}
	return false;
}

static const char *state_doing(int *hp, int *max) {
	if (director_megaman_hp(hp, max)) return "battle (the older net's)";
	/* (its opening and its end, the screen white or BN5's: still its) */
	if (guest_active()) return "battle (the older net's, beginning or ending)";
	int mode = main_mode(), sub = emu_read8(BN6_GAMESTATE);
	return mode == BN6_MODE_GAME_OVER ? "gameover"
		: mode != BN6_MODE_GAME ? "menu"
		: sub == BN6_SUB_MAP ? "map" : sub == BN6_SUB_BATTLE || sub == BN6_SUB_BATTLE_INIT ? "battle" : "other";
}

/* In a battle, the panel MegaMan stands on, from the left and the top: a
 * player sees it at a glance, a playtester reading stills misread it turn
 * after turn, as BN6 draws him half a row above his panel. In an older
 * net's battle too, with its gauge (a playtester misread rows against
 * KnightMan and held a charge in his row, session 68; MegaMan is its first
 * object, bn5.h). */
static void battle_describe(FILE *f) {
	if (guest_on_screen() && guest_read8(BN5_T1_OBJECTS + BN6_T1_PANEL_X)) {
		int g = guest_custom_gauge();
		if (g >= 0) fprintf(f, "custom gauge %d%%\n", g * 100 / 0x4000);
		fprintf(f, "megaman stands column %d row %d\n", guest_read8(BN5_T1_OBJECTS + BN6_T1_PANEL_X), guest_read8(BN5_T1_OBJECTS + BN6_T1_PANEL_Y));
		return;
	}
	if (on_map()) return;
	for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		if ((emu_read8(o) & 1) && emu_read8(o + BN6_T1_ALLIANCE) == 0 && emu_read8(o + BN6_T1_PANEL_X)) {
			fprintf(f, "megaman stands column %d row %d\n", emu_read8(o + BN6_T1_PANEL_X), emu_read8(o + BN6_T1_PANEL_Y));
			return;
		}
	}
}

/* at home: in one of its places the way on (the PC, the stairs up, the
 * way out), in the town its objects and Lan's front door */
static void describe_home(FILE *f) {
	int group = emu_read8(BN6_MAP_GROUP), number = emu_read8(BN6_MAP_NUMBER), x, y;
	fprintf(f, "map %02x:%02x\n", group, number);
	/* (the jobs' flags, job_words.h: taken 0-2, held, settled; the run's job) */
	fprintf(f, "jobs %d%d%d held %d settled %d job %d from %d state %d got %d\n", flag_get(JOB_TAKE_FLAG), flag_get(JOB_TAKE_FLAG + 1),
		flag_get(JOB_TAKE_FLAG + 2), flag_get(JOB_HELD_FLAG), flag_get(JOB_SETTLED_FLAG), run.job.kind, run.job.asker, run.job.state, run.job.got);
	if (home_places_way(group, number, &x, &y)) fprintf(f, "goal %d %d\n", x, y);
	else if (director_in_town()) { town_objects(print_near, f); fprintf(f, "port %d %d\n", town_info()->port_x, town_info()->port_y); }
}

/* For the developer reproducing a playtest (CYBERWORLD_STATE_POS): where
 * Lan or MegaMan is, the floor around him, the layer drawn in letters
 * (=map), and the game's NPC objects near him */
static void describe_pos(FILE *f) {
	fprintf(f, "pos %d %d %d locked %d jt %02x ace0 %d canmove %d f1718 %d f1719 %d cinema %d\n", bn6_player_x(),
		bn6_player_y(), bn6_player_z(), emu_read8(BN6_PLAYER_LOCKED),
		emu_read8(BN6_PLAYER_STATE), emu_read8(BN6_DIALOGUE_LOCK), flag_get(BN6_FLAG_PLAYER_CAN_MOVE), flag_get(BN6_FLAG_DIALOGUE_1718),
		flag_get(BN6_FLAG_DIALOGUE_1719), cinema_input_mode());
	/* (what of BN6 holds him, issue #23) */
	fprintf(f, "held %s\n", held_names(bn6_held()));
	if (D.town) describe_home(f);
	else {
		int ns = 0, nf = 0;
		for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) { ns += D.seen[y][x]; nf += D.seen[y][x] && layer.cell[y][x] == C_PATH; }
		fprintf(f, "seen %d floor %d\n", ns, nf);
		fprintf(f, "exit %d %d\nscripts shop %d heal %d gift %d programs %d\n", D.objs.exit_x, D.objs.exit_y, D.objs.script_of[OBJ_SHOP],
			D.objs.script_of[OBJ_HEAL], D.objs.script_of[OBJ_GIFT], D.objs.script_of[OBJ_PROGRAMS]);
		/* (the room the layer is remembered by, its anchor: docs/LEVEL_DESIGN.md, Identity) */
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

void director_describe(FILE *f) {
	if (!D.active) { fprintf(f, "where none\n"); return; }
	int sub = emu_read8(BN6_GAMESTATE), hp, max;
	const char *doing = state_doing(&hp, &max);
	fprintf(f, "where %s\ndoing %s\nchat %s\ntalk %s\n", D.town ? "town" : "layer", doing,
		emu_read8(BN6_CHATBOX) ? "open" : "closed", talk_busy() ? "director" : "none");
	if (D.town) fprintf(f, "place %s\n", director_place_name());
	/* (GameState's protected zenny, then its BugFrags) */
	fprintf(f, "hp %d/%d\nzenny %u\nbugfrags %u\n", hp, max, (unsigned)emu_read32(BN6_ZENNY), (unsigned)emu_read32(BN6_BUGFRAGS));
	if (sub == BN6_SUB_BATTLE) fprintf(f, "custom gauge %d%%\n", emu_read16(BN6_CUSTOM_GAUGE) * 100 / 0x4000);
	battle_describe(f);
	/* (for the developer reproducing a playtest: where Lan or MegaMan is) */
	if (getenv("CYBERWORLD_STATE_POS")) describe_pos(f);
	if (D.town) return;
	fprintf(f, "layer %d\narea %s\nscrtdata %d\n", run.depth, guardian_area_in_text(run.biome, run.side_kind), run.fragments);
	/* (a trip back, and the Net's clock: docs/HOME.md) */
	if (run.home_depth) fprintf(f, "back from layer %d\n", run.home_depth);
	if (run.clock) fprintf(f, "clock %d\n", run.clock);
	/* (named as the game shows him: a playtester reads this; a dev's
	 * state, CYBERWORLD_STATE_POS, names him always, as the scripts that
	 * find a guardian's seed read it) */
	if (D.objs.guardian.navi)
		fprintf(f, "guardian %s %s\n",
			guardian_known(D.objs.guardian.navi) || guardian_heard() || boss_cinematic() || boss_fighting() || boss_beaten() || boss_done() ||
					getenv("CYBERWORLD_STATE_POS")
				? guardian(D.objs.guardian.navi)->name : "???",
			boss_done() ? "done" : boss_beaten() ? "beaten" : boss_fighting() ? "fighting" : "waiting");
}
