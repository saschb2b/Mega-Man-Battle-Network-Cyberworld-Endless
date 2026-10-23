/* settings.ini's music and effects in the core (sound_heard.h): a hook on
 * BN6's TrkVolPitSet, where a track's volume is worked out from its own,
 * the fade's and its pan, gives a silenced player's tracks none, right
 * and left. Its notes play on silent: their channels, their priorities
 * and every sound of the players heard as before. A change, or a state
 * loaded with the volumes another setting left, has the tracks work their
 * volume out again before the next frame. On BN6's own core alone: the
 * Arena's two cores make the same sound on both players' systems, their
 * states hashed whole (arena_pair.c), and there the device's sound is
 * silenced only where both are off (audio.c). */
#ifndef CW_SOUND_QUIET_H
#define CW_SOUND_QUIET_H

/* The hook, once (game_hooks.c) */
void sound_quiet_install(void);
/* Before each frame of BN6's core (emu_frame): a change taken in */
void sound_quiet_frame(void);
/* A state loaded into it (emu.c): every track told at the next frame */
void sound_quiet_loaded(void);

#endif
