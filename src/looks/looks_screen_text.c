/* looks_screen_text.h. Where a palette comes from is said as the routes give
 * it (issue #854: milestones, the home shop, a rare find), plainly, as
 * KeyItem says what an item is. A colour's name is worked out from its hue,
 * its saturation and its value, so every palette's sixteen have one. */
#include "looks_screen_text.h"

#include <stdio.h>

static const char *const words[] = {
	"COLOURS", "Look", "LOOK", "PLAIN", "LOCKED", "INFORMATION", "L/R", "PALETTE", "ALL PLAIN", "SELECT",   /* (LooksWord's order) */
};
_Static_assert(sizeof words / sizeof *words == LW_COUNT, "a word for each LooksWord");

const char *looks_word(LooksWord w) { return (unsigned)w < LW_COUNT ? words[w] : ""; }

static const char *const channels[LOOK_CHANNELS] = { "Armour", "Suit", "Accent" };
static const char *const parts[LOOK_CHANNELS] = { "his helmet,arms and legs", "his bodysuit", "his fins and emblem" };

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
static const char *const origins[LOOK_PALETTES] = {   /* (LookPalette's order) */
	"Every MegaMan has these.",   /* LOOK_CLASSIC */
	"Earned in the Undernet.",   /* LOOK_UNDERNET */
	"Earned when the Nest falls.",   /* LOOK_GREGAR */
	"Earned with Chaud's respect.",   /* LOOK_PROTOMAN */
	"Earned against Nebula's Darkloids.",   /* LOOK_NEBULA */
	"Earned in the tournament.",   /* LOOK_TOURNAMENT */
	"Earned at the highest threat.",   /* LOOK_HAZARD */
	"Sold at AsterLand's counter.",   /* LOOK_CENTRAL */
	"Sold at AsterLand's counter.",   /* LOOK_SEASIDE */
	"Sold at AsterLand's counter.",   /* LOOK_SKY */
	"Found in Mystery Data now and then.",   /* LOOK_MYSTERY */
	"Found on a Glitch layer.",   /* LOOK_GLITCH */
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
	if (mx < 16 || (k == 1 && mx < 23)) return hues[k].dark;   /* (an orange not bright is brown) */
	if (d * 100 < mx * 45 && mx > 24) return hues[k].pale;
	return hues[k].name;
}

void looks_copy_row(char *out, size_t n, int to) { snprintf(out, n, "COPY TO %s", looks_slot_name(to)); }

/* (a channel's colour in words: "Classic 14" or "plain") */
static void colour_of(char *out, size_t n, const Look *l, int c) {
	if (l->palette[c] == LOOK_NONE) snprintf(out, n, "plain");
	else snprintf(out, n, "%s %d", looks_palette_name(l->palette[c]), l->colour[c] + 1);
}

/* (the look's channels in words: those coloured by name and colour, the plain ones together) */
static void look_words(char *out, size_t n, const Look *look) {
	size_t at = 0;
	int plains = 0, plain_of[LOOK_CHANNELS];
	out[0] = 0;
	for (int c = 0; c < LOOK_CHANNELS; ++c) {
		if (look->palette[c] == LOOK_NONE) { plain_of[plains++] = c; continue; }
		char col[24];
		colour_of(col, sizeof col, look, c);
		at += (size_t)snprintf(out + at, at < n ? n - at : 0, "%s%s %s,%s.", at ? " " : "", looks_channel_name(c), col,
			looks_colour_word(looks_colour(look->palette[c], look->colour[c])));
	}
	if (plains == LOOK_CHANNELS) { snprintf(out, n, "As he always is."); return; }
	if (!plains || at >= n) return;
	at += (size_t)snprintf(out + at, n - at, " %s", looks_channel_name(plain_of[0]));
	for (int k = 1; k < plains && at < n; ++k) at += (size_t)snprintf(out + at, n - at, "%s%s", k == plains - 1 ? " and " : ",", looks_channel_name(plain_of[k]));
	if (at < n) snprintf(out + at, n - at, " plain.");
}

void looks_slot_words(char *out, size_t n, int slot, const Look *look) {
	char w[160];
	look_words(w, sizeof w, look);
	snprintf(out, n, "Look %s. %s", looks_slot_name(slot), w);
}

void looks_channel_words(char *out, size_t n, int c, const Look *look) {
	const char *p = part(c);
	char first = (char)(p[0] >= 'a' && p[0] <= 'z' ? p[0] - 32 : p[0]);
	if (look->palette[c] == LOOK_NONE) {
		snprintf(out, n, "%c%s. As he always is.", first, p + 1);
		return;
	}
	char col[24];
	colour_of(col, sizeof col, look, c);
	snprintf(out, n, "%c%s in %s,%s.", first, p + 1, col, looks_colour_word(looks_colour(look->palette[c], look->colour[c])));
}

void looks_colour_words(char *out, size_t n, int palette, int colour, bool owned) {
	if (colour < 0) snprintf(out, n, "His own colour,as he always is.");
	else if (owned) snprintf(out, n, "%s %d,%s. %s", looks_palette_name(palette), colour + 1, looks_colour_word(looks_colour(palette, colour)), origin(palette));
	else snprintf(out, n, "%s %d,%s. Locked. %s", looks_palette_name(palette), colour + 1, looks_colour_word(looks_colour(palette, colour)), origin(palette));
}

void looks_locked_words(char *out, size_t n, int palette) { snprintf(out, n, "%s is locked. %s", looks_palette_name(palette), origin(palette)); }

void looks_popup_words(char *out, size_t n, int slot, int row, const Look *there) {
	char w[160];
	look_words(w, sizeof w, there);
	bool plain = true;
	for (int c = 0; c < LOOK_CHANNELS; ++c) plain &= there->palette[c] == LOOK_NONE;
	const char *to = looks_slot_name((slot + 1 + row) % LOOK_SLOTS);
	if (row < LOOK_SLOTS - 1 && plain) snprintf(out, n, "Look %s is as he always is.", to);
	else if (row < LOOK_SLOTS - 1) snprintf(out, n, "Look %s has %s", to, w);
	else snprintf(out, n, "Look %s back to his own colours.", looks_slot_name(slot));
}

void looks_ask_words(char *out, size_t n, int slot, int row) {
	if (row < LOOK_SLOTS - 1) snprintf(out, n, "Replace Look %s?", looks_slot_name((slot + 1 + row) % LOOK_SLOTS));
	else snprintf(out, n, "Clear Look %s?", looks_slot_name(slot));
}

const char *looks_yes_no(bool yes) { return yes ? "Yes" : "No"; }

void looks_told_words(char *out, size_t n, bool copied, int slot, int from) {
	if (copied) snprintf(out, n, "Look %s is a copy of Look %s. MegaMan wears it.", looks_slot_name(slot), looks_slot_name(from));
	else snprintf(out, n, "Look %s is back to his own colours.", looks_slot_name(slot));
}
