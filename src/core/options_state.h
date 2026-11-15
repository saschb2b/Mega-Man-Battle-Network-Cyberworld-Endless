/* Options' state, shared by its flow (options.c) and its picture
 * (options_draw.c, options_scene.c): the page shown, its lines and the
 * cursor on them, and the question putting a page's lines back. */
#ifndef CW_OPTIONS_STATE_H
#define CW_OPTIONS_STATE_H

#include <SDL.h>
#include <stdbool.h>

/* A line's kind: the Display page's (Fullscreen, Scale, Smooth motion, and
 * the GBA's screen's Colour correction, Frame blending and LCD 3x, where
 * the system has them: display.h), the Audio page's (Music and Effects,
 * sound_heard.h; Output and Sound, sound_filter.h; Vibration and Trigger
 * effects where the system has
 * them, feel_out.h) and the Game page's (the update found, on the
 * title: update.h; Controls, Fast forward, Statistics, and Updates where
 * the build looks for them) */
enum { LINE_CONTROLS, LINE_STATS, LINE_UPDATE, LINE_FULLSCREEN, LINE_SCALE, LINE_SMOOTH, LINE_SOUND, LINE_MUSIC, LINE_EFFECTS, LINE_FAST,
	LINE_VIBRATION, LINE_TRIGGERS, LINE_COLOUR, LINE_BLEND, LINE_LCD3X, LINE_OUTPUT, LINE_VOXEL, LINE_UPDATES };
#define LINES_MAX 7

/* A line whose value Left, Right, A or a tap turns, and keeps at once */
static inline bool options_line_turns(int kind) { return kind >= LINE_FULLSCREEN && kind <= LINE_UPDATES; }

/* The pages, L and R turning from one to the next as the Library turns
 * its own (Valhalla's tabs, docs/engine/settings-screens.md) */
enum { PAGE_DISPLAY, PAGE_AUDIO, PAGE_GAME, PAGE_COUNT };

/* The rows the list shows at once, a page's others scrolled to */
#define OPTIONS_ROWS 3

typedef struct {
	bool open;
	int page;            /* (PAGE_*) */
	int at[PAGE_COUNT];  /* the cursor: a line's place in options_lines' list, a page each */
	int top;             /* the list's first row shown */
	int t;               /* frames since it or its page opened (its fade) */
	bool asking;         /* START's question: the page's lines back as they came? */
	bool yes;            /* ... its cursor on Yes */
	SDL_FingerID finger; /* (a finger that went down on it: only its lift taps) */
	bool finger_down;
} OptionsState;
extern OptionsState O;

/* The page's lines shown, top to bottom: their kinds, and how many */
int options_lines(int kinds[LINES_MAX]);
/* ... the line under the cursor's kind */
int options_line_kind(void);
/* Whether START puts the page's lines back (some line there has a default) */
bool options_page_resets(void);
/* Where a tap at canvas pixel (x, y) from the picture's corner lands: a
 * row's place in the list (0 up), or one of these (TAP_PAGE - p: page p's
 * tab) */
enum { TAP_NONE = -1, TAP_PREV = -2, TAP_NEXT = -3, TAP_DEFAULT = -4, TAP_YES = -5, TAP_NO = -6, TAP_PAGE = -7 };
int options_tap(int x, int y);

#endif
