/* looks_screen_text.h. Where a palette comes from is said as the routes give
 * it (issue #854: milestones, the home shop, a rare find), plainly, as
 * KeyItem says what an item is. A colour's name is worked out from its hue,
 * its saturation and its value, so every palette's sixteen have one. */
#include "looks_screen_text.h"

#include <stdio.h>

static const char *const words[LW_COUNT] = {
	[LW_TITLE] = "COLOURS",
	[LW_LOOK] = "Look",
	[LW_PLAIN] = "PLAIN",
	[LW_LOCKED] = "LOCKED",
	[LW_INFORMATION] = "INFORMATION",
	[LW_KEYS] = "L/R",
	[LW_PAGES] = "PALETTE",
	[LW_ALL_PLAIN] = "All plain",
};

const char *looks_word(LooksWord w) { return (unsigned)w < LW_COUNT ? words[w] : ""; }

static const char *const channels[LOOK_CHANNELS] = { "Armour", "Suit", "Accent" };
static const char *const parts[LOOK_CHANNELS] = { "his helmet, arms and legs", "his bodysuit", "his fins and emblem" };

const char *looks_channel_name(int c) { return c >= 0 && c < LOOK_CHANNELS ? channels[c] : ""; }
static const char *part(int c) { return parts[c >= 0 && c < LOOK_CHANNELS ? c : 0]; }

const char *looks_slot_name(int slot) {
	static const char *const names[LOOK_SLOTS] = { "A", "B", "C" };
	return slot >= 0 && slot < LOOK_SLOTS ? names[slot] : "";
}

const char *looks_band_name(int p) {
	static const char *const bands[LOOK_PALETTES] = { "CLASSIC", "UNDERNET", "GREGAR", "PROTOMAN", "NEBULA", "TOURNEY",
		"HAZARD", "CENTRAL", "SEASIDE", "SKY", "MYSTERY", "GLITCH" };
	return p >= 0 && p < LOOK_PALETTES ? bands[p] : "";
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
static const char *origin(int p) { return p >= 0 && p < LOOK_PALETTES ? origins[p] : ""; }

const char *looks_colour_word(uint16_t c) {
	int r = c & 31, g = c >> 5 & 31, b = c >> 10 & 31;
	int mx = r > g ? (r > b ? r : b) : (g > b ? g : b), mn = r < g ? (r < b ? r : b) : (g < b ? g : b), d = mx - mn;
	if (mx < 5) return "black";
	if (d * 100 < mx * 18) return mx > 27 ? "white" : mx > 18 ? "light grey" : mx > 10 ? "grey" : "dark grey";
	/* (the hue in degrees, by its largest channel) */
	int h = mx == r ? (60 * (g - b) / d + 360) % 360 : mx == g ? 60 * (b - r) / d + 120 : 60 * (r - g) / d + 240;
	static const struct { int to; const char *name, *dark, *pale; } hues[] = {
		{ 15, "red", "dark red", "pink" }, { 40, "orange", "brown", "peach" }, { 65, "yellow", "olive", "cream" },
		{ 150, "green", "dark green", "pale green" }, { 195, "teal", "dark teal", "aqua" }, { 250, "blue", "navy", "sky blue" },
		{ 290, "violet", "indigo", "lilac" }, { 335, "magenta", "plum", "pink" }, { 360, "red", "dark red", "pink" },
	};
	int k = 0;
	while (h >= hues[k].to) ++k;
	if (mx < 16) return hues[k].dark;
	if (d * 100 < mx * 45 && mx > 24) return hues[k].pale;
	return hues[k].name;
}

void looks_copy_row(char *out, size_t n, int to) { snprintf(out, n, "Copy to %s", looks_slot_name(to)); }

/* (a channel's colour in words: "Classic 14" or "plain") */
static void colour_of(char *out, size_t n, const Look *l, int c) {
	if (l->palette[c] == LOOK_NONE) snprintf(out, n, "plain");
	else snprintf(out, n, "%s %d", looks_palette_name(l->palette[c]), l->colour[c] + 1);
}

void looks_slot_words(char *out, size_t n, int slot, const Look *look) {
	bool plain = true;
	for (int c = 0; c < LOOK_CHANNELS; ++c) plain &= look->palette[c] == LOOK_NONE;
	if (plain) {
		snprintf(out, n, "Look %s: as he always is.", looks_slot_name(slot));
		return;
	}
	char a[24], b[24], c[24];
	colour_of(a, sizeof a, look, 0);
	colour_of(b, sizeof b, look, 1);
	colour_of(c, sizeof c, look, 2);
	snprintf(out, n, "Look %s: %s, %s, %s.", looks_slot_name(slot), a, b, c);
}

void looks_channel_words(char *out, size_t n, int c, const Look *look) {
	if (look->palette[c] == LOOK_NONE) {
		snprintf(out, n, "%s: %s, as he always is.", looks_channel_name(c), part(c));
		return;
	}
	char col[24];
	colour_of(col, sizeof col, look, c);
	snprintf(out, n, "%s: %s, in %s, %s.", looks_channel_name(c), part(c), col, looks_colour_word(looks_colour(look->palette[c], look->colour[c])));
}

void looks_colour_words(char *out, size_t n, int palette, int colour, bool owned) {
	if (colour < 0) snprintf(out, n, "His own colour, as he always is.");
	else if (owned) snprintf(out, n, "%s %d, %s. %s", looks_palette_name(palette), colour + 1, looks_colour_word(looks_colour(palette, colour)), origin(palette));
	else snprintf(out, n, "%s %d, %s. Locked: %s", looks_palette_name(palette), colour + 1, looks_colour_word(looks_colour(palette, colour)), origin(palette));
}

void looks_locked_words(char *out, size_t n, int palette) { snprintf(out, n, "%s is locked. %s", looks_palette_name(palette), origin(palette)); }

void looks_popup_words(char *out, size_t n, int slot, int row) {
	if (row < LOOK_SLOTS - 1) snprintf(out, n, "Look %s over Look %s.", looks_slot_name(slot), looks_slot_name((slot + 1 + row) % LOOK_SLOTS));
	else snprintf(out, n, "Look %s back to his own colours.", looks_slot_name(slot));
}
