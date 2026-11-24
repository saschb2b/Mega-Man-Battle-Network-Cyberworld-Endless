/* The colours screen's preview (looks_screen.h): BN6's own MegaMan in the
 * look, his chat box's face over his body. On the list the body runs a
 * loop that shows him whole: his field sprite walking round all eight
 * directions, front, side and back, then his battle body standing, firing
 * the buster, throwing and firing a charged shot. In the grid it holds
 * his battle stance, the body that shows every channel largest, with a
 * shot of the buster on each change. A channel chosen flashes on him.
 * Each sprite in its own palette as the look makes it (look_palette_of),
 * BN6's own from the player's ROM where the core gives none. Its course is
 * run in the screen's update; the draw reads it. */
#ifndef CW_LOOKS_PREVIEW_H
#define CW_LOOKS_PREVIEW_H

#include <stdbool.h>
#include <stdint.h>

#include "gfx.h"
#include "looks.h"

enum { LOOKS_ART_FIELD, LOOKS_ART_BATTLE, LOOKS_ART_FACE, LOOKS_ARTS };

typedef struct {
	Anim anim;                   /* the body and its frame now */
	int step, held;              /* the course's step, and the frames it has been held */
	bool hold;                   /* the grid's course, else the list's loop */
	bool battle;                 /* the step's body: the battle's, else the field's */
	int mark, mark_t;            /* a channel flashing on him, and its frames left */
	uint8_t pal[LOOKS_ARTS][32]; /* the field's, the battle's and the face's palettes in the look (GBA BGR555) */
	bool have[LOOKS_ARTS];
} LooksPreview;

/* The list's loop from its start, his front walking */
void looks_preview_loop(LooksPreview *p);
/* The grid's course: his battle stance, a shot of the buster first */
void looks_preview_hold(LooksPreview *p);
/* The look changed whole (another look worn): seen from the front at once */
void looks_preview_front(LooksPreview *p);
/* Channel `c` flashed on him a moment */
void looks_preview_mark(LooksPreview *p, int c);
/* The palettes for `look` (the course goes on where it is) */
void looks_preview_colours(LooksPreview *p, const Look *look);
/* A frame on */
void looks_preview_step(LooksPreview *p);
/* Drawn into the slot's field `x, y, w, h`: the face at its top, the body's feet at its foot; reads only */
void looks_preview_draw(const LooksPreview *p, int x, int y, int w, int h);

#endif
