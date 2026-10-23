/* title_setup_text.h. Menu words, short and plain: what each row sets,
 * and in the information box what the choice under the cursor does, as
 * KeyItem's says what an item is. */
#include "title_setup_text.h"

#include <stdio.h>

/* (in SetupWord's order, its name beside each) */
static const char *const words[] = {
	"JACK-IN SETUP",   /* SW_TITLE */
	"Help",   /* SW_HELP_TITLE */
	"Net",   /* SW_NET */
	"Folder",   /* SW_FOLDER */
	"Cross",   /* SW_CROSS */
	"Threat",   /* SW_THREAT */
	"Help",   /* SW_HELP */
	"Short",   /* SW_SHORT */
	"Endless",   /* SW_ENDLESS */
	"None",   /* SW_NONE */
	"On",   /* SW_ON */
	"Off",   /* SW_OFF */
	"JACK IN!",   /* SW_JACK_IN */
	"INFORMATION",   /* SW_INFORMATION */
	"NEW",   /* SW_NEW */
	"Three acts, then the Nest",   /* SW_SHORT_NOTE */
	"Three acts, then the Nest. Win it for the endless net",   /* SW_SHORT_WIN_NOTE */
	"The net repeats, harder each time",   /* SW_ENDLESS_NOTE */
	"The net as it comes",   /* SW_THREAT0_NOTE */
	"The net as it comes. Win it for threat 1",   /* SW_THREAT0_WIN_NOTE */
	"Delete a Cross Navi to start in his Cross",   /* SW_CROSS_NONE_NOTE */
	"Crosses from the guardians we delete",   /* SW_CROSS_GUARDIANS_NOTE */
	"From the first battle",   /* SW_CROSS_CHOSEN_NOTE */
	"No other Cross this run",   /* SW_CROSS_ONE */
	"Start in more: delete Navis",   /* SW_CROSS_MORE */
	"Helpers, each on or off. A helped run's summary names them",   /* SW_HELP_NOTE */
	"Into the net with these. They stay for the next run",   /* SW_GO_NOTE */
};
const char *setup_word(SetupWord w) {
	_Static_assert(sizeof words / sizeof *words == SW_COUNT, "a word for each SetupWord");
	return (unsigned)w < SW_COUNT ? words[w] : "";
}

/* (its costs said: BN6's own weakness, which a playtester was told wrong;
 * and "break it": a playtester knew it from BN6, a newer player would not) */
const char *setup_cross_note(char *buf, size_t n, const char *strong, const char *weak) {
	if (strong) snprintf(buf, n, "%s.|%s hits do 2x and break it", strong, weak);
	else snprintf(buf, n, "From the first battle. %s hits do 2x and break it", weak);
	return buf;
}

const char *setup_helper_note(char *buf, size_t n, const char *about, const char *more) {
	if (more) snprintf(buf, n, "%s. %s", about, more);
	else snprintf(buf, n, "%s", about);
	return buf;
}

const char *setup_shut_folder(char *buf, size_t n, const char *name, const char *opens) {
	snprintf(buf, n, "%s: %s", name, opens);
	return buf;
}

const char *setup_helpers_on(char *buf, size_t n, int on) {
	snprintf(buf, n, "%d on", on);
	return buf;
}
