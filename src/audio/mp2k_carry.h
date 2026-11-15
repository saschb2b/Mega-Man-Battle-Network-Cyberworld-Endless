/* BN6's music handed from the game's own MP2K to the engine's and back
 * (docs/engine/emulation/engine-sound.md, The game's music under the
 * engine's screens): the music's player (31) and its eight tracks, and the
 * channels their notes sound on, as the game keeps them (mp2k_layout.h),
 * taken by the engine's player 31, so its song plays on from where the
 * game was held; and given back, laid out the same, where the engine has
 * brought it. The two run the same code byte for byte
 * (tools/mp2k_verify.py), so the song goes on as the game would have
 * played it. A channel sounding another's sound (an effect of the
 * engine's as the song is taken, of the game's as it is given back) keeps
 * it: the music's note there is let go. ROM-free but for the song's data,
 * which the engine reads from the game's ROM copy (mp2k_game_rom). */
#ifndef CW_MP2K_CARRY_H
#define CW_MP2K_CARRY_H

#include <stdbool.h>
#include <stdint.h>

#include "mp2k_layout.h"
#include "mp2k_state.h"

#define MP2K_CARRY_TRACKS 8   /* the music player's (BN6_MPLAYS' row 31) */

/* The game's music as its RAM holds it */
typedef struct {
	uint8_t player[MP2K_PLAYER_BYTES];                    /* its MusicPlayerInfo */
	uint8_t track[MP2K_CARRY_TRACKS][MP2K_TRACK_BYTES];   /* its tracks, where mp2k_track_addr has the engine's player 31's */
	uint8_t ds[MP2K_DS_MAX][MP2K_CHAN_BYTES];             /* SoundInfo's DirectSound channels (4 in use) */
	uint8_t cgb[MP2K_CGB][MP2K_CHAN_BYTES];               /* the PSG's */
	uint8_t reverb, c15;                                  /* SoundInfo's reverb and the PSG's 15-frame count */
} Mp2kGameMusic;

/* A word the game's RAM is to take besides: the first channel of another
 * player's track whose list a channel given back left */
typedef struct {
	uint32_t at, value;
} Mp2kWrite;
#define MP2K_CARRY_WRITES (MP2K_DS_MAX + MP2K_CGB)

/* Where the game keeps the music player's track `i` (0 without the engine) */
uint32_t mp2k_carry_track_at(int i);
/* Whether the game's music player plays a song in `g` (not stopped, a
 * track on) */
bool mp2k_carry_plays(const Mp2kGameMusic *g);
/* The engine's player 31 takes `g`'s song where it stands, its notes
 * sounding on, the PSG's set going on the engine's; false where there is
 * no engine (no BN6 ROM) */
bool mp2k_carry_in(const Mp2kGameMusic *g);
/* ... and gives it back: `g` (as it was taken) made what the engine has
 * played since, the PSG's notes set on `to` (the game's registers), the
 * words of other tracks' lists the game must change too into `writes`
 * (MP2K_CARRY_WRITES at most), how many returned; then the engine's
 * player 31 stopped, its notes cut. -1, `g` untouched, where the engine
 * holds no song taken. */
int mp2k_carry_out(Mp2kGameMusic *g, const Mp2kRegs *to, Mp2kWrite *writes);
/* The engine's player 31 stopped, its notes cut, nothing given back */
void mp2k_carry_drop(void);

#endif
