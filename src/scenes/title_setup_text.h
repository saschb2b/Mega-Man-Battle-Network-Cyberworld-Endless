/* The JACK-IN SETUP's words (title_setup_text.c): its rows, their values,
 * the information on the row under the cursor and what is still shut, in
 * the plain style of the game's menus (docs/voice/rules.md), apart from the
 * logic that says when. The folders', helpers' and threat rungs' own
 * names and lines are meta.c's, the Crosses' powers_words.c's. */
#ifndef CW_TITLE_SETUP_TEXT_H
#define CW_TITLE_SETUP_TEXT_H

#include <stddef.h>

typedef enum {
	SW_TITLE,          /* the band's name */
	SW_HELP_TITLE,     /* ... the helpers' page's */
	SW_NET, SW_FOLDER, SW_CROSS, SW_THREAT, SW_HELP,   /* the rows */
	SW_SHORT, SW_ENDLESS, SW_NONE, SW_ON, SW_OFF,      /* their values */
	SW_JACK_IN,        /* the one action */
	SW_INFORMATION,    /* the information box's label, KeyItem's */
	SW_NEW,            /* what the last run opened, as BN6 marks a new mail */
	SW_SHORT_NOTE, SW_SHORT_WIN_NOTE, SW_ENDLESS_NOTE,
	SW_THREAT0_NOTE, SW_THREAT0_WIN_NOTE,
	SW_CROSS_NONE_NOTE, SW_CROSS_GUARDIANS_NOTE, SW_CROSS_CHOSEN_NOTE,
	SW_CROSS_ONE, SW_CROSS_MORE,
	SW_HELP_NOTE, SW_GO_NOTE,
	SW_COUNT
} SetupWord;
const char *setup_word(SetupWord w);

/* The Cross row's note for a Cross with its `strong` power and `weak`
 * element (strong NULL: only the weakness), into buf: '|' breaks a line */
const char *setup_cross_note(char *buf, size_t n, const char *strong, const char *weak);
/* a helper's note: what it does, and what else, `more` (NULL: nothing) */
const char *setup_helper_note(char *buf, size_t n, const char *about, const char *more);
/* a folder still shut, and how it opens */
const char *setup_shut_folder(char *buf, size_t n, const char *name, const char *opens);
/* the Help row's value with `on` helpers on, where their names run too long */
const char *setup_helpers_on(char *buf, size_t n, int on);

#endif
