/* director_home.h's trip home and the ways (the town's frame:
 * director_town.c). Home after an act (docs/HOME.md): the exit pad of an
 * act's last layer jacks MegaMan out to the run's town, as BN6's own
 * jack-out does (its warp's transition type, internet to real world,
 * takes Lan back to where he jacked in), and the town's ports lead to the
 * next act's ways, its first layer built for the one taken. */
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
#include "mapslot.h"
#include "meta.h"
#include "net.h"
#include "run.h"
#include "town.h"

/* BN6_WARP_GROUP_KIND's internet to real world (bn6f
 * MAP_GROUP_TRANSITION_TYPE_INTERNET_TO_REAL_WORLD): the map-enter code
 * then sets Lan down at BN6_SAVED_X's place */
enum { INTERNET_TO_REAL_WORLD = 1 };

/* The next act's ways (run_ways), worked out from the run's seed at home
 * and again on a CONTINUE there; the one whose first layer is built */
static RunWay ways[TOWN_PORTS];
static int nways = 1, taken;
static bool sealed;

/* the act the ways lead to, 0-based: the next layer's */
static int next_act(void) { return (run.depth - 1) % CYCLE_LAYERS / 3; }

static void work_out_ways(void) {
	nways = run_ways(next_act(), meta_dark_way_open(), ways, &sealed);
	taken = 0;
	town_ways(nways);
	if (emu_debug_on())
		for (int k = 0; k < nways; ++k) fprintf(stderr, "home: way %d area %d guardian %d (meets %d)%s\n", k, ways[k].biome, ways[k].navi,
			ways[k].meets, sealed && k == nways - 1 ? ", the dark way sealed after it" : "");
}

const RunWay *home_ways(int *n, bool *dark_sealed) {
	*n = nways;
	*dark_sealed = sealed;
	return ways;
}

bool home_take_way(int port) {
	if (!D.home) return false;
	if (port < 0 || port >= nways) port = 0;
	if (port == taken) return true;
	int act = next_act();
	run.biome_order[act] = (uint8_t)ways[port].biome;
	run.boss_order[ways[port].biome] = (uint8_t)ways[port].navi;
	taken = port;
	if (emu_debug_on()) fprintf(stderr, "home: way %d taken, area %d\n", port, ways[port].biome);
	if (!new_layer(false)) return false;
	mapslot_jack_to(D.group, D.number, D.start_x, D.start_y, 4);
	return true;
}

bool home_due(void) {
	return boss_beaten() && run.side_kind == LAYER_NORMAL && is_boss_depth(run.depth) && !run_short_nest(run.depth);
}

/* Where BN6's jack-out sets Lan down: where he jacked in, when that was
 * this town's port; else (a run begun in the net) short of the port */
static bool set_down_place(const TownInfo *ti) {
	uint32_t town = (uint32_t)ti->group | (uint32_t)ti->number << 8;
	if ((emu_read32(BN6_SAVED_MAP) & 0xFFFF) == town) return true;
	int x, y, face;
	if (!town_home_spot(&x, &y, &face)) return false;
	emu_write32(BN6_SAVED_X, (uint32_t)x << 16);
	emu_write32(BN6_SAVED_Y, (uint32_t)y << 16);
	emu_write32(BN6_SAVED_Z, 0);
	emu_write32(BN6_SAVED_FACING, (uint32_t)face);
	emu_write32(BN6_SAVED_MAP, town);
	return true;
}

bool home_begin(const char *beaten) {
	work_out_ways();
	if (!town_plan(town_seed(run.seed)) || !town_install(D.group, D.number, D.start_x, D.start_y)) return false;
	if (!set_down_place(town_info())) return false;
	emu_write8(BN6_WARP_GROUP_KIND, INTERNET_TO_REAL_WORLD);
	/* R jacks in there again, MegaMan back in the PET as BN6's own jack-out
	 * puts him (else: "MegaMan isn't in the PET...") */
	flag_clear(BN6_FLAG_NO_JACK);
	flag_set(BN6_FLAG_NAVI_IN_PET);
	D.town = true;
	D.home = true;
	D.town_seen = false;
	D.town_frames = 0;
	D.intro_said = false;
	D.home_beaten = beaten;
	D.home_saved = false;
	D.home_told = 0;
	return true;
}

bool home_rebuild(void) {
	work_out_ways();
	return town_plan(town_seed(run.seed)) && town_install(D.group, D.number, D.start_x, D.start_y);
}

void home_resume(void) {
	const TownInfo *ti = town_info();
	/* (the game reloads the town's people and tiles from its tables,
	 * which a state does not hold) */
	emu_warp(ti->group, ti->number, (int)emu_read32(BN6_PLAYER_X) >> 16, (int)emu_read32(BN6_PLAYER_Y) >> 16, (int)emu_read8(BN6_PLAYER_FACING));
	/* (R jacks in, which the run's lock had stopped) */
	flag_clear(BN6_FLAG_NO_JACK);
	flag_set(BN6_FLAG_NAVI_IN_PET);
	D.town = true;
	D.home = true;
	D.town_seen = false;
	D.town_frames = 0;
	D.intro_said = true;
	D.home_saved = true;
	D.home_told = 0;
}

void director_dev_home(void) {
	int x, y, face;
	if (!home_begin(NULL) || !town_home_spot(&x, &y, &face)) return;
	emu_warp(town_info()->group, town_info()->number, x, y, face);
}
