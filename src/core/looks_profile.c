/* The profile's looks (docs/game/looks.md): the palettes it owns, its three
 * looks and the one worn, kept at the profile's end (save.h's `looks`). */
#include "looks.h"

#include <stdlib.h>
#include <string.h>

#include "save.h"

static void (*on_change)(void);
static void (*describer)(FILE *f);

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
	if (on_change) on_change();
}

Look looks_get(int slot) {
	if (!slot_ok(slot) || !look_valid(&profile.looks[slot])) return look_plain();
	return profile.looks[slot];
}

bool looks_set(int slot, const Look *look) {
	if (!slot_ok(slot) || !look_valid(look)) return false;
	profile.looks[slot] = *look;
	profile_save();
	if (on_change && slot == looks_slot()) on_change();
	return true;
}

Look looks_worn(void) { return looks_get(looks_slot()); }

void looks_profile_loaded(void) {
	if (profile.looks_kept) return;
	for (int s = 0; s < LOOK_SLOTS; ++s) profile.looks[s] = look_plain();
	profile.looks_owned = 1u << LOOK_CLASSIC;
	profile.look_slot = 0;
	profile.looks_kept = 1;
}

void looks_on_change(void (*fn)(void)) { on_change = fn; }
void looks_describe_with(void (*fn)(FILE *f)) { describer = fn; }

void looks_describe(FILE *f) {
	Look w = looks_worn();
	fprintf(f, "look");
	for (int c = 0; c < LOOK_CHANNELS; ++c)
		if (w.palette[c] == LOOK_NONE) fprintf(f, "%s-", c ? "," : " ");
		else fprintf(f, "%s%d.%d", c ? "," : " ", w.palette[c], w.colour[c]);
	fprintf(f, " slot %d\n", looks_slot());
	if (describer) describer(f);
}

bool looks_parse(const char *words, Look *out) {
	Look l = look_plain();
	const char *p = words;
	for (int c = 0; c < LOOK_CHANNELS && *p; ++c) {
		if (*p == '-') ++p;
		else {
			char *end;
			long pal = strtol(p, &end, 10);
			if (end == p || *end != '.') return false;
			const char *q = end + 1;
			long col = strtol(q, &end, 10);
			if (end == q || pal < 0 || pal >= LOOK_PALETTES || col < 0 || col >= LOOK_COLOURS) return false;
			l.palette[c] = (uint8_t)pal;
			l.colour[c] = (uint8_t)col;
			p = end;
		}
		if (*p == ',' || *p == '+') ++p;
		else if (*p) return false;
	}
	if (*p) return false;
	*out = l;
	return true;
}
