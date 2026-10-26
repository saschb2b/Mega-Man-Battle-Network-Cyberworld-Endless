/* meta_text.h. Menu words, short and plain: a folder's style and its cost
 * in a line, as KeyItem's box says what an item is, and its milestone as
 * the summary's goal line says it ("Delete an Aqua guardian: Storm", 32
 * letters at most with the name). */
#include "meta_text.h"

#include <stddef.h>

/* (in the folders' order, FOLDER_STANDARD first) */
static const FolderInfo folders[FOLDER_COUNT] = {
	{ "Standard", "BN6's own starting folder", NULL },
	{ "Blade", "Swords in S for LifeSword. Nothing reaches the back", "delete any guardian" },
	{ "Storm", "Elec chips: double on Aqua, plain on the rest", "delete an Aqua guardian" },
	{ "Z-Canon", "Cannons in A, B and C for GigaCan1. Little hits behind cover", "use a Program Advance" },
	{ "Busting", "Heavy hitters in L and R. No Recovery chips", "clear an act, no Recov" },
	{ "Lean", "Seven kinds in B or *: whole hands, little else", "reach layer 10" },
};

const FolderInfo *meta_folder_words(int folder) { return &folders[folder >= 0 && folder < FOLDER_COUNT ? folder : 0]; }
