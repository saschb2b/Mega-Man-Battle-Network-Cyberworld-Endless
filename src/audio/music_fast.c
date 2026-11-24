/* Fast forward's music (audio.h, music_fast_update). */
#include "audio.h"

#include <string.h>

#include "bn6.h"
#include "emu.h"
#include "emu_sound.h"
#include "fast_forward.h"
#include "mp2k.h"
#include "mp2k_carry.h"
#include "music_game.h"
#include "sound_quiet.h"

/* (after the song is given back, the engine's frames still heard: the one
 * it mixed last, which the GBA plays a frame after, and what it held over) */
#define ENDING_SAMPLES 1700
#define RING_PART 704   /* a frame's part of BN6's ring at most (audio.c's) */

/* OFF: the game's music its own; ON: the engine's, the game's away;
 * ENDING: given back, the engine's last frame still heard. ON and ENDING
 * are read by the core's sound (mix), on its thread where it has one,
 * between the frames the main thread waits out first. */
static volatile enum { OFF, ON, ENDING } state;
static volatile int ending;                /* (samples still to hear of ENDING) */
static uint8_t seen[MP2K_PLAYER_BYTES];    /* the game's music player at the last look */

/* (the core's sound heard, a frame of the game's: the engine's added) */
static void mix(int16_t *frames, int n) {
	if (state == OFF) return;
	audio_engine_add(frames, n);
	if (state == ENDING && (ending -= n) <= 0) {
		audio_engine_in_ring(false);
		emu_sound_mixer(NULL);
		state = OFF;
	}
}

static bool can(void) { return emu_ready() && audio_on() && mp2k_carry_track_at(0); }

/* the song taken where the game has it, the game's player heard no more */
static void take(void) {
	Mp2kGameMusic g;
	music_game_read(&g);
	audio_lock();
	mp2k_game_rom(emu_rom_byte);
	bool taken = mp2k_carry_plays(&g);
	if (taken) {
		mp2k_carry_in(&g);
		mp2k_carry_hush(&g, &music_game_regs);
		music_game_write(&g, NULL, 0);
		memcpy(seen, g.player, sizeof seen);
	} else {
		/* (none playing: the next song the game begins is begun here) */
		mp2k_carry_drop();
		memset(seen, 0, sizeof seen);
	}
	audio_unlock();
	audio_engine_in_ring(true);
	/* (the game's last mix, whose end the GBA plays in the next frame, the
	 * engine's to play out, the song's notes in it: m4aSoundVSync's count
	 * gives its part of the ring, as mp2k_frame works it out) */
	int count = emu_read8(BN6_SOUND_DMA_COUNT), period = emu_read8(BN6_SOUND_DMA_PERIOD);
	int n = (int)emu_read32(BN6_SOUND_PER_FRAME), part = count > 1 ? period - (count - 1) : 0;
	if (taken && n > 0 && n <= RING_PART && part >= 0 && part < period) {
		int8_t right[RING_PART], left[RING_PART];
		emu_read_bytes(BN6_SOUND_RING + (uint32_t)(part * n), (uint8_t *)right, (uint32_t)n);
		emu_read_bytes(BN6_SOUND_RING_LEFT + (uint32_t)(part * n), (uint8_t *)left, (uint32_t)n);
		audio_engine_prime(right, left, n);
	}
	sound_quiet_music_away(true);
	emu_sound_mixer(mix);
	state = ON;
}

/* the engine kept on what the game's player does */
static void follow(void) {
	uint8_t now[MP2K_PLAYER_BYTES];
	emu_read_bytes(BN6_MUSIC_PLAYER, now, sizeof now);
	if (!memcmp(now, seen, sizeof now)) return;
	audio_lock();
	mp2k_carry_follow(seen, now);
	audio_unlock();
	memcpy(seen, now, sizeof seen);
}

/* the song given back where the engine has brought it, the game's player
 * heard again from its next frame */
static void give(void) {
	Mp2kGameMusic g;
	music_game_read(&g);
	Mp2kWrite writes[MP2K_CARRY_WRITES];
	audio_lock();
	int n = mp2k_carry_out(&g, &music_game_regs, writes);
	mp2k_game_rom(NULL);
	audio_unlock();
	if (n >= 0) music_game_write(&g, writes, n);
	sound_quiet_music_away(false);
	ending = ENDING_SAMPLES;
	state = ENDING;
}

void music_fast_update(bool fast) {
	bool wanted = fast_music();
	if (state == OFF && !(fast && wanted)) return;
	/* (the frame before ended, its sound heard: the core's thread idle) */
	emu_sync();
	if (!can()) {
		music_fast_drop();
		return;
	}
	if (fast && wanted) {
		if (state == ON) follow();
		else take();
	} else if (state == ON) give();
}

/* the engine's sound beside the core's again, what it still had to say
 * heard there */
static void apart(void) {
	if (state == OFF) return;
	emu_sync();
	emu_sound_mixer(NULL);
	audio_engine_in_ring(false);
	state = OFF;
}

void music_fast_held(void) {
	if (state == OFF) return;
	emu_sync();
	if (state == ON && can()) give();
	apart();
}

void music_fast_drop(void) {
	if (state == OFF) return;
	emu_sync();
	if (state == ON) {
		audio_lock();
		mp2k_carry_drop();
		mp2k_game_rom(NULL);
		audio_unlock();
		sound_quiet_music_away(false);
	}
	apart();
}
