/* meta_text.h. Menu words, short and plain: a folder's style and its cost
 * in a line, as KeyItem's box says what an item is, and its milestone as
 * the setup's shut folders list it ("Storm: delete an Aqua guardian"); the
 * summary's closest goal, a sentence of three lines at most, its period
 * as BN6's KeyItem descriptions end theirs. */
#include "meta_text.h"

#include <stdio.h>

#include "run.h"

/* (in the folders' order, FOLDER_STANDARD first; an id no folder holds
 * left blank, its name NULL: meta_folder_exists) */
static const FolderInfo folders[FOLDER_COUNT] = {
	[FOLDER_STANDARD] = { "Standard", "BN6's own starting folder", NULL, NULL, NULL },
	/* (each hint points at the kind of deed, in the folder's own spirit,
	 * never the deed: issue #864's directions) */
	[FOLDER_BLADE] = { "Blade", "Swords in S for LifeSword. Nothing reaches the back", "delete any guardian", "when something big falls", "Swords, up close" },
	[FOLDER_STORM] = { "Storm", "Elec chips: double on Aqua, plain on the rest", "delete an Aqua guardian", "where lightning strikes hardest",
		"Elec chips for Aqua foes" },
	[FOLDER_ZCANON] = { "Z-Canon", "Three Program Advances. Most of it hits close", "use a Program Advance",
		"when chips work as one, in order", "Three Program Advances" },
	[FOLDER_BUSTING] = { "Busting", "Heavy hitters in L and R. No Recovery chips", "clear an act with no Recov", "for an act with no safety net",
		"Heavy hitters, no Recovery" },
	[FOLDER_LEAN] = { "Lean", "Seven kinds in B or *: whole hands, little else", "reach layer 10", "deep down in the Net", "Any five chips make a hand" },
	/* (BN6's presets made a start's, ids 6 and 7, issue #865) */
	[FOLDER_STEADY] = { "Steady", "A step up from Standard, four Recov30. Few codes", "reach act 3", "for a Navi who lasts", "A step up, with heals" },
	[FOLDER_NAVI] = { "Navi", "Three Navi chips and help for them. The rest hits soft", "use a Navi chip", "when a friend helps",
		"Navi chips at its heart" },
	/* (issue #865's archetypes, ids 9-11) */
	[FOLDER_EMBER] = { "Ember", "Grass first, then Fire on it for double. Short reach", "delete a guardian in HeatCross",
		"in the heat of a big fight", "Fire chips on grass" },
	[FOLDER_TIDE] = { "Tide", "Aqua chips: double on Fire, plain on the rest", "delete a Fire guardian", "where the flames burn hottest",
		"Aqua chips for Fire foes" },
};

const FolderInfo *meta_folder_words(int folder) { return &folders[folder >= 0 && folder < FOLDER_COUNT ? folder : FOLDER_STANDARD]; }

/* (BN4's AirShot A and Vulcan1 V are codes BN6's records lack: in the
 * nearest they hold, xfolder_code) */
const char *meta_standard_about(int form) {
	if (form == RUN_FORM_SOUL_BN5) return "BN5's own starting folder";
	if (form == RUN_FORM_SOUL_BN4) return "BN4's own starting folder. AirShot in *, Vulcan1 in S";
	return folders[FOLDER_STANDARD].about;
}

/* (MegaMan's forms by system, as the owner named them, issue #774; the
 * mix, an older save's run, by what it holds) */
const char *meta_form_name(int form) {
	static const char *const names[RUN_FORMS] = { "MIX", "CROSS", "SOUL BN5", "SOUL BN4" };
	return names[form >= 0 && form < RUN_FORMS ? form : RUN_FORM_MIX];
}

const char *meta_goal_words(char *buf, size_t n, MetaGoal goal, int arg) {
	const FolderInfo *f = meta_folder_words(arg);
	switch (goal) {
	case GOAL_ENDLESS: return "Win a short net to open the endless net.";
	case GOAL_CROSS: return "Delete a Cross Navi to start in his Cross.";
	case GOAL_THREAT: snprintf(buf, n, "Win on threat %d to open threat %d.", arg, arg + 1); return buf;
	case GOAL_FOLDER:
		if (!f->opens) break;
		/* (its milestone, as the setup lists it, made a sentence) */
		snprintf(buf, n, "%c%s to open the %s folder.", f->opens[0] - 'a' + 'A', f->opens + 1, f->name);
		return buf;
	case GOAL_FOLDER_HINT:
		/* (locked and not yet heard: the shelf's hint, made a sentence) */
		if (!f->hint) break;
		snprintf(buf, n, "The %s folder opens %s.", f->name, f->hint);
		return buf;
	}
	return "";
}
