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
	"Nothing chosen.",   /* W_CANCELLED */
};
_Static_assert(sizeof words / sizeof *words == W_COUNT, "a word for each LauncherWord");

const char *launcher_word(LauncherWord w) { return (unsigned)w < W_COUNT ? words[w] : ""; }

/* each cartridge's names (`named` as a refusal says it), and what an
 * extra one's game lends a run: in a line of its own (`adds`), in a word
 * (`lends`, "BN5's net in") and while it is gone (`stays`) */
static const struct { const char *shortname, *caption, *brief, *label[3], *tag, *named, *adds, *lends, *stays; } slots[SLOTS] = {
	[SLOT_BN6] = { "BN6", "Cybeast Gregar (USA)", "Gregar (USA)", { "GREGAR", "CYBEAST", "GREGAR" }, "BN6 Cybeast Gregar (USA)",
		"BN6 Gregar", "", "", "" },
	[SLOT_BN5] = { "BN5", "Team Colonel (USA)", "Colonel (USA)", { "COLONEL", "TEAM", "COLONEL" }, "BN5 Team Colonel (USA)",
		"BN5 Team Colonel", "Its net joins ours: BN5's areas, viruses, Navis and chips in runs.", "net", "its areas stay" },
	[SLOT_BN4] = { "BN4", "Red Sun (USA)", "Red Sun (USA)", { "RED SUN", "RED", "SUN" }, "BN4 Red Sun (USA)",
		"BN4 Red Sun", "Its viruses join ours: BN4's families in runs' battles.", "viruses", "its viruses stay" },
};

static int clamp_slot(int slot) { return slot >= 0 && slot < SLOTS ? slot : SLOT_BN6; }

const char *slot_short(int slot) { return slots[clamp_slot(slot)].shortname; }
const char *slot_caption(int slot, bool brief) { return brief ? slots[clamp_slot(slot)].brief : slots[clamp_slot(slot)].caption; }
const char *slot_label(int slot, int line, bool large) { return slots[clamp_slot(slot)].label[large ? 1 + (line ? 1 : 0) : 0]; }

/* The cartridges in `bits` by their short names: "BN5", "BN5 and BN4",
 * "BN6, BN5 and BN4"; or (`lends`) by what each lends: "BN5's net and
 * BN4's viruses" */
static void list_of(unsigned bits, bool lends, char *out, size_t n) {
	int count = 0, done = 0;
	for (int s = 0; s < SLOTS; ++s) count += (bits >> s) & 1;
	if (n) out[0] = 0;
	for (int s = 0; s < SLOTS; ++s) {
		if (!((bits >> s) & 1)) continue;
		size_t m = strlen(out);
		const char *sep = !done ? "" : done == count - 1 ? " and " : ", ";
		if (lends) snprintf(out + m, n - m, "%s%s's %s", sep, slots[s].shortname, slots[s].lends);
		else snprintf(out + m, n - m, "%s%s", sep, slots[s].shortname);
		++done;
	}
}

/* (pick.h's bits, named here so that this file needs none of it) */
#define KIND_FILE 1
#define KIND_FOLDER 2

void note_slot_empty(int slot, unsigned kind, const char *where, unsigned shown, char *out, size_t n) {
	if (slot != SLOT_BN6) {
		const char *tag = slots[clamp_slot(slot)].tag, *adds = slots[clamp_slot(slot)].adds;
		if (kind & KIND_FOLDER) snprintf(out, n, "Optional: %s, in the same folder. %s", tag, adds);
		else if (kind & KIND_FILE) snprintf(out, n, "Optional: %s. %s", tag, adds);
		else snprintf(out, n, "Optional: %s, beside BN6. %s", tag, adds);
		return;
	}
	char all[64];
	list_of(shown, false, all, sizeof all);
	if (kind & KIND_FOLDER)
		snprintf(out, n, "Choose the folder your ROMs are in. Only its .gba files are opened, and only %s copied in.", all);
	else if (kind & KIND_FILE)
		snprintf(out, n, "Your own %s, unmodified: choose its .gba file, or drop it on this window.", slots[SLOT_BN6].tag);
	else
		snprintf(out, n, "Put your %s .gba in Downloads, Emulation/roms/gba, retrodeck/roms/gba or %s.", slots[SLOT_BN6].tag, where);
}

void note_slot_ready(int slot, const char *kept, char *out, size_t n) {
	if (slot != SLOT_BN6) snprintf(out, n, "%s, checked by its SHA-1. %s", slots[clamp_slot(slot)].tag, slots[clamp_slot(slot)].adds);
	else if (kept && *kept) snprintf(out, n, "%s, checked by its SHA-1. A copy is kept in %s.", slots[SLOT_BN6].tag, kept);
	else snprintf(out, n, "%s, checked by its SHA-1.", slots[SLOT_BN6].tag);
}

void note_play(unsigned in, unsigned missing, bool from_title, const char *title_button, char *out, size_t n) {
	char with[64], later[64], lends[96];
	list_of(in | 1u << SLOT_BN6, false, with, sizeof with);
	list_of(missing, false, later, sizeof later);
	list_of(in, true, lends, sizeof lends);
	if (from_title && in) snprintf(out, n, "Back to the title, %s in.", lends);
	else if (from_title) snprintf(out, n, "Back to the title.");
	else if (!missing) snprintf(out, n, "Play with %s.", with);
	else snprintf(out, n, "Play with %s. %s can come later: %s on the title brings this screen back.", with, later, title_button);
}

void note_alt(unsigned kind, unsigned shown, char *out, size_t n) {
	int count = 0;
	for (int s = 0; s < SLOTS; ++s) count += (shown >> s) & 1;
	if (kind & KIND_FOLDER)
		snprintf(out, n, "Choose the ROM files themselves, %s at once: for ROMs the folder chooser cannot reach.", count > 2 ? "all" : "both");
	else
		snprintf(out, n, "Looks again in Downloads, EmuDeck's and RetroDECK's folders and the game's own rom folder.");
}

void note_took(int slot, const char *file, unsigned beside, char *out, size_t n) {
	char with[64];
	list_of(beside, false, with, sizeof with);
	if (beside) snprintf(out, n, "Took %s from %s, and %s beside it.", slots[clamp_slot(slot)].tag, file, with);
	else snprintf(out, n, "Took %s from %s.", slots[clamp_slot(slot)].tag, file);
}

void note_refused(const char *file, const char *why, char *out, size_t n) { snprintf(out, n, "Not taken: %s: %s.", file, why); }

/* The other versions of the games on the cartridges, by their header's
 * game code (android/.../RomLook.java's and src/core/ios.m's), each named
 * where its cartridge is shown */
static const struct { int slot; const char *code, *why; } others[] = {
	{ SLOT_BN6, "BR6E", "BN6 Cybeast Falzar, not Gregar" },
	{ SLOT_BN6, "BR6P", "BN6 Cybeast Falzar (Europe), not Gregar (USA)" },
	{ SLOT_BN6, "BR6J", "Rockman EXE 6 Falzar (Japan), not BN6 Gregar (USA)" },
	{ SLOT_BN6, "BR5P", "BN6 Cybeast Gregar (Europe), not the USA version" },
	{ SLOT_BN6, "BR5J", "Rockman EXE 6 Gregar (Japan), not the USA version" },
	{ SLOT_BN5, "BRBE", "BN5 Team ProtoMan, not Team Colonel" },
	{ SLOT_BN5, "BRBP", "BN5 Team ProtoMan (Europe), not Team Colonel (USA)" },
	{ SLOT_BN5, "BRBJ", "Rockman EXE 5 Team of Blues (Japan), not BN5 Team Colonel (USA)" },
	{ SLOT_BN5, "BRKP", "BN5 Team Colonel (Europe), not the USA version" },
	{ SLOT_BN5, "BRKJ", "Rockman EXE 5 Team of Colonel (Japan), not the USA version" },
	{ SLOT_BN4, "B4BE", "BN4 Blue Moon, not Red Sun" },
	{ SLOT_BN4, "B4BJ", "Rockman EXE 4 Blue Moon (Japan), not BN4 Red Sun (USA)" },
	{ SLOT_BN4, "B4WJ", "Rockman EXE 4 Red Sun (Japan), not the USA version" },
};

const char *refuse_why(const char code[4], int slot_of_code, bool changed, unsigned shown) {
	static char s[128];
	if (changed && slot_of_code >= 0) {
		snprintf(s, sizeof s, "%s, but changed: patched, trimmed or a bad dump", slots[clamp_slot(slot_of_code)].tag);
		return s;
	}
	for (size_t i = 0; i < sizeof others / sizeof *others; ++i)
		if ((shown >> others[i].slot & 1) && !memcmp(code, others[i].code, 4)) return others[i].why;
	/* ("not BN6 Gregar or BN5 Team Colonel") */
	int count = 0, done = 0;
	for (int k = 0; k < SLOTS; ++k) count += (shown >> k) & 1;
	snprintf(s, sizeof s, "not ");
	for (int k = 0; k < SLOTS; ++k) {
		if (!((shown >> k) & 1)) continue;
		size_t m = strlen(s);
		const char *sep = !done ? "" : done == count - 1 ? " or " : ", ";
		snprintf(s + m, sizeof s - m, "%s%s", sep, slots[k].named);
		++done;
	}
	return s;
}

const char *refuse_not_rom(bool zipped) { return zipped ? "zipped: unzip it first" : "not a GBA ROM"; }

void note_found(const char *where, char *out, size_t n) {
	snprintf(out, n, "Found %s in %s, checked by its SHA-1.", slots[SLOT_BN6].tag, where);
}

void note_gone(int slot, char *out, size_t n) {
	snprintf(out, n, "%s is gone from where it was: %s out of runs until it is back.", slots[clamp_slot(slot)].tag,
		slots[clamp_slot(slot)].stays);
}

void note_no_picker(const char *where, char *out, size_t n) {
	snprintf(out, n, "No file chooser here: put the .gba in Downloads or %s, or drop it on this window.", where);
}

void saves_line_desktop(const char *where, char *out, size_t n) { snprintf(out, n, "Saves: %s", where); }

void saves_line_phone(const char *folder, bool automatic, bool refused, char *out, size_t n) {
	char f[20];
	/* (a long name cut, so that the line stays one) */
	if (folder && strlen(folder) > 14) snprintf(f, sizeof f, "%.12s...", folder);
	else snprintf(f, sizeof f, "%s", folder ? folder : "");
	if (!automatic) snprintf(out, n, "Saves: on this device\nNo copy: turned off in SAVES");
	else if (!f[0]) snprintf(out, n, "Saves: on this device only\nChoose a folder in SAVES for a copy");
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
