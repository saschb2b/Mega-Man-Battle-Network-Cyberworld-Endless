/* A liberation on a dark layer (liberation.h; docs/LIBERATION.md, stage 1).
 *
 * The board is the layer's floor (liberation_board.c). On it MegaMan
 * walks as BN6 walks him, as BN5's Navis walk theirs, over the clean
 * panels: the dark ones are fenced with walls of BN6's own
 * (liberation_fence.c). Beside one, A takes his command on the panel he
 * faces, one a phase: Liberation, a battle of BN6's for it (harder as
 * more dark panels stand around it; won in its first turn, the eight
 * round it liberated too), LongSwrd for an Order Point (two panels in a
 * line, one battle), or Pass (half his HP back). Then the Darkloids'
 * phase as BN5 plays it (liberation_turn.c): the Darkloid, then each
 * guardian, warping toward him and striking what it reaches; a DarkHole
 * still standing sends its guardian back. Every DarkHole
 * down, the Darkloid's ring can be liberated, and the Darkloid fought
 * from it; deleted, the board is done, the par's prize given and the
 * exit open. The units are the layer's own map objects (OBJ_UNIT), moved
 * by their objects' places. Each battle is BN5's liberation battle on
 * BN6's loop (liberation_battle.c): three turns, the one-turn win, a time
 * up that leaves the panel dark. MegaMan alone for now: teammates come
 * with stage 3. */
#include "liberation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "devtools.h"
#include "director_layer.h"
#include "cinema.h"
#include "debug.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "flags.h"
#include "guardians.h"
#include "liberation_battle.h"
#include "liberation_pieces.h"
#include "liberation_state.h"
#include "liberation_words.h"
#include "loot.h"
#include "bn5_liberation.h"
#include "netmap.h"
#include "run.h"
#include "talk.h"

LibState LB;
static bool viewing;          /* L held on the board: its camera the D-pad's (lib_keys) */
static int nudge_x, nudge_y;  /* ... pixels a frame across and down */

#define BANNER_FRAMES  70
#define PARK_X         (-2000)   /* where a deleted unit's object waits, off the floor */
#define TOUR_HOLD      60     /* frames the opening's camera rests on the Darkloid */

static const int dirs[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
/* BN6's facings and walking keys along the grid's ways (net.h: grid x
 * runs down-right on screen, y down-left) */
static const int facing[4] = { 3, 5, 7, 1 };

bool lib_way_dark(int act, int biome) {
	if (act < 1 || act >= 6) return false;
	if (dev.dark) return true;
	int area = run_dark_area(act);
	return area >= 0 && biome == area;
}

int lib_layer_tiles(int tiles, int biome) {
	int a = lib_look_area(lib_look_mission(biome, (run.depth - 1) % CYCLE_LAYERS / 3));
	return a >= 0 ? a : tiles;
}

bool lib_layer_dark(void) {
	if (run.side_kind != LAYER_NORMAL) return false;
	int p = (run.depth - 1) % CYCLE_LAYERS;
	if (p >= 18 || p % 3 != 1) return false;
	return lib_way_dark(p / 3, run.biome_order[p / 3]);
}

bool lib_on(void) { return D.active && !D.town && LB.on && LB.step != LIB_STEP_DONE; }

bool lib_route_found(void) { return LB.on && LB.route_found; }

bool lib_exit_shut(void) { return LB.on && LB.step != LIB_STEP_DONE && LB.step != LIB_STEP_PRIZE; }

int lib_unit_at(int x, int y) {
	for (int u = 0; u < LB.b.nunits; ++u)
		if (!LB.b.unit[u].gone && LB.b.unit[u].x == x && LB.b.unit[u].y == y) return u;
	return -1;
}

void lib_faced(int *x, int *y) { *x = LB.tx; *y = LB.ty; }

/* ---- the layer made ---- */

/* A unit as a layer object: its look (list-6 sprite, animation) and its
 * number, OBJ_UNIT's param (layer_objs.c) */
static void unit_object(int u) {
	if (layer.nobj >= MAX_OBJS) return;
	const LibUnit *n = &LB.b.unit[u];
	int sprite = GUARDIAN_HEEL_SPRITE, anim = 5, list = 6;
	if (n->kind == UNIT_DARKLOID) {
		lib_darkloid_dress();
		NpcBody body = guardian_body(LB.darkloid, 5);
		list = body.list; sprite = body.index; anim = body.anim;
	} else lib_look_unit(n->kind, &list, &sprite, &anim);
	LB.anim[u] = (uint8_t)anim;
	NetObj *o = &layer.obj[layer.nobj++];
	memset(o, 0, sizeof *o);
	o->type = OBJ_UNIT;
	o->x = n->x; o->y = n->y;
	o->param = sprite | anim << 8 | list << 16 | u << 20;
	o->prop = -1;
}

static bool x_is_start(int y) { return y == LB.b.start_y; }

/* (debug) the board as text, a row per grid y: '.' clean, '#' dark, 'i'
 * an item's, '?' a BonusPanel, 'O' a DarkHole, 'r' the Darkloid's ring,
 * 'D' him, 'B' a barrier, 't' a trap, 'S' the start, 'g' a guardian */
static void board_print(void) {
	fprintf(stderr, "liberation: %d panels, %d dark, %d holes, %d units, par %d, darkloid %d (board try %d), start %d,%d, rules %x, barriers %d (sealed %d, route %d,%d), traps %d\n",
		LB.b.panels, LB.b.dark, LB.b.nholes, LB.b.nunits, LB.b.par, LB.darkloid, LB.b.tries, LB.b.start_x, LB.b.start_y, LB.b.rules, LB.b.nbars, LB.b.sealed,
		LB.b.route_x, LB.b.route_y, LB.b.traps);
	fprintf(stderr, "liberation: the planner: %d of %d plays won, phases %d-%d-%d, dead %d of %d (%d strikes), moves %d, the route's %d\n", LB.b.m.wins, LB.b.m.plays,
		LB.b.m.p25, LB.b.m.median, LB.b.m.p75, LB.b.m.dead, LB.b.m.phases, LB.b.m.strikes, LB.b.m.moves, LB.b.m.route);
	for (int y = 0; y < MAP_H; ++y) {
		char row[MAP_W + 1];
		bool any = false;
		for (int x = 0; x < MAP_W; ++x) {
			row[x] = " .#i?OrDBt"[LB.b.kind[y][x] < LIB_KINDS ? LB.b.kind[y][x] : 0];
			any |= LB.b.kind[y][x] != LIB_VOID;
			for (int u = 1; u < LB.b.nunits; ++u) if (LB.b.unit[u].x == x && LB.b.unit[u].y == y) row[x] = 'g';
		}
		if (x_is_start(y)) row[LB.b.start_x] = 'S';
		row[MAP_W] = 0;
		if (any) fprintf(stderr, "liberation %2d %s\n", y, row);
	}
	int wx, wy;
	for (int h = 0; h < LB.b.nholes; ++h) { netmap_world(LB.b.hole_x[h], LB.b.hole_y[h], &wx, &wy); fprintf(stderr, "liberation: hole %d,%d at world %d %d\n", LB.b.hole_x[h], LB.b.hole_y[h], wx, wy); }
	netmap_world(LB.b.boss_x, LB.b.boss_y, &wx, &wy);
	fprintf(stderr, "liberation: darkloid %d,%d at world %d %d\n", LB.b.boss_x, LB.b.boss_y, wx, wy);
	for (int u = 1; u < LB.b.nunits; ++u) {
		netmap_world(LB.b.unit[u].x, LB.b.unit[u].y, &wx, &wy);
		fprintf(stderr, "liberation: %s %d,%d at world %d %d\n", lib_unit_name(LB.b.unit[u].kind), LB.b.unit[u].x, LB.b.unit[u].y, wx, wy);
	}
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			int k = LB.b.kind[y][x];
			if (k != LIB_BAR && k != LIB_TRAP && !(k == LIB_ITEM && (LB.b.item[y][x] & 15) >= LIB_GOT_KEY)) continue;
			netmap_world(x, y, &wx, &wy);
			fprintf(stderr, "liberation: %s %d,%d at world %d %d\n", k == LIB_BAR ? "barrier" : k == LIB_TRAP ? "trap" : "item", x, y, wx, wy);
		}
}

/* A panel of the layer's map as built: floor at the ground's height (its
 * middle's wall cell, as the fence reads the floor), no stair. The map's
 * floor goes by panels (netmap_floor_cell), so two such panels side by
 * side are a way MegaMan walks. */
static bool map_floor(int x, int y) {
	int wx, wy;
	netmap_world(x, y, &wx, &wy);
	return netmap_floor_cell(wx >> 3, wy >> 3, 0) && !netmap_stair_cell(wx >> 3, wy >> 3);
}

bool lib_layer_made(void) {
	memset(&LB, 0, sizeof LB);
	int act = (run.depth - 1) % CYCLE_LAYERS / 3;
	if (!lib_board_make(&LB.b, run.layer_seed, act, 1, map_floor)) {
		if (emu_debug_on()) {
			fprintf(stderr, "liberation: no board in %d tries (%d panels), rejected for", LB.b.tries, LB.b.panels);
			for (int w = 1; w < LIB_WHYS; ++w) if (LB.b.rejected[w]) fprintf(stderr, " %d:%d", w, LB.b.rejected[w]);
			fprintf(stderr, ", the last %d\n", LB.b.why);
		}
		return false;
	}
	LB.darkloid = run_darkloid(act, run.biome);
	/* (the exit where the Darkloid stands: it shows as he goes) */
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_EXIT) { layer.obj[i].x = (float)LB.b.boss_x; layer.obj[i].y = (float)LB.b.boss_y; }
	for (int u = 0; u < LB.b.nunits; ++u) unit_object(u);
	LB.on = true;
	return true;
}

/* The Darkloid's HP in his battle, as it spawns (make_boss's version) */
static int darkloid_hp(void) {
	Encounter e = make_boss(run.depth, run.biome, LB.darkloid);
	return navi_hp(e.foes[0].family, e.foes[0].version);
}

void lib_begin(void) {
	if (!layer.dark) { LB.on = false; return; }
	LB.step = LIB_STEP_WAIT;
	LB.phase = 1;
	LB.op = LIB_OP_START;
	LB.mx = LB.b.start_x;
	LB.my = LB.b.start_y;
	LB.tx = LB.ty = -1;
	LB.face = 0;
	LB.words[0] = 0;
	LB.briefed = LB.holes_told = LB.prize_given = LB.route_found = false;
	LB.tour = LB.tour_t = 0;
	LB.nfinds = 0;
	LB.effect.sprite = -1;
	LB.harm = 0;
	LB.reach_lit = false;
	memset(LB.hidden, 0, sizeof LB.hidden);
	LB.ret_hole = LB.ret_step = LB.ret_t = LB.rise_mosaic = 0;
	LB.ret_unit = LB.rise_unit = -1;
	lib_battle_disarm();
	/* (the board's HP: the Darkloid's his own battle's, BN5's on the board
	 * through his battles; our guardians' none, their battles the act's
	 * viruses) */
	memset(LB.hp, 0, sizeof LB.hp);
	memset(LB.hp_max, 0, sizeof LB.hp_max);
	LB.hp[0] = LB.hp_max[0] = darkloid_hp();
	if (dev.lib_hp > 0 && dev.lib_hp < LB.hp[0]) LB.hp[0] = dev.lib_hp;
	if (emu_debug_on()) fprintf(stderr, "liberation: the Darkloid's HP %d of %d on the board\n", LB.hp[0], LB.hp_max[0]);
	memcpy(LB.hp_shown, LB.hp, sizeof LB.hp_shown);
	for (int u = 0; u < LB.b.nunits; ++u) {
		LB.slot[u] = -1;
		netmap_world(LB.b.unit[u].x, LB.b.unit[u].y, &LB.ux[u], &LB.uy[u]);
		LB.from_x[u] = LB.ux[u]; LB.from_y[u] = LB.uy[u];
	}
	lib_look_begin();
	if (dev.lib_par > 0) LB.b.par = dev.lib_par;
	/* (the world's places once the layer's map is built) */
	if (emu_debug_on()) board_print();
}

/* ---- the units' objects ---- */

static uint32_t npc(int slot) { return BN6_NPC_OBJECTS + (uint32_t)slot * BN6_NPC_SIZE; }

static void npc_put(int slot, int x, int y) {
	int16_t v[2] = { (int16_t)x, (int16_t)y };
	emu_write(npc(slot) + BN6_NPC_X16, &v[0], 2);
	emu_write(npc(slot) + BN6_NPC_Y16, &v[1], 2);
}

static int put_x[LIB_UNITS], put_y[LIB_UNITS];   /* where each unit's object was put last */

/* Each unit's object found where the map put it (again after a battle,
 * whose return makes the map's objects afresh), and set where it stands */
static void units_show(void) {
	for (int u = 0; u < LB.b.nunits; ++u) {
		int wx = LB.from_x[u], wy = LB.from_y[u], s = LB.slot[u];
		bool held = s >= 0 && (emu_read8(npc(s)) & 1);
		bool away = LB.b.unit[u].gone || LB.hidden[u];
		int want_x = away ? PARK_X : LB.ux[u], want_y = away ? 0 : LB.uy[u];
		if (held) {
			int x = (int16_t)emu_read16(npc(s) + BN6_NPC_X16), y = (int16_t)emu_read16(npc(s) + BN6_NPC_Y16);
			/* (where it was put last, or where the map put it) */
			held = (x == put_x[u] && y == put_y[u]) || (x == wx && y == wy);
		}
		if (!held) {
			LB.slot[u] = -1;
			for (int k = 0; k < BN6_NPC_COUNT && LB.slot[u] < 0; ++k)
				if ((emu_read8(npc(k)) & 1) && (int16_t)emu_read16(npc(k) + BN6_NPC_X16) == wx && (int16_t)emu_read16(npc(k) + BN6_NPC_Y16) == wy)
					LB.slot[u] = k;
		}
		if (LB.slot[u] >= 0) { npc_put(LB.slot[u], want_x, want_y); put_x[u] = want_x; put_y[u] = want_y; }
		/* (its animation the board's, its object's own byte) */
		uint32_t a = D.objs.lib_anim[u];
		if (a && emu_read8(a) != LB.anim[u]) emu_write8(a, LB.anim[u]);
	}
}

/* ---- MegaMan on the board ---- */

static void megaman_at(int x, int y) {
	int wx, wy;
	netmap_world(x, y, &wx, &wy);
	emu_write32(BN6_PLAYER_X, (uint32_t)wx << 16);
	emu_write32(BN6_PLAYER_Y, (uint32_t)wy << 16);
	emu_write32(BN6_PLAYER_NEXT_X, (uint32_t)wx << 16);
	emu_write32(BN6_PLAYER_NEXT_Y, (uint32_t)wy << 16);
}

static void turn(int f) {
	LB.face = f;
	emu_write8(BN6_PLAYER_FACING, (uint8_t)facing[f]);
	emu_write8(BN6_PLAYER_ANIM, (uint8_t)facing[f]);
}

static int hp_now(void) { return emu_read16(BN6_NAVI_HP); }
static int hp_max(void) { return emu_read16(BN6_NAVI_MAX_HP); }
static void hp_set(int hp) {
	uint16_t v = (uint16_t)(hp < 1 ? 1 : hp > hp_max() ? hp_max() : hp);
	emu_write(BN6_NAVI_HP, &v, 2);
}
int lib_hp(void) { return hp_now(); }
void lib_hp_set(int hp) { hp_set(hp); }

static void say(const char *words, int after) {
	snprintf(LB.words, sizeof LB.words, "%s", words ? words : "");
	LB.after = after;
	LB.step = LIB_STEP_TALK;
}

/* ... and more said after what is due */
static void say_more(const char *words) {
	if (!words || !words[0]) return;
	size_t k = strlen(LB.words);
	snprintf(LB.words + k, sizeof LB.words - k, "%s%s", k ? "|" : "", words);
}

static void open_menu(void) {
	int fx, fy;
	lib_faced(&fx, &fy);
	int u = lib_unit_at(fx, fy);
	bool darkloid = u == 0 && !lib_holes_left(&LB.b);
	/* (a BonusPanel too: BN5's frees it without a battle, liberation_bonus.c) */
	LB.can[LIB_CMD_LIBERATE] = (u > 0) || darkloid || (u < 0 && (lib_liberable(&LB.b, fx, fy) || LB.b.kind[fy][fx] == LIB_BONUS));
	LB.can[LIB_CMD_LONGSWRD] = LB.op > 0 && u < 0 && lib_liberable(&LB.b, fx, fy) && LB.b.kind[fy][fx] != LIB_HOLE;
	LB.can[LIB_CMD_PASS] = true;
	LB.menu = LB.can[LIB_CMD_LIBERATE] ? LIB_CMD_LIBERATE : LIB_CMD_PASS;
	LB.step = LIB_STEP_MENU;
}

/* ---- a liberation's battle ---- */

static void fight(int cmd) {
	int fx, fy;
	lib_faced(&fx, &fy);
	int u = lib_unit_at(fx, fy);
	LB.fight_cmd = cmd;
	LB.fight_unit = u;
	LB.fx = fx; LB.fy = fy;
	Encounter e;
	if (u == 0) { LB.fight = LIB_FIGHT_DARKLOID; e = lib_darkloid_battle(); }
	else if (u > 0) { LB.fight = LIB_FIGHT_GUARDIAN; e = make_encounter(run.depth, run.biome, ENC_CHALLENGE); }
	else if (LB.b.kind[fy][fx] == LIB_HOLE) { LB.fight = LIB_FIGHT_HOLE; e = make_encounter(run.depth, run.biome, ENC_CHALLENGE); }
	else {
		/* (the more dark panels round it, the harder: BN5's text) */
		int d = lib_dark_around(&LB.b, fx, fy);
		LB.fight = LIB_FIGHT_PANEL;
		e = make_encounter(run.depth, run.biome, d <= 2 ? ENC_EASY : d <= 6 ? ENC_NORMAL : ENC_CHALLENGE);
	}
	/* (its field BN5's by the dark round the panel: Surrounded where it is
	 * thick, liberation_field.c) */
	int level = lib_battle_field(&e, fx, fy, LB.fight == LIB_FIGHT_DARKLOID);
	if (cmd == LIB_CMD_LONGSWRD) --LB.op;
	LB.fight_seen = false;
	/* (BN5's rules on BN6's loop: three turns, the Darkloid's too;
	 * liberation_battle.c. No running from it, as from BN5's) */
	e.held = true;
	set_encounter(&e, true);
	lib_battle_arm(LIB_TURNS);
	/* (his battle from the HP the board keeps, BN5's) */
	lib_battle_foe(u >= 0 ? LB.hp[u] : 0);
	if (emu_debug_on()) fprintf(stderr, "liberation: fight %d at %d,%d (dark level %d, sides %02X)\n", LB.fight, fx, fy, level, e.sides);
	LB.step = LIB_STEP_BATTLE;
}

void lib_battle_frame(void) {
	if (!LB.on || LB.step != LIB_STEP_BATTLE) return;
	LB.fight_seen = true;
	lib_battle_watch();
}

/* ... more words of what a panel held */
static void held_more(char *held, size_t n, const char *words) {
	size_t k = strlen(held);
	if (words && words[0] && k + 1 < n) snprintf(held + k, n - k, "%s%s", k ? "|" : "", words);
}

/* An item's icon over its panel as BN5's pops, while the words of it run */
static void pop(int x, int y, int kind) {
	if (LB.npops < LIB_POPS) LB.pop[LB.npops++] = (__typeof__(LB.pop[0])){ (int8_t)x, (int8_t)y, (int8_t)kind, 0 };
}

/* A chip's or zenny's find: its script (layer_objs.c's), after the words */
static void find_due(int script) {
	if (script >= 0 && LB.nfinds < LIB_FINDS) LB.finds[LB.nfinds++] = script;
}

/* What an item panel held, given */
static void take_item(int x, int y, char *held, size_t n) {
	int it = LB.b.item[y][x], got = it & 15;
	switch (got) {
	case LIB_GOT_HEART:
		hp_set(hp_now() + hp_max() * 3 / 10 / 10 * 10);
		held_more(held, n, lib_item_words(true, LB.op));
		pop(x, y, BN5_ITEM_HEART);
		break;
	case LIB_GOT_OP:
		LB.op = LB.op + (it >> 4) > LIB_OP_MAX ? LIB_OP_MAX : LB.op + (it >> 4);
		held_more(held, n, lib_item_words(false, LB.op));
		pop(x, y, BN5_ITEM_ORDER_POINTS);
		break;
	case LIB_GOT_KEY:
		lib_open_barrier(&LB.b, it >> 4);
		held_more(held, n, lib_key_words((it >> 4) == LB.b.sealed));
		pop(x, y, BN5_ITEM_KEY);
		break;
	case LIB_GOT_ZENNY: find_due(D.objs.lib_zenny); pop(x, y, BN5_ITEM_ZENNY); break;
	case LIB_GOT_CHIP: find_due((it >> 4) < LIB_LOOT ? D.objs.lib_loot[it >> 4] : -1); pop(x, y, BN5_ITEM_CHIP); break;
	default:
		/* (the sealed region's route: its hook, docs/LIBERATION.md, Routes) */
		LB.route_found = true;
		held_more(held, n, lib_route_words());
		break;
	}
}

/* Panel (x, y) liberated: what it held given; into `held`, words of it */
static void free_panel(int x, int y, char *held, size_t n) {
	int k = LB.b.kind[y][x];
	if (k == LIB_ITEM) take_item(x, y, held, n);
	if (k == LIB_TRAP) {
		/* (shown as a trap from the first: its harm the toll, never his last HP) */
		hp_set(hp_now() - lib_trap_harm(LB.b.act));
		held_more(held, n, lib_trap_words(lib_trap_harm(LB.b.act)));
	}
	if (k == LIB_DARK || k == LIB_ITEM || k == LIB_RING || k == LIB_HOLE || k == LIB_TRAP) {
		LB.b.kind[y][x] = LIB_CLEAN;
		LB.b.item[y][x] = 0;
		lib_look_freed(x, y);
	}
}

static int free_around(int x, int y, char *held, size_t n) {
	int freed = 0;
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) {
			if ((!dx && !dy) || !lib_swept(x + dx, y + dy)) continue;
			free_panel(x + dx, y + dy, held, n);
			++freed;
		}
	return freed;
}

/* A DarkHole's guardian deleted: back at a Darkloids' phase's end while
 * its hole stands (lib_guardian_return) */
static bool guardian_back(int u) {
	int h = LB.b.unit[u].hole;
	return h >= 0 && LB.b.kind[LB.b.hole_y[h]][LB.b.hole_x[h]] == LIB_HOLE;
}

static void fight_won(bool one_turn) {
	char held[320] = "";
	int freed = 0;
	bool back = false;
	if (LB.fight == LIB_FIGHT_DARKLOID) {
		/* (his panel liberated with him: the exit shows on it) */
		LB.b.unit[0].gone = true;
		LB.b.kind[LB.fy][LB.fx] = LIB_CLEAN;
		lib_look_freed(LB.fx, LB.fy);
		return;
	}
	if (LB.fight == LIB_FIGHT_GUARDIAN) {
		/* (his panel liberated with him: the prototype's rule) */
		LB.b.unit[LB.fight_unit].gone = true;
		free_panel(LB.fx, LB.fy, held, sizeof held);
		if (one_turn) freed += free_around(LB.fx, LB.fy, held, sizeof held);
		back = guardian_back(LB.fight_unit);
	} else if (LB.fight == LIB_FIGHT_HOLE) {
		LB.b.kind[LB.fy][LB.fx] = LIB_CLEAN;
		freed = free_around(LB.fx, LB.fy, held, sizeof held);
		for (int u = 1; u < LB.b.nunits; ++u)
			if (LB.b.unit[u].hole >= 0 && LB.b.hole_x[LB.b.unit[u].hole] == LB.fx && LB.b.hole_y[LB.b.unit[u].hole] == LB.fy) LB.b.unit[u].gone = true;
	} else {
		free_panel(LB.fx, LB.fy, held, sizeof held);
		if (LB.fight_cmd == LIB_CMD_LONGSWRD) {
			int x2 = LB.fx + dirs[LB.face][0], y2 = LB.fy + dirs[LB.face][1];
			if (lib_liberable(&LB.b, x2, y2) && lib_unit_at(x2, y2) < 0 && LB.b.kind[y2][x2] != LIB_HOLE) { free_panel(x2, y2, held, sizeof held); ++freed; }
		}
		if (one_turn) freed += free_around(LB.fx, LB.fy, held, sizeof held);
	}
	say_more(lib_battle_words(LB.fight, true, one_turn, freed, lib_holes_left(&LB.b), back));
	say_more(held);
	/* (BN5's banner was the battle's own, LIBERATE SUCCESS! or 1 TURN
	 * LIBERATE!, as BN5 shows it at the win: liberation_battle.c) */
	LB.cheer = LIB_CHEER_NONE;
	LB.cheer_t = 0;
}

static void after_fight(void) {
	int turn = 0, out = lib_battle_end(&turn);
	bool won = out == LIB_OUT_WON, one_turn = won && turn <= 1;
	if (emu_debug_on()) fprintf(stderr, "liberation: battle %d at %d,%d %s%s (turn %d)\n", LB.fight, LB.fx, LB.fy,
		won ? "won" : out == LIB_OUT_TIME_UP ? "time up" : "lost", one_turn ? " in one turn" : "", turn);
	LB.words[0] = 0;
	/* (a unit not deleted keeps what his battle left him, BN5's; one left
	 * at none by a battle not won a quarter of his max) */
	int u = LB.fight_unit, left = lib_battle_foe_left();
	if (!won && u >= 0 && u < LB.b.nunits && LB.hp_max[u] && left >= 0) {
		LB.hp[u] = LB.hp_shown[u] = left > 0 ? left : LB.hp_max[u] / 4;
		if (emu_debug_on()) fprintf(stderr, "liberation: unit %d left at %d HP on the board\n", u, LB.hp[u]);
	}
	if (won) fight_won(one_turn);
	else say_more(lib_battle_words(LB.fight, false, false, 0, 0, false));
	if (LB.b.unit[0].gone) {
		LB.prize = LB.phase <= LB.b.par - 2 ? 0 : LB.phase <= LB.b.par ? 1 : 2;
		flag_set(LAYER_EXIT_OPEN_FLAG);
		LB.step = LIB_STEP_CLEAR;
		LB.banner_t = 0;
		return;
	}
	char said[sizeof LB.words];
	snprintf(said, sizeof said, "%s", LB.words);
	say(said, LIB_STEP_BANNER);
	LB.enemy_banner = true;
}

/* ---- the Darkloids' phase ---- */

/* The phase as BN5 plays it: the Darkloid first, then each guardian in
 * its order, each its turn, then the DarkHoles' deleted guardians back
 * beside them (liberation_turn.c); then the player's phase */
static void enemy_update(void) {
	/* (the Darkloid's word at his first turn, as BN5's says his once,
	 * before anything of his phase: his face, his habit of speech) */
	if (!LB.met && LB.phase == 1 && LB.enemy_unit == 0 && LB.enemy_t == 0 && LB.enemy_steps == 0 && !LB.b.unit[0].gone) {
		if (talk_start(lib_entrance_words(guardian(LB.darkloid)->name), guardian_face(LB.darkloid))) LB.met = true;
		return;
	}
	if (LB.enemy_unit < LB.b.nunits && !lib_turn(LB.enemy_unit)) return;
	if (LB.enemy_unit < LB.b.nunits && ++LB.enemy_unit < LB.b.nunits) return;
	if (!lib_return()) return;
	LB.enemy_unit = 0;
	LB.shield = false;
	++LB.phase;
	say("", LIB_STEP_BANNER);
	LB.banner_t = 0;
	LB.enemy_banner = false;
}

/* ---- the frame ---- */

static bool quiet(void) { return !talk_busy() && !emu_read8(BN6_CHATBOX) && !D.area_card && !cinema_busy() && !D.beat[0] && on_map(); }
bool lib_quiet(void) { return quiet(); }

/* Where MegaMan stands and what he faces, as BN6 walks him: his panel (a
 * walked one: the fence keeps him to them), and the panel beside it his
 * facing points at, a little ahead of him (a diagonal one is none) */
static const int fwx[8] = { 7, 10, 7, 0, -7, -10, -7, 0 }, fwy[8] = { -7, 0, 7, 10, 7, 0, -7, -10 };

static void track_update(void) {
	int px = bn6_player_x(), py = bn6_player_y(), x, y;
	if (netmap_panel(px, py, &x, &y) && x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && lib_walkable(&LB.b, x, y) && (x != LB.mx || y != LB.my)) {
		LB.mx = x; LB.my = y;
	}
	int f = emu_read8(BN6_PLAYER_FACING) & 7;
	LB.tx = LB.ty = -1;
	for (int d = 12; d <= 40; d += 4) {
		if (!netmap_panel(px + fwx[f] * d / 10, py + fwy[f] * d / 10, &x, &y) || (x == LB.mx && y == LB.my)) continue;
		for (int k = 0; k < 4; ++k)
			if (x == LB.mx + dirs[k][0] && y == LB.my + dirs[k][1]) { LB.tx = x; LB.ty = y; LB.face = k; }
		return;
	}
}

static void talk_update_lib(void) {
	/* (BN5's banner first, over the board faded back in) */
	if (LB.cheer && lib_look_on() && LB.cheer_t < LIB_CHEER_WAIT + LIB_CHEER_FRAMES) return;
	if (LB.words[0]) {
		if (!quiet() || !talk_start(LB.words, FACE_MEGAMAN)) return;
		LB.words[0] = 0;
		/* (an item found: BN5's jingle as its line begins) */
		if (LB.npops) lib_sound(BN5_LIB_ITEM_SOUND);
		return;
	}
	if (!quiet()) return;
	/* (a chip's or zenny's find: its script, BN6's own give) */
	if (LB.nfinds) {
		if (!talk_script(D.objs.archive, LB.finds[0])) return;
		memmove(LB.finds, LB.finds + 1, sizeof LB.finds[0] * (size_t)--LB.nfinds);
		return;
	}
	LB.step = LB.after;
	LB.cheer = LIB_CHEER_NONE;
	LB.npops = 0;
	if (LB.step == LIB_STEP_BANNER) LB.banner_t = 0;
}

static void prize_update(void) {
	if (!quiet()) return;
	if (!LB.prize_given && D.objs.lib_prize[LB.prize] >= 0 && !talk_script(D.objs.archive, D.objs.lib_prize[LB.prize])) return;
	LB.prize_given = true;
	say(lib_exit_words(), LIB_STEP_DONE);
}

/* The board won as BN5's mission (BN5_LIB_CLEAR, traced): BN5's song for
 * it at once, SUCCESS! in the phases' band BN5_LIB_CLEAR_BAND frames on,
 * and BN5_LIB_CLEAR_TALK frames after the band MegaMan's tally of the
 * phases against the target, in BN5's own words (lib_piece_tally) where
 * they are true here (over par its first page); then the prize, BN5's
 * tiers: a chip, or over par zenny */
static void clear_update(void) {
	int frames = lib_piece_banner_frames(), band = LB.banner_t - BN5_LIB_CLEAR_BAND;
	if (LB.banner_t == 0) {
		if (!quiet() || !lib_camera_home()) return;
		lib_cue_won();
	}
	if (frames && band == BN5_LIB_WORDS_AT) lib_sound(BN5_LIB_PHASE_SOUND);
	if (++LB.banner_t < (frames ? BN5_LIB_CLEAR_BAND + frames + BN5_LIB_CLEAR_TALK : 1)) return;
	static char tally[sizeof LB.words];
	if (!lib_piece_tally(LB.prize, LB.phase, LB.b.par, LB.prize < 2 ? 2 : 1, tally, sizeof tally)) tally[0] = 0;
	say(lib_won_words(tally, LB.prize, LB.phase, LB.b.par), LIB_STEP_PRIZE);
}

/* BN5's camera on the Darkloid before his phase's banner: true once there */
static bool darkloid_framed(void) {
	int wx, wy;
	netmap_world(LB.b.boss_x, LB.b.boss_y, &wx, &wy);
	return LB.b.unit[0].gone || lib_camera_at(wx, wy);
}

/* The board's opening: MegaMan on the start, BN5's mission song, BN5's
 * camera to the Darkloid and back (his entrance), then the briefing */
static void opening_update(void) {
	if (!LB.tour) {
		if (!quiet() || D.frame < 60) return;
		megaman_at(LB.mx, LB.my);
		turn(0);
		lib_song(BN5_SONG_MISSION_OPEN);
		LB.tour = 1;
		LB.tour_t = 0;
	}
	if (LB.tour == 1) {
		int wx, wy;
		netmap_world(LB.b.boss_x, LB.b.boss_y, &wx, &wy);
		if (lib_camera_at(wx, wy) && ++LB.tour_t >= TOUR_HOLD) LB.tour = 2;
		return;
	}
	if (!lib_camera_home()) return;
	say(lib_briefing_words(LB.b.nholes, LB.b.par), LIB_STEP_BANNER);
}

/* A phase's banner: BN5's where its ROM is there (its length, its sound
 * as the words slide in; the first phase's brings the board's song), begun
 * once nothing else shows (a CONTINUE's act card first) and BN5's camera
 * is on the Darkloid, or back on MegaMan */
static void banner_update(void) {
	int frames = lib_piece_banner_frames();
	if (LB.banner_t == 0 && (!quiet() || !(LB.enemy_banner ? darkloid_framed() : lib_camera_home()))) return;
	if (LB.banner_t == 0 && LB.phase == 1 && !LB.enemy_banner) lib_song(BN5_SONG_LIBERATION);
	if (frames && LB.banner_t == BN5_LIB_WORDS_AT) lib_sound(BN5_LIB_PHASE_SOUND);
	if (++LB.banner_t >= (frames ? frames : BANNER_FRAMES)) LB.step = LB.enemy_banner ? LIB_STEP_ENEMY : LIB_STEP_MOVE;
}

static void steps_update(void) {
	switch (LB.step) {
	case LIB_STEP_WAIT: opening_update(); break;
	case LIB_STEP_BANNER: banner_update(); break;
	case LIB_STEP_MOVE: break;
	case LIB_STEP_BATTLE:
		if (LB.fight_seen && on_map() && !emu_battle_forcing()) after_fight();
		break;
	case LIB_STEP_ENEMY:
		/* (after a CONTINUE, once its act card is gone) */
		if (quiet()) enemy_update();
		break;
	case LIB_STEP_TALK: talk_update_lib(); break;
	case LIB_STEP_PRIZE: prize_update(); break;
	case LIB_STEP_BONUS: lib_bonus_update(); break;

	case LIB_STEP_CLEAR: clear_update(); break;
	default: break;
	}
}

/* The HP each unit's digits show, stepped toward its HP as BN5 steps them */
static int roll(int shown, int hp) {
	int d = hp - shown;
	return shown + (d > BN5_LIB_HP_ROLL ? BN5_LIB_HP_ROLL : d < -BN5_LIB_HP_ROLL ? -BN5_LIB_HP_ROLL : d);
}

static void hp_roll(void) {
	for (int u = 0; u < LB.b.nunits; ++u) LB.hp_shown[u] = roll(LB.hp_shown[u], LB.hp[u]);
	/* (MegaMan's from the HP box's as the Darkloids' phase begins) */
	LB.megaman_shown = lib_megaman_hp_shown() ? roll(LB.megaman_shown, hp_now()) : hp_now();
}

bool lib_megaman_hp_shown(void) { return LB.step == LIB_STEP_ENEMY || (LB.step == LIB_STEP_BANNER && LB.enemy_banner); }

void lib_update(void) {
	if (!LB.on || !D.active || D.town || !on_map()) return;
	if (LB.step != LIB_STEP_DONE) flag_set(BN6_FLAG_NO_ENCOUNTERS);
	lib_fence_update();
	lib_look_update();
	lib_cue_frame();
	++LB.cheer_t;
	for (int i = 0; i < LB.npops && i < LIB_POPS; ++i) ++LB.pop[i].t;
	hp_roll();
	if (LB.step != LIB_STEP_WAIT) track_update();
	steps_update();
	/* (the units' objects where the step just put them: BN6 draws them so
	 * in its next frame) */
	units_show();
	/* (the L view's camera, and back on MegaMan once L is let go) */
	if (viewing && LB.step == LIB_STEP_MOVE) lib_camera_nudge(nudge_x, nudge_y);
	else if (LB.step == LIB_STEP_MOVE || LB.step == LIB_STEP_MENU) lib_camera_home();
}

/* The menu: up and down, A takes a command, B closes it */
static void menu_keys(uint32_t pressed) {
	if (pressed & KEY_B) { LB.step = LIB_STEP_MOVE; return; }
	int dir = (pressed & KEY_DOWN) ? 1 : (pressed & KEY_UP) ? LIB_CMDS - 1 : 0;
	if (dir) LB.menu = (LB.menu + dir) % LIB_CMDS;
	if (!(pressed & KEY_A) || !LB.can[LB.menu]) return;
	if (LB.menu == LIB_CMD_PASS) {
		hp_set(hp_now() + hp_max() / 2);
		say(lib_rest_words(), LIB_STEP_BANNER);
		LB.enemy_banner = true;
		return;
	}
	int fx, fy;
	lib_faced(&fx, &fy);
	if (LB.menu == LIB_CMD_LIBERATE && lib_unit_at(fx, fy) < 0 && LB.b.kind[fy][fx] == LIB_BONUS) { lib_bonus_begin(fx, fy); return; }
	fight(LB.menu);
}

/* A command can be taken on the panel MegaMan faces: one to liberate, a
 * guardian, the Darkloid once his DarkHoles are gone */
bool lib_faced_takes(void) {
	if (LB.tx < 0) return false;
	int u = lib_unit_at(LB.tx, LB.ty);
	return u > 0 || (u == 0 && !lib_holes_left(&LB.b)) || (u < 0 && (lib_liberable(&LB.b, LB.tx, LB.ty) || LB.b.kind[LB.ty][LB.tx] == LIB_BONUS));
}

uint32_t lib_keys(uint32_t keys) {
	uint32_t pressed = keys & ~LB.held;
	LB.held = keys;
	/* (a chat box's: its pages, its questions) */
	if (talk_busy() || emu_read8(BN6_CHATBOX)) return keys;
	/* his phase: he walks as BN6 walks him, and A beside what he can
	 * liberate opens his command; else he stands as the board goes on */
	if (LB.step == LIB_STEP_MOVE) {
		/* (L held, BN5's view: the D-pad moves the camera, B faster, and
		 * MegaMan stands) */
		viewing = (keys & KEY_L) && lib_look_on();
		if (viewing) {
			int s = keys & KEY_B ? BN5_LIB_CAMERA_FAST : BN5_LIB_CAMERA_STEP;
			nudge_x = (keys & KEY_RIGHT ? s : 0) - (keys & KEY_LEFT ? s : 0);
			nudge_y = (keys & KEY_DOWN ? s : 0) - (keys & KEY_UP ? s : 0);
			return keys & ~(uint32_t)(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT | KEY_A | KEY_B);
		}
		if ((pressed & KEY_A) && lib_faced_takes()) { open_menu(); return keys & (KEY_START | KEY_SELECT); }
		return keys;
	}
	viewing = false;
	if (LB.step == LIB_STEP_MENU) menu_keys(pressed);
	if (LB.step == LIB_STEP_BONUS) lib_bonus_keys(pressed);
	return keys & (KEY_START | KEY_SELECT);
}
