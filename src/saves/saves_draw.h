/* The SAVES screen's picture (saves_draw.c), in the PET panels' colours
 * (second_frame.h), and what the pointer and the cursor do over it. */
#ifndef CW_SAVES_DRAW_H
#define CW_SAVES_DRAW_H

#include <stdbool.h>

/* The view as SV has it */
void saves_draw(void);
/* The cursor moved in the direction (dx, dy) */
void saves_move_focus(int dx, int dy);
/* A tap or a click this frame: its button chosen, or `back` called; true
 * where there was one */
bool saves_pointer(bool (*back)(void));

#endif
