/* Which of a controller's buttons, triggers and stick directions is which
 * GBA button (issue #37): the map per layout family, pad.ini's text, the
 * face-button presets, the labels a pad has printed on its buttons, and
 * Android's Joy-Con mappings. Arithmetic alone, tested without SDL
 * (tests/test_core.c): a pad's inputs are numbered as SDL numbers its
 * buttons (SDL_CONTROLLER_BUTTON_*, held to that in pads.c), its axes'
 * directions after them, and named as SDL names them in its mapping
 * strings ("a", "leftshoulder", "lefttrigger", "-lefty"). */
#ifndef CW_PADMAP_H
#define CW_PADMAP_H

#include <stdbool.h>
#include <stdint.h>

enum {
	PAD_BUTTONS = 21,               /* SDL_CONTROLLER_BUTTON_MAX, since SDL 2.0.14 */
	PAD_AXES = 6,                   /* SDL_CONTROLLER_AXIS_MAX */
	PAD_AXIS_FIRST = PAD_BUTTONS,   /* an axis pushed one way: PAD_AXIS_FIRST + axis * 2, + 1 the positive way */
	PAD_INPUTS = PAD_AXIS_FIRST + PAD_AXES * 2,
	PAD_NONE = 255,
};
#define PAD_AXIS(axis, positive) (PAD_AXIS_FIRST + (axis) * 2 + ((positive) ? 1 : 0))
/* (SDL's buttons this file names) */
enum { PAD_A, PAD_B, PAD_X, PAD_Y, PAD_BACK, PAD_GUIDE, PAD_START, PAD_LEFTSTICK, PAD_RIGHTSTICK,
       PAD_LEFTSHOULDER, PAD_RIGHTSHOULDER, PAD_DPUP, PAD_DPDOWN, PAD_DPLEFT, PAD_DPRIGHT, PAD_MISC1 };
#define PAD_LEFTTRIGGER PAD_AXIS(4, true)
#define PAD_RIGHTTRIGGER PAD_AXIS(5, true)

/* What SDL's names mean on a pad: where an Xbox pad has the button (a at
 * the bottom, b on the right: Xbox, PlayStation and other pads, a browser's
 * every pad), or, on Nintendo's (with SDL 2's default labels), the button
 * marked so (a on the right). A map each, so a choice made on one kind of
 * pad keeps its place on the other. */
enum { PAD_FAMILY_XBOX, PAD_FAMILY_NINTENDO, PAD_FAMILIES };
/* What is printed on the buttons, for the screen's words */
enum { PAD_STYLE_GENERIC, PAD_STYLE_XBOX, PAD_STYLE_PLAYSTATION, PAD_STYLE_NINTENDO };

#define PAD_GBA 10   /* the GBA's buttons, a bit each in buttons.h's order */
#define PAD_PER 4    /* the most inputs one GBA button has */

/* Per family and GBA button its inputs, PAD_NONE after the last. */
typedef struct { uint8_t in[PAD_FAMILIES][PAD_GBA][PAD_PER]; } PadMap;

/* The defaults, the same names on both families: the D-pad and the left
 * stick move, a is A and b is B, the shoulders and triggers L and R,
 * start is START and back SELECT. */
void padmap_default(PadMap *m);
void padmap_default_family(PadMap *m, int family);
bool padmap_same(const PadMap *a, const PadMap *b);
/* The GBA buttons (BTN_*) an input presses on a family's pads. */
uint32_t padmap_bits(const PadMap *m, int family, int input);
/* `input` made GBA button `gba`'s only one; a button that had it gives it
 * up, and one left with none takes the inputs `gba` had (two swap), so
 * every GBA button keeps one. */
void padmap_bind(PadMap *m, int family, int gba, int input);
/* SDL's name for an input ("a", "-lefty", "lefttrigger") and back
 * (PAD_NONE for none such). */
const char *padmap_name(int input);
int padmap_input(const char *name);
/* The GBA button's name in the files ("UP" ... "SELECT") and back (-1). */
const char *padmap_gba_name(int gba);
int padmap_gba(const char *name);
/* The words on the button, in a pad's style: "A", "Cross", "LB", "ZL". */
const char *padmap_label(int style, int input);

/* The face-button presets: as labeled; on an Xbox-like pad B on X, left
 * of A as the GBA has it (issue #37, CybeastID); A and B swapped. */
int padmap_presets(int family);
const char *padmap_preset_name(int family, int style, int preset);
/* What it puts on A and B, in the style's words ("A, B on X") */
const char *padmap_preset_about(int family, int style, int preset);
void padmap_preset_apply(PadMap *m, int family, int preset);
/* The preset A and B are on now, or -1 (set by hand) */
int padmap_preset_of(const PadMap *m, int family);

/* pad.ini: [pad] and [nintendo], a line per GBA button, "A = a, x".
 * Parsed over the defaults (a family or button the file leaves out keeps
 * them); returns the lines not understood, said in `errors`. */
int padmap_parse(const char *text, PadMap *m, char *errors, int size);
int padmap_format(const PadMap *m, char *out, int size);

/* Android's Joy-Cons (SDL 2.32 makes no mapping for a left one, which has
 * no A, B, X or Y): the mapping after the GUID, a name and SDL's fields;
 * each its half of a pad where both are there, else a small pad held
 * sideways. NULL for another product. */
const char *padmap_joycon(int product, bool pair);

#endif
