/* How loud the game's sounds are heard, as settings.ini's `music` and
 * `effects` say (Options' Audio): its music, the songs on BN6's music
 * player (31), and its effects, the sounds on every other, each at a
 * level from 0 (none) to SOUND_LEVELS (the game's own), 3 dB a step. A
 * player turns the music off to play their own, the effects kept.
 *
 * The louder of the two is the device's gain, after everything is mixed
 * (sound_filter.h); the quieter is that gain's share in the game's own
 * volumes, which the core's players follow (sound_quiet.h, a hook on
 * BN6's own volume) and the engine's (mp2k.h's mp2k_shares_set), each
 * player's notes playing on, quieter or silent. So where both are turned
 * down alike, BN6's 8-bit mix plays as it always does, only quieter. */
#ifndef SOUND_HEARD_H
#define SOUND_HEARD_H

#define SOUND_LEVELS 10
#define SOUND_SHARE_ALL 256   /* a share of the louder level: all of it */

void sound_heard_set(int music, int effects);
int sound_heard_music(void);
int sound_heard_effects(void);
/* settings.ini's value: a level, or on (yes: the game's own) and off
 * (no), as it was written before the levels */
int sound_heard_read(const char *val);
/* a level's gain on the device: 1 at the game's own, 3 dB less a step
 * under it, 0 at none */
double sound_heard_gain(int level);
/* `level`'s share of `louder`'s, in 256ths of it (SOUND_SHARE_ALL where
 * they are alike, 0 for none): 3 dB less a step between them */
int sound_heard_share(int level, int louder);

#endif
