/* MP2K's state (mp2k_state.h) laid out as BN6 keeps it in its RAM, and
 * read back from there: SoundChannel and CgbChannel, MusicPlayerTrack and
 * MusicPlayerInfo, each field at the offset mp2k_state.h names
 * (docs/rom/sound.md, BN6's sound engine). --mp2k-trace's records are
 * written with it (mp2k_trace.h), and the game's music is handed between
 * the game and the engine with it (mp2k_carry.h). */
#ifndef CW_MP2K_LAYOUT_H
#define CW_MP2K_LAYOUT_H

#include <stdbool.h>
#include <stdint.h>

#include "mp2k_state.h"

#define MP2K_CHAN_BYTES   0x40
#define MP2K_TRACK_BYTES  0x50
#define MP2K_PLAYER_BYTES 0x40
/* a channel's track and its neighbours on that track's list (words) */
#define MP2K_CHAN_TRACK   0x2C
#define MP2K_CHAN_PREV    0x30
#define MP2K_CHAN_NEXT    0x34
/* a track's list of channels: its first (a word) */
#define MP2K_TRACK_CHANS  0x20

/* Where BN6 keeps channel `c` (the DirectSound ones from 0, the PSG's from
 * MP2K_DS_MAX) and track `t` (any player's, mp2k.c's players_open): 0 for
 * MP2K_NONE; and back, MP2K_NONE where nothing is kept at `addr`. */
uint32_t mp2k_chan_addr(int c);
uint32_t mp2k_track_addr(int t);
int mp2k_chan_at(uint32_t addr);
int mp2k_track_at(uint32_t addr);

/* Laid out as BN6 keeps them: every field, the lists by BN6's addresses */
void mp2k_put_chan(uint8_t *b, const Mp2kChan *c, bool cgb);
void mp2k_put_track(uint8_t *b, const Mp2kTrack *t);
void mp2k_put_player(uint8_t *b, const Mp2kPlayer *pl);
/* ... and read back: every field but the lists (a channel's track and
 * neighbours, a track's first channel), which the caller links, and the
 * engine's own (where a track is kept, a player's tracks and ROM) */
void mp2k_get_chan(Mp2kChan *c, const uint8_t *b, bool cgb);
void mp2k_get_track(Mp2kTrack *t, const uint8_t *b);
void mp2k_get_player(Mp2kPlayer *pl, const uint8_t *b);

#endif
