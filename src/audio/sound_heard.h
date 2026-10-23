/* Which of the game's sounds are heard, as settings.ini's `music` and
 * `effects` say (Options' Audio): its music, the songs on BN6's music
 * player (31), and its effects, the sounds on every other. A player
 * turns the music off to play their own, the effects kept. The core's
 * players follow it (sound_quiet.h, a hook on BN6's own volume) and the
 * engine's (mp2k.h's mp2k_quiet), each player's notes playing on
 * silent, so what is heard of the rest is as before. */
#ifndef SOUND_HEARD_H
#define SOUND_HEARD_H

#include <stdbool.h>

void sound_heard_set(bool music, bool effects);
bool sound_heard_music(void);
bool sound_heard_effects(void);

#endif
