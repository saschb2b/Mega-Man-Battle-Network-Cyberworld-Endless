/* Dev tools for testing a run quickly (devtools.c, docs/DEVTOOLS.md): a
 * menu over the game (hold SELECT, press R) and the same switches from the
 * command line (--dev). */
#ifndef CW_DEVTOOLS_H
#define CW_DEVTOOLS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	bool god;       /* MegaMan's HP stays full, in battle and out */
	bool onehit;    /* enemies keep 1 HP: one hit deletes them */
	bool quiet;     /* no random battles */
	int speed;      /* game frames per frame shown: 1, 2, 4, 8 */
} DevFlags;

extern DevFlags dev;

/* "god,onehit,quiet,speed=4" */
void devtools_parse(const char *spec);
/* The player's GBA keys: the menu takes them while it is open. */
uint32_t devtools_keys(uint32_t keys);
/* The menu is open: the game holds still. */
bool devtools_open(void);
/* After each game frame: the switches' effects. */
void devtools_update(void);
void devtools_draw(void);

/* A frame to save after the next draw (the tour sets it); "" for none. */
extern char devtools_shot[512];

#endif
