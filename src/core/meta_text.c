/* meta_text.h. Menu words, short and plain: a folder's style and its cost
 * in a line, as KeyItem's box says what an item is, and its milestone as
 * the setup's shut folders list it ("Storm: delete an Aqua guardian"); the
 * summary's closest goal, a sentence of three lines at most, its period
 * as BN6's KeyItem descriptions end theirs. */
#include "meta_text.h"

#include <stdio.h>

#include "run.h"

/* (in the folders' order, FOLDER_STANDARD first) */
static const FolderInfo folders[FOLDER_COUNT] = {
	{ "Standard", "BN6's own starting folder", NULL },
	{ "Blade", "Swords in S for LifeSword. Nothing reaches the back", "delete any guardian" },
	{ "Storm", "Elec chips: double on Aqua, plain on the rest", "delete an Aqua guardian" },
	{ "Z-Canon", "Cannons in A, B and C for GigaCan1. Little hits behind cover", "use a Program Advance" },
	{ "Busting", "Heavy hitters in L and R. No Recovery chips", "clear an act with no Recov" },
	{ "Lean", "Seven kinds in B or *: whole hands, little else", "reach layer 10" },
};

const FolderInfo *meta_folder_words(int folder) { return &folders[folder >= 0 && folder < FOLDER_COUNT ? folder : 0]; }

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
	}
	return "";
}
