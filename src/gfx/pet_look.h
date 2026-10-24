/* BN6's PET screens' look for the screens the engine draws where BN6 has
 * none (pet_look.c): the screen's rails and header band, the framed navy
 * window, the information box, the cursor, and the letters each part
 * takes, measured from Gregar's own KeyItem, E-Mail, ChipFolder and
 * SubChip screens (docs/FIDELITY.md, The PET's look). Everything is drawn
 * with fill_rects, so it draws into memory too (the second screen). */
#ifndef CW_PET_LOOK_H
#define CW_PET_LOOK_H

#include <SDL.h>
#include <stdbool.h>

#include "gfx.h"

#define LOOK_BAND      rgba(0, 123, 49, 255)    /* the header band */
#define LOOK_GREEN     rgba(8, 189, 115, 255)   /* the screen's body, the band's stripes */
#define LOOK_GREEN_LO  rgba(0, 123, 74, 255)    /* outlines and shadows on the body */
#define LOOK_GREEN_HI  rgba(74, 231, 115, 255)  /* the information box */
#define LOOK_METAL_HI  rgba(198, 222, 239, 255) /* the rails round the screen */
#define LOOK_METAL     rgba(165, 189, 214, 255)
#define LOOK_METAL_LO  rgba(99, 132, 148, 255)
#define LOOK_EDGE_HI   rgba(107, 222, 255, 255) /* a window's light edge */
#define LOOK_EDGE      rgba(66, 198, 231, 255)  /* ... its band */
#define LOOK_EDGE_LO   rgba(16, 140, 239, 255)  /* ... its blue edge */
#define LOOK_NAVY      rgba(16, 82, 107, 255)   /* a window's field */
#define LOOK_NAVY_LO   rgba(10, 58, 80, 255)    /* ... where what it holds is not yet had */
#define LOOK_SLOT      rgba(16, 99, 107, 255)   /* a value's slot (MegaMan's HP) */
#define LOOK_TEXT      rgba(247, 255, 247, 255) /* a list's letters */
#define LOOK_SHADE     rgba(33, 115, 140, 255)  /* ... their shade on the field */
#define LOOK_PAGE      rgba(247, 247, 247, 255) /* the information's page, the band's words */
#define LOOK_INK       rgba(66, 57, 57, 255)    /* the page's letters */
#define LOOK_INK_SHADE rgba(222, 231, 222, 255)
#define LOOK_ORANGE    rgba(247, 173, 33, 255)  /* a hint's action (E-Mail's SORT) */
#define LOOK_GOLD      rgba(255, 206, 24, 255)  /* the cursor's, for what is marked */
#define LOOK_DIM       rgba(99, 156, 173, 255)  /* a list's line that cannot be taken */

/* The screen in w x h from (x, y): the metal rails round it, the dark
 * band with `title` in heavy capitals and its stripes running off to the
 * right, the green body: the body inside the rails, under the band */
#define LOOK_BAND_H 14
#define LOOK_RAIL 5
SDL_Rect look_screen(int x, int y, int w, int h, const char *title);
/* ... its band alone, w wide (the rails' ends in it), LOOK_BAND_H + 1 tall */
void look_band(int x, int y, int w, const char *title);
/* ... its stripes from (sx, y + 6) to x `right`, running down-left to the
 * band's foot `h` rows down */
void look_stripes(int sx, int y, int right, int h);
/* ... a note at its right end where a hint goes, small capitals in `c` */
void look_band_note(int x, int y, int w, const char *s, SDL_Color c);
/* A key's hint at the band's right end (E-Mail's "START:SORT"): `key`
 * and a colon in white, `what` in orange, small capitals with their
 * shade: where it lies (a tap's place) */
SDL_Rect look_band_hint(int x, int y, int w, const char *key, const char *what);
/* ... such a hint anywhere on the body, from (x, y), and its width */
void look_hint(int x, int y, const char *key, const char *what);
int look_hint_width(const char *key, const char *what);

/* A window: a raised edge (light, then blue) round a cyan band round the
 * navy field, sunk in it (blue, then light), the corners rounded, its
 * shadow on the body a pixel right and below. `top` more rows of band
 * over the field (a title's), `bar` a scroll bar's track in its right
 * band: the field */
#define LOOK_FRAME 5
#define LOOK_FRAME_BAR 9
SDL_Rect look_window(int x, int y, int w, int h, int top, bool bar);
/* ... a slim one, for a small panel: one pixel of edge, one of band, one
 * sunk (under 16 tall the edge alone), `fill` the field: the field */
SDL_Rect look_box(int x, int y, int w, int h, SDL_Color fill);
/* The scroll bar's thumb in the track of the window `win` (look_window's
 * with `bar`, `top` as it had): `shown` of `n` lines from `first` */
void look_scroll(SDL_Rect win, int top, int first, int shown, int n);
/* BN6 keeps the window above for a screen's one list. What else a screen
 * holds takes the kind BN6 gives it (docs/UI.md, A screen's parts): */
/* ... a chip: the Library's and the folder editor's card, grey with its
 * top left corner cut, outlined, the contacts' gold along its foot: the
 * card's face above the contacts */
#define LOOK_CARD_FOOT 10
#define LOOK_CARD_GREY  rgba(198, 198, 214, 255)
#define LOOK_CARD_DARK  rgba(41, 41, 41, 255)   /* the picture's frame, the code's strip */
#define LOOK_CARD_INK   rgba(41, 41, 41, 255)
SDL_Rect look_card(int x, int y, int w, int h);
/* ... a thing to press: STATUS's pill, cyan with pointed ends, 15 tall,
 * its words the caller's (white, LOOK_SHADE under them) */
#define LOOK_PILL_H 15
void look_pill(int x, int y, int w);
/* ... the one action: NaviCust's RUN!, bright green in a light edge */
void look_button(int x, int y, int w, int h);
/* ... words that explain: SubChip's flat dark green panel, no frame: the
 * panel; letters on it white, LOOK_PANEL_SHADE under them */
#define LOOK_PANEL_SHADE rgba(0, 74, 41, 255)
SDL_Rect look_panel(int x, int y, int w, int h);
/* ... a value or a picture: STATUS's slot, navy in a white edge: the field */
SDL_Rect look_slot(int x, int y, int w, int h);
/* ... and what a screen marks as its own: the ChipFolder's EQUIP corners,
 * orange, round x, y, w, h */
void look_corners(int x, int y, int w, int h);

/* The information box: light green, outlined, `label` in small capitals
 * on its top left: the room under the label */
SDL_Rect look_info(int x, int y, int w, int h, const char *label);
/* BN6's NAVIGATOR box (E-Mail's, STATUS's, NaviCust's question): the
 * information box with `label` (the caller's word, NAVIGATOR), the face
 * `face` (a mugshot of the ROM's, FACE_MEGAMAN) on its left, the white
 * page beside it for his words, LOOK_NAV_H tall from (x, y): the page.
 * His words go on it in the chat font, LOOK_NAV_LINE apart, three at most */
#define LOOK_NAV_H 59
#define LOOK_NAV_LINE 14
SDL_Rect look_navigator(int x, int y, int w, const char *label, int face);
/* A tag on the information box (its KEYITEM): a green pill, `s` in small
 * capitals, white, in its middle */
void look_tag(int x, int y, int w, int h, const char *s, SDL_Color fill);
/* What is new, as E-Mail marks a new mail: `s` (NEW) in small capitals,
 * orange over their dark red, from (x, y) (3 down on a list's line) */
#define LOOK_NEW       rgba(255, 173, 41, 255)
#define LOOK_NEW_SHADE rgba(165, 49, 74, 255)
void look_new(int x, int y, const char *s);

/* The cursor: an orange arrow pointing right, 9 x 10 from (x, y), as
 * KeyItem's steps: one pixel right, back, two right, six frames each */
#define LOOK_CURSOR_W 9
void look_cursor(int x, int y);
/* ... held still, each of its pixels k x k, at `alpha` (the touch
 * controls' fast forward button, ▶▶) */
void look_cursor_scaled(int x, int y, int k, uint8_t alpha);
/* A screen opening, as KeyItem's: up from black over LOOK_FADE_IN frames
 * in the GBA's sixteen steps, over x, y, w, h; `t` frames since it opened */
#define LOOK_FADE_IN 15
void look_fade_in(int x, int y, int w, int h, int t);
/* The choice's cursor, BN6's green triangle before Yes and No (and
 * NaviCust's RUN!): 9 x 14 from (x, y), its shade on a page */
#define LOOK_CHOICE_W 9
void look_choice(int x, int y);
/* A page's arrow, BN6's green one at a window band's end where L or R
 * turns to another page (the Library's ▶, the folder editor's Pack):
 * 8 x 8 from (x, y), pointing right or left */
#define LOOK_ARROW_W 8
void look_arrow(int x, int y, bool right);
/* The PET's small arrow beside a value the d-pad turns (Options' lines, the
 * JACK-IN SETUP's rows), or a section's chevron: `size` pixels long,
 * pointing left or right from (x, y + 2) */
void look_pointer(int x, int y, int size, bool left, SDL_Color c);

/* Small capitals, 5 rows (BN6's labels: INFORMATION, START:SORT), `shade`
 * a pixel right and below (alpha 0: none) */
#define LOOK_CAPS_H 5
int look_caps_width(const char *s);
void look_caps(int x, int y, const char *s, SDL_Color c, SDL_Color shade);
/* minifont's small letters on the body, `c` with the body's dark shade under them */
void look_note(int x, int y, const char *s, SDL_Color c);
/* Heavy capitals, 7 rows (a screen's name on its band) */
int look_title_width(const char *s);
void look_title(int x, int y, const char *s, SDL_Color c);
/* A list's letters: the chat font's, thin, read from the player's ROM, in
 * `c` with `shade` under them; `align` as text_draw's */
#define LOOK_TEXT_H 12
int look_text_width(const char *s);
void look_text(int x, int y, const char *s, SDL_Color c, SDL_Color shade, int align);
/* ... in lines no wider than `w` (a '|' breaks one too), at most `most`
 * (6), LOOK_LINE apart: the y under them */
#define LOOK_LINE 13
int look_wrapped(const char *s, int x, int y, int w, int most, SDL_Color c, SDL_Color shade);

#endif
