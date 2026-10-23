/* MP2K, BN6's sound engine (Nintendo's "m4a", the GBA's standard library),
 * as the engine runs it on the title and the PET's screens: its state,
 * shared by the sequencer (mp2k_seq.c, mp2k_note.c), the PSG's driver
 * (mp2k_cgb.c) and the mixer (mp2k_mix.c). Each field is the library's own,
 * at the place BN6's RAM keeps it (docs/ROM_DATA.md, BN6's sound engine:
 * SoundInfo, its channels, a music player and its tracks), so a frame of
 * ours can be laid beside a frame of the game's (tools/mp2k_diff.py). */
#ifndef CW_MP2K_STATE_H
#define CW_MP2K_STATE_H

#include <stdbool.h>
#include <stdint.h>

#define MP2K_PLAYERS 32       /* BN6's music players (BN6_MPLAYS) */
#define MP2K_SONGS 474        /* BN6's song table's entries */
#define MP2K_TRACKS 112       /* every player's tracks together (BN6's: 8 and 31 of 1-3) */
#define MP2K_DS_MAX 12        /* DirectSound channels at most (the library's); BN6 mixes 4 */
#define MP2K_CGB 4            /* the PSG's channels: square 1 and 2, wave, noise */
#define MP2K_CHANS (MP2K_DS_MAX + MP2K_CGB)
#define MP2K_RING_MAX (12 * 224)   /* a channel's mix ring: pcmDmaPeriod frames of samples */
#define MP2K_NONE (-1)

/* the library's flags: a channel's status, a track's */
enum {
	CHN_START = 0x80, CHN_STOP = 0x40, CHN_LOOP = 0x10, CHN_ECHO = 0x04, CHN_ENV = 0x03, CHN_ON = 0xC7,
	TRK_EXIST = 0x80, TRK_START = 0x40, TRK_VOLSET = 0x01, TRK_VOLCHG = 0x03, TRK_PITSET = 0x04, TRK_PITCHG = 0x0C,
	PLAYER_PAUSE = (int)0x80000000u,
};

/* A channel: SoundChannel (the DirectSound ones) or CgbChannel (the PSG's),
 * their fields by their offsets */
typedef struct {
	uint8_t status;           /* +0x00 */
	uint8_t type;             /* +0x01: 0 sampled, 1-4 the PSG's; 8 fixed pitch */
	uint8_t right, left;      /* +0x02, +0x03: volumes */
	uint8_t attack, decay, sustain, release;   /* +0x04-0x07 */
	uint8_t key;              /* +0x08 */
	uint8_t env;              /* +0x09: the envelope's volume */
	uint8_t env_right;        /* +0x0A: (sampled) or envelopeGoal (PSG) */
	uint8_t env_left;         /* +0x0B: (sampled) or envelopeCounter (PSG) */
	uint8_t echo_volume, echo_length;   /* +0x0C, +0x0D: the pseudo-echo */
	uint8_t gate;             /* +0x10 */
	uint8_t midi_key;         /* +0x11 */
	uint8_t velocity;         /* +0x12 */
	uint8_t priority;         /* +0x13 */
	int8_t rhythm_pan;        /* +0x14 */
	uint8_t sustain_goal;     /* +0x19 (PSG) */
	uint8_t n4;               /* +0x1A (PSG): its NRx4's length bits and frequency */
	uint8_t pan, pan_mask;    /* +0x1B, +0x1C (PSG): its NR51 bits */
	uint8_t modify;           /* +0x1D (PSG): 1 volume, 2 frequency changed */
	uint8_t length, sweep;    /* +0x1E, +0x1F (PSG) */
	uint32_t count;           /* +0x18 (sampled): samples left */
	uint32_t fw;              /* +0x1C (sampled): the position's fraction, 23 bits */
	uint32_t frequency;       /* +0x20 */
	uint32_t wav;             /* +0x24: a sample's ROM address, or a PSG tone's duty, wave or noise */
	uint32_t cur;             /* +0x28: the next sample's ROM address; the PSG's wave loaded */
	int track;                /* +0x2C: its track (MP2K_NONE) */
	int prev, next;           /* +0x30, +0x34: its track's list */
	uint8_t rom;              /* (ours) the ROM its sample or wave is in: its player's at its note */
} Mp2kChan;

/* A track: MusicPlayerTrack */
typedef struct {
	uint8_t flags, wait, pattern_level, repeat;   /* +0x00-0x03 */
	uint8_t gate, key, velocity, running;         /* +0x04-0x07 */
	int8_t key_m; uint8_t pit_m;                  /* +0x08, +0x09 */
	int8_t key_shift, key_shift_x, tune; uint8_t pit_x;   /* +0x0A-0x0D */
	int8_t bend; uint8_t bend_range;              /* +0x0E, +0x0F */
	uint8_t vol_mr, vol_ml, vol, vol_x;           /* +0x10-0x13 */
	int8_t pan, pan_x, mod_m;                     /* +0x14-0x16 */
	uint8_t mod, mod_t, lfo_speed, lfo_speed_c, lfo_delay, lfo_delay_c;   /* +0x17-0x1C */
	uint8_t priority, echo_volume, echo_length;   /* +0x1D-0x1F */
	int chan;                                      /* +0x20: its channels' list */
	uint8_t tone[12];                              /* +0x24: ToneData (type, key, length, pan or sweep, wav, attack, decay, sustain, release) */
	uint32_t cmd;                                  /* +0x40: its next command's ROM address */
	uint32_t stack[3];                             /* +0x44: PATT's returns */
	uint32_t addr;                                 /* where BN6 keeps it (the engine compares tracks by it) */
} Mp2kTrack;

/* A music player: MusicPlayerInfo, and its row of BN6_MPLAYS */
typedef struct {
	uint32_t song;            /* +0x00: its song's header (ROM address, 0 none) */
	uint32_t status;          /* +0x04: its tracks' bits, PLAYER_PAUSE */
	uint8_t priority;         /* +0x09 */
	uint8_t check_priority;   /* +0x0B: a song of lower priority waits (the effects' players) */
	uint32_t clock;           /* +0x0C */
	uint16_t tempo_d, tempo_u, tempo_i, tempo_c;   /* +0x1C-0x22 */
	uint16_t fade_oi, fade_oc, fade_ov;            /* +0x24-0x28 */
	uint32_t tone;            /* +0x30: its voice group */
	int first;                /* its tracks in Mp2k.tracks */
	uint8_t tracks;           /* its tracks (BN6_MPLAYS) */
	uint8_t rom;              /* (ours) the ROM its song is in: 0 BN6's, 1 + XRomId another's (mp2k.h) */
	uint16_t loops;           /* (ours) its first track's GOTOs since its song began: the times it went round */
} Mp2kPlayer;

/* SoundInfo, and what the library reads from the ROM */
typedef struct {
	bool ready;
	uint8_t dma_counter;      /* +0x04 */
	uint8_t reverb;           /* +0x05 */
	uint8_t max_chans;        /* +0x06 */
	uint8_t master_volume;    /* +0x07 */
	uint8_t c15;              /* +0x0A: the PSG's envelope's 15-frame count */
	uint8_t dma_period;       /* +0x0B */
	int per_frame;            /* +0x10: samples mixed a frame */
	uint32_t div_freq;        /* +0x18 */
	Mp2kPlayer players[MP2K_PLAYERS];
	Mp2kTrack tracks[MP2K_TRACKS];
	Mp2kChan chans[MP2K_CHANS];   /* the DirectSound ones from 0, the PSG's from MP2K_DS_MAX */
	int8_t ring[2][MP2K_RING_MAX];   /* +0x350: the mix, right then left (DMA A, DMA B) */
	int slice;                /* the ring's part this frame mixed */
	const uint8_t *data;      /* (ours) the ROM the part running reads: a player's, a channel's (mp2k_read) */
	uint32_t quiet;           /* (ours) the players silenced (mp2k_quiet_set) */
	/* the library's tables in BN6's ROM (docs/ROM_DATA.md) */
	uint32_t song_table, clock_table, scale_table, freq_table, cgb_scale_table, cgb_freq_table, noise_table, cgb3_vol_table;
} Mp2k;
extern Mp2k M;

#define CGB_CHAN(i) (&M.chans[MP2K_DS_MAX + (i)])   /* the PSG's channel i, 0-3 */

/* ---- the parts' calls into each other ---- */

/* the bytes at a bus address of the ROM being read (0 outside it): a
 * song's, its voices', its samples'; mp2k_read picks the ROM (0 BN6's, 1 +
 * XRomId another's), BN6's where it is not there. BN5's library is BN6's,
 * byte for byte but for its literals' addresses, and its tables BN6's
 * (docs/ROM_DATA.md, Another game's songs), so another game's song plays
 * on BN6's code and tables: the library's tables are read from BN6's ROM,
 * whichever is being read (mp2k_lib_u8) */
uint8_t mp2k_u8(uint32_t addr);
uint32_t mp2k_u32(uint32_t addr);
void mp2k_read(int rom);
uint8_t mp2k_lib_u8(uint32_t addr);
uint32_t mp2k_lib_u32(uint32_t addr);
/* (mp2k_seq.c) a frame of every player, BN6's order: MPlayMain */
void mp2k_players_frame(void);
/* (mp2k_note.c) a note (ply_note), its end (ply_endtie), a track's volume and
 * pitch worked out (TrkVolPitSet), a channel's volumes (ChnVolSetAsm), a
 * sample's pitch (MidiKeyToFreq) */
void mp2k_note(int player, int track, int clock_index);
void mp2k_end_tie(Mp2kTrack *t);
void mp2k_track_vol_pit(int player, Mp2kTrack *t);
void mp2k_chan_volume(Mp2kChan *c, const Mp2kTrack *t);
uint32_t mp2k_key_to_freq(uint32_t wav, int key, int fine);
/* a channel taken off its track's list (ClearChain), a track's channels
 * stopped (TrackStop) */
void mp2k_clear_chain(int chan);
void mp2k_track_stop(Mp2kTrack *t);
/* (mp2k_cgb.c) the PSG's frame (CgbSound), a channel's pitch (MidiKeyToCgbFreq),
 * a channel silenced (CgbOscOff) */
void mp2k_cgb_frame(void);
uint32_t mp2k_cgb_key_to_freq(int type, int key, int fine);
void mp2k_cgb_off(int type);
/* (mp2k_mix.c) the DirectSound channels' frame into the ring (SoundMainRAM) */
void mp2k_mix_frame(void);

#endif
