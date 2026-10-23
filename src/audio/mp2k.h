/* MP2K, BN6's sound engine, run by the engine for the screens before and
 * beside the game (the title, the run's setup, Options, SAVES, the PET's
 * screens over the held game): the library's own logic read from BN6's
 * code (bn6f's disassembly of it, Nintendo's "m4a"; docs/ROM_DATA.md,
 * BN6's sound engine), its data the player's ROM's, so a song plays as BN6
 * plays it. mp2k_frame is one GBA frame of it: what the library's VCOUNT
 * interrupt runs (m4aSoundVSync, then SoundMain: every music player, the
 * PSG's driver, the mixer), its mix and the PSG's registers handed to the
 * GBA's sound (gba_sound.h). Its state is mp2k_state.h's. */
#ifndef CW_MP2K_H
#define CW_MP2K_H

#include <stdbool.h>
#include <stdint.h>

/* The engine as BN6's start leaves it (m4aSoundInit, MPlayExtender,
 * m4aSoundMode): false without BN6's ROM. */
bool mp2k_init(void);
/* A song of BN6's song table on the player the table names (m4aSongNumStart) */
void mp2k_song_start(int song);
/* The player that plays `song` faded out over 16 x `speed` frames
 * (m4aMPlayFadeOut) */
void mp2k_song_fade(int song, int speed);
/* Whether `song`'s player plays a song; whether anything sounds at all
 * (a player playing, a channel still on) */
bool mp2k_song_playing(int song);
bool mp2k_busy(void);
/* One GBA frame */
void mp2k_frame(void);
/* What it mixed this frame: per_frame samples of the right and left
 * channels (DMA A and B), and their count */
const int8_t *mp2k_mix_right(void);
const int8_t *mp2k_mix_left(void);
int mp2k_per_frame(void);

#endif
