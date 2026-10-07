/* The controls screen's state, shared by its flow (controls.c) and its
 * picture (controls_draw.c). Laid out as the big games lay a remapper
 * (issue #112: The Last of Us Part II, Hades, Elden Ring, and RetroArch's
 * Set All): a tab per device, CONTROLLER and KEYBOARD, switched with L and
 * R; a row per GBA button, each with a MAIN and an ALSO slot and a dot
 * that lights while it is pressed, so the map is tried on the spot; the
 * D-pad's four directions one row, asked in turn; a slot asked for in a
 * box over the rows, a clash swapped and said; Set all, which asks every
 * button in turn for a pad nobody laid out; DEFAULTS per device; DONE,
 * kept once the new A is pressed. */
#ifndef CW_CONTROLS_STATE_H
#define CW_CONTROLS_STATE_H

#include <SDL.h>
#include <stdbool.h>

#include "padmap.h"
#include "platform.h"

/* (desktops and browsers have a keyboard to set as well) */
#if defined(CW_DESKTOP) || defined(__EMSCRIPTEN__)
#define KEYS 1
#else
#define KEYS 0
#endif

/* The list's rows (the statistics' last, where the build can send them),
 * then the bottom line's items */
enum { ROW_PRESET, ROW_DPAD, ROW_A, ROW_B, ROW_L, ROW_R, ROW_START, ROW_SELECT, ROW_STATS, ROWS, ROW_BOTTOM = ROWS };
enum { ITEM_SETALL, ITEM_DEFAULTS, ITEM_DONE, ITEMS };
enum { TAB_PAD, TAB_KEYS };
enum { SLOT_MAIN, SLOT_ALSO };
/* what a box over the rows asks for: a slot's input, the D-pad's four
 * directions for one slot, or Set all's ten buttons */
enum { ASK_NONE, ASK_SLOT, ASK_DPAD, ASK_ALL };

#define ASK_TIME 300   /* frames a box waits for a press (Set all: then the button is skipped) */
#define CONFIRM 600    /* ... and "press the new A to keep" */
#define NOTE 150       /* ... a passing word stays */
#define LIT 20         /* ... a row's dot stays lit after a press */
#define SETTLE 8       /* ... the last press is let go before the next is asked */

typedef struct {
	bool open;
	int tab, row, col, item, t;
	int ask, ask_gba, ask_slot, ask_step, ask_left, settle;
	int confirm;
	int family, style;            /* the pad shown: the one used last */
	bool has_pad;
	char name[40];
	PadMap pads, pads_was, pads_before;
	KeyMap keys, keys_was, keys_before;
	char said[72];
	int said_t;
	bool said_warn;
	int lit[PAD_GBA];
	SDL_FingerID finger;          /* (a finger that went down on it: only its lift taps) */
	bool finger_down;
} ControlsState;
extern ControlsState C;

/* The row's GBA button (ROW_A ... ROW_SELECT; the D-pad's directions
 * are UP 0 to RIGHT 3), and the rows shown */
int controls_gba_of(int row);
int controls_rows(void);
/* A slot's input in words, on the tab shown ("-" for none): one GBA
 * button's, or the D-pad's four as one ("D-pad", "L stick", "WASD") */
void controls_slot_words(int gba, int slot, char *out, size_t n);
/* A key's word, short where its name is long ("Enter", "Bksp", "KP 8") */
const char *controls_key_word(int scancode);
void controls_dpad_words(int slot, char *out, size_t n);
/* The words for the screen's own buttons on the device used last: A
 * (choose), B (back), X (clear), L and R (tabs) */
const char *controls_nav_word(int which);
enum { NAV_A, NAV_B, NAV_X, NAV_L, NAV_R };
/* Where a tap at canvas pixel (x, y) from the picture's corner lands: a
 * tab, a bottom item (-1 for none), or a row's slot (false for none) */
int controls_tap_tab(int x, int y);
int controls_tap_item(int x, int y);
bool controls_tap_cell(int x, int y, int *row, int *col);

#endif
