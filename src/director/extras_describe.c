/* The run's extras in a state's words (director_describe;
 * docs/devtools/scenarios.md, The extras): each part's own lines, called
 * here so that the state's file names them in one, and the side layer's,
 * which no part owns. They only read. */
#include "director_layer.h"

#include <stdio.h>

#include "boss.h"
#include "director_battler.h"
#include "director_dark.h"
#include "director_state.h"
#include "glitch.h"
#include "net.h"
#include "run.h"

/* The side layer MegaMan is on and the ways off the run's own this layer
 * holds, in a state's words (director_describe; tests/scenarios' extras):
 * `sidelayer none`, undernet, secret, back or glitch; the Undernet's dark
 * warp and the golden gate, here or none */
static void side_describe(FILE *f) {
	static const char *const sides[] = { "none", "undernet", "secret", "back", "glitch" };
	bool warp = false, gate = false;
	for (int i = 0; i < layer.nobj; ++i) {
		warp |= layer.obj[i].type == OBJ_UNDERNET;
		gate |= layer.obj[i].type == OBJ_SECRET_GATE;
	}
	fprintf(f, "sidelayer %s\ndarkwarp %s\ngoldgate %s\n", run.side_kind >= 0 && run.side_kind <= LAYER_GLITCH ? sides[run.side_kind] : "?",
		warp ? "here" : "none", gate ? "here" : "none");
}

void extras_describe(FILE *f) {
	side_describe(f);
	glitch_describe(f);
	boss_bracket_describe(f);
	battler_describe(f);
	dark_describe(f);
}
