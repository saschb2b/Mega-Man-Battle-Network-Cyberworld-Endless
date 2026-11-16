/* The screen a player is on, in a word: a headless run's state says it, so
 * an agent reads where a batch of input left the game without a picture
 * (tools/cw.py, docs/devtools/agents-tools.md). */
#include "screen_name.h"

#include "devtools.h"
#include "director.h"
#include "game.h"
#include "jukebox.h"
#include "library.h"
#include "netpass_screen.h"
#include "options.h"
#include "saves.h"
#include "second.h"

const char *screen_name(void) {
	if (devtools_open()) return "devmenu";
	const char *o = options_screen();
	if (o) return o;
	/* (SAVES and the ROMs screen are scenes of their own: their names) */
	if (scene_current() != &scene_emu) return scene_current() ? scene_current()->name : "none";
	if (director_pick_shown()) return "reward-pick";
	if (jukebox_shown()) return "jukebox";
	if (library_shown()) return "library";
	if (netpass_screen_shown()) return "netpass";
	return second_screen_word();
}

/* (Options and what shows over it, SAVES, the ROMs: each its own lines
 * where it is up, docs/devtools/scenarios.md, The screens' lines) */
void screens_describe(FILE *f) {
	options_describe(f);
	saves_describe(f);
	launcher_describe(f);
}
