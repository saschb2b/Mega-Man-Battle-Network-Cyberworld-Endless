/* The colours screen's preview (looks_screen.h): BN6's own MegaMan in the
 * look, on a loop that shows him whole: his field sprite walking round all
 * eight directions, front, side and back, then his battle body standing,
 * firing the buster, throwing and firing a charged shot. Each sprite in its
 * own palette as the look makes it (look_palette_of), BN6's own where the
 * core gives none. The loop is run in the screen's update; the draw reads
 * it. */
#ifndef CW_LOOKS_PREVIEW_H
#define CW_LOOKS_PREVIEW_H

#include <stdbool.h>
#include <stdint.h>

#include "gfx.h"
#include "looks.h"

typedef struct {
	Anim anim;            /* the sprite and its frame now */
	int step, held;       /* the loop's step, and the frames it has been held */
	bool battle;          /* the step's sprite: the battle body, else the field's */
	uint8_t pal[2][32];   /* the field's and the battle's palettes in the look (GBA BGR555) */
	bool have[2];         /* ... where the core gave them */
} LooksPreview;

/* The loop from its start */
void looks_preview_start(LooksPreview *p);
/* The palettes for `look` (a change: the loop goes on where it is) */
void looks_preview_colours(LooksPreview *p, const Look *look);
/* A frame on */
void looks_preview_step(LooksPreview *p);
/* Drawn with his feet at (x, y); reads only */
void looks_preview_draw(const LooksPreview *p, int x, int y);

#endif
