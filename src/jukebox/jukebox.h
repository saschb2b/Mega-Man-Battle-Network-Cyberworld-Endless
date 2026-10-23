/* The Jukebox (docs/PET.md, The Jukebox): a PET screen over the held game,
 * opened from its key item, that plays every song of the music player of
 * the games in use (BN6's, BN5's where its ROM is beside it), by game and
 * section, on the engine's own MP2K (mp2k.h), BN6's code. The game holds
 * still under it, its music with it, and goes on where it was once the
 * Jukebox closes. L and R turn the sections, Up and Down pick, A plays,
 * START turns the play mode, B closes; a tap does each. The last section,
 * song and mode are kept in the data folder's jukebox.ini. */
#ifndef CW_JUKEBOX_H
#define CW_JUKEBOX_H

#include <stdbool.h>

/* the play modes: the song over and over, the section in its order, the
 * section in any order */
typedef enum { JUKE_REPEAT, JUKE_IN_ORDER, JUKE_SHUFFLE, JUKE_MODES } JukeMode;

void jukebox_open(void);
bool jukebox_shown(void);
/* The frame's course while it is up, or its A or B still held after it
 * closed: true while it holds the game */
bool jukebox_update(void);
/* ... drawn over the game's picture (P.core_x, P.core_y) while it is up */
void jukebox_draw(void);
/* Put away at once, its song stopped: the game's scene left (a soft reset, a quit) */
void jukebox_shut(void);

#endif
