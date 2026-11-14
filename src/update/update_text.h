/* The updater's words (update_text.c): Options' Updates lines and their
 * notes, the title's note beside the version, and settings.ini's comment,
 * in the plain style of the game's menus (docs/voice/rules.md), apart
 * from the logic that decides when they show (update.c). */
#ifndef CW_UPDATE_TEXT_H
#define CW_UPDATE_TEXT_H

typedef enum {
	UW_ROW,            /* the switch's line */
	UW_ON_NOTE,        /* ... its notes */
	UW_OFF_NOTE,
	UW_GET,            /* the update's line, a version after it */
	UW_INSTALL_NOTE,   /* ... its notes, by the way this copy updates (UpdateWay) */
	UW_PAGE_NOTE,
	UW_RELEASES_NOTE,
	UW_ITCH_NOTE,
	UW_GETTING_NOTE,   /* ... while it downloads */
	UW_FAILED,         /* ... the line's value after a failed download, */
	UW_FAILED_NOTE,    /* ... and its note */
	UW_NEW,            /* the title's note beside the version: the new one after it */
	UW_COUNT
} UpdateWord;

const char *update_word(UpdateWord w);
/* settings.ini's comment before the updates line, where one is added */
const char *update_setting_note(void);

#endif
