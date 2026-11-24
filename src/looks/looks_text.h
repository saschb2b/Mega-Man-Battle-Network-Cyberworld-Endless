/* The colours screen's words (looks_screen.h; docs/game/looks.md): its
 * name, its labels, the channels' names and parts, and what its
 * INFORMATION box says of the cursor's place, in the PET's own plain voice
 * as KeyItem's (docs/voice/rules.md); and each palette's one line on where
 * it comes from, shown while it is locked. */
#ifndef CW_LOOKS_TEXT_H
#define CW_LOOKS_TEXT_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
	LW_TITLE,         /* the band: COLOURS */
	LW_LOOK,          /* the list's first row, the saved looks' */
	LW_PLAIN,         /* the grid's last cell: the channel as BN6 draws it */
	LW_LOCKED,        /* a locked palette's mark */
	LW_INFORMATION,   /* the box's label */
	LW_COUNT
} LooksWord;
const char *looks_word(LooksWord w);

/* A channel's name ("Armour") and the parts of him it colours */
const char *looks_channel_name(int c);
/* "A", "B", "C" */
const char *looks_slot_name(int slot);

/* The box on the looks' row: which one he wears */
void looks_slot_words(char *out, size_t n, int slot);
/* ... on a channel: what it colours, and the colour it has (palette NULL: his own) */
void looks_channel_words(char *out, size_t n, int c, const char *palette, int colour);
/* ... on a palette's colour, or its plain cell (colour -1), and where the palette came from */
void looks_colour_words(char *out, size_t n, int palette, const char *name, int colour);
/* ... on a palette not owned: locked, and where it comes from */
void looks_locked_words(char *out, size_t n, int palette, const char *name);

#endif
