/* Fast forward on the running game (fast_forward.h's rules, docs/GUIDE.md):
 * the button read once an update, BN6's state read before each frame of a
 * batch, the batch kept to the update's time, and the mark that shows it
 * running. */
#ifndef CW_FAST_FORWARD_GAME_H
#define CW_FAST_FORWARD_GAME_H

#include <stdbool.h>

/* Once an update, before its frames: the button, and the batch begun */
void fast_forward_update(void);
/* Before frame `i` of the update's batch (0 its first, always played):
 * whether it is played (BN6's state allows it, the speed and the time
 * have room), and how (EMU_FRAME_*: unshown where another is to follow,
 * unheard past the first) */
bool fast_forward_goes_on(int i);
unsigned fast_forward_how(int i);
/* Whether this update's batch runs fast (its mark shows) */
bool fast_forward_running(void);
/* Whether it could run where the game is now, turned on or not (the
 * place allows it, Options' speed not off): elsewhere its button does
 * nothing and the touch controls hide theirs */
bool fast_forward_available(void);
/* The mark over the picture's corner while it runs (drawing only) */
void fast_forward_draw(void);
/* ... the same ▶▶, the PET's orange cursor twice, held still, about `h`
 * pixels high round (cx, cy) at `alpha` 0-1: the touch controls' button */
void fast_forward_glyph(float cx, float cy, float h, float alpha);

#endif
