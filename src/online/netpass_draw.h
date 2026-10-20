/* The NetPass drawn (issue #131, docs/ONLINE.md, The NetPass): a PET
 * screen of the project's own, in the colours of Gregar's PET
 * (second_frame.h, tools/site_art.py) and with the cartridges as the ROMs
 * screen draws them (launcher_art.c): the header with the pass's level,
 * the operator card with MegaMan's face read from the player's ROM as it
 * is drawn (never kept in a file), the four stamps, and the cartridge
 * slots with the PET's line on what they open. Drawing only: it reads the
 * view it is given and changes nothing. */
#ifndef CW_NETPASS_DRAW_H
#define CW_NETPASS_DRAW_H

#include <stdbool.h>

#include "gfx.h"
#include "netpass.h"

#define NETPASS_W 240
#define NETPASS_H 160
#define NETPASS_STRIP_H 54   /* the gate's strip: the header and the stamps in a row */

typedef struct {
	bool held[STAMP_COUNT];   /* the stamps drawn as held */
	bool full;                /* the header's level */
	int landing;              /* a stamp landing now (the stamp moment), -1 none */
	int landing_t;            /* ... frames since it landed */
	bool into;                /* drawn into memory (the second screen: gfx_draw_into) */
	int t;                    /* frames, for what moves (an open slot's dashes) */
} NetPassView;

/* The PET screen's parts, Trade Central's screens' too (trade_screen_draw.c):
 * a panel, `fill` inside a 1-pixel `edge`, its corners cut; the screen's
 * ground, the PET's dark blue under a dotted grid */
void netpass_panel(int x, int y, int w, int h, SDL_Color fill, SDL_Color edge);
void netpass_ground(int x, int y, int w, int h);

/* The view of this game's pass as it stands */
void netpass_view_now(NetPassView *v);
/* The whole pass, NETPASS_W x NETPASS_H from (x, y) */
void netpass_draw(int x, int y, const NetPassView *v);
/* The gate's strip, NETPASS_W x NETPASS_STRIP_H from (x, y) */
void netpass_draw_strip(int x, int y, const NetPassView *v);

#endif
