/* The statistics' words (analytics_text.c): the question at the first
 * start, the controls screen's row and its notes, settings.ini's lines,
 * in the plain style of the game's menus (docs/VOICE.md), apart from the
 * logic that says when. */
#ifndef CW_ANALYTICS_TEXT_H
#define CW_ANALYTICS_TEXT_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
	AW_TITLE,        /* the question's heading */
	AW_QUESTION,
	AW_WHAT,         /* what is sent, and what for */
	AW_NOT,          /* what never is */
	AW_YES, AW_NO,
	AW_ROW,          /* the controls screen's row */
	AW_ON, AW_OFF,
	AW_ROW_NOTE,     /* its note */
	AW_COUNT
} AnalyticsWord;
const char *analytics_word(AnalyticsWord w);

/* Under the question: where it is changed later (Options, on the title's
 * menu and the PET's) */
const char *analytics_later(void);
/* settings.ini's lines before the statistics line: what it is */
const char *analytics_setting_note(void);

#endif
