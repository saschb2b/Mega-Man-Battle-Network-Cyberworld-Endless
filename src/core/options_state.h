/* Options' state, shared by its flow (options.c) and its picture
 * (options_draw.c): the hub's lines, or a section's (Display, Audio), and
 * the cursor on them. */
#ifndef CW_OPTIONS_STATE_H
#define CW_OPTIONS_STATE_H

#include <SDL.h>
#include <stdbool.h>

/* A line's kind: the hub's (Controls, Display, Audio, Fast forward,
 * Statistics), the
 * Display section's (Fullscreen, Scale, Smooth motion, where the system
 * has them: display.h), the Audio section's (Sound; Music and Effects,
 * sound_heard.h), and Done */
enum { LINE_CONTROLS, LINE_DISPLAY, LINE_AUDIO, LINE_STATS, LINE_FULLSCREEN, LINE_SCALE, LINE_SMOOTH, LINE_SOUND, LINE_MUSIC, LINE_EFFECTS, LINE_FAST };
#define LINES_MAX 5

/* A line whose value Left, Right, A or a tap turns, and keeps at once */
static inline bool options_line_turns(int kind) { return kind >= LINE_FULLSCREEN && kind <= LINE_FAST; }

/* The section open: the hub's lines, or a section's in their place */
enum { SECTION_HUB, SECTION_DISPLAY, SECTION_AUDIO };

typedef struct {
	bool open;
	int section;         /* (SECTION_*) */
	int at;              /* the cursor: a line's place in options_lines' list */
	int hub_at;          /* ... the hub's, kept while a section is open */
	int t;               /* frames since it or its section opened (its fade) */
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
