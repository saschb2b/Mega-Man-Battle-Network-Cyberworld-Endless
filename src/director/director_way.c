/* Where things lie from MegaMan and the way on: as the crow flies and
 * along the floor (net_route.c), the town's way to its port, the heal while
 * he is hurt, and the way-on arrow that shows it. */
#include "director_way.h"

#include <stdlib.h>

#include "bn6_fields.h"
#include "boss.h"
#include "cinema.h"
#include "director_folder.h"
#include "director_state.h"
#include "net_route.h"
#include "netmap.h"
#include "save.h"
#include "talk.h"
#include "town.h"

/* Which way the exit pad lies from MegaMan, as the screen shows it (the
 * d-pad's UP moves +X -Y, RIGHT +X +Y: a world step (dx, dy) goes
 * dx + dy across and (dy - dx) / 2 down), and how far. */
int way_dir;   /* the index of the last way_to: 0 right, then clockwise */
const char *const ways[8] = {
	"to the right", "to the lower right", "straight down", "to the lower left",
	"to the left", "to the upper left", "straight up", "to the upper right",
};

/* How far `panels` is in L's words: close under five, a long way from
 * twenty-five. A homepage's whole walk from its arrival to its exit is
 * sixteen to twenty panels, the other areas' thirty to forty-five; from
 * fourteen, "a long way yet" named a homepage's exit eight panels on
 * (session 62). */
static int far_of(int panels) { return panels < 5 ? 0 : panels < 25 ? 1 : 2; }

const char *way_to(int tx, int ty, int *far) {
	int px = bn6_player_x(), py = bn6_player_y();
	int dx = tx - px, dy = ty - py;
	*far = far_of((abs(dx) + abs(dy)) / 32);
	/* (on the grid, whose +x is the world's +Y and +y its -X: the pad's
	 * ways, net_route.c) */
	way_dir = route_grid_way(dy / 32.0, -dx / 32.0);
	return ways[way_dir];
}

/* ProtoMan, while his duel waits on this layer (not taken, not a netbattle
 * named for later): where he stands in the world. */
bool duel_waiting(int *wx, int *wy) {
	if (layer_objs_duel_later) return false;
	for (int i = 0; i < D.objs.nchoices; ++i)
		if (D.objs.choice[i].type == OBJ_DUEL && (D.chosen & (1u << i))) return false;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_DUEL) { netmap_world((int)layer.obj[i].x, (int)layer.obj[i].y, wx, wy); return true; }
	return false;
}

/* The town's way to the port on foot (town_walk): the first stretch of the
 * walk, not the line to it, which led a playtester into a house front
 * ("straight up") on two runs; how far that walk is. */
const char *town_way(int *far) {
	int px = bn6_player_x(), py = bn6_player_y();
	int wx, wy, cells;
	if (!town_walk(px, py, 8, &wx, &wy, &cells)) return way_to(town_info()->port_x, town_info()->port_y, far);
	const char *way = way_to(wx, wy, far);
	int panels = cells / 4;   /* (a panel is 32 units, a cell 8) */
	*far = panels < 5 ? 0 : panels < 14 ? 1 : 2;
	return way;
}

/* What is gone from the layer's floor, for the walk round what stands on
 * it (route_gone): each Mystery Data taken, and the guardian once his
 * Guardian Data, where he stood, is taken. The walk went round his empty
 * panel to the exit pad shown behind it: the arrow pointed up, away from
 * a pad three panels down on the same floor, and L said the way wound
 * (session 65). */
void route_floor(void) {
	uint64_t gone = 0;
	for (int k = 0; k < D.objs.nmd; ++k)
		if (flag_get(MAPSLOT_MD_FLAG + k)) gone |= 1ull << D.objs.md_obj[k];
	for (int i = 0; i < layer.nobj && boss_done(); ++i)
		if (layer.obj[i].type == OBJ_BOSS) gone |= 1ull << i;
	route_gone = gone;
}

/* The way on along the floor, not as the crow flies (net_route.c), and
 * how far that walk is. NULL when either end is off the floor. */
const char *route_to(int tx, int ty, int *far) {
	int px = bn6_player_x(), py = bn6_player_y();
	double gx, gy;
	int ex, ey, len;
	netmap_grid(px, py, &gx, &gy);
	if (!netmap_panel(tx, ty, &ex, &ey)) return NULL;
	route_floor();
	int w = route_way(gx, gy, ex, ey, &len);
	if (w < 0) return NULL;
	*far = far_of(len);
	way_dir = w;
	return ways[w];
}

/* Where something lies from MegaMan, as L and the map's mark give it,
 * with how far the walk there is (*far) and whether that walk sets off
 * another way (*winds); way_dir left as it was. */
const char *lie_and_walk(int wx, int wy, int *far, bool *winds) {
	int keep = way_dir, walk_far;
	const char *lies = way_to(wx, wy, far);
	int lies_dir = way_dir, seen_far = *far;
	const char *walk = route_to(wx, wy, &walk_far);
	int apart = walk ? abs(way_dir - lies_dir) : 0;
	if (apart > 4) apart = 8 - apart;
	if (walk) *far = walk_far;
	/* (a walk longer in words than the line to it: how far it stands, and
	 * that the way winds; "far off" named ProtoMan four panels from
	 * MegaMan on the screen, session 62) */
	if (walk && walk_far > seen_far) { *far = seen_far; apart = 2; }
	*winds = apart >= 2;
	way_dir = keep;
	return lies;
}

/* MegaMan below three quarters of his HP (at 220 of 240 the heal led L's
 * words before the way on) */
bool hurt_now(void) { return emu_read16(BN6_NAVI_HP) * 4 < emu_read16(BN6_NAVI_MAX_HP) * 3; }

/* (his one patch given on this layer, issue #71; the Heals helper's heals
 * as often as asked) */
bool heal_spent(void) { return !(run.helpers & HELP_HEALS) && flag_get(LAYER_HEAL_TOLD_FLAG); }

/* Where the layer's Recovery Mr. Prog stands, in the world; false for
 * none. */
static bool heal_spot(int *wx, int *wy) {
	if (heal_spent()) return false;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_HEAL) { netmap_world((int)layer.obj[i].x, (int)layer.obj[i].y, wx, wy); return true; }
	return false;
}

/* The way on as the arrow shows it: along the floor to the exit or the
 * guardian (the port in the town); way_dir holds it. While MegaMan is
 * hurt, to the layer's Recovery Mr. Prog first, as L says (a playtester
 * at 180 HP heard which way it was, and the arrow led to the exit). */
void goal_way(void) {
	int far;
	if (D.town) { cinema_arrow_heal(false); town_way(&far); return; }
	int gx = D.objs.exit_x, gy = D.objs.exit_y;
	if (D.objs.guardian.navi && !boss_beaten()) { gx = D.objs.guardian.x; gy = D.objs.guardian.y; }
	cinema_arrow_heal(hurt_now() && heal_spot(&gx, &gy));
	if (!route_to(gx, gy, &far)) way_to(gx, gy, &far);
}

/* MegaMan pushing a while where the pad goes nowhere (a platform's
 * corner, a lane's end, with no walkway in reach to line him up with):
 * the way-on arrow shows along the floor, as after L's words. */
void push_arrow(void) {
	static int ax, ay, pushed;
	int px = bn6_player_x(), py = bn6_player_y();
	/* (the edge's push-out jostles him a unit or two) */
	bool moved = abs(px - ax) > 4 || abs(py - ay) > 4;
	if (moved || !D.dir_held || D.walk_t || emu_read8(BN6_CHATBOX) || talk_busy() || cinema_busy() || boss_cinematic()) {
		ax = px; ay = py; pushed = 0;
		return;
	}
	if (++pushed != 40 || cinema_arrow_on()) return;
	goal_way();
	cinema_arrow(way_dir, 150);
}

/* As the layer's map closes (SELECT let go): the way-on arrow, as after
 * L's words, for a player who looked for the way there (a playtester
 * looked at the map on a strip where the arrow had faded, then probed
 * directions for four calls). */
static void map_arrow(void) {
	static bool was;
	if (was && !D.map_shown && on_map()) {
		goal_way();
		if (cinema_arrow_on()) cinema_arrow_extend(300);
		else cinema_arrow(way_dir, 300);
	}
	was = D.map_shown;
}

/* The way-on arrow: on while L's words last, however many boxes, then ten
 * seconds more, and on while MegaMan walks, up to half a minute; and after
 * the map. */
void arrow_update(void) {
	/* (not over a warp's or the jack-in's flash and tunnel) */
	if (D.warping || emu_read8(BN6_WARP_PENDING)) {
		if (cinema_arrow_on()) cinema_arrow(0, 0);
		D.arrow_pending = false;
		return;
	}
	map_arrow();
	/* (it turns as MegaMan walks: frozen, it pointed into the gap he had
	 * walked past) */
	/* (a new way twice running before it turns: at a walkway's mouth the
	 * route's first leg flipped as MegaMan crossed a panel's border, and
	 * the arrow with it; a look every 5 frames, as every 15 a running
	 * MegaMan was two panels past a turn before it turned) */
	/* (and not for a way just past the edge of the one shown: the arrow
	 * wobbled between neighbouring eighths as the walk's aim moved, a third
	 * of its turns swung back within a second, and a playtester holding
	 * the way a picture showed ran past the turns) */
	/* (and at once for the first frames after MegaMan stops: standing, the
	 * way can't flip on a border, and a playtester's pictures 4 frames
	 * after each step showed the way from before it, three steps running
	 * at one walkway's mouth) */
	static int tick, pending = -1, settle;
	settle = D.dir_held ? 4 : settle > 0 ? settle - 1 : 0;
	bool stopping = !D.dir_held && settle > 0;
	if (cinema_arrow_on() && on_map() && (++tick % 5 == 0 || stopping)) {
		goal_way();
		if ((way_dir == pending || stopping) && !route_way_holds(cinema_arrow_dir(), 0.8)) cinema_arrow_turn(way_dir);
		pending = way_dir;
	}
	/* (it faded three seconds after the words, and in the Aquarium Comp's
	 * mazes of short walkways a playtester lost a dozen moves at a time
	 * between one L and the next) */
	if (cinema_arrow_on() && !D.arrow_pending && D.dir_held && on_map() && cinema_arrow_age() < 1800) cinema_arrow_extend(180);
	if (!D.arrow_pending) return;
	/* (three seconds after a briefing of eight boxes, and a playtester who
	 * closed its last had lost it before he set off) */
	if (talk_busy()) cinema_arrow_extend(60);
	else { D.arrow_pending = false; cinema_arrow_extend(600); }
}
