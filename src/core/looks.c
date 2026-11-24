/* looks.h. STUB until looks-core's lands: Classic owned, the slots kept in
 * memory alone, made-up colours. */
#include "looks.h"

static const char *const names[LOOK_PALETTES] = { "Classic", "Undernet", "Gregar", "ProtoMan", "Nebula", "Tournament", "Hazard", "Central", "Seaside", "Sky", "Mystery", "Glitch" };
static Look slots[LOOK_SLOTS] = { { { LOOK_NONE, LOOK_NONE, LOOK_NONE }, { LOOK_NONE, LOOK_NONE, LOOK_NONE } },
	{ { LOOK_NONE, LOOK_NONE, LOOK_NONE }, { LOOK_NONE, LOOK_NONE, LOOK_NONE } },
	{ { LOOK_NONE, LOOK_NONE, LOOK_NONE }, { LOOK_NONE, LOOK_NONE, LOOK_NONE } } };
static int worn;
static uint16_t owned = 1u << LOOK_CLASSIC;

const char *looks_palette_name(int p) { return p >= 0 && p < LOOK_PALETTES ? names[p] : "?"; }

uint16_t looks_colour(int p, int i) {
	/* (made up: a hue a column, a value a row, turned by the palette) */
	int h = (i % 4 * 6 + p * 5) % 24, v = 31 - i / 4 * 6;
	int r = h < 8 ? 8 - h : h >= 16 ? h - 16 : 0, g = h < 16 ? 8 - (h > 8 ? h - 8 : 8 - h) : 0, b = h >= 8 ? 8 - (h > 16 ? h - 16 : 16 - h) : 0;
	r = r * v / 8; g = g * v / 8; b = b * v / 8;
	return (uint16_t)(r | g << 5 | b << 10);
}

bool looks_owned(int p) { return p >= 0 && p < LOOK_PALETTES && (owned >> p & 1); }

bool looks_grant(int p) {
	if (p < 0 || p >= LOOK_PALETTES || looks_owned(p)) return false;
	owned |= (uint16_t)(1u << p);
	return true;
}

int looks_slot(void) { return worn; }
void looks_select(int slot) { if (slot >= 0 && slot < LOOK_SLOTS) worn = slot; }
Look looks_get(int slot) { return slots[slot >= 0 && slot < LOOK_SLOTS ? slot : 0]; }
void looks_set(int slot, const Look *look) { if (slot >= 0 && slot < LOOK_SLOTS && look_valid(look)) slots[slot] = *look; }
Look looks_worn(void) { return slots[worn]; }

bool look_valid(const Look *look) {
	for (int c = 0; c < LOOK_CHANNELS; ++c)
		if (look->palette[c] == LOOK_NONE ? look->colour[c] != LOOK_NONE : look->palette[c] >= LOOK_PALETTES || look->colour[c] >= LOOK_COLOURS) return false;
	return true;
}

bool looks_channel_live(int c) { return c == 0 || c == 1; }
