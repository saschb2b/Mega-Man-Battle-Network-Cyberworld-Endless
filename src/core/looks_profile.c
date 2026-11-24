/* The profile's looks (docs/game/looks.md): the palettes it owns, its three
 * looks and the one worn, kept at the profile's end (save.h's `looks`). */
#include "looks.h"

#include <string.h>

#include "save.h"

static bool override_on;
static Look override_look;

static bool slot_ok(int slot) { return slot >= 0 && slot < LOOK_SLOTS; }

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

int looks_slot(void) { return slot_ok(profile.look_slot) ? profile.look_slot : 0; }

void looks_select(int slot) {
	if (!slot_ok(slot)) return;
	profile.look_slot = (uint8_t)slot;
	profile_save();
}

Look looks_get(int slot) {
	if (!slot_ok(slot) || !look_valid(&profile.looks[slot])) return look_plain();
	return profile.looks[slot];
}

bool looks_set(int slot, const Look *look) {
	if (!slot_ok(slot) || !look_valid(look)) return false;
	profile.looks[slot] = *look;
	profile_save();
	return true;
}

Look looks_worn(void) { return override_on ? override_look : looks_get(looks_slot()); }

void looks_set_override(const Look *look) {
	override_on = look && look_valid(look);
	if (override_on) override_look = *look;
}

void looks_profile_loaded(void) {
	if (profile.looks_kept) return;
	for (int s = 0; s < LOOK_SLOTS; ++s) profile.looks[s] = look_plain();
	profile.looks_owned = 1u << LOOK_CLASSIC;
	profile.look_slot = 0;
	profile.looks_kept = 1;
}
