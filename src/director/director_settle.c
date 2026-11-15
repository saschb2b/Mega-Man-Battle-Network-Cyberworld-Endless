/* A quit on a won battle's way back to the map. BN6 holds MegaMan from
 * the battle's RESULT through the fade to the map's entry, and the quit's
 * save waits for him free (director_hold.c, issue #23): a playtester's
 * quit in that fade went back to the layer's start, a won battle, its
 * 100z and the map walked since lost (session 76). Now the game is run on
 * to him free first (scene_emu.c's emu_keep_run), A paging the RESULT as
 * the player would and the director's talks held back, and the run is
 * kept where the battle left him (docs/engine/saves.md). A battle under
 * way is not run on, nor a guardian's, which the arena's door keeps. */
#include "director_state.h"

#include "bn6.h"
#include "bn6_battler.h"
#include "boss.h"
#include "director.h"
#include "director_hold.h"
#include "emu.h"
#include "talk.h"

static bool won_back;   /* a won battle's RESULT reached, MegaMan not yet free on the map after it */

/* each frame: set at a won battle's results, let go once MegaMan is free
 * on the map after it, or as a battle's Custom screen or fight is on */
void director_settle_watch(void) {
	if (!D.active || D.town || D.battler) {
		won_back = false;
		return;
	}
	if (main_mode() != BN6_MODE_GAME) return;
	int sub = emu_read8(BN6_GAMESTATE), phase = emu_read8(BN6_BATTLE_PHASE);
	if (sub == BN6_SUB_BATTLE && phase == BN6_PHASE_RESULTS && emu_read8(BN6_BATTLE_RESULT) == BN6_RESULT_WON) won_back = true;
	else if (sub == BN6_SUB_BATTLE && (phase == BN6_PHASE_CUSTOM || phase == BN6_PHASE_FIGHT)) won_back = false;
	else if (sub == BN6_SUB_MAP && hold_save_ok()) won_back = false;
}

bool director_settles(void) {
	return won_back && D.active && !D.town && !D.gameover && !D.warping && boss_idle() && !D.challenge && !D.battler_series &&
		!talk_busy() && !director_pick_shown();
}

/* A on the RESULT's pages, a press every eight frames; none after it, on
 * the way to the map, where an A could talk to whoever MegaMan faces */
uint32_t director_settle_keys(int frame) {
	bool result = main_mode() == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) == BN6_SUB_BATTLE &&
		emu_read8(BN6_BATTLE_PHASE) == BN6_PHASE_RESULTS;
	return result && frame % 8 < 4 ? KEY_A : 0;
}

/* (a talk due after the battle waits: begun while the game runs on, its
 * box would hold MegaMan, and no state is saved under one) */
void director_settling(bool on) { talk_refuse(on); }

bool director_quit_keeps(void) { return director_can_suspend() || director_settles(); }
