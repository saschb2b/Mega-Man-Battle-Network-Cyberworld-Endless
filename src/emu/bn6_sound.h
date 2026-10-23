/* BN6 Cybeast Gregar (USA)'s sound engine, part of bn6.h's addresses (it
 * includes this): MP2K, Nintendo's "m4a", as the game runs it and the
 * engine's own player follows it (src/audio/mp2k*.c): its RAM, its tables
 * in the ROM, its routines. Names follow the bn6f disassembly where it
 * names them; docs/ROM_DATA.md, BN6's sound engine, says how each was found
 * and how to verify it. */
#ifndef CW_BN6_SOUND_H
#define CW_BN6_SOUND_H

/* RAM */
#define BN6_SOUND_INFO        0x030045C0u /* SoundInfo ("Smsh", the BIOS's pointer at 0x03007FF0 reads it): +0x04 the DMA's count, +0x05 the reverb, +0x06 4 channels, +0x07 master volume 15, +0x08 rate index 3, +0x0B the ring's 9 frames, +0x10 176 samples a frame, +0x50 the channels, +0x350 the mix's ring */
#define BN6_SOUND_CHANS       (BN6_SOUND_INFO + 0x50) /* its DirectSound channels, 0x40 bytes each */
#define BN6_SOUND_PCM_FREQ    (BN6_SOUND_INFO + 0x14) /* s32: the mix's rate in Hz, 10512 (above its half the GBA's mix holds only images) */
#define BN6_SOUND_CGB_CHANS   0x03005600u /* the PSG's four CgbChannels, 0x40 bytes each */
#define BN6_SOUND_JUMP_TABLE  0x03005570u /* MPlayJumpTable: the commands 0xB1-0xCE, then the library's own calls (36 words) */

/* the library's tables in the ROM */
#define BN6_M4A_JUMP_TEMPLATE 0x081C1F68u /* gMPlayJumpTableTemplate, copied to BN6_SOUND_JUMP_TABLE (MEMACC and XCMD put in at the start) */
#define BN6_M4A_SCALE_TABLE   0x081C1FF8u /* gScaleTable: a key's exponent and mantissa, 179 bytes */
#define BN6_M4A_FREQ_TABLE    0x081C20ACu /* gFreqTable: 12 words */
#define BN6_M4A_CGB_SCALE_TABLE 0x081C20F4u /* gCgbScaleTable: the PSG's keys, 132 bytes */
#define BN6_M4A_CGB_FREQ_TABLE 0x081C2178u /* gCgbFreqTable: 12 halfwords */
#define BN6_M4A_NOISE_TABLE   0x081C2190u /* gNoiseTable: the noise channel's NR43 by key, 60 bytes */
#define BN6_M4A_CGB3_VOL_TABLE 0x081C21CCu /* the wave channel's NR32 by envelope volume, 16 bytes */
#define BN6_M4A_CLOCK_TABLE   0x081C21DCu /* gClockTable: a wait's or a note's ticks, 49 bytes */
#define BN6_M4A_XCMD_TABLE    0x081C2210u /* XCMD's twelve sub-commands */

/* its routines */
#define BN6_M4A_SOUND_MAIN    0x0814F754u /* SoundMain: the players, CgbSound, then the mixer copied to IWRAM 0x03005700 (SoundMainRAM, 0x0814F7D8) */
#define BN6_M4A_MPLAY_MAIN    0x0814FDB0u /* MPlayMain: a music player's frame */
#define BN6_M4A_PLY_NOTE      0x081500A8u /* ply_note: a note given a channel by priority */
#define BN6_M4A_SONG_START    0x08150474u /* m4aSongNumStart: the song table's entry on its player (MPlayStart 0x08150C54) */
#define BN6_M4A_CGB_SOUND     0x08151058u /* CgbSound: the PSG's envelopes and registers */
#define BN6_M4A_SOUND_MODE    0x0800059Cu /* the word m4aSoundMode is given at the start: 0x0093040D (4 channels, rate 3) */

#endif
