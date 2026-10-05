/* The older net's wait (docs/MULTIROM.md, Guest battles: its boot): where a
 * guest battle must wait for BN5's first boot, a screen on the battle
 * switch's white in BN6's own look, drawn at runtime: the battle HUD's
 * gauge filling as the boot runs, MegaMan in BN6's chat box saying what
 * happens, the net's light streaming past. It opens only where the wait is
 * long enough to need it, and closes as the boot ends, the battle opening
 * behind it: it never holds the player longer than the boot. */
#ifndef CW_GUEST_WAIT_H
#define CW_GUEST_WAIT_H

#include <stdbool.h>

/* A frame of it (the scene's update): `waiting` while a battle waits for
 * the boot (guest_boot_waiting), `white` once the switch's white is whole */
void guest_wait_update(bool waiting, bool white);
/* Drawn over the white, the picture's top-left at (x0, y0), while it
 * shows (opening, open or closing); nothing of the game's state changes */
void guest_wait_draw(int x0, int y0);

#endif
