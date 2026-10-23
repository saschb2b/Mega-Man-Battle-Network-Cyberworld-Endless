/* Options' state, shared by its flow (options.c) and its picture
 * (options_draw.c): the hub's lines, or the Audio section's, and the
 * cursor on them. */
#ifndef CW_OPTIONS_STATE_H
#define CW_OPTIONS_STATE_H

#include <SDL.h>
#include <stdbool.h>

/* A line's kind: the hub's (Controls, Audio, Statistics, Done), the Audio
 * section's (Sound, Done) */
enum { LINE_CONTROLS, LINE_AUDIO, LINE_STATS, LINE_SOUND, LINE_DONE };
#define LINES_MAX 4

typedef struct {
	bool open;
	bool audio;          /* the Audio section, in the hub's place */
	int at;              /* the cursor: a line's place in options_lines' list */
	int hub_at;          /* ... the hub's, kept while a section is open */
	SDL_FingerID finger; /* (a finger that went down on it: only its lift taps) */
	bool finger_down;
} OptionsState;
extern OptionsState O;

/* The lines shown, top to bottom: their kinds, and how many */
int options_lines(int kinds[LINES_MAX]);
/* The line a tap at canvas pixel (x, y) from the picture's corner lands
 * on, its place in the list (-1 for none) */
int options_tap_line(int x, int y);

#endif
