/* The second screen (issue #72): the 3DS's bottom screen, Android's second
 * display. It follows BN6's screen as the DS games' PET followed theirs:
 * the PET's frame, and in it the panel for the screen the player is on. */
#ifndef CW_SECOND_H
#define CW_SECOND_H

#include <stdbool.h>

/* Once a frame, after the game's: which screen the player is on, and what
 * its panel shows (the draw reads this, never the game's memory: on the
 * 3DS it runs while the core's thread runs the next frame). */
void second_update(void);
/* The second screen in w x h (platform_second_screen's): false for dark. */
bool second_draw(int w, int h);
/* Whether its picture would differ from the last drawn (the map's always
 * may: MegaMan's mark pulses on it). */
bool second_changed(void);
/* The title's: the PET at rest, the profile's record (second_home.c,
 * issue #79); its picture changes only as the title is entered again. */
bool second_title_draw(int w, int h);
/* (the title drew over the run's frame: it is drawn whole next time) */
void second_frame_forget(void);
bool second_title_changed(void);

#endif
