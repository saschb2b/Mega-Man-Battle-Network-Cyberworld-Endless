/* The colours screen's words (looks_screen.h; docs/game/looks.md): its
 * name, its labels, the channels' names and parts, the pop-up's rows, a
 * colour's plain name, and what its INFORMATION box says of the cursor's
 * place, in the PET's own plain voice as KeyItem's (docs/voice/rules.md);
 * and each palette's short name for its window's band and its one line on
 * where it comes from. */
#ifndef CW_LOOKS_SCREEN_TEXT_H
#define CW_LOOKS_SCREEN_TEXT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "looks.h"

typedef enum {
	LW_TITLE,         /* the band: COLOURS */
	LW_LOOK,          /* the list's first row, the saved looks' */
	LW_PLAIN,         /* the grid's last cell: the channel as BN6 draws it */
	LW_LOCKED,        /* a locked palette's mark */
	LW_INFORMATION,   /* the box's label */
	LW_KEYS,          /* the band's hint: L/R */
	LW_PAGES,         /* ... PALETTE */
	LW_ALL_PLAIN,     /* the pop-up's last row */
	LW_COUNT
} LooksWord;
const char *looks_word(LooksWord w);

/* A channel's name ("Armour") */
const char *looks_channel_name(int c);
/* "A", "B", "C" */
const char *looks_slot_name(int slot);
/* Palette p's name on its window's band, eight capitals at most */
const char *looks_band_name(int p);
/* A colour's plain name ("dark red", "grey"), so no colour is told by its colour alone */
const char *looks_colour_word(uint16_t bgr555);

/* The pop-up's row: "Copy to B" */
void looks_copy_row(char *out, size_t n, int to);

/* The box on the looks' row: the look worn, channel by channel */
void looks_slot_words(char *out, size_t n, int slot, const Look *look);
/* ... on a channel: what it colours, and the colour it has */
void looks_channel_words(char *out, size_t n, int c, const Look *look);
/* ... on a palette's colour (owned or tried on while locked), or its plain cell (colour -1) */
void looks_colour_words(char *out, size_t n, int palette, int colour, bool owned);
/* ... on a palette not owned, from the list: locked, and where it comes from */
void looks_locked_words(char *out, size_t n, int palette);
/* ... on the pop-up's row: what A does there */
void looks_popup_words(char *out, size_t n, int slot, int row);

#endif
