/* STUB (looks-screen's branch, until looks-core's src/core/looks.h is on
 * dev; dropped then): MegaMan's look, issue #854's agreed interface. A
 * profile's three saved looks, each a palette and a colour for each of his
 * colour channels, from the palettes the profile owns. */
#ifndef CW_LOOKS_H
#define CW_LOOKS_H

#include <stdbool.h>
#include <stdint.h>

#define LOOK_CHANNELS 3     /* 0 armour, 1 suit, 2 accent */
#define LOOK_PALETTES 12
#define LOOK_COLOURS 16     /* a palette's colours, a 4x4 grid */
#define LOOK_SLOTS 3        /* A, B, C */
#define LOOK_NONE 0xFF      /* the channel as BN6 draws it */

typedef struct {
	uint8_t palette[LOOK_CHANNELS], colour[LOOK_CHANNELS];
} Look;

enum LookPalette { LOOK_CLASSIC, LOOK_UNDERNET, LOOK_GREGAR, LOOK_PROTOMAN, LOOK_NEBULA, LOOK_TOURNAMENT, LOOK_HAZARD, LOOK_CENTRAL, LOOK_SEASIDE, LOOK_SKY, LOOK_MYSTERY, LOOK_GLITCH };

const char *looks_palette_name(int p);
uint16_t looks_colour(int p, int i);   /* RGB555 */
bool looks_owned(int p);
bool looks_grant(int p);
int looks_slot(void);
void looks_select(int slot);
Look looks_get(int slot);
void looks_set(int slot, const Look *look);
Look looks_worn(void);
bool look_valid(const Look *look);

#endif
