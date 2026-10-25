/* Options' words (options_text.c): its lines, their notes, the Display
 * and Audio sections' choices, settings.ini's notes for the display's
 * lines, the sound, the music, the effects, fast forward and the
 * controller's vibration and trigger effects,
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
	OW_SOUND,           /* the Audio section's line */
	OW_GBA, OW_GBA_NOTE,
	OW_SOFT, OW_SOFT_NOTE,
	OW_DISPLAY, OW_DISPLAY_NOTE,
	OW_FULLSCREEN, OW_FULLSCREEN_ON_NOTE, OW_FULLSCREEN_OFF_NOTE,   /* the Display section's lines */
	OW_SCALE,
	OW_AUTO, OW_WHOLE, OW_FILL, OW_STRETCH,   /* (in DisplayScale's order) */
	OW_AUTO_NOTE, OW_WHOLE_NOTE, OW_FILL_NOTE, OW_STRETCH_NOTE,
	OW_SMOOTH, OW_SMOOTH_ON_NOTE, OW_SMOOTH_OFF_NOTE,
	OW_ON, OW_OFF,
	OW_INFORMATION,     /* the information box's label, KeyItem's */
	OW_MUSIC, OW_MUSIC_ON_NOTE, OW_MUSIC_OFF_NOTE,   /* the Audio section's lines (sound_heard.h) */
	OW_EFFECTS, OW_EFFECTS_ON_NOTE, OW_EFFECTS_OFF_NOTE,
	OW_FAST, OW_FAST_ON_NOTE, OW_FAST_OFF_NOTE,   /* the hub's fast forward line (fast_forward.h) */
	OW_VIBRATION, OW_WEAK, OW_MEDIUM, OW_STRONG,   /* the Audio section's controller lines (feel_out.h; FEEL_WEAK's order) */
	OW_VIBRATION_ON_NOTE, OW_VIBRATION_OFF_NOTE,
	OW_TRIGGERS, OW_TRIGGERS_ON_NOTE, OW_TRIGGERS_OFF_NOTE,
	OW_COUNT
} OptionsWord;
const char *options_word(OptionsWord w);

/* settings.ini's lines before the sound line: what it is */
const char *options_sound_note(void);
/* ... before the music's and the effects' */
const char *options_music_note(void);
const char *options_effects_note(void);
/* ... before the display's: screen, smooth_motion and fullscreen */
const char *options_screen_note(void);
const char *options_smooth_note(void);
const char *options_fullscreen_note(void);
/* ... before fast forward's */
const char *options_fast_note(void);
/* ... before the controller's vibration and trigger effects */
const char *options_vibration_note(void);
const char *options_triggers_note(void);

#endif
