/* The screen a player is on, in a word, for a headless run's state
 * (capture.c's write_state; tools/cw.py reads it). */
#ifndef CW_SCREEN_NAME_H
#define CW_SCREEN_NAME_H

#include <stdio.h>

/* Ours over the game first (the dev menu, Options, the Jukebox, the
 * NetPass), then the scene's name ("title", "saves"), or in the game
 * BN6's screen ("map", "battle", "pet", "keyitem": second_screen_word).
 * It only looks. */
const char *screen_name(void);
/* What stands over the run that its director does not say, a fact a line
 * after the director's: online play's (online_describe.h: a visit, the
 * Square, a trade, a NetBattle, a dive's link). It only looks. */
void online_screen_describe(FILE *f);
/* The engine's own screens in that state, a fact a line: Options and what
 * shows over it (options_describe), SAVES (saves_describe), the ROMs read
 * and their screen (launcher_describe). They only look. */
void screens_describe(FILE *f);

#endif
