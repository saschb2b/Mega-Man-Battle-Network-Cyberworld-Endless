/* The screen a player is on, in a word: a headless run's state says it, so
 * an agent reads where a batch of input left the game without a picture
 * (tools/cw.py, docs/devtools/agents-tools.md). */
#include "screen_name.h"

#include "devtools.h"
#include "game.h"
#include "jukebox.h"
#include "netpass_screen.h"
#include "options.h"
#include "second.h"

const char *screen_name(void) {
	if (devtools_open()) return "devmenu";
	const char *o = options_screen();
	if (o) return o;
	/* (SAVES and the ROMs screen are scenes of their own: their names) */
	if (scene_current() != &scene_emu) return scene_current() ? scene_current()->name : "none";
	if (jukebox_shown()) return "jukebox";
	if (netpass_screen_shown()) return "netpass";
	return second_screen_word();
}
