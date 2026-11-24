/* looks_screen_text.h. Where a palette comes from is said as the routes give it
 * (issue #854: milestones, the home shop, a rare find), plainly, as
 * KeyItem says what an item is. */
#include "looks_screen_text.h"

#include <stdio.h>

#include "looks.h"

static const char *const words[LW_COUNT] = {
	[LW_TITLE] = "COLOURS",
	[LW_LOOK] = "Look",
	[LW_PLAIN] = "PLAIN",
	[LW_LOCKED] = "LOCKED",
	[LW_INFORMATION] = "INFORMATION",
};

const char *looks_word(LooksWord w) { return w >= 0 && w < LW_COUNT ? words[w] : ""; }

static const char *const channels[LOOK_CHANNELS] = { "Armour", "Suit", "Accent" };
static const char *const parts[LOOK_CHANNELS] = { "his helmet, arms and legs", "his bodysuit", "his fins and emblem" };

const char *looks_channel_name(int c) { return c >= 0 && c < LOOK_CHANNELS ? channels[c] : ""; }

const char *looks_slot_name(int slot) {
	static const char *const names[LOOK_SLOTS] = { "A", "B", "C" };
	return slot >= 0 && slot < LOOK_SLOTS ? names[slot] : "";
}

/* where each palette comes from (the routes: issue #854's table) */
static const char *const origins[LOOK_PALETTES] = {
	[LOOK_CLASSIC] = "Every MegaMan has these.",
	[LOOK_UNDERNET] = "Earned in the Undernet.",
	[LOOK_GREGAR] = "Earned when the Nest falls.",
	[LOOK_PROTOMAN] = "Earned with Chaud's respect.",
	[LOOK_NEBULA] = "Earned against Nebula's Darkloids.",
	[LOOK_TOURNAMENT] = "Earned in the tournament.",
	[LOOK_HAZARD] = "Earned at the highest threat.",
	[LOOK_CENTRAL] = "Sold at AsterLand's counter.",
	[LOOK_SEASIDE] = "Sold at AsterLand's counter.",
	[LOOK_SKY] = "Sold at AsterLand's counter.",
	[LOOK_MYSTERY] = "Found in Mystery Data, now and then.",
	[LOOK_GLITCH] = "Found on a Glitch layer.",
};

void looks_slot_words(char *out, size_t n, int slot) {
	snprintf(out, n, "MegaMan wears Look %s. Three looks are kept.", looks_slot_name(slot));
}

void looks_channel_words(char *out, size_t n, int c, const char *palette, int colour) {
	if (!palette) snprintf(out, n, "%s: %s, as he always is.", looks_channel_name(c), parts[c >= 0 && c < LOOK_CHANNELS ? c : 0]);
	else snprintf(out, n, "%s: %s, in %s %d.", looks_channel_name(c), parts[c >= 0 && c < LOOK_CHANNELS ? c : 0], palette, colour + 1);
}

void looks_colour_words(char *out, size_t n, int palette, const char *name, int colour) {
	if (colour < 0) snprintf(out, n, "His own colour, as he always is.");
	else snprintf(out, n, "%s %d. %s", name, colour + 1, palette >= 0 && palette < LOOK_PALETTES ? origins[palette] : "");
}

void looks_locked_words(char *out, size_t n, int palette, const char *name) {
	snprintf(out, n, "%s is locked. %s", name, palette >= 0 && palette < LOOK_PALETTES ? origins[palette] : "");
}
