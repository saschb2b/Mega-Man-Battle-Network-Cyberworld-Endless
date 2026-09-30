/* Where the touch controls stand on the screen and what a finger there
 * holds: arithmetic alone, tested without SDL (tests/test_core.c). Places
 * are in the screen's own pixels and sizes in dp, a 160th of an inch
 * (a millimetre is 6.3), turned into pixels by the screen's density, so a
 * button is as big under the thumb on every phone. */
#ifndef CW_TOUCH_LAYOUT_H
#define CW_TOUCH_LAYOUT_H

#include <stdbool.h>
#include <stdint.h>

enum { TOUCH_DPAD, TOUCH_A, TOUCH_B, TOUCH_L, TOUCH_R, TOUCH_START, TOUCH_SELECT, TOUCH_MENU, TOUCH_CONTROLS };
/* The three arrangements, each the player's own: under the picture (a
 * screen held upright), beside it (on its side), and over its edges where
 * neither has room. */
enum { TOUCH_BELOW, TOUCH_SIDE, TOUCH_OVER, TOUCH_SHAPES };

/* A control's middle and size in screen pixels; a round one's (the D-pad,
 * A, B) w and h are its diameter. */
typedef struct { float cx, cy, w, h; } TouchBox;

/* The player's arrangement (touch.ini; the touch menu and its editor): all
 * the controls' size and opacity in percent, haptics, the D-pad on the
 * right (left-handed); and per arrangement and control its middle in
 * thousandths of the screen where moved, its own size and opacity in
 * percent (0: as the rest). */
typedef struct { int16_t x, y, size, alpha; bool moved; } TouchPlace;
typedef struct {
	int size, opacity;
	bool haptics, left_handed;
	TouchPlace place[TOUCH_SHAPES][TOUCH_CONTROLS];
} TouchPrefs;
#define TOUCH_SIZE_MIN 60
#define TOUCH_SIZE_MAX 180
#define TOUCH_ALPHA_MIN 20
void touch_prefs_default(TouchPrefs *p);

/* The screen (w x h pixels, dp pixels to a dp) and the game's picture on
 * it (px, py, pw x ph). */
typedef struct { int w, h; float dp; int px, py, pw, ph; } TouchScreen;

typedef struct {
	TouchBox box[TOUCH_CONTROLS];
	float alpha[TOUCH_CONTROLS];   /* 0-1 */
	int shape;
	float dp, unit;                /* pixels to a dp, and to a dp of the controls' size */
	int size_max;                  /* the most of their size (percent) that fits */
} TouchLayout;

/* The whole scale for the 240 x 160 picture on a sw x sh screen with the
 * controls shown, at most `scale`: upright, the largest that leaves them
 * room under it; on its side, beside it; else `scale`, and they stand over
 * its edges. The picture keeps at least half the screen's width (upright)
 * or height. */
int touch_fit_scale(int sw, int sh, float dp, int scale);
/* Upright with room under the picture at `scale`: its top in screen
 * pixels, clear of the status bar; else -1 (it stays in the middle). */
int touch_picture_top(int sw, int sh, float dp, int scale);
/* The controls for screen s as arranged in p. */
void touch_layout_for(const TouchScreen *s, const TouchPrefs *p, TouchLayout *t);
/* The control a finger at (x, y) reaches, or -1. Each reaches past its art
 * (A and B by a third of their radius, the rest by 10 dp) and the one the
 * finger held (`held`, or -1) further, so a thumb's wobble keeps it;
 * between A and B, the nearer. */
int touch_control_at(const TouchLayout *t, float x, float y, int held);
/* The directions (BTN_*) a thumb steering the D-pad holds at (x, y), `last`
 * those it held: nothing within an eighth of the middle, `last` out to a
 * fifth, then eight directions of 45 degrees (four of 90 where diagonals
 * only get in the way), each kept 8 degrees past its edge so a thumb on
 * the line does not flicker. A thumb slid off the D-pad still steers it. */
uint32_t touch_dpad_steer(const TouchLayout *t, float x, float y, uint32_t last, bool four_way);
/* The button (BTN_*) a control presses; 0 for the D-pad and MENU. */
uint32_t touch_button(int control);
/* touch.ini's text, read (unknown lines passed over, missing or bad ones
 * as the defaults) and written (the bytes, at most size - 1). */
void touch_prefs_parse(const char *text, TouchPrefs *p);
int touch_prefs_format(const TouchPrefs *p, char *out, int size);
/* A control's name in touch.ini and on the editor. */
const char *touch_control_name(int control);

#endif
