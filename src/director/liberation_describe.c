/* A liberation's board in the state's words (director_describe;
 * docs/devtools/scenarios.md), a fact a line, and the dev step that
 * stands the Navi in play beside a panel of a kind (play.py's `board
 * KIND`). The lines only read: a scenario checks what a player ends up
 * with by them (the panels freed, the phase against the par, the Order
 * Points, a unit's HP on the board, the prize's tier, the exit).
 *
 *   board STEP       opening, pick, banner, move, menu, battle, darkloids,
 *                    talk, bonus, switch, clear, prize, done; none on a
 *                    layer with no board, the one line there
 *   boardlook HOW    bn5 (BN5's own map, panels and pieces, its ROM beside
 *                    BN6's), plain (the act's floor recoloured)
 *   side WHOSE       player, darkloids: whose phase it is, or is about to be
 *   boardphase N     the phase (a battle's own `phase` is another's line), and
 *   par N, orderpoints N
 *   darkpanels N     the panels dark now (items', traps, DarkHoles and the
 *                    Darkloid's ring among them, as the board counts them)
 *   freed N          ... fewer than as the board began
 *   items N, traps N, bonuspanels N, barriers N
 *   darkholes N      standing (closed N: gone)
 *   guardians N      standing (deleted N: gone; a DarkHole sends its own back)
 *   darkloid NAME standing|deleted, darkloidhp HP/MAX (his HP on the board)
 *   shield on|off    no harm on the board this phase
 *   sealed none|shut|open, sealedroute none|hidden|found
 *   prize pending|under|par|over    the par's prize's tier, once he is deleted
 *   boardexit shut|open
 *   inplay NAME      the Navi in play
 *   mateK NAME HP/MAX inplay|ready|acted|out [sung]    each Navi of the team, MegaMan
 *                    mate0 (sung: a LifeMelody sung for him this phase)
 *   unitK NAME standing|gone at X Y [weak]    each unit, the Darkloid unit0 (weak:
 *                    struck by SneakAttack, its next battle the easier)
 *   stands X Y       the panel of the Navi in play, and
 *   under KIND       ... what lies under his feet as BN6 has walked him:
 *                    clean, bonus; rim on the edge of a panel he does not
 *                    walk (a fence stops him 4 units in); its kind where
 *                    a fence let him through
 *   faces KIND       what he faces: none, clean, dark, item, trap, bonus,
 *                    darkhole, ring, barrier, guardian, darkloid, mate
 *   command LABEL    the menu's line under the cursor, and
 *   takes yes|no     ... whether it can be taken
 *   fights WHAT      the battle under way: panel, guardian, darkhole, darkloid
 *   lastfight HOW    how the board's last battle ended: none, won, oneturn,
 *                    timeup, lost (the Navi in it deleted, or struck down)
 *   teampick NAMES, teamrow NAME|GO    the arrival's pick (liberation_pick.h)
 *   roulette SLOT, bonus SLOT    a BonusPanel's (liberation_bonus.h)
 *   darkshown N, facedshown N    the picture's (liberation_draw.c) */
#include "liberation.h"

#include <stdlib.h>
#include <string.h>

#include "director.h"
#include "bn6_fields.h"
#include "director_state.h"
#include "guardians.h"
#include "liberation_bonus.h"
#include "liberation_pick.h"
#include "liberation_state.h"
#include "liberation_team.h"
#include "liberation_words.h"
#include "netmap.h"

static const int dirs[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
/* (BN6's facings along the grid's ways, liberation.c's) */
static const int facing[4] = { 3, 5, 7, 1 };

static int count(int kind) {
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) n += LB.b.kind[y][x] == kind;
	return n;
}

static int dark_now(void) {
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) n += lib_dark_kind(LB.b.kind[y][x]);
	return n;
}

/* Panel (x, y)'s kind in a word, and what stands on it: a unit or a teammate before its kind */
static const char *panel_word(int x, int y) {
	static const char *const kinds[LIB_KINDS] = { "none", "clean", "dark", "item", "bonus", "darkhole", "ring", "darkloid", "barrier", "trap" };
	if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return "none";
	return LB.b.kind[y][x] < LIB_KINDS ? kinds[LB.b.kind[y][x]] : "none";
}

static const char *kind_word(int x, int y) {
	if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return "none";
	int u = lib_unit_at(x, y), m = lib_mate_at(&LB.b, x, y);
	if (u >= 0) return u ? "guardian" : "darkloid";
	if (m >= 0 && m != LB.in_play) return "mate";
	return panel_word(x, y);
}

/* What lies under the player's feet: the panel's kind; "rim" within 8
 * units of the edge of a panel he does not walk, where its fence holds him */
static const char *under_word(void) {
	int px = bn6_player_x(), py = bn6_player_y(), x, y, wx, wy;
	if (!netmap_panel(px, py, &x, &y)) return "none";
	if (lib_walkable(&LB.b, x, y)) return panel_word(x, y);
	netmap_world(x, y, &wx, &wy);
	return abs(px - wx) > 8 || abs(py - wy) > 8 ? "rim" : panel_word(x, y);
}

static void describe_units(FILE *f) {
	int standing = 0, gone = 0;
	for (int u = 1; u < LB.b.nunits; ++u) {
		standing += !LB.b.unit[u].gone;
		gone += LB.b.unit[u].gone;
	}
	const char *darkloid = guardian(LB.darkloid)->name;
	fprintf(f, "darkholes %d\nclosed %d\nguardians %d\ndeleted %d\n", lib_holes_left(&LB.b), LB.b.nholes - lib_holes_left(&LB.b), standing, gone);
	fprintf(f, "darkloid %s %s\ndarkloidhp %d/%d\n", darkloid, LB.b.unit[0].gone ? "deleted" : "standing", LB.b.unit[0].gone ? 0 : LB.hp[0], LB.hp_max[0]);
	for (int u = 0; u < LB.b.nunits; ++u)
		fprintf(f, "unit%d %s %s at %d %d%s\n", u, u ? lib_unit_name(LB.b.unit[u].kind) : darkloid, LB.b.unit[u].gone ? "gone" : "standing", LB.b.unit[u].x,
			LB.b.unit[u].y, LB.weak[u] ? " weak" : "");
}

static void describe_team(FILE *f) {
	fprintf(f, "inplay %s\n", lib_mate_name(LB.in_play));
	for (int k = 0; k < LB.b.team.n; ++k)
		fprintf(f, "mate%d %s %d/%d %s%s\n", k, lib_mate_name(k), lib_mate_hp(k), lib_mate_max(k),
			LB.b.mate[k].out ? "out" : LB.mate[k].acted ? "acted" : k == LB.in_play ? "inplay" : "ready", LB.mate[k].melody ? " sung" : "");
}

/* The sealed region: its barrier shut or opened by its key, its route hidden or found */
static void describe_sealed(FILE *f) {
	bool shut = false;
	for (int y = 0; y < MAP_H && LB.b.sealed; ++y)
		for (int x = 0; x < MAP_W; ++x) shut |= LB.b.kind[y][x] == LIB_BAR && LB.b.item[y][x] == LB.b.sealed;
	fprintf(f, "sealed %s\nsealedroute %s\n", !LB.b.sealed ? "none" : shut ? "shut" : "open",
		LB.route_found ? "found" : LB.b.sealed && LB.b.route_x >= 0 ? "hidden" : "none");
}

void lib_describe(FILE *f) {
	static const char *const steps[] = { "opening", "banner", "move", "menu", "battle", "darkloids", "talk", "prize", "done", "bonus", "clear", "switch", "pick" };
	static const char *const prizes[] = { "under", "par", "over" };
	static const char *const fights[] = { "panel", "guardian", "darkhole", "darkloid" };
	static const char *const fought[] = { "none", "won", "oneturn", "timeup", "lost" };
	if (!D.active || D.town) return;
	if (!LB.on) { fprintf(f, "board none\n"); return; }
	int dark = dark_now();
	fprintf(f, "boardlook %s\n", lib_look_on() ? "bn5" : "plain");
	fprintf(f, "board %s\nside %s\nboardphase %d\npar %d\norderpoints %d\n", LB.step >= 0 && LB.step < (int)(sizeof steps / sizeof *steps) ? steps[LB.step] : "none",
		LB.enemy_banner || LB.step == LIB_STEP_ENEMY ? "darkloids" : "player", LB.phase, LB.b.par, LB.op);
	fprintf(f, "darkpanels %d\nfreed %d\nitems %d\ntraps %d\nbonuspanels %d\nbarriers %d\n", dark, LB.b.dark - dark, count(LIB_ITEM), count(LIB_TRAP),
		count(LIB_BONUS), count(LIB_BAR));
	describe_units(f);
	fprintf(f, "shield %s\n", LB.shield ? "on" : "off");
	describe_sealed(f);
	fprintf(f, "prize %s\nboardexit %s\n", !LB.b.unit[0].gone ? "pending" : prizes[LB.prize < 0 || LB.prize > 2 ? 2 : LB.prize], lib_exit_shut() ? "shut" : "open");
	describe_team(f);
	fprintf(f, "stands %d %d\nunder %s\nfaces %s\n", LB.mx, LB.my, under_word(), kind_word(LB.tx, LB.ty));
	if (LB.step == LIB_STEP_MENU)
		fprintf(f, "command %s\ntakes %s\n", lib_command_label(LB.menu, lib_mate_command(LB.in_play)), LB.can[LB.menu] ? "yes" : "no");
	if (LB.step == LIB_STEP_BATTLE) fprintf(f, "fights %s\n", fights[LB.fight & 3]);
	fprintf(f, "lastfight %s\n", fought[LB.fought >= 0 && LB.fought < 5 ? LB.fought : 0]);
	lib_pick_describe(f);
	lib_bonus_describe(f);
	lib_draw_describe(f);
}

/* ---- the dev step ---- */

/* Whether panel (x, y) is one of kind `what` as the step names them: a
 * panel's kind, a unit on it, or what an item panel holds; the Darkloid
 * and his ring only once a command can be taken on them (every DarkHole
 * gone) */
static bool wanted(const char *what, int x, int y) {
	static const char *const got[] = { "orderpoints", "heart", "key", "zenny", "chip", "route" };
	bool ring = LB.b.kind[y][x] == LIB_RING || lib_unit_at(x, y) == 0;
	if (!strcmp(what, kind_word(x, y))) return !ring || !lib_holes_left(&LB.b);
	if (LB.b.kind[y][x] != LIB_ITEM || lib_unit_at(x, y) >= 0) return false;
	int it = LB.b.item[y][x] & 15;
	return it < (int)(sizeof got / sizeof *got) && !strcmp(what, got[it]);
}

/* (pair: a dark panel with another to liberate behind it, LongSwrd's two;
 * sneak: a guardian on the panel faced or the one behind it, SneakAttack's
 * reach; over: a plain dark panel BombLiberation's flier hovers over) */
static bool wanted_from(const char *what, int x, int y, int d) {
	int bx = x + dirs[d][0], by = y + dirs[d][1];
	bool behind = bx >= 0 && by >= 0 && bx < MAP_W && by < MAP_H;
	if (!strcmp(what, "over")) return lib_mate_command(LB.in_play) == LIB_DO_BOMB && LB.b.kind[y][x] == LIB_DARK && lib_unit_at(x, y) < 0;
	if (!strcmp(what, "sneak")) return LB.b.kind[y][x] != LIB_VOID && (lib_unit_at(x, y) > 0 || (behind && lib_unit_at(bx, by) > 0));
	if (strcmp(what, "pair")) return wanted(what, x, y);
	return wanted("dark", x, y) && behind && wanted("dark", bx, by);
}

bool director_dev_board(const char *what) {
	static int16_t walk[MAP_H][MAP_W];
	static uint16_t q[MAP_W * MAP_H];
	if (!lib_on() || LB.step != LIB_STEP_MOVE || !director_on_map()) return false;
	/* (next: where the autopilot would stand him, liberation_auto.c) */
	int sx, sy, face, wx, wy;
	if (!strcmp(what, "next")) {
		if (!lib_auto_stand(&sx, &sy, &face)) return false;
		netmap_world(sx, sy, &wx, &wy);
		director_dev_place(wx, wy, facing[face]);
		return true;
	}
	/* (the nearest by the walk from where he stands over the panels he
	 * walks, clean ones and BonusPanels, through his teammates as BN5's
	 * Navis walk through theirs: nowhere a player could not go; his stand
	 * no teammate's panel) */
	memset(walk, 0xFF, sizeof walk);
	int head = 0, tail = 0;
	walk[LB.my][LB.mx] = 0;
	q[tail++] = (uint16_t)(LB.my * MAP_W + LB.mx);
	while (head < tail) {
		int c = q[head++], x = c % MAP_W, y = c / MAP_W, m = lib_mate_at(&LB.b, x, y);
		bool free = m < 0 || m == LB.in_play;
		for (int d = 0; d < 4; ++d) {
			int nx = x + dirs[d][0], ny = y + dirs[d][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H) continue;
			if (free && wanted_from(what, nx, ny, d)) {
				/* (beside it; the flier over it) */
				bool over = !strcmp(what, "over");
				netmap_world(over ? nx : x, over ? ny : y, &wx, &wy);
				director_dev_place(wx, wy, facing[d]);
				return true;
			}
			if (walk[ny][nx] >= 0 || !lib_walkable(&LB.b, nx, ny) || lib_unit_at(nx, ny) >= 0) continue;
			walk[ny][nx] = (int16_t)(walk[y][x] + 1);
			q[tail++] = (uint16_t)(ny * MAP_W + nx);
		}
	}
	return false;
}
