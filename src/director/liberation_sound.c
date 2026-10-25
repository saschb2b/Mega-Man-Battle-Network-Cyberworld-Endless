/* A liberation's music and sounds as BN5's (liberation.h; docs/game/liberation/bn5/look-and-motion.md,
 * How the board looks and moves): its songs copied from BN5's ROM into
 * BN6's free space (src/audio/xsong.c) and started by BN6's own PlayMusic,
 * its sounds BN6's where BN6 has the same (their tracks compared) or BN5's
 * copied. Nothing of BN5's is kept but what the copies are made of. */
#include "liberation.h"

#include <stdio.h>

#include "bn5_liberation.h"
#include "bn6.h"
#include "debug.h"
#include "emu.h"
#include "gamecall.h"
#include "rom.h"
#include "xsong.h"

int lib_board_song(int song) {
	if (!lib_look_on()) return song;
	return xsong_install(XROM_BN5_COLONEL_US, BN5_SONG_LIBERATION, song);
}

void lib_song(int bn5_song) {
	if (!lib_look_on()) return;
	int slot = xsong_install(XROM_BN5_COLONEL_US, bn5_song, -1);
	if (slot >= 0) game_call_then_update(BN6_PLAY_MUSIC, (uint32_t)slot, 0);
	/* (the BGM's player read back: its song's header the slot's) */
	if (emu_debug_on() && slot >= 0)
		fprintf(stderr, "liberation: BN5's song 0x%02X as BN6's 0x%02X, the music player's song until now %08x, the slot's %08x\n", bn5_song, slot,
			emu_read32(BN6_MUSIC_PLAYER), emu_read32(0x08000000u + R.layout->song_table + 8u * (uint32_t)slot));
}

void lib_sound(int bn5_sound) {
	if (!lib_look_on()) return;
	/* (one of BN5's that BN6 has under its number is BN6's own; the rest copied to their slots) */
	int id = xsong_sound_slot(XROM_BN5_COLONEL_US, bn5_sound) >= 0 ? xsong_install_sound(XROM_BN5_COLONEL_US, bn5_sound, -1) : bn5_sound;
	if (id >= 0) game_call_then_update(BN6_PLAY_SOUND, (uint32_t)id, 0);
}
