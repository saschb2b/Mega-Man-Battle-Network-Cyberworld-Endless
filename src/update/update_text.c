/* update_text.h. Menu words, short and plain: what the check does and
 * sends, what a press does, as docs/GUIDE.md's Updates says it. */
#include "update_text.h"


/* (in UpdateWord's order, its name beside each) */
static const char *const words[] = {
	"Updates",   /* UW_ROW */
	"Looks on GitHub for a new version at each start. Sends nothing about you",   /* UW_ON_NOTE */
	"The game never looks for a new version",   /* UW_OFF_NOTE */
	"Update to",   /* UW_GET */
#ifdef __ANDROID__
	/* (Android's installer puts it in place: the first time it asks) */
	"A: downloads it and installs it. Your saves and your run stay as they are",   /* UW_INSTALL_NOTE */
#else
	"A: downloads it and starts it. Your saves and your run stay as they are",   /* UW_INSTALL_NOTE */
#endif
	"A: opens the download page",   /* UW_PAGE_NOTE */
	"On the game's releases page, where this one came from",   /* UW_RELEASES_NOTE */
	"The itch app installs it for you",   /* UW_ITCH_NOTE */
	"In Prospero Store, or on the game's releases page",   /* UW_STORE_NOTE */
	"A: downloads it and closes the game. Your saves and your run stay as they are",   /* UW_INSTALL_END_NOTE */
	"Downloading. The game closes when it is done: start it again",   /* UW_GETTING_END_NOTE */
#ifdef __ANDROID__
	"Downloading. Android installs it when it is done",   /* UW_GETTING_NOTE */
	"Android installs it and closes the game. Then open it again",   /* UW_READY_NOTE */
	"Failed",   /* UW_FAILED */
	"The update stopped. A: try again",   /* UW_FAILED_NOTE */
#else
	"Downloading. The game starts again when it is done",   /* UW_GETTING_NOTE */
	"Done. The game starts again as the new version",   /* UW_READY_NOTE */
	"Failed",   /* UW_FAILED */
	"The download stopped. A: try again",   /* UW_FAILED_NOTE */
#endif
	"New",   /* UW_NEW */
};
_Static_assert(sizeof words / sizeof *words == UW_COUNT, "a word for each UpdateWord");

const char *update_word(UpdateWord w) { return (unsigned)w < UW_COUNT ? words[w] : ""; }

const char *update_setting_note(void) {
	return "\n# updates: on looks for a new version at each start (the release's\n"
		"# update.json on GitHub; nothing about you or your game is sent), and\n"
		"# Options offers it. off never looks.\n";
}
