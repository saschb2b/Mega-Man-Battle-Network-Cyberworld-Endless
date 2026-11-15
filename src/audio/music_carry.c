/* The game's music under the engine's screens (audio.h, music_carry_held):
 * its RAM read and written back, its registers, the course of a hold. */
#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "emu.h"
#include "mp2k.h"
#include "mp2k_carry.h"
#include "platform.h"

#define IO 0x04000000u

static enum { CARRY_NONE, CARRY_GAME, CARRY_OWN } carry;
static Mp2kGameMusic taken;   /* the game's music as the engine took it */

/* the game's music as its RAM holds it */
static void game_read(Mp2kGameMusic *g) {
	emu_read_bytes(BN6_MUSIC_PLAYER, g->player, sizeof g->player);
	for (int i = 0; i < MP2K_CARRY_TRACKS; ++i) emu_read_bytes(mp2k_carry_track_at(i), g->track[i], sizeof g->track[i]);
	emu_read_bytes(BN6_SOUND_CHANS, g->ds[0], sizeof g->ds);
	emu_read_bytes(BN6_SOUND_CGB_CHANS, g->cgb[0], sizeof g->cgb);
	g->reverb = emu_read8(BN6_SOUND_REVERB);
	g->c15 = emu_read8(BN6_SOUND_C15);
}

static void game_write(const Mp2kGameMusic *g, const Mp2kWrite *writes, int n) {
	emu_write(BN6_MUSIC_PLAYER, g->player, sizeof g->player);
	for (int i = 0; i < MP2K_CARRY_TRACKS; ++i) emu_write(mp2k_carry_track_at(i), g->track[i], sizeof g->track[i]);
	emu_write(BN6_SOUND_CHANS, g->ds[0], sizeof g->ds);
	emu_write(BN6_SOUND_CGB_CHANS, g->cgb[0], sizeof g->cgb);
	for (int i = 0; i < n; ++i) emu_write32(writes[i].at, writes[i].value);
}

/* the game's own PSG registers */
static uint8_t game_read8(int reg) { return emu_read8(IO + (uint32_t)reg); }
static void game_write8(int reg, uint8_t v) { emu_io_write8(IO + (uint32_t)reg, v); }
static void game_write32(int reg, uint32_t v) { emu_io_write32(IO + (uint32_t)reg, v); }
static const Mp2kRegs game_regs = { game_read8, game_write8, game_write32 };

/* ---- the check (CYBERWORLD_CARRY_CHECK=N:M:HOW@FRAME): at the engine's
 * frame FRAME the game's music handed to the engine for N frames and given
 * back (HOW 1), or left to the game those N (HOW 0), then M frames of the
 * game; the music's state and the PSG's registers on stderr, and the game
 * ends. Two runs' lines are the same where the hand-over is exact
 * (docs/engine/emulation/engine-sound.md, The game's music under the
 * engine's screens). ---- */

static void check_line(const char *what, const uint8_t *x, int n) {
	fprintf(stderr, "carrycheck %s:", what);
	for (int i = 0; i < n; ++i) fprintf(stderr, " %02X", x[i]);
	fprintf(stderr, "\n");
}

static void check(void) {
	static int at = -2, frames, then, how;
	if (at == -2) {
		const char *spec = getenv("CYBERWORLD_CARRY_CHECK");
		at = -1;
		if (spec && sscanf(spec, "%d:%d:%d@%d", &frames, &then, &how, &at) != 4) at = -1;
	}
	if (at < 0 || P.frame != (uint64_t)at || !mp2k_carry_track_at(0)) return;
	game_read(&taken);
	if (how) {
		Mp2kWrite w[MP2K_CARRY_WRITES];
		audio_lock();
		mp2k_game_rom(emu_rom_byte);
		mp2k_carry_in(&taken);
		for (int f = 0; f < frames; ++f) mp2k_frame();
		int n = mp2k_carry_out(&taken, &game_regs, w);
		mp2k_game_rom(NULL);
		audio_unlock();
		game_write(&taken, w, n);
	} else
		for (int f = 0; f < frames; ++f) emu_frame(0);
	for (int f = 0; f < then; ++f) emu_frame(0);
	Mp2kGameMusic g;
	game_read(&g);
	check_line("player", g.player, MP2K_PLAYER_BYTES);
	for (int i = 0; i < MP2K_CARRY_TRACKS; ++i) check_line("track", g.track[i], MP2K_TRACK_BYTES);
	for (int c = 0; c < MP2K_DS_MAX + MP2K_CGB; ++c) check_line("channel", c < MP2K_DS_MAX ? g.ds[c] : g.cgb[c - MP2K_DS_MAX], MP2K_CHAN_BYTES);
	uint8_t io[0x40];
	for (uint32_t i = 0; i < sizeof io; ++i) io[i] = emu_read8(IO + 0x60 + i);
	check_line("psg", io, sizeof io);
	exit(0);
}

/* ---- a hold ---- */

void music_carry_held(void) {
	if (carry != CARRY_NONE || !emu_ready() || !audio_on() || !mp2k_carry_track_at(0)) return;
	game_read(&taken);
	bool ok = false;
	if (mp2k_carry_plays(&taken)) {
		audio_lock();
		mp2k_game_rom(emu_rom_byte);
		ok = mp2k_carry_in(&taken);
		audio_unlock();
	}
	if (ok) {
		carry = CARRY_GAME;
		return;
	}
	/* (none the game plays: the NaviCust's theme, the song of BN6's own
	 * screen of the PET's, faded out as the game runs again) */
	audio_play_song(BN6_SONG_NAVI_CUSTOMIZER, true);
	carry = CARRY_OWN;
}

void music_carry_resume(void) {
	check();
	if (carry == CARRY_OWN) audio_music_id(0);
	if (carry != CARRY_GAME) {
		carry = CARRY_NONE;
		return;
	}
	carry = CARRY_NONE;
	/* (given back only to the game as it was taken: one loaded meanwhile,
	 * a test battle's snapshot, keeps its own) */
	Mp2kGameMusic now;
	game_read(&now);
	bool same = !memcmp(&now, &taken, sizeof now);
	Mp2kWrite writes[MP2K_CARRY_WRITES];
	int n = -1;
	audio_lock();
	if (same) n = mp2k_carry_out(&taken, &game_regs, writes);
	else mp2k_carry_drop();
	mp2k_game_rom(NULL);
	audio_unlock();
	if (n >= 0) game_write(&taken, writes, n);
}

void music_carry_drop(void) {
	if (carry == CARRY_GAME) {
		audio_lock();
		mp2k_carry_drop();
		mp2k_game_rom(NULL);
		audio_unlock();
	} else if (carry == CARRY_OWN) audio_music_id(0);
	carry = CARRY_NONE;
}
