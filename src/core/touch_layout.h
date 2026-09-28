/* Where the touch controls stand on the canvas and what a finger there
 * holds: arithmetic alone, tested without SDL (tests/test_core.c). */
#ifndef CW_TOUCH_LAYOUT_H
#define CW_TOUCH_LAYOUT_H

#include <stdbool.h>
#include <stdint.h>

enum { TOUCH_DPAD, TOUCH_A, TOUCH_B, TOUCH_L, TOUCH_R, TOUCH_START, TOUCH_SELECT, TOUCH_CONTROLS };

/* A control's box on the canvas; the D-pad's and A's and B's is the square
 * round their circle. */
typedef struct { int x, y, w, h; } TouchBox;

typedef struct {
	TouchBox box[TOUCH_CONTROLS];
	bool below;   /* under the picture (a tall screen) */
	bool over;    /* no room under or beside it: over its corners, see-through */
} TouchLayout;

/* The largest whole scale up to `scale` that leaves the controls room
 * beside the picture on a wide screen (sw x sh pixels): the picture a size
 * smaller rather than under the thumbs. A tall screen keeps its scale. */
int touch_fit_scale(int sw, int sh, int scale);
/* A tall canvas (w x h) with the controls shown puts the picture this far
 * from its top, the controls under it; -1 where it stays in the middle. */
int touch_picture_top(int w, int h);
/* The controls on a w x h canvas whose 240x160 picture stands at (px, py). */
void touch_layout_for(int w, int h, int px, int py, TouchLayout *t);
/* The control at canvas (x, y), each reaching a little past its art, or -1. */
int touch_control_at(const TouchLayout *t, int x, int y);
/* The buttons (BTN_*) a finger at (x, y) holds. One that went down on the
 * D-pad (`from`) steers it wherever it slides; the others press what is
 * under them now, so a thumb rolls from B to A. */
uint32_t touch_hit(const TouchLayout *t, int x, int y, int from);

#endif
