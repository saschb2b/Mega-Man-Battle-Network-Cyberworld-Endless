/* The looks' palettes' names, as the look screen shows them: each named for
 * where it is earned (docs/game/looks.md), in LookPalette's order. */
#include "looks.h"

static const char *const names[LOOK_PALETTES] = {
	"Classic", "Undernet", "Gregar", "ProtoMan", "Nebula", "Tournament",
	"Hazard", "Central Town", "Seaside", "Sky", "Mystery", "Glitch",
};

const char *looks_palette_name(int p) { return p >= 0 && p < LOOK_PALETTES ? names[p] : ""; }
