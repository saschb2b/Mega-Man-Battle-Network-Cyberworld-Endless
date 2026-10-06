/* director_home.h's home frame (the trip home and the ways:
 * director_home.c). Lan's HP after each act and at every jack-in, and the
 * town at a run's start and whenever Lan jacks out (docs/HOME.md): Lan's
 * and MegaMan's words, the act's card held over Lan's HP, the run saved
 * there, MegaMan at each portal, the portal taken, and the next layer's
 * arrival. */
#include "director_home.h"

#include <stdlib.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "cinema.h"
#include "director_keys.h"
#include "director_layer.h"
#include "director_save.h"
#include "director_state.h"
#include "director_way.h"
#include "emu.h"
#include "flags.h"
#include "home_words.h"
#include "home_places.h"
#include "lesson_words.h"
#include "lanhp.h"
#include "mapslot.h"
#include "run.h"
#include "save.h"
#include "talk.h"
#include "town.h"

#define PORTAL_REACH 20   /* world units from a portal's cells MegaMan names it from */

static bool map_is(int group, int number) { return emu_read8(BN6_MAP_GROUP) == group && emu_read8(BN6_MAP_NUMBER) == number; }

bool home_in_hp(void) { return D.town && map_is(LANHP_GROUP, LANHP_NUMBER); }

/* the map Lan or MegaMan stands on as home_places sees it */
static int place_here(void) { return home_places_at(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER)); }

const char *home_place_name(void) {
	if (home_in_hp()) return "Lan's HP";
	const char *name = home_places_name(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER));
	if (name) return name;
	return town_info()->name ? town_info()->name : "Town";
}

bool home_map(int group, int number) {
	return (group == town_info()->group && number == town_info()->number) || home_places_at(group, number) != HOME_PLACE_NONE ||
		(group == LANHP_GROUP && number == LANHP_NUMBER);
}

bool home_indoors(void) { return D.town && place_here() != HOME_PLACE_NONE; }

const char *home_check(void) {
	return D.town ? home_places_check(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER), bn6_player_x(), bn6_player_y(),
		emu_read8(BN6_PLAYER_FACING) & 7) : NULL;
}

bool home_counter(uint32_t *archive, int *script) {
	return D.town && home_places_counter(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER), bn6_player_x(), bn6_player_y(),
		emu_read8(BN6_PLAYER_FACING) & 7, archive, script);
}

/* In Lan's HP: its open portals on, every frame (BN6's own homepage sets
 * the warp-off flags of the links its story has not opened, after the
 * game enters it), and R asks BN6's "jack out?" (the run's lock had
 * stopped it) */
void home_entered(void) {
	if (!home_in_hp()) return;
	lanhp_lit(home_lit());
	flag_clear(BN6_FLAG_NO_JACK);
}

/* Lan's HP's corridor to its pink pad, BN6's own map: its mouth at x 119,
 * open from y -24 to -4, which a walk straight from the blue pad (-198, 6)
 * meets a few units off its line, at its lip (session 69: sixteen calls
 * pushing there, the arrow pointing at the pad through the wall). L names
 * where the pad lies; the arrow leads onto the corridor's line first,
 * across once MegaMan is near it. */
#define HP_MOUTH_X 119
#define HP_MOUTH_Y (-14)
#define HP_MOUTH_HALF 8      /* the line's half-width the arrow wants */
#define HP_MOUTH_NEAR 64     /* how near in x it turns onto the line */

static const char *hp_way(int *far) {
	int x, y, px = bn6_player_x(), py = bn6_player_y(), f;
	lanhp_portal_spot(0, &x, &y);
	const char *lies = way_to(x, y, far);
	if (px < HP_MOUTH_X && abs(py - HP_MOUTH_Y) > HP_MOUTH_HALF)
		way_to(px < HP_MOUTH_X - HP_MOUTH_NEAR ? HP_MOUTH_X : px, HP_MOUTH_Y, &f);
	return lies;
}

const char *home_way(int *far) {
	int x, y;
	if (home_in_hp()) return hp_way(far);
	if (home_places_way(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER), &x, &y)) return way_to(x, y, far);
	return town_way(far);
}

const char *home_status(void) {
	int far, at = place_here();
	const char *way = home_way(&far);
	if (home_in_hp()) {
		const char *words = home_hp_status(way, !D.port_told, run.clock);
		D.port_told = true;
		return home_jobs_status(words);
	}
	int dx, dy;
	if (at == HOME_PLACE_HOUSE && home_places_door(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER), &dx, &dy))
		return home_jobs_status(home_door_words(port_words(PORT_HOUSE, way), lies_at(dx, dy)));
	if (at == HOME_PLACE_ROOM || at == HOME_PLACE_HOUSE) return home_jobs_status(port_words(at == HOME_PLACE_ROOM ? PORT_ROOM : PORT_HOUSE, way));
	const char *words = port_words(at == HOME_PLACE_NONE ? PORT_HOME : PORT_OUT, way);
	if (at == HOME_PLACE_ASTER) words = home_aster_words(words);
	/* (in the town, once a visit: the requests posted, AsterLand's order) */
	if (at == HOME_PLACE_NONE && !D.errands_told) {
		D.errands_told = true;
		int ax, ay;
		words = home_errands_words(words, run.job.kind == JOB_NONE, home_order_open(), home_places_aster_front(&ax, &ay) ? lies_at(ax, ay) : NULL);
	}
	return home_jobs_status(words);
}

/* home: Lan held while the act's card shows, A ending it early, as on a
 * layer */
static void home_hold(void) {
	bool hold = D.home && D.town_seen && cinema_busy();
	if (hold && cinema_input_mode() == CINEMA_FREE) cinema_input(CINEMA_HOLD);
	else if (!hold && D.arrival_hold && cinema_input_mode() == CINEMA_HOLD) cinema_input(CINEMA_FREE);
	D.arrival_hold = hold;
}

/* MegaMan coming home: the act done, and the portals lit; or a trip back */
static const char *arrival_words(void) {
	if (D.home_back) return home_back_words(run.clock);
	int open = 1;
	bool sealed = false;
	home_ways(&open, &sealed);
	const char *names[LANHP_PORTALS];
	for (int k = 0; k < LANHP_PORTALS; ++k) names[k] = home_portal_name(k);
	return home_words(D.home_beaten, open, names, sealed ? names[2] : NULL);
}

/* the map settled: Lan or MegaMan out, the warp done, the card gone */
static bool settled(void) {
	return D.town_seen && on_map() && emu_read8(BN6_WARP_PENDING) == 0 && D.town_frames > 40 && !cinema_busy();
}

/* Their words (in the town at the run's start, Dad's call the first time;
 * in Lan's HP the act just done, or what it is the run's first time
 * there), once a place */
static void home_talk(void) {
	if (!settled()) return;
	if (home_in_hp()) {
		if (D.home ? D.intro_said : D.hp_said) return;
		if (!talk_start(D.home ? arrival_words() : home_hp_words(profile.hp_taught), FACE_MEGAMAN)) return;
		if (D.home) D.intro_said = true;
		else D.hp_said = true;
		/* (three boxes at every run's first jack-in were one too many for a
		 * returning player, session 69) */
		if (!D.home && !profile.hp_taught) { profile.hp_taught = 1; profile_save(); }
		return;
	}
	if (D.home || D.intro_said || !talk_script(town_info()->talk_archive, town_info()->intro)) return;
	D.intro_said = true;
	if (!profile.seen_intro) { profile.seen_intro = true; profile_save(); }
	/* (Dad's word on the CybeastButton, all of it once: town_intro) */
	if (run_beast_start() && !(profile.beast & BEAST_TAUGHT)) { profile.beast |= BEAST_TAUGHT; profile_save(); }
}

/* home's checkpoint in Lan's HP, once its words are said and MegaMan is
 * free */
static void home_checkpoint(void) {
	if (!D.home || !home_in_hp() || !D.intro_said || D.home_saved || !on_map() || talk_busy() || emu_read8(BN6_CHATBOX) ||
		emu_read8(BN6_DIALOGUE_LOCK) || !flag_get(BN6_FLAG_PLAYER_CAN_MOVE)) return;
	D.home_saved = true;
	home_save();
}

/* MegaMan beside a portal, once a visit each: what he reads through it
 * (its data's feel, a strong Navi's signal); the dark way's sealed
 * (docs/HOME.md) */
static void portal_words(void) {
	if (!home_in_hp() || !settled() || talk_busy() || emu_read8(BN6_CHATBOX)) return;
	int n = 1, k = lanhp_portal_near(bn6_player_x(), bn6_player_y(), PORTAL_REACH);
	bool sealed = false;
	const RunWay *w = home_ways(&n, &sealed);
	if (k < 0 || D.home_told >> k & 1) return;
	/* (an older portal: where it goes back to, and its price) */
	if (home_older(k) >= 0) {
		if (!talk_start(home_back_portal_words(home_older(k), profile.back_taught, run.clock), FACE_MEGAMAN)) return;
		D.home_told |= 1u << k;
		if (!profile.back_taught) { profile.back_taught = 1; profile_save(); }
		return;
	}
	if (k >= n && !(k == 2 && sealed)) return;
	bool shut = k >= n;
	if (talk_start(home_port_words(shut ? 0 : w[k].biome, shut ? 0 : w[k].meets, k == 2, shut), FACE_MEGAMAN)) D.home_told |= 1u << k;
}

/* MegaMan on a portal: its way's first layer built (another way's than the
 * one built at the exit), or an older portal's trip back, while BN6's link
 * plays, and the run locked again */
static void portal_taken(void) {
	if (!home_in_hp() || D.portal_taken || emu_read8(BN6_WARP_PENDING) != 1) return;
	int k = lanhp_portal_of(emu_read8(BN6_WARP_INDEX));
	if (k < 0) return;
	D.portal_taken = true;
	if (home_older(k) >= 0) home_go_back(k);
	else home_take_way(k);
	lock_run();
}

/* At home: nothing to watch but the portals and the jack-out, whose
 * arrival on the layer's map starts the act as a layer's warp does. */
/* The day goes on as the run does: morning as it begins, afternoon after
 * act 1, evening after act 2, night before the Nest (the short net's 3
 * acts; the endless net's 6 two acts an hour, and a new morning as its
 * next cycle begins) */
int home_hour(void) {
	int acts = (run.depth - 1) % CYCLE_LAYERS / 3, last = run.mode == RUN_SHORT ? 3 : 6;
	return acts >= last ? 3 : acts * 3 / last;
}

/* the town's tint as Lan stands on its map, BN6's own indoors lit as ever:
 * through a door's fade too, where the game is off the map (a flash of
 * daylight as Lan stepped into AsterLand, issue #92), but not over the
 * PET's menu */
static void town_tint(void) {
	D.tint = main_mode() == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) != BN6_SUB_PET && map_is(town_info()->group, town_info()->number) ?
		home_hour() : 0;
	D.tint_box = emu_read8(BN6_CHATBOX) != 0;
}

void home_update(void) {
	static bool was_hp;
	map_label();
	arrow_update();
	/* (and the way-on arrow after pushing a while where the pad goes
	 * nowhere, as on a layer: at the HP's corridor lip, session 69) */
	if (on_map()) { unwedge(); push_arrow(); }
	bool hp = home_in_hp();
	if (hp != was_hp) { D.town_frames = 0; D.home_told = 0; }
	if (hp) home_entered();
	home_places_frame(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER));
	town_tint();
	was_hp = hp;
	if (on_map() && home_map(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER))) { D.town_seen = true; ++D.town_frames; }
	talk_update();
	home_hold();
	home_talk();
	home_checkpoint();
	portal_words();
	portal_taken();
	bool arrived = D.town_seen && on_map() && emu_read8(BN6_WARP_PENDING) == 0 &&
		emu_read8(BN6_MAP_GROUP) == D.group && emu_read8(BN6_MAP_NUMBER) == D.number;
	if (!arrived) return;
	D.town = false;
	D.home = false;
	D.portal_taken = false;
	D.frame = 0;
	D.checkpoint = true;
	lock_run();
	mapslot_music_forget_town();
}
