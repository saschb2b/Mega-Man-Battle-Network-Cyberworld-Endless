/* director_home.h's town frame (the trip home and the ways:
 * director_home.c). The town at a run's start and at home between acts
 * (docs/HOME.md): Lan's words, the act's card held over the town, the
 * run saved there, MegaMan at each port, and the next layer's arrival
 * after the jack-in. */
#include "director_home.h"

#include "bn6.h"
#include "cinema.h"
#include "director_keys.h"
#include "director_layer.h"
#include "director_save.h"
#include "director_state.h"
#include "director_way.h"
#include "emu.h"
#include "flags.h"
#include "home_words.h"
#include "mapslot.h"
#include "run.h"
#include "save.h"
#include "talk.h"
#include "text.h"
#include "town.h"

bool home_map(int group, int number) { return group == town_info()->group && number == town_info()->number; }

/* home: Lan held while the act's card shows, A ending it early, as on a
 * layer */
static void home_hold(void) {
	bool hold = D.home && D.town_seen && cinema_busy();
	if (hold && cinema_input_mode() == CINEMA_FREE) cinema_input(CINEMA_HOLD);
	else if (!hold && D.arrival_hold && cinema_input_mode() == CINEMA_HOLD) cinema_input(CINEMA_FREE);
	D.arrival_hold = hold;
}

/* MegaMan and Lan coming home: the act done, and the ports open */
static const char *arrival_words(void) {
	int open = 1;
	bool sealed = false;
	home_ways(&open, &sealed);
	const char *ports[TOWN_PORTS];
	for (int k = 0; k < TOWN_PORTS; ++k) ports[k] = town_port_name(k) ? town_port_name(k) : "port";
	return home_words(D.home_beaten, open, ports, sealed ? ports[2] : NULL);
}

/* Lan and MegaMan's words (Dad's call, the first time; at home, the act
 * just done), once Lan is out, the map has settled and the card gone */
static void town_words(void) {
	if (!D.town_seen || !on_map() || D.intro_said || emu_read8(BN6_WARP_PENDING) != 0 || ++D.town_frames <= 40 || cinema_busy()) return;
	if (!(D.home ? talk_start(arrival_words(), FACE_MEGAMAN) : talk_script(town_info()->talk_archive, town_info()->intro))) return;
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

/* MegaMan at a port, once a visit each: where it leads and who waits
 * there; the dark way's sealed (docs/HOME.md) */
static void port_words(void) {
	if (!D.home || !D.intro_said || !on_map() || talk_busy() || emu_read8(BN6_CHATBOX)) return;
	int n = 1, port = town_port_at((int)emu_read32(BN6_PLAYER_X) >> 16, (int)emu_read32(BN6_PLAYER_Y) >> 16);
	bool sealed = false;
	const RunWay *w = home_ways(&n, &sealed);
	if (port < 0 || D.home_told >> port & 1 || (port >= n && !(port == 2 && sealed))) return;
	bool shut = port >= n;
	if (talk_start(home_port_words(shut ? 0 : w[port].biome, shut ? 0 : w[port].meets, port == 2, shut), FACE_MEGAMAN)) D.home_told |= 1u << port;
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
	port_words();
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
