/* The emote strip (issue #178; docs/ONLINE.md, Emotes): SELECT in the Net
 * Square opens a PET screen of the project's own over the bottom of BN6's
 * picture, laid out as the Custom screen lays out its chips (BN6 has no
 * screen for emotes; its art is not copied): the twelve emotes as icons in
 * two rows, each MegaMan's own pose drawn from the player's ROM as it is
 * drawn, and the one under the cursor played by MegaMan in a window above
 * with its name and whom it would go to. L and R turn to the phrases' page
 * and back. A sends the one under the cursor and closes the strip, so the
 * Square's answer to it is seen whole; B or SELECT closes it. SELECT held
 * with a direction keeps the one under the cursor on L and that direction
 * (network.ini's `favourites`), the box at the right showing the four. On
 * touch, a tap on an icon sends it, on a tab turns the page, on a
 * favourite sends that one, and on the window closes it. The Square goes on
 * behind it; MegaMan stands while it is up, as BN6 holds him in a talk. */
#ifndef CW_EMOTE_STRIP_H
#define CW_EMOTE_STRIP_H

#include <stdbool.h>
#include <stdint.h>

#include "online_types.h"

/* As MegaMan arrives in the Square (closed, its cursor on Wave; the
 * second screen's palette too, emote_palette.h). */
void emote_strip_reset(void);
void emote_strip_open(void);
void emote_strip_close(void);
bool emote_strip_shown(void);
/* The frame's course (the keys and taps while it is up, the palette's
 * taps on the second screen always), `s` the Square as shown: true while
 * it holds the game's keys (and the moment after it closed, its last key
 * still held). */
bool emote_strip_update(const OnlineSnapshot *s);
/* ... drawn over the game's picture while it is up. */
void emote_strip_draw(void);

#endif
