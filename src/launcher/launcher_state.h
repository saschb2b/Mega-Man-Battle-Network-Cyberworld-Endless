/* What the launcher's parts share (launcher.c the screen's course,
 * launcher_draw.c its layout and picture): its state, and where each of
 * its pieces stands on the canvas. */
#ifndef CW_LAUNCHER_STATE_H
#define CW_LAUNCHER_STATE_H

#include <SDL.h>
#include <stdbool.h>

#include "launcher_text.h"

/* What the cursor can be on: the cartridges, the second button, PLAY */
enum { FOCUS_BN6, FOCUS_BN5, FOCUS_ALT, FOCUS_PLAY, FOCUSES };

typedef struct {
	bool from_title;          /* opened from the title: the game runs, DONE goes back */
	void (*begin)(void);      /* (from the start) the game begun, once PLAY is chosen */
	char rom_dir[600];        /* where copies are kept and a phone's picks land */
	int t;                    /* frames since it opened */
	int focus;                /* FOCUS_* */
	unsigned kinds;           /* the pickers there are (pick.h) */
	int busy_slot;            /* a picker open for this cartridge, -1 none */
	char note[1100];          /* what the note says (a result), "" for the cursor's own */
	int note_kind;            /* NOTE_* */
	int note_focus;           /* the cursor's place when the note was set: moving on clears it */
	int filled_at[SLOTS];     /* the frame a cartridge went in (its drop), -1 */
	bool had[SLOTS];          /* in at the last look */
	int looked_at;            /* the frame of the last look for a ROM put in by hand */
	char saves[200];          /* the saves' line */
	char folder[256];         /* the folder kept (a phone's), "" none */
	int pointed;              /* what the mouse or a finger is over, -1 nothing */
} Launcher;

extern Launcher L;

/* Where the pieces stand: computed for the canvas each frame, the same
 * for the frame's input and its picture */
typedef struct {
	bool upright;            /* stacked, a phone held upright */
	bool large;              /* the cartridges' larger art */
	SDL_Rect head, body;
	SDL_Rect cart[SLOTS];    /* each cartridge or its open spot */
	SDL_Rect caption[SLOTS]; /* the line under it */
	SDL_Rect note;
	int note_lines;
	SDL_Rect saves;          /* the saves' line (h 0: no room for it) */
	SDL_Rect alt, play;      /* the buttons (alt w 0: none) */
	SDL_Rect hint;           /* the keys' line (h 0: none) */
} LauncherLayout;

void launcher_layout(LauncherLayout *lay);
/* The picture (launcher_draw.c) */
void launcher_draw_all(const LauncherLayout *lay);
/* The note for what the cursor is on (launcher.c), into `out` */
void launcher_cursor_note(char *out, size_t n, int *kind);

#endif
