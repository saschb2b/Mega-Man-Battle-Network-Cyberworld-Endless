/* A stand-in for looks-core's model (src/core/looks.c, looks_profile.c,
 * looks_text.c; issue #854) until it is on dev: the palettes owned and
 * their names alone, which the routes that earn them need. Removed when
 * dev brings the real one in. */
#include "looks.h"

#include "save.h"

static const char *const names[LOOK_PALETTES] = {
	"Classic", "Undernet", "Gregar", "ProtoMan", "Nebula", "Tournament", "Hazard", "Central Town", "Seaside", "Sky", "Mystery", "Glitch",
};

const char *looks_palette_name(int p) { return p >= 0 && p < LOOK_PALETTES ? names[p] : ""; }

bool looks_owned(int p) {
	if (p == LOOK_CLASSIC) return true;
	return p > 0 && p < LOOK_PALETTES && (profile.looks_owned >> p & 1);
}

bool looks_grant(int p) {
	if (p < 0 || p >= LOOK_PALETTES || looks_owned(p)) return false;
	profile.looks_owned |= (uint16_t)(1u << p);
	profile_save();
	return true;
}
