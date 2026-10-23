/* A liberation on a dark layer (liberation.h; docs/LIBERATION.md, stage 1).
 *
 * The board is the layer's floor (liberation_board.c). On it MegaMan
 * walks as BN6 walks him, as BN5's Navis walk theirs, over the clean
 * panels: the dark ones are fenced with walls of BN6's own
 * (liberation_fence.c). Beside one, A takes his command on the panel he
 * faces, one a phase: Liberation, a battle of BN6's for it (harder as
 * more dark panels stand around it; won in its first turn, the eight
 * round it liberated too), LongSwrd for an Order Point (two panels in a
 * line, one battle), or Pass (half his HP back). Then the guardians move
 * toward him and hit what they reach, the Darkloid hits him in his reach,
 * and a DarkHole still standing sends its guardian back. Every DarkHole
 * down, the Darkloid's ring can be liberated, and the Darkloid fought
 * from it; deleted, the board is done, the par's prize given and the
 * exit open. The units are the layer's own map objects (OBJ_UNIT), moved
 * by their objects' places. MegaMan alone for now: teammates (stage 3)
 * and BN5's battle rules (stage 2) come later. */
#include "liberation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "devtools.h"
#include "director.h"
#include "director_layer.h"
#include "debug.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "flags.h"
#include "guardians.h"
#include "liberation_state.h"
#include "liberation_words.h"
#include "loot.h"
#include "bn5_liberation.h"
#include "netmap.h"
#include "rom.h"
#include "run.h"
#include "talk.h"
#include "text.h"

LibState LB;

#define BANNER_FRAMES  70
#define GLIDE_FRAMES   10    /* a guardian's step on the board */
#define PARK_X         (-2000)   /* where a deleted unit's object waits, off the floor */
#define BACK_PHASES    2      /* the Darkloids' phases a DarkHole takes to send its guardian back */

static const int dirs[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
/* BN6's facings and walking keys along the grid's ways (net.h: grid x
 * runs down-right on screen, y down-left) */
static const int facing[4] = { 3, 5, 7, 1 };

/* The guardians by act from the second (BN5's guides' numbers for its
 * missions, docs/LIBERATION.md; tools/liberation.py's GUARDS): their steps,
 * reach and damage on the board; the Darkloid's reach and damage */
static const struct { int move, reach; bool all; } kinds[4] = { { 0, 0, false }, { 2, 1, true }, { 3, 3, false }, { 2, 1, false } };
static const int harm[4][5] = { { 40, 50, 80, 80, 100 }, { 20, 40, 50, 60, 80 }, { 20, 30, 40, 60, 70 }, { 50, 50, 70, 100, 120 } };
static const int darkloid_reach[5] = { 2, 2, 3, 2, 3 };

static int tier(void) { int t = LB.b.act - 1; return t < 0 ? 0 : t > 4 ? 4 : t; }

bool lib_way_dark(int act, int biome) {
	if (act < 1 || act >= 6) return false;
	if (dev.dark) return true;
	int area = run_dark_area(act);
	return area >= 0 && biome == area;
}

int lib_layer_tiles(int tiles) {
	for (int k = 0; k < XAREAS_MAX; ++k) {
		const NetAreaDef *d = net_area_def(NET_AREAS + k);
		if (d && d->xrom > 0 && d->group == BN5_LIB_MAP_GROUP) return NET_AREAS + k;
	}
	return tiles;
}

bool lib_layer_dark(void) {
	if (run.side_kind != LAYER_NORMAL) return false;
	int p = (run.depth - 1) % CYCLE_LAYERS;
	if (p >= 18 || p % 3 != 1) return false;
	return lib_way_dark(p / 3, run.biome_order[p / 3]);
}

bool lib_on(void) { return D.active && !D.town && LB.on && LB.step != LIB_STEP_DONE; }

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
		NpcBody body = guardian_body(LB.darkloid, 5);
		list = body.list; sprite = body.index; anim = body.anim;
	}
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
 * 'D' him, 'S' the start, 'g' a guardian */
static void board_print(void) {
	fprintf(stderr, "liberation: %d panels, %d dark, %d holes, %d units, par %d, darkloid %d (board try %d), start %d,%d\n",
		LB.b.panels, LB.b.dark, LB.b.nholes, LB.b.nunits, LB.b.par, LB.darkloid, LB.b.tries, LB.b.start_x, LB.b.start_y);
	for (int y = 0; y < MAP_H; ++y) {
		char row[MAP_W + 1];
		bool any = false;
		for (int x = 0; x < MAP_W; ++x) {
			row[x] = " .#i?OrD"[LB.b.kind[y][x] & 7];
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
}

bool lib_layer_made(void) {
	memset(&LB, 0, sizeof LB);
	int act = (run.depth - 1) % CYCLE_LAYERS / 3;
	if (!lib_board_make(&LB.b, run.layer_seed, act, 1)) return false;
	LB.darkloid = run_darkloid(act, run.biome);
	/* (the exit where the Darkloid stands: it shows as he goes) */
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_EXIT) { layer.obj[i].x = (float)LB.b.boss_x; layer.obj[i].y = (float)LB.b.boss_y; }
	for (int u = 0; u < LB.b.nunits; ++u) unit_object(u);
	LB.on = true;
	if (emu_debug_on()) board_print();
	return true;
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
	LB.briefed = LB.holes_told = LB.prize_given = false;
	for (int u = 0; u < LB.b.nunits; ++u) {
		LB.slot[u] = -1;
		netmap_world(LB.b.unit[u].x, LB.b.unit[u].y, &LB.ux[u], &LB.uy[u]);
		LB.from_x[u] = LB.ux[u]; LB.from_y[u] = LB.uy[u];
	}
	lib_look_begin();
}

/* ---- the units' objects ---- */

static uint32_t npc(int slot) { return BN6_NPC_OBJECTS + (uint32_t)slot * BN6_NPC_SIZE; }

static void npc_put(int slot, int x, int y) {
	int16_t v[2] = { (int16_t)x, (int16_t)y };
	emu_write(npc(slot) + BN6_NPC_X16, &v[0], 2);
	emu_write(npc(slot) + BN6_NPC_Y16, &v[1], 2);
}

/* Each unit's object found where the map put it (again after a battle,
 * whose return makes the map's objects afresh), and set where it stands */
static void units_show(void) {
	for (int u = 0; u < LB.b.nunits; ++u) {
		int wx = LB.from_x[u], wy = LB.from_y[u], s = LB.slot[u];
		bool held = s >= 0 && (emu_read8(npc(s)) & 1);
		int gone_x = PARK_X, want_x = LB.b.unit[u].gone ? gone_x : LB.ux[u];
		if (held) {
			int x = (int16_t)emu_read16(npc(s) + BN6_NPC_X16), y = (int16_t)emu_read16(npc(s) + BN6_NPC_Y16);
			held = (x == want_x && y == (LB.b.unit[u].gone ? 0 : LB.uy[u])) || (x == wx && y == wy);
		}
		if (!held) {
			LB.slot[u] = -1;
			for (int k = 0; k < BN6_NPC_COUNT && LB.slot[u] < 0; ++k)
				if ((emu_read8(npc(k)) & 1) && (int16_t)emu_read16(npc(k) + BN6_NPC_X16) == wx && (int16_t)emu_read16(npc(k) + BN6_NPC_Y16) == wy)
					LB.slot[u] = k;
		}
		if (LB.slot[u] >= 0) npc_put(LB.slot[u], want_x, LB.b.unit[u].gone ? 0 : LB.uy[u]);
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

/* A BonusPanel stepped on: BN5's roulette, its slot by the board's seed
 * and the phase */
static void bonus(int x, int y) {
	LB.b.kind[y][x] = LIB_CLEAN;
	int outcome = (int)((run.layer_seed >> 3) + (uint32_t)(LB.phase * 7 + x * 3 + y)) % 4, near = -1, nd = 1 << 30;
	for (int u = 1; u < LB.b.nunits; ++u) {
		int d = abs(LB.b.unit[u].x - x) + abs(LB.b.unit[u].y - y);
		if (!LB.b.unit[u].gone && d < nd) { nd = d; near = u; }
	}
	if (outcome == 2 && near < 0) outcome = 1;
	if (outcome == 0) hp_set(hp_max());
	else if (outcome == 1) LB.op = LB.op + 5 > LIB_OP_MAX ? LIB_OP_MAX : LB.op + 5;
	else if (outcome == 2) LB.b.unit[near].gone = true;
	else LB.shield = true;
	say(lib_bonus_words(outcome), LIB_STEP_MOVE);
}

static void open_menu(void) {
	int fx, fy;
	lib_faced(&fx, &fy);
	int u = lib_unit_at(fx, fy);
	bool darkloid = u == 0 && !lib_holes_left(&LB.b);
	LB.can[LIB_CMD_LIBERATE] = (u > 0) || darkloid || (u < 0 && lib_liberable(&LB.b, fx, fy));
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
	if (u == 0) { LB.fight = LIB_FIGHT_DARKLOID; e = make_boss(run.depth, run.biome, LB.darkloid); }
	else if (u > 0) { LB.fight = LIB_FIGHT_GUARDIAN; e = make_encounter(run.depth, run.biome, ENC_CHALLENGE); }
	else if (LB.b.kind[fy][fx] == LIB_HOLE) { LB.fight = LIB_FIGHT_HOLE; e = make_encounter(run.depth, run.biome, ENC_CHALLENGE); }
	else {
		/* (the more dark panels round it, the harder: BN5's text) */
		int d = lib_dark_around(&LB.b, fx, fy);
		LB.fight = LIB_FIGHT_PANEL;
		e = make_encounter(run.depth, run.biome, d <= 2 ? ENC_EASY : d <= 6 ? ENC_NORMAL : ENC_CHALLENGE);
	}
	if (cmd == LIB_CMD_LONGSWRD) --LB.op;
	LB.fight_seen = LB.custom_open = false;
	LB.customs = 0;
	set_encounter(&e, true);
	LB.step = LIB_STEP_BATTLE;
}

void lib_battle_frame(void) {
	if (!LB.on || LB.step != LIB_STEP_BATTLE) return;
	LB.fight_seen = true;
	bool open = emu_read8(BN6_BATTLE_PHASE) == BN6_PHASE_CUSTOM;
	if (open && !LB.custom_open) ++LB.customs;
	LB.custom_open = open;
}

/* Panel (x, y) liberated: what it held given; into `held`, words of it */
static void free_panel(int x, int y, char *held, size_t n) {
	int k = LB.b.kind[y][x];
	if (k == LIB_ITEM) {
		int it = LB.b.item[y][x];
		if ((it & 15) == LIB_GOT_HEART) hp_set(hp_now() + hp_max() * 3 / 10 / 10 * 10);
		else LB.op = LB.op + (it >> 4) > LIB_OP_MAX ? LIB_OP_MAX : LB.op + (it >> 4);
		snprintf(held, n, "%s", lib_item_words((it & 15) == LIB_GOT_HEART, LB.op));
	}
	if (k == LIB_DARK || k == LIB_ITEM || k == LIB_RING || k == LIB_HOLE) { LB.b.kind[y][x] = LIB_CLEAN; lib_look_freed(x, y); }
}

static int free_around(int x, int y, char *held, size_t n) {
	int freed = 0;
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) {
			int k = dx || dy ? LB.b.kind[y + dy][x + dx] : LIB_VOID;
			if (k != LIB_DARK && k != LIB_ITEM && !(k == LIB_RING && !lib_holes_left(&LB.b))) continue;
			if (lib_unit_at(x + dx, y + dy) >= 0) continue;
			free_panel(x + dx, y + dy, held, n);
			++freed;
		}
	return freed;
}

/* A DarkHole's guardian deleted: back in the Darkloid's phase while its
 * hole stands */
static bool guardian_back(int u) {
	int h = LB.b.unit[u].hole;
	return h >= 0 && LB.b.kind[LB.b.hole_y[h]][LB.b.hole_x[h]] == LIB_HOLE;
}

static void fight_won(bool one_turn) {
	char held[96] = "";
	int freed = 0;
	bool back = false;
	if (LB.fight == LIB_FIGHT_DARKLOID) { LB.b.unit[0].gone = true; return; }
	if (LB.fight == LIB_FIGHT_GUARDIAN) {
		/* (his panel liberated with him: the prototype's rule) */
		LB.b.unit[LB.fight_unit].gone = true;
		LB.back_in[LB.fight_unit] = BACK_PHASES;
		free_panel(LB.fx, LB.fy, held, sizeof held);
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
}

static void after_fight(void) {
	bool won = emu_read8(BN6_BATTLE_RESULT) == 1, one_turn = won && LB.customs <= 1;
	if (emu_debug_on()) fprintf(stderr, "liberation: battle %d at %d,%d %s%s (custom screens %d)\n", LB.fight, LB.fx, LB.fy, won ? "won" : "not won",
		one_turn ? " in one turn" : "", LB.customs);
	LB.words[0] = 0;
	if (won) fight_won(one_turn);
	else say_more(lib_battle_words(LB.fight, false, false, 0, 0, false));
	if (LB.b.unit[0].gone) {
		LB.prize = LB.phase <= LB.b.par - 2 ? 0 : LB.phase <= LB.b.par ? 1 : 2;
		flag_set(LAYER_EXIT_OPEN_FLAG);
		say(lib_won_words(LB.phase, LB.b.par), LIB_STEP_PRIZE);
		return;
	}
	char said[sizeof LB.words];
	snprintf(said, sizeof said, "%s", LB.words);
	say(said, LIB_STEP_BANNER);
	LB.enemy_banner = true;
}

/* ---- the Darkloids' phase ---- */

/* A guardian's step toward MegaMan over the dark (Bladia's onto clean
 * panels too, darkening them): false where none brings it nearer */
static bool guardian_step(int u) {
	LibUnit *n = &LB.b.unit[u];
	int d0 = abs(n->x - LB.mx) + abs(n->y - LB.my), best = -1, bd = d0;
	for (int f = 0; f < 4; ++f) {
		int x = n->x + dirs[f][0], y = n->y + dirs[f][1], k = LB.b.kind[y][x];
		bool ok = k == LIB_DARK || k == LIB_ITEM || (n->kind == UNIT_BLADIA && k == LIB_CLEAN);
		int d = abs(x - LB.mx) + abs(y - LB.my);
		if (!ok || lib_unit_at(x, y) >= 0 || (x == LB.mx && y == LB.my) || d >= bd) continue;
		bd = d; best = f;
	}
	if (best < 0) return false;
	n->x = (uint8_t)(n->x + dirs[best][0]);
	n->y = (uint8_t)(n->y + dirs[best][1]);
	if (n->kind == UNIT_BLADIA && LB.b.kind[n->y][n->x] == LIB_CLEAN) LB.b.kind[n->y][n->x] = LIB_DARK;
	return true;
}

static void hit(const char *who, int dmg) {
	if (LB.shield || dmg <= 0) return;
	hp_set(hp_now() - dmg);
	LB.hurt += dmg;
	snprintf(LB.hurt_by, sizeof LB.hurt_by, "%s", who);
}

/* A DarkHole still standing sends its deleted guardian back beside it,
 * the Darkloids' phase after next (BN5 sends him in the next: with a team
 * another Navi strikes the hole meanwhile; MegaMan alone gets his phase
 * for it) */
static void guardians_back(void) {
	for (int u = 1; u < LB.b.nunits; ++u) {
		LibUnit *n = &LB.b.unit[u];
		if (!n->gone || n->hole < 0 || !guardian_back(u) || --LB.back_in[u] > 0) continue;
		int hx = LB.b.hole_x[n->hole], hy = LB.b.hole_y[n->hole];
		for (int f = 0; f < 4; ++f) {
			int x = hx + dirs[f][0], y = hy + dirs[f][1], k = LB.b.kind[y][x];
			if ((k != LIB_DARK && k != LIB_ITEM) || lib_unit_at(x, y) >= 0 || (x == LB.mx && y == LB.my)) continue;
			n->x = (uint8_t)x; n->y = (uint8_t)y; n->gone = false;
			netmap_world(x, y, &LB.ux[u], &LB.uy[u]);
			break;
		}
	}
}

/* Unit u's turn, a frame at a time: its steps glide, then its blow */
static bool enemy_turn(int u) {
	LibUnit *n = &LB.b.unit[u];
	if (n->gone) return true;
	int wx, wy;
	netmap_world(n->x, n->y, &wx, &wy);
	if (LB.ux[u] != wx || LB.uy[u] != wy) {
		LB.ux[u] += (wx - LB.ux[u]) / (GLIDE_FRAMES - LB.enemy_t > 1 ? GLIDE_FRAMES - LB.enemy_t : 1);
		LB.uy[u] += (wy - LB.uy[u]) / (GLIDE_FRAMES - LB.enemy_t > 1 ? GLIDE_FRAMES - LB.enemy_t : 1);
		if (++LB.enemy_t >= GLIDE_FRAMES) { LB.ux[u] = wx; LB.uy[u] = wy; LB.enemy_t = 0; }
		return false;
	}
	int d = abs(n->x - LB.mx) + abs(n->y - LB.my);
	if (u == 0) {
		if (d <= darkloid_reach[tier()]) hit(guardian(LB.darkloid)->name, harm[0][tier()]);
		return true;
	}
	if (d > kinds[n->kind].reach && LB.enemy_steps < kinds[n->kind].move && guardian_step(u)) { ++LB.enemy_steps; return false; }
	if (d <= kinds[n->kind].reach) hit(lib_unit_name(n->kind), harm[n->kind][tier()]);
	return true;
}

static void enemy_update(void) {
	if (LB.enemy_unit == 0 && LB.enemy_t == 0 && LB.enemy_steps == 0) { LB.hurt = 0; guardians_back(); }
	/* (the guardians first, the Darkloid last) */
	int u = LB.enemy_unit + 1 < LB.b.nunits ? LB.enemy_unit + 1 : 0;
	if (!enemy_turn(u)) return;
	LB.enemy_steps = 0;
	LB.enemy_t = 0;
	if (u != 0) { ++LB.enemy_unit; return; }
	LB.enemy_unit = 0;
	LB.shield = false;
	++LB.phase;
	say(LB.hurt ? lib_hurt_words(LB.hurt_by, LB.hurt) : "", LIB_STEP_BANNER);
	LB.banner_t = 0;
	LB.enemy_banner = false;
}

/* ---- the frame ---- */

static bool quiet(void) { return !talk_busy() && !emu_read8(BN6_CHATBOX) && !D.area_card && !D.beat[0] && on_map(); }

/* Where MegaMan stands and what he faces, as BN6 walks him: his panel (a
 * walked one: the fence keeps him to them), and the panel beside it his
 * facing points at, a little ahead of him (a diagonal one is none);
 * a BonusPanel stepped on spins its roulette */
static const int fwx[8] = { 7, 10, 7, 0, -7, -10, -7, 0 }, fwy[8] = { -7, 0, 7, 10, 7, 0, -7, -10 };

static void track_update(void) {
	int px = bn6_player_x(), py = bn6_player_y(), x, y;
	if (netmap_panel(px, py, &x, &y) && x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && lib_walkable(&LB.b, x, y) && (x != LB.mx || y != LB.my)) {
		LB.mx = x; LB.my = y;
		if (LB.b.kind[y][x] == LIB_BONUS && LB.step == LIB_STEP_MOVE) bonus(x, y);
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
	if (LB.words[0]) {
		if (!quiet() || !talk_start(LB.words, FACE_MEGAMAN)) return;
		LB.words[0] = 0;
		return;
	}
	if (!quiet()) return;
	LB.step = LB.after;
	if (LB.step == LIB_STEP_BANNER) LB.banner_t = 0;
}

static void prize_update(void) {
	if (!quiet()) return;
	if (!LB.prize_given && D.objs.lib_prize[LB.prize] >= 0 && !talk_script(D.objs.archive, D.objs.lib_prize[LB.prize])) return;
	LB.prize_given = true;
	LB.step = LIB_STEP_DONE;
}

static void steps_update(void) {
	switch (LB.step) {
	case LIB_STEP_WAIT:
		if (!quiet() || D.frame < 60) return;
		megaman_at(LB.mx, LB.my);
		turn(0);
		say(lib_briefing_words(LB.b.nholes, LB.b.par), LIB_STEP_BANNER);
		break;
	case LIB_STEP_BANNER:
		if (++LB.banner_t >= BANNER_FRAMES) LB.step = LB.enemy_banner ? LIB_STEP_ENEMY : LIB_STEP_MOVE;
		break;
	case LIB_STEP_MOVE: break;
	case LIB_STEP_BATTLE:
		if (LB.fight_seen && on_map() && !emu_battle_forcing()) after_fight();
		break;
	case LIB_STEP_ENEMY: enemy_update(); break;
	case LIB_STEP_TALK: talk_update_lib(); break;
	case LIB_STEP_PRIZE: prize_update(); break;
	default: break;
	}
}

void lib_update(void) {
	if (!LB.on || !D.active || D.town || !on_map()) return;
	if (LB.step != LIB_STEP_DONE) flag_set(BN6_FLAG_NO_ENCOUNTERS);
	units_show();
	lib_fence_update();
	lib_look_update();
	if (LB.step != LIB_STEP_WAIT) track_update();
	steps_update();
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
	fight(LB.menu);
}

/* A command can be taken on the panel MegaMan faces: one to liberate, a
 * guardian, the Darkloid once his DarkHoles are gone */
static bool faced_takes(void) {
	if (LB.tx < 0) return false;
	int u = lib_unit_at(LB.tx, LB.ty);
	return u > 0 || (u == 0 && !lib_holes_left(&LB.b)) || (u < 0 && lib_liberable(&LB.b, LB.tx, LB.ty));
}

uint32_t lib_keys(uint32_t keys) {
	uint32_t pressed = keys & ~LB.held;
	LB.held = keys;
	/* (a chat box's: its pages, its questions) */
	if (talk_busy() || emu_read8(BN6_CHATBOX)) return keys;
	/* his phase: he walks as BN6 walks him, and A beside what he can
	 * liberate opens his command; else he stands as the board goes on */
	if (LB.step == LIB_STEP_MOVE) {
		if ((pressed & KEY_A) && faced_takes()) { open_menu(); return keys & (KEY_START | KEY_SELECT); }
		return keys;
	}
	if (LB.step == LIB_STEP_MENU) menu_keys(pressed);
	return keys & (KEY_START | KEY_SELECT);
}
