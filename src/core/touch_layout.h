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
/* The screen shapes the controls are laid out for, each arranged apart
 * (a phone turned is the other one): under the picture, beside it, over
 * its corners. */
enum { TOUCH_SHAPE_BELOW, TOUCH_SHAPE_SIDE, TOUCH_SHAPE_OVER, TOUCH_SHAPES };
int touch_shape(const TouchLayout *t);

/* The player's arrangement (touch.ini, the touch controls' editor): per
 * shape and control, its middle in thousandths of the canvas's width and
 * height where moved, and its size in percent of the laid-out one (0 as
 * laid out). */
typedef struct { int16_t x, y, size; bool moved; } TouchPlace;
typedef struct { TouchPlace place[TOUCH_SHAPES][TOUCH_CONTROLS]; } TouchCustom;
#define TOUCH_SIZE_MIN 50
#define TOUCH_SIZE_MAX 200
/* t, laid out by touch_layout_for on a w x h canvas, as the player
 * arranged it: sized, moved, and kept whole on the canvas. */
void touch_layout_custom(TouchLayout *t, int w, int h, const TouchCustom *c);
/* touch.ini's lines ("below a 850 470 120", "-" for a place not moved);
 * others are passed over. Formatting gives the bytes written (at most
 * size - 1). */
void touch_custom_parse(const char *text, TouchCustom *c);
int touch_custom_format(const TouchCustom *c, char *out, int size);

/* The control at canvas (x, y), each reaching a little past its art, or -1. */
int touch_control_at(const TouchLayout *t, int x, int y);
/* The buttons (BTN_*) a finger at (x, y) holds. One that went down on the
 * D-pad (`from`) steers it wherever it slides; the others press what is
 * under them now, so a thumb rolls from B to A. */
uint32_t touch_hit(const TouchLayout *t, int x, int y, int from);
/* The directions a finger steering the D-pad holds at (x, y), `last` those
 * it held a moment ago: nothing in the middle, eight directions from a
 * third of the way out, and what it held in between. */
uint32_t touch_dpad_steer(const TouchLayout *t, int x, int y, uint32_t last);

#endif
