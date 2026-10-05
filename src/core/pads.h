/* The controllers (issue #37): opened through SDL's game controller API,
 * read through the player's map (padmap.c, pad.ini in the data folder),
 * and what the controls screen (controls.c) asks of them. platform.c hands
 * them SDL's events and reads them once a frame in platform_poll. */
#ifndef CW_PADS_H
#define CW_PADS_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "padmap.h"

/* A pad's input pressed (a button, or a trigger or stick pushed past the
 * middle), with the pad's family and style: what "press a button" takes. */
typedef struct { int input, family, style; } PadPress;

/* The controllers there are opened (at the start and when one comes);
 * on Android a Joy-Con is given its mapping first (padmap_joycon). */
void pads_open(void);
void pads_close(void);
/* An SDL event, taken where it is a controller's (one coming or going, a
 * button); true when a button went down (the touch controls go away). */
bool pads_event(const SDL_Event *e);
/* A frame's reading: pads_begin before its events, pads_poll after them
 * (the sticks and triggers). */
void pads_begin(void);
void pads_poll(void);
/* The GBA buttons (BTN_*) held through the map, and those pressed since
 * the last call (a tap shorter than a frame counts). */
uint32_t pads_held(void);
uint32_t pads_taken(void);
/* The same, fixed, never the map's (the controls screen reads these, so a
 * map it edits cannot lock it): the D-pad and the left stick move, a or
 * start is A, b or back is B. */
uint32_t pads_menu_held(void);
uint32_t pads_menu_taken(void);
/* Back and start held on one pad: the way out no map takes away. */
bool pads_quit_held(void);
bool pads_present(void);
/* The pad used last (else the first): its family, style and name; false
 * for none. */
bool pads_last(int *family, int *style, const char **name);
/* This frame's presses, at most `most`. */
int pads_presses(PadPress *out, int most);
/* The map in play, and a new one put in play (pads_save writes it). */
const PadMap *pads_map(void);
void pads_set_map(const PadMap *m);
/* pad.ini at `path`: read (written with the defaults where missing), and
 * written again after the controls screen changed the map. */
void pads_load(const char *path);
bool pads_save(void);

#endif
