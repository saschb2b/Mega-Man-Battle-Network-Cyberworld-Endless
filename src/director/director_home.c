/* director_home.h. Home after an act (docs/HOME.md): the exit pad of an
 * act's last layer jacks MegaMan out to the run's town, as BN6's own
 * jack-out does (its warp's transition type, internet to real world,
 * takes Lan back to where he jacked in), and the town's port leads to the
 * next act's first layer. */
#include "director_home.h"

#include <stdint.h>

#include "bn6.h"
#include "boss.h"
#include "cinema.h"
#include "director_keys.h"
#include "director_layer.h"
#include "director_save.h"
#include "director_state.h"
#include "director_way.h"
#include "emu.h"
#include "flags.h"
#include "gamecall.h"
#include "home_words.h"
#include "mapslot.h"
#include "net.h"
#include "run.h"
#include "save.h"
#include "talk.h"
#include "text.h"
#include "town.h"

/* BN6_WARP_GROUP_KIND's internet to real world (bn6f
 * MAP_GROUP_TRANSITION_TYPE_INTERNET_TO_REAL_WORLD): the map-enter code
 * then sets Lan down at BN6_SAVED_X's place */
enum { INTERNET_TO_REAL_WORLD = 1 };

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
	return true;
}

bool home_rebuild(void) {
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
}

bool home_map(int group, int number) { return group == town_info()->group && number == town_info()->number; }

/* home: Lan held while the act's card shows, A ending it early, as on a
 * layer */
static void home_hold(void) {
	bool hold = D.home && D.town_seen && cinema_busy();
	if (hold && cinema_input_mode() == CINEMA_FREE) cinema_input(CINEMA_HOLD);
	else if (!hold && D.arrival_hold && cinema_input_mode() == CINEMA_HOLD) cinema_input(CINEMA_FREE);
	D.arrival_hold = hold;
}

/* Lan and MegaMan's words (Dad's call, the first time; at home, the act
 * just done), once Lan is out, the map has settled and the card gone */
static void town_words(void) {
	if (!D.town_seen || !on_map() || D.intro_said || emu_read8(BN6_WARP_PENDING) != 0 || ++D.town_frames <= 40 || cinema_busy()) return;
	if (!(D.home ? talk_start(home_words(D.home_beaten), FACE_MEGAMAN) : talk_script(town_info()->talk_archive, town_info()->intro))) return;
	D.intro_said = true;
	if (!D.home && !profile.seen_intro) { profile.seen_intro = true; profile_save(); }
}

/* home's checkpoint, once its words are said and Lan is free */
static void home_checkpoint(void) {
	if (!D.home || !D.intro_said || D.home_saved || !on_map() || talk_busy() || emu_read8(BN6_CHATBOX) ||
		emu_read8(BN6_DIALOGUE_LOCK) || !flag_get(BN6_FLAG_PLAYER_CAN_MOVE)) return;
	D.home_saved = true;
	home_save();
}

/* In the town: nothing to watch but the jack-in, whose arrival on the
 * layer's map starts the run (or the next act) as a layer's warp does. */
void home_update(void) {
	map_label();
	arrow_update();
	if (on_map()) unwedge();
	if (home_map(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER))) D.town_seen = true;
	talk_update();
	home_hold();
	town_words();
	home_checkpoint();
	bool arrived = D.town_seen && on_map() && emu_read8(BN6_WARP_PENDING) == 0 &&
		emu_read8(BN6_MAP_GROUP) == D.group && emu_read8(BN6_MAP_NUMBER) == D.number;
	if (!arrived) return;
	D.town = false;
	D.home = false;
	D.frame = 0;
	D.checkpoint = true;
	lock_run();
	mapslot_music_forget_town();
}
