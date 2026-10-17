#include "title_recovery_text.h"

const char title_recovery_heading[] = "SAVE RECOVERY";
const char title_recovery_help[] = "Up/Down: Choose   A: Select";
const char *const title_recovery_lines[TITLE_RECOVERY_PAGES][3] = {
	{ "This new run did not finish saving.", "It cannot be continued yet.", "Discard its unfinished start?" },
	{ "A trade still needs recovery.", "Your run is on hold until it settles.", "Keep these files to finish the swap." },
	{ "Saved files could not be verified.", "Your run stays on hold.", "Check storage, then try recovery." },
	{ "Could not open this run.", "Return to the title to choose again.", "No new run has been started." },
	{ "Discard the unfinished new run?", "Its run checkpoint will be removed.", "Your permanent progress is kept." },
};
const char *const title_recovery_choices[TITLE_RECOVERY_PAGES][2] = {
	{ "Discard unfinished start", "Quit" },
	{ "Retry recovery", "Quit" },
	{ "Retry recovery", "Quit" },
	{ "Back to title", "Quit" },
	{ "Keep saved files", "Discard unfinished start" },
};
