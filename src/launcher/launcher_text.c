/* launcher_text.h. Menu words: short, plain, the ROMs named as a player
 * finds them on the cartridge (BN6 Cybeast Gregar (USA)), the same names
 * Android's and iOS's ROM looks and the browser's page give. */
#include "launcher_text.h"

#include <stdio.h>
#include <string.h>

/* (in LauncherWord's order, one a line, its name beside it) */
static const char *const words[] = {
	"ROMS",   /* W_TITLE */
	"Cyberworld Endless",   /* W_PLACE */
	"Cyberworld",   /* W_PLACE_SHORT */
	"NEEDED",   /* W_NEEDED */
	"OPTIONAL",   /* W_OPTIONAL */
	"READY",   /* W_READY */
	"CHOOSE FILE",   /* W_CHOOSE_FILE */
	"CHOOSE FOLDER",   /* W_CHOOSE_FOLDER */
	"LOOK AGAIN",   /* W_LOOK_AGAIN */
	"WAITING",   /* W_WAITING */
	"PLAY",   /* W_PLAY */
	"DONE",   /* W_DONE */
	"FILES INSTEAD",   /* W_FILES_INSTEAD */
	"BN6 first: the game runs on it.",   /* W_PLAY_LOCKED */
	"Its net joins ours: BN5's areas in runs, their battles in its own engine.",   /* W_FIVE_ADDS */
	"Nothing chosen.",   /* W_CANCELLED */
};
_Static_assert(sizeof words / sizeof *words == W_COUNT, "a word for each LauncherWord");

const char *launcher_word(LauncherWord w) { return (unsigned)w < W_COUNT ? words[w] : ""; }

static const struct { const char *shortname, *caption, *label[3], *tag, *full; } slots[SLOTS] = {
	[SLOT_BN6] = { "BN6", "Cybeast Gregar (USA)", { "GREGAR", "CYBEAST", "GREGAR" }, "BN6 Cybeast Gregar (USA)",
		"Mega Man Battle Network 6: Cybeast Gregar (USA)" },
	[SLOT_BN5] = { "BN5", "Team Colonel (USA)", { "COLONEL", "TEAM", "COLONEL" }, "BN5 Team Colonel (USA)",
		"Mega Man Battle Network 5: Team Colonel (USA)" },
};

static int clamp_slot(int slot) { return slot == SLOT_BN5 ? SLOT_BN5 : SLOT_BN6; }

const char *slot_short(int slot) { return slots[clamp_slot(slot)].shortname; }
const char *slot_caption(int slot) { return slots[clamp_slot(slot)].caption; }
const char *slot_label(int slot, int line, bool large) { return slots[clamp_slot(slot)].label[large ? 1 + (line ? 1 : 0) : 0]; }

/* (pick.h's bits, named here so that this file needs none of it) */
#define KIND_FILE 1
#define KIND_FOLDER 2

void note_slot_empty(int slot, unsigned kind, const char *where, char *out, size_t n) {
	if (slot == SLOT_BN5) {
		if (kind & KIND_FOLDER) snprintf(out, n, "Optional: %s, in the same folder. %s", slots[SLOT_BN5].tag, words[W_FIVE_ADDS]);
		else if (kind & KIND_FILE) snprintf(out, n, "Optional: %s. %s", slots[SLOT_BN5].tag, words[W_FIVE_ADDS]);
		else snprintf(out, n, "Optional: %s, beside BN6. %s", slots[SLOT_BN5].tag, words[W_FIVE_ADDS]);
		return;
	}
	if (kind & KIND_FOLDER)
		snprintf(out, n, "Choose the folder your ROMs are in. Only its .gba files are opened, and only BN6 and BN5 copied in.");
	else if (kind & KIND_FILE)
		snprintf(out, n, "Your own %s, unmodified: choose its .gba file, or drop it on this window.", slots[SLOT_BN6].tag);
	else
		snprintf(out, n, "Put your %s .gba in Downloads, Emulation/roms/gba, retrodeck/roms/gba or %s.", slots[SLOT_BN6].tag, where);
}

void note_slot_ready(int slot, const char *kept, char *out, size_t n) {
	if (slot == SLOT_BN5) snprintf(out, n, "%s, checked by its SHA-1. %s", slots[SLOT_BN5].tag, words[W_FIVE_ADDS]);
	else if (kept && *kept) snprintf(out, n, "%s, checked by its SHA-1. A copy is kept in %s.", slots[SLOT_BN6].tag, kept);
	else snprintf(out, n, "%s, checked by its SHA-1.", slots[SLOT_BN6].tag);
}

void note_play(bool bn5, bool from_title, const char *title_button, char *out, size_t n) {
	if (from_title) snprintf(out, n, "Back to the title%s.", bn5 ? ", BN5's net in" : "");
	else if (bn5) snprintf(out, n, "Play with BN6 and BN5.");
	else snprintf(out, n, "Play with BN6. BN5 can come later: %s on the title brings this screen back.", title_button);
}

void note_alt(unsigned kind, char *out, size_t n) {
	if (kind & KIND_FOLDER)
		snprintf(out, n, "Choose the ROM files themselves, both at once: for ROMs the folder chooser cannot reach.");
	else
		snprintf(out, n, "Looks again in Downloads, EmuDeck's and RetroDECK's folders and the game's own rom folder.");
}

void note_took(int slot, const char *file, bool five_beside, char *out, size_t n) {
	if (five_beside) snprintf(out, n, "Took %s from %s, and BN5 beside it.", slots[clamp_slot(slot)].tag, file);
	else snprintf(out, n, "Took %s from %s.", slots[clamp_slot(slot)].tag, file);
}

void note_refused(const char *file, const char *why, char *out, size_t n) { snprintf(out, n, "Not taken: %s: %s.", file, why); }

/* Battle Network 6's and 5's other versions, by their header's game code
 * (android/.../RomActivity.java's and src/core/ios.m's) */
static const struct { const char *code, *why; } others[] = {
	{ "BR6E", "BN6 Cybeast Falzar, not Gregar" },
	{ "BR6P", "BN6 Cybeast Falzar (Europe), not Gregar (USA)" },
	{ "BR6J", "Rockman EXE 6 Falzar (Japan), not BN6 Gregar (USA)" },
	{ "BR5P", "BN6 Cybeast Gregar (Europe), not the USA version" },
	{ "BR5J", "Rockman EXE 6 Gregar (Japan), not the USA version" },
	{ "BRBE", "BN5 Team ProtoMan, not Team Colonel" },
	{ "BRBP", "BN5 Team ProtoMan (Europe), not Team Colonel (USA)" },
	{ "BRBJ", "Rockman EXE 5 Team of Blues (Japan), not BN5 Team Colonel (USA)" },
	{ "BRKP", "BN5 Team Colonel (Europe), not the USA version" },
	{ "BRKJ", "Rockman EXE 5 Team of Colonel (Japan), not the USA version" },
};

const char *refuse_why(const char code[4], int slot_of_code, bool changed) {
	static char s[96];
	if (changed && slot_of_code >= 0) {
		snprintf(s, sizeof s, "%s, but changed: patched, trimmed or a bad dump", slots[clamp_slot(slot_of_code)].tag);
		return s;
	}
	for (size_t i = 0; i < sizeof others / sizeof *others; ++i)
		if (!memcmp(code, others[i].code, 4)) return others[i].why;
	return "not BN6 Gregar or BN5 Team Colonel";
}

const char *refuse_not_rom(bool zipped) { return zipped ? "zipped: unzip it first" : "not a GBA ROM"; }

void note_found(const char *where, char *out, size_t n) {
	snprintf(out, n, "Found %s in %s, checked by its SHA-1.", slots[SLOT_BN6].tag, where);
}

void note_five_gone(char *out, size_t n) {
	snprintf(out, n, "%s is gone from where it was: its areas stay out of runs until it is back.", slots[SLOT_BN5].tag);
}

void note_no_picker(const char *where, char *out, size_t n) {
	snprintf(out, n, "No file chooser here: put the .gba in Downloads or %s, or drop it on this window.", where);
}

void saves_line_desktop(const char *where, char *out, size_t n) { snprintf(out, n, "Saves: %s", where); }

void saves_line_phone(const char *folder, bool refused, char *out, size_t n) {
	char f[20];
	/* (a long name cut, so that the line stays one) */
	if (folder && strlen(folder) > 14) snprintf(f, sizeof f, "%.12s...", folder);
	else snprintf(f, sizeof f, "%s", folder ? folder : "");
	if (!f[0]) snprintf(out, n, "Saves: on this device only\nChoose a folder to keep a copy");
	else if (refused) snprintf(out, n, "Saves: on this device only\nNo copy in %s: choose it again", f);
	else snprintf(out, n, "Saves: on this device\nA copy in %s, for a reinstall", f);
}

const char *quit_words(bool pad) {
#ifdef __ANDROID__
	return pad ? "Hold SELECT+START again to quit" : "Press Back again to quit";
#else
	return pad ? "Hold SELECT+START again to quit" : "Press Esc again to quit";
#endif
}

void hint_words(const char *a, const char *start, const char *b, bool from_title, bool brief, char *out, size_t n) {
	if (brief) snprintf(out, n, "%s: %s", start, from_title ? "done" : "play");
	else if (from_title) snprintf(out, n, "%s: choose  %s: done  %s: back", a, start, b);
	else snprintf(out, n, "%s: choose  %s: play", a, start);
}
