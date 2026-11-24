/* The looks' palettes' names, as the look screen shows them: each named for
 * where it is earned (docs/game/looks.md). */
#include "looks.h"

static const char *const names[LOOK_PALETTES] = {
	[LOOK_CLASSIC] = "Classic",
	[LOOK_UNDERNET] = "Undernet",
	[LOOK_GREGAR] = "Gregar",
	[LOOK_PROTOMAN] = "ProtoMan",
	[LOOK_NEBULA] = "Nebula",
	[LOOK_TOURNAMENT] = "Tournament",
	[LOOK_HAZARD] = "Hazard",
	[LOOK_CENTRAL] = "Central Town",
	[LOOK_SEASIDE] = "Seaside",
	[LOOK_SKY] = "Sky",
	[LOOK_MYSTERY] = "Mystery",
	[LOOK_GLITCH] = "Glitch",
};

const char *looks_palette_name(int p) { return p >= 0 && p < LOOK_PALETTES ? names[p] : ""; }
