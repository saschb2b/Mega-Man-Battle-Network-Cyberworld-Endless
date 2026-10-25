#include "feel_sound.h"

#include <stdio.h>

#include "bn6.h"
#include "emu.h"
#include "feel_out.h"
#include "hook.h"
#include "platform.h"
#include "rom.h"

/* the sounds started, and whether MegaMan's side started each, kept two
 * frames: BN6 queues a sound, its player begins it at the next frame */
#define STARTED_MOST 16
#define STARTED_FRAMES 2
typedef struct { uint16_t song; bool mine; uint8_t age; } Started;
static Started started[STARTED_MOST];
static int nstarted;
/* the players: their tracks and MusicPlayerInfo (BN6_MPLAYS), what each
 * played at the last frame, and whether it plays a sound of his */
static uint32_t tracks[BN6_MPLAY_COUNT], info[BN6_MPLAY_COUNT];
static uint8_t counts[BN6_MPLAY_COUNT];
static uint32_t was_song[BN6_MPLAY_COUNT], was_clock[BN6_MPLAY_COUNT];
static bool his[BN6_MPLAY_COUNT];
static bool hooked;

/* PlaySoundEffect: started while an object of MegaMan's side runs (him,
 * his attacks: BN6_OBJECT_RUNNING's node, its object after it), his */
static HookAct sounded(HookRegs *r, void *user) {
	(void)user;
	uint32_t node = hook_read32(BN6_OBJECT_RUNNING);
	bool mine = node && hook_read8(node + BN6_OBJECT_NODE + BN6_T1_ALLIANCE) == 0;
	if (nstarted < STARTED_MOST) started[nstarted++] = (Started){ (uint16_t)r->r[0], mine, 0 };
	return HOOK_CONTINUE;
}

static void hook_once(void) {
	if (hooked) return;
	for (int p = 0; p < BN6_MPLAY_COUNT; ++p) {
		uint32_t row = BN6_MPLAYS + (uint32_t)p * BN6_MPLAY_ROW;
		info[p] = emu_read32(row);
		tracks[p] = emu_read32(row + BN6_MPLAY_TRACKS);
		counts[p] = emu_read8(row + BN6_MPLAY_TRACK_COUNT);
	}
	hooked = emu_hook(BN6_PLAY_SOUND, sounded, NULL);
}

/* a song's header, as the song table has it */
static uint32_t header_of(uint16_t song) {
	if (!R.layout || !R.layout->song_table) return 0;
	return emu_read32(0x08000000u + R.layout->song_table + (uint32_t)song * BN6_SONG_ROW + BN6_SONG_HEADER);
}

static int player_of_track(uint32_t t) {
	for (int p = 0; p < BN6_MPLAY_COUNT; ++p)
		if (t >= tracks[p] && t < tracks[p] + counts[p] * BN6_TRACK_SIZE) return p;
	return -1;
}

/* Whose each player's sound is: where a song began on it since the last
 * frame (another song, or its clock back), the started sound that is it,
 * begun the frame after its start (MP2K turns away a start of lower
 * priority, which leaves the player as it was); a song begun by none of the hook's (the music, a chat's), not
 * his. True where one of his began. */
static bool take_started(void) {
	bool began = false;
	for (int p = 0; p < BN6_MPLAY_COUNT; ++p) {
		uint32_t song = emu_read32(info[p] + BN6_MPLAY_SONG), clock = emu_read32(info[p] + BN6_MPLAY_CLOCK);
		if (song != was_song[p] || clock < was_clock[p]) {
			bool mine = false;
			for (int i = 0; i < nstarted; ++i)
				if (header_of(started[i].song) == song) {
					/* (the buster's charging hum is felt by the charge itself, feel_game.c) */
					mine = started[i].mine && started[i].song != BN6_SOUND_BUSTER_CHARGE;
					if (mine && feel_tracing()) printf("feel: frame %llu: his sound 0x%X\n", (unsigned long long)P.frame, started[i].song);
				}
			his[p] = mine;
			began |= mine;
		}
		was_song[p] = song;
		was_clock[p] = clock;
	}
	int kept = 0;
	for (int i = 0; i < nstarted; ++i)
		if (++started[i].age < STARTED_FRAMES) started[kept++] = started[i];
	nstarted = kept;
	return began;
}

/* a sample's loudness where it plays: its next bytes' mean size, 0-255
 * (a sample's envelope stays full while it plays; the sound is in its bytes) */
#define SAMPLE_WINDOW 64
static int sample_loudness(uint32_t at) {
	if (at < 0x08000000u || at >= 0x0A000000u - SAMPLE_WINDOW) return 0;
	int sum = 0;
	for (uint32_t i = 0; i < SAMPLE_WINDOW; ++i) {
		int v = (int8_t)emu_read8(at + i);
		sum += v < 0 ? -v : v;
	}
	int loud = sum * 2 / SAMPLE_WINDOW;
	return loud > 255 ? 255 : loud;
}

static void put(int *m, int v) {
	if (v > *m) *m = v;
}

/* a channel of his sounds playing: its level, 0-255, at the motors its
 * kind moves (a sample both, the heavy one more; the PSG's noise the
 * heavy one; its squares and wave the light one) */
static void channel(uint32_t c, bool psg, int *low, int *high) {
	if (!emu_read8(c + BN6_CHAN_STATUS)) return;
	int p = player_of_track(emu_read32(c + BN6_CHAN_TRACK));
	if (p < 0 || !his[p]) return;
	int type = emu_read8(c + BN6_CHAN_TYPE) & 7, env = emu_read8(c + BN6_CHAN_ENV);
	int loud = psg ? env * 17 : env * sample_loudness(emu_read32(c + BN6_CHAN_CUR)) / 255;
	int level = loud * emu_read8(c + BN6_CHAN_VELOCITY) / 127;
	if (level > 255) level = 255;
	if (!psg) {
		put(low, level);
		put(high, level / 2);
	} else if (type == 4) {
		put(low, level * 3 / 4);
		put(high, level / 2);
	} else put(high, level);
}

bool feel_sound_frame(bool fight, uint8_t *low, uint8_t *high) {
	*low = *high = 0;
	/* (the core on a thread of its own runs the next frame meanwhile: the 3DS, which rumbles nothing) */
	if (emu_threaded()) return false;
	hook_once();
	bool began = take_started();
	if (!fight) return began;
	int lo = 0, hi = 0, n = emu_read8(BN6_SOUND_MAX_CHANS);
	for (int i = 0; i < n && i < 12; ++i) channel(BN6_SOUND_CHANS + (uint32_t)i * BN6_CHAN_SIZE, false, &lo, &hi);
	for (int i = 0; i < BN6_CGB_CHAN_COUNT; ++i) channel(BN6_SOUND_CGB_CHANS + (uint32_t)i * BN6_CHAN_SIZE, true, &lo, &hi);
	*low = (uint8_t)lo;
	*high = (uint8_t)hi;
	return began;
}
