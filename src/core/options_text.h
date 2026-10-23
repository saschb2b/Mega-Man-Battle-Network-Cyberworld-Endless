/* Options' words (options_text.c): its lines, their notes, the Audio
 * section's choices, the keys' line, settings.ini's lines for the sound,
 * in the plain style of the game's menus (docs/VOICE.md), apart from the
 * logic that says when. */
#ifndef CW_OPTIONS_TEXT_H
#define CW_OPTIONS_TEXT_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
	OW_TITLE,           /* the head's name */
	OW_CONTROLS, OW_CONTROLS_NOTE,
	OW_AUDIO, OW_AUDIO_NOTE,
	OW_DONE, OW_DONE_NOTE, OW_BACK_NOTE,   /* Done in the hub, and in a section */
	OW_SOUND,           /* the Audio section's line */
	OW_GBA, OW_GBA_NOTE,
	OW_SOFT, OW_SOFT_NOTE,
	OW_TAP,             /* a phone's keys' line */
	OW_COUNT
} OptionsWord;
const char *options_word(OptionsWord w);

/* The keys' line: "J: choose  K: back", or on the Sound line "J: change" */
void options_keys(const char *a, const char *b, bool change, char *out, size_t n);
/* settings.ini's lines before the sound line: what it is */
const char *options_sound_note(void);

#endif
