/* The statistics' question (analytics.h), in the PET's panel the setup and
 * the controls screen have: what is sent and what never is, Yes and No,
 * the cursor on No, so a START or A pressed through the start's screens
 * answers no, and a moment passes before it takes a press at all. Shown
 * in the controls screen's place (controls.c hosts it: its frame, its
 * picture, Escape and taps), over the title at the first start where the
 * build can send, and on the 3DS at SELECT on the title, where it is the
 * one setting (the 3DS's buttons are the console's own). */
#ifndef CW_ANALYTICS_ASK_H
#define CW_ANALYTICS_ASK_H

#include <stdbool.h>

/* The title's first entry: the question, where the build can send and the
 * player has not answered */
void analytics_ask_offer(void);
/* Asked for (the 3DS's SELECT): the question, the cursor on the answer as
 * it stands; false where the build cannot send */
bool analytics_ask_open(void);
/* Whether it can be: the build can send (the 3DS's title says its SELECT) */
bool analytics_ask_here(void);
bool analytics_ask_shown(void);
/* Its frame in the scene's place, and its picture over the canvas
 * (drawing changes nothing) */
void analytics_ask_update(void);
void analytics_ask_draw(void);
/* Escape or a phone's Back: no, at the first start; else away, the answer
 * as it was */
bool analytics_ask_back(void);
/* A finger lifted at (x, y), canvas pixels from the picture's corner */
void analytics_ask_tap(int x, int y);

#endif
