/* What a player sees, in words, one fact a line (director_describe): the
 * state play.py prints after each batch. What a developer reproducing a
 * playtest needs beside it (CYBERWORLD_STATE_POS: where he stands, the layer
 * drawn in letters) is director_describe_pos.c's. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "boss.h"
#include "cinema.h"
#include "director.h"
#include "director_crew.h"
#include "director_describe_pos.h"
#include "director_folder.h"
#include "director_hold.h"
#include "director_home.h"
#include "director_layer.h"
#include "director_state.h"
#include "liberation.h"
#include "meta.h"
#include "guardians.h"
#include "older_describe.h"
#include "pet.h"
#include "player_body.h"
#include "save.h"
#include "save_mask.h"
#include "talk.h"

/* What the game is doing, for a state, and MegaMan's HP there: in a
 * battle the HUD's is his battle object's (in a Virus Battler's, the
 * first virus's: his own, as it stays) */
void director_megaman_hp(int *hp, int *max) {
	*hp = emu_read16(BN6_NAVI_HP);
	*max = emu_read16(BN6_NAVI_MAX_HP);
	if (on_map() || D.battler) return;
	for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		if ((emu_read8(o) & 1) && emu_read8(o + BN6_T1_ALLIANCE) == 0) { *hp = emu_read16(o + BN6_T1_HP); *max = emu_read16(o + BN6_T1_MAX_HP); break; }
	}
}

static const char *state_doing(int *hp, int *max) {
	director_megaman_hp(hp, max);
	int mode = main_mode(), sub = emu_read8(BN6_GAMESTATE);
	return mode == BN6_MODE_GAME_OVER ? "gameover"
		: mode != BN6_MODE_GAME ? "menu"
		: sub == BN6_SUB_MAP ? "map" : sub == BN6_SUB_BATTLE || sub == BN6_SUB_BATTLE_INIT ? "battle" : "other";
}

/* In a battle, the panel MegaMan stands on, from the left and the top: a
 * player sees it at a glance, a playtester reading stills misread it turn
 * after turn, as BN6 draws him half a row above his panel. */
static void battle_describe(FILE *f) {
	if (on_map()) return;
	/* (a Virus Battler's: BN6's VIRUS LIST, then the viruses on their own) */
	if (D.battler) {
		fprintf(f, "virus battle phase %02x placed %d\n", emu_read8(BN6_BATTLE_PHASE), emu_read8(BN6_VB_LIST_PLACED));
		return;
	}
	for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		if ((emu_read8(o) & 1) && emu_read8(o + BN6_T1_ALLIANCE) == 0 && emu_read8(o + BN6_T1_PANEL_X)) {
			fprintf(f, "megaman stands column %d row %d\n", emu_read8(o + BN6_T1_PANEL_X), emu_read8(o + BN6_T1_PANEL_Y));
			return;
		}
	}
}

/* A battle's phase in a word: `won` from its RESULT on, which a quit
 * keeps, and under way before it; `start` before the Custom screen first
 * opens, and again as the battle fades out, BN6's phase gone back
 * (tools/scenarios_fuzz.py remembers a `won`) */
static const char *battle_word(void) {
	int phase = emu_read8(BN6_BATTLE_PHASE);
	return phase < BN6_PHASE_CUSTOM ? "start" : phase == BN6_PHASE_CUSTOM ? "custom" : phase < BN6_PHASE_RESULTS ? "fight" :
		emu_read8(BN6_BATTLE_RESULT) == BN6_RESULT_WON ? "won" : "over";
}

/* the turns the NaviCust adds to a Soul's three, where a program gives
 * any: SoulT+1 (docs/rom/navicust.md, Program 48) */
static void describe_turns(FILE *f) {
	int turns = (int8_t)emu_read8(BN6_NAVI_STATS + BN6_NAVI_TURN_EXTRA);
	if (turns) fprintf(f, "soulturns %+d\n", turns);
}

/* The list-6 sprite the player's spawn is given: his own, or a
 * liberation teammate's in play (issue #696) */
static void describe_body(FILE *f) {
	if (player_body_now() < 0) fprintf(f, "body own\n");
	else fprintf(f, "body %d\n", player_body_now());
}

/* Lan's place in the town; on a layer its map over the picture: shown,
 * or opened and hidden by a chat (issue #840) */
static void describe_place(FILE *f) {
	if (D.town) fprintf(f, "place %s\n", director_place_name());
	else fprintf(f, "map %s\n", director_map_over() ? "open" : D.map_open ? "hidden" : "closed");
}

void director_describe(FILE *f) {
	/* (off the run's maps, the Net Square's visit: where MegaMan stands, for a tour's walk) */
	if (!D.active) { fprintf(f, "where none\npos %d %d %d\n", bn6_player_x(), bn6_player_y(), bn6_player_z()); return; }
	int sub = emu_read8(BN6_GAMESTATE), hp, max;
	const char *doing = state_doing(&hp, &max);
	fprintf(f, "where %s\ndoing %s\nchat %s\ntalk %s\n", D.town ? "town" : "layer", doing,
		emu_read8(BN6_CHATBOX) ? "open" : "closed", talk_busy() ? "director" : "none");
	/* (what of BN6 holds him, issue #23: none where Lan or MegaMan stands free) */
	fprintf(f, "held %s\n", held_names(bn6_held()));
	describe_body(f);
	/* (the game's own count of its main loop's frames: one that stands while
	 * the engine's frames go on is a game standing still, issue #482;
	 * tools/old_checkpoint.py watches it) */
	fprintf(f, "gameframe %u\n", emu_read16(BN6_FRAME_COUNT));
	describe_place(f);
	/* (GameState's protected zenny, then its BugFrags) */
	fprintf(f, "hp %d/%d\nzenny %u\nbugfrags %u\n", hp, max, (unsigned)emu_read32(BN6_ZENNY), (unsigned)emu_read32(BN6_BUGFRAGS));
	save_mask_describe(f);   /* (BN6's own save's mask: issue #734) */
	/* (a battle's gauge, its turn and the rules it runs by, a liberation's
	 * or BN6's own: issue #669 left a liberation's on the next run's
	 * battles; tests/scenarios reads them) */
	if (sub == BN6_SUB_BATTLE) {
		fprintf(f, "custom gauge %d%%\n", emu_read16(BN6_CUSTOM_GAUGE) * 100 / 0x4000);
		lib_battle_describe(f);
		fprintf(f, "battlestep %s\n", battle_word());
	}
	director_quit_describe(f);
	looks_describe(f);
	describe_turns(f);
	battle_describe(f);
	battle_state_describe(f);
	/* (for the developer reproducing a playtest: where Lan or MegaMan is) */
	if (getenv("CYBERWORLD_STATE_POS")) director_describe_pos(f);
	/* (a special route kept for its fork, and the act it led to) */
	if (run.route_found) fprintf(f, "route kept from act %d\n", run.route_found);
	if (run.route_way) fprintf(f, "route taken to act %d\n", run.route_way);
	/* (a chat's question, a reward pick's rows, what MegaMan holds, BN6's shop screen's row: tests/scenarios) */
	talk_describe(f);
	director_pick_describe(f);
	holds_describe(f);
	shop_describe(f);
	/* (the PET's and home's own lines, each feature's: docs/devtools/scenarios.md) */
	pet_describe(f);
	home_describe(f);
	older_describe(f);   /* (the older nets' lines: older_describe.h) */
	director_crew_describe(f);   /* (a dive's pull-back and its crew's last battle: director_crew.h) */
	/* (what the run began with, as the JACK-IN SETUP says its choice) */
	meta_setup_describe(f, run.mode, run.folder, run.cross, run.threat, run.helpers, run.extras_off);
	if (D.town) return;
	fprintf(f, "layer %d\narea %s\nscrtdata %d\n", run.depth, guardian_area_in_text(run.biome, run.side_kind), run.fragments);
	layer_describe(f);
	/* (the run's tally, the card that shows, the rival, the guardian's course: each its own part's) */
	tally_describe(f);
	cinema_describe(f);
	duel_describe(f);
	boss_describe(f);
	/* (the extras: the side layer and the ways off, the Glitch layer, the tournament's bracket, the Virus Battler, DarkChips) */
	extras_describe(f);
	/* (a trip back, and the Net's clock: docs/world/home/) */
	if (run.home_depth) fprintf(f, "back from layer %d\n", run.home_depth);
	if (run.clock) fprintf(f, "clock %d\n", run.clock);
	/* (a dark layer's board, its lines its own: liberation_describe.c) */
	lib_describe(f);
	/* (a dark layer's sealed region's route, found: docs/game/liberation/special-routes.md) */
	if (lib_route_found()) fprintf(f, "route found\n");
	/* (named as the game shows him: a playtester reads this; a dev's
	 * state, CYBERWORLD_STATE_POS, names him always, as the scripts that
	 * find a guardian's seed read it) */
	if (D.objs.guardian.navi)
		fprintf(f, "guardian %s %s\n",
			guardian_met(D.objs.guardian.navi) || guardian_heard() || boss_cinematic() || boss_fighting() || boss_beaten() || boss_done() ||
					getenv("CYBERWORLD_STATE_POS")
				? guardian(D.objs.guardian.navi)->name : "???",
			boss_done() ? "done" : boss_beaten() ? "beaten" : boss_fighting() ? "fighting" : "waiting");
}
