/* director_home.h's trip home and the ways (the home's frame:
 * director_town.c). Home after an act (docs/HOME.md): the exit pad of an
 * act's last layer takes MegaMan to Lan's HP, BN6's own homepage
 * (lanhp.c), where the next act's ways are portals; the next act's first
 * layer is built for the first way as he leaves, and again for another
 * way when he steps on its portal. The older portals beside them go back
 * to the areas the run has won, a trip of one layer each, once a visit,
 * each a notch on the Net's clock; its exit leads home again. From Lan's
 * HP he jacks out to Lan's room, as BN6 does, and the PC's jack-in takes
 * him back there. */
#include "director_home.h"

#include <stdint.h>
#include <stdio.h>

#include "bn6.h"
#include "boss.h"
#include "debug.h"
#include "director.h"
#include "director_layer.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "gamecall.h"
#include "lanhp.h"
#include "mapslot.h"
#include "meta.h"
#include "net.h"
#include "pacing.h"
#include "run.h"
#include "lan_house.h"
#include "town.h"

#define HOME_WAYS 3   /* run_ways's most: the act's own, the other way, the dark way; the older portals after them */
#define ARRIVE_FACE 4

/* The next act's ways (run_ways), worked out from the run's seed at home
 * and again on a CONTINUE there; the one whose first layer is built; and
 * the older acts a trip back may go to (pacing_older_acts), newest first */
static RunWay ways[HOME_WAYS];
static int nways = 1, taken;
static bool sealed;
static int older[PACING_OLDER], nolder;

/* the act the ways lead to, 0-based: the next layer's */
static int next_act(void) { return (run.depth - 1) % CYCLE_LAYERS / 3; }

static void work_out_ways(void) {
	nways = run_ways(next_act(), meta_dark_way_open(), ways, &sealed);
	taken = 0;
	nolder = pacing_older_acts(run.depth, older);
	if (!emu_debug_on()) return;
	for (int k = 0; k < nways; ++k) fprintf(stderr, "home: way %d area %d guardian %d (meets %d)%s\n", k, ways[k].biome, ways[k].navi,
		ways[k].meets, sealed && k == nways - 1 ? ", the dark way sealed after it" : "");
	for (int j = 0; j < nolder; ++j) fprintf(stderr, "home: back %d to depth %d area %d%s\n", j, older[j], biome_for_depth(older[j]),
		run.back_spent >> j & 1 ? ", taken this visit" : "");
}

unsigned home_lit(void) {
	unsigned lit = (1u << nways) - 1;
	for (int j = 0; j < nolder; ++j) if (!(run.back_spent >> j & 1)) lit |= 1u << (HOME_WAYS + j);
	return lit;
}

int home_older(int k) {
	int j = k - HOME_WAYS;
	return j < 0 || j >= nolder || run.back_spent >> j & 1 ? -1 : biome_for_depth(older[j]);
}

int director_older_portal(void) {
	for (int k = 0; k < LANHP_PORTALS; ++k) if (home_older(k) >= 0) return k;
	return -1;
}

bool home_go_back(int k) {
	if (home_older(k) < 0) return false;
	int j = k - HOME_WAYS;
	uint8_t was_spent = run.back_spent, was_clock = run.clock;
	run.back_spent |= (uint8_t)(1u << j);
	run.home_depth = (uint16_t)run.depth;
	run.depth = older[j];
	run.side_kind = LAYER_BACK;
	if (run.clock < 250) ++run.clock;
	if (emu_debug_on()) fprintf(stderr, "home: back to depth %d area %d, the Net's clock at %d\n", run.depth, biome_for_depth(run.depth), run.clock);
	if (!new_layer(false)) {
		/* (none made: the portal goes on leading home) */
		run.depth = run.home_depth;
		run.home_depth = 0;
		run.side_kind = LAYER_NORMAL;
		run.back_spent = was_spent;
		run.clock = was_clock;
		return false;
	}
	lanhp_portal(k, D.group, D.number, D.start_x, D.start_y, ARRIVE_FACE);
	return true;
}

const RunWay *home_ways(int *n, bool *dark_sealed) {
	*n = nways;
	*dark_sealed = sealed;
	return ways;
}

/* Lan's HP installed, each open way's portal to the layer built (the
 * first way's) */
static bool hp_ready(void) {
	if (!lanhp_install()) return false;
	for (int k = 0; k < nways; ++k) lanhp_portal(k, D.group, D.number, D.start_x, D.start_y, ARRIVE_FACE);
	return true;
}

bool home_take_way(int k) {
	if (k < 0 || k >= nways) return false;
	run.back_spent = 0;   /* (the next visit's older portals open again) */
	if (k == taken) return true;
	int act = next_act();
	run.biome_order[act] = (uint8_t)ways[k].biome;
	run.boss_order[ways[k].biome] = (uint8_t)ways[k].navi;
	taken = k;
	if (emu_debug_on()) fprintf(stderr, "home: way %d taken, area %d\n", k, ways[k].biome);
	if (!new_layer(false)) return false;
	lanhp_portal(k, D.group, D.number, D.start_x, D.start_y, ARRIVE_FACE);
	return true;
}

bool home_due(void) {
	return boss_beaten() && run.side_kind == LAYER_NORMAL && is_boss_depth(run.depth) && !run_short_nest(run.depth);
}

/* Where BN6's jack-out from Lan's HP sets Lan down: where he jacked in,
 * his room's PC (a run begun in the net never stood there) */
static void set_down_place(void) {
	uint32_t room = LAN_HOUSE_GROUP | LAN_ROOM << 8;
	if ((emu_read32(BN6_SAVED_MAP) & 0xFFFF) == room) return;
	int x, y;
	if (!lan_house_goal(LAN_ROOM, &x, &y)) return;
	emu_write32(BN6_SAVED_X, (uint32_t)x << 16);
	emu_write32(BN6_SAVED_Y, (uint32_t)y << 16);
	emu_write32(BN6_SAVED_Z, 0);
	emu_write32(BN6_SAVED_FACING, 7);
	emu_write32(BN6_SAVED_MAP, room);
}

/* the town, Lan's house and room, and Lan's HP with its portals */
static bool home_install(void) {
	int x, y;
	lanhp_arrival(&x, &y);
	return town_plan(town_seed(run.seed)) && town_install(LANHP_GROUP, LANHP_NUMBER, x, y) &&
		lan_house_install(LANHP_GROUP, LANHP_NUMBER, x, y) && hp_ready();
}

bool home_run_start(bool abandoned) {
	town_after_abandon = abandoned;
	work_out_ways();
	town_ways(1);   /* (the town's ports are no ways: docs/HOME.md) */
	if (!home_install()) return false;
	/* R jacks in at his PC; the PET's own Save stays off */
	flag_clear(BN6_FLAG_NO_JACK);
	flag_set(BN6_FLAG_NO_PET_SAVE);
	flag_set(BN6_FLAG_NAVICUST);
	int x, y, face;
	lan_room_start(&x, &y, &face);
	emu_warp(LAN_HOUSE_GROUP, LAN_ROOM, x, y, face);
	D.town = true;
	D.home = false;
	D.hp_said = false;
	D.portal_taken = false;
	D.town_seen = false;
	D.town_frames = 0;
	D.intro_said = false;
	D.port_told = false;
	D.free_x = x;
	D.free_y = y;
	return true;
}

/* home after an act or a trip back: Lan's HP and the town installed, the
 * ways worked out, the exit's warp to Lan's HP */
static bool home_arrive(const char *beaten, bool back) {
	work_out_ways();
	if (!home_install()) return false;
	set_down_place();
	/* the exit's warp (BN6's link departure, as every exit pad plays it)
	 * to Lan's HP's arrival */
	int x, y;
	lanhp_arrival(&x, &y);
	mapslot_exit_to(LANHP_GROUP, LANHP_NUMBER, x, y, ARRIVE_FACE);
	D.town = true;
	D.home = true;
	D.town_seen = false;
	D.town_frames = 0;
	D.intro_said = false;
	D.home_beaten = beaten;
	D.home_back = back;
	D.home_saved = false;
	D.home_told = 0;
	D.hp_said = true;
	D.portal_taken = false;
	return true;
}

bool home_begin(const char *beaten) { return home_arrive(beaten, false); }

bool home_return(void) { return home_arrive(NULL, true); }

bool home_rebuild(void) {
	work_out_ways();
	return home_install();
}

void home_resume(void) {
	/* (the game reloads the map's people and tiles from its tables, which a
	 * state does not hold: Lan's HP, or the town where Lan stood) */
	emu_warp(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER), (int)emu_read32(BN6_PLAYER_X) >> 16, (int)emu_read32(BN6_PLAYER_Y) >> 16,
		(int)emu_read8(BN6_PLAYER_FACING));
	/* (R jacks out there, or in, which the run's lock had stopped) */
	flag_clear(BN6_FLAG_NO_JACK);
	D.town = true;
	D.home = true;
	D.town_seen = false;
	D.town_frames = 0;
	D.intro_said = true;
	D.home_saved = true;
	D.home_told = 0;
	D.hp_said = true;
	D.portal_taken = false;
}

void director_dev_hush_home(void) { D.intro_said = D.hp_said = true; }

void director_dev_home(void) {
	if (!home_begin(NULL)) return;
	int x, y;
	lanhp_arrival(&x, &y);
	emu_warp(LANHP_GROUP, LANHP_NUMBER, x, y, ARRIVE_FACE);
}
