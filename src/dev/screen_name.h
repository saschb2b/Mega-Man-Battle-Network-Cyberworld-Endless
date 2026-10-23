/* The screen a player is on, in a word, for a headless run's state
 * (capture.c's write_state; tools/cw.py reads it). */
#ifndef CW_SCREEN_NAME_H
#define CW_SCREEN_NAME_H

/* Ours over the game first (the dev menu, Options, the Jukebox, the
 * NetPass), then the scene's name ("title", "saves"), or in the game
 * BN6's screen ("map", "battle", "pet", "keyitem": second_screen_word).
 * It only looks. */
const char *screen_name(void);

#endif
