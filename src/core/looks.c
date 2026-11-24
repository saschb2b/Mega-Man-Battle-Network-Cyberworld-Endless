/* STUB of looks-core's model (issue #854, step 2), so that the Square's
 * peers' looks (step 4) build before it lands on dev: the look worn from
 * CYBERWORLD_LOOK ("p0,p1,p2,c0,c1,c2"), nothing kept. Dropped for
 * looks-core's own when dev brings it in. */
#include "looks.h"

#include <stdio.h>
#include <stdlib.h>

bool look_valid(const Look *look) {
	if (!look) return false;
	for (int c = 0; c < LOOK_CHANNELS; ++c) {
		if (look->palette[c] == LOOK_NONE && look->colour[c] == LOOK_NONE) continue;
		if (look->palette[c] >= LOOK_PALETTES || look->colour[c] >= LOOK_COLOURS) return false;
	}
	return true;
}

Look look_plain(void) {
	Look l;
	for (int c = 0; c < LOOK_CHANNELS; ++c) l.palette[c] = l.colour[c] = LOOK_NONE;
	return l;
}

static Look override;
static bool overridden;

int looks_slot(void) { return 0; }

bool looks_set(int slot, const Look *look) {
	if (slot || !look_valid(look)) return false;
	overridden = true;
	override = *look;
	return true;
}

Look looks_worn(void) {
	if (overridden) return override;
	Look l = look_plain();
	const char *e = getenv("CYBERWORLD_LOOK");
	unsigned v[6];
	if (e && sscanf(e, "%u,%u,%u,%u,%u,%u", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]) == 6) {
		for (int c = 0; c < LOOK_CHANNELS; ++c) { l.palette[c] = (uint8_t)v[c]; l.colour[c] = (uint8_t)v[3 + c]; }
		if (!look_valid(&l)) l = look_plain();
	}
	return l;
}
