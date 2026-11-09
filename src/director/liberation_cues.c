/* BN5's songs at a liberation's moments (liberation.h; docs/multirom/music.md,
 * BN5's music in a run): copied by src/audio/xsong.c with the board's
 * songs and begun by BN6's own player. A first turn's win answers BN6's
 * win through the hook on its song (win_song.c), as BN5's win picks its own
 * (0x08007A82). */
#include "liberation.h"

#include <stdio.h>

#include "bn5.h"
#include "bn6.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"
#include "run.h"
#include "win_song.h"
#include "xsong.h"

/* The board won: its own song again once BN5's for the win has ended, on
 * the layer it was won on */
static struct {
	bool due;
	uint32_t layer;
} resume;

void lib_cue_battle(bool counted) {
	int slot = counted && lib_look_on() ? xsong_install(XROM_BN5_COLONEL_US, BN5_SONG_ONE_TURN_WIN, -1) : -1;
	win_song_one_turn(slot);
	if (emu_debug_on() && slot >= 0) fprintf(stderr, "liberation: a first turn's win plays BN5's song 0x%02X as BN6's 0x%02X\n", BN5_SONG_ONE_TURN_WIN, slot);
}

void lib_cue_won(void) {
	if (!lib_look_on()) return;
	lib_song(BN5_SONG_LIB_WON);
	resume.due = true;
	resume.layer = run.layer_seed;
}

void lib_cue_frame(void) {
	if (!resume.due) return;
	if (resume.layer != run.layer_seed || !lib_look_on()) { resume.due = false; return; }
	/* (BN5's song for the win plays once and ends: the music player stopped) */
	if (!(emu_read32(BN6_MUSIC_STATUS) & BN6_MUSIC_STOPPED)) return;
	resume.due = false;
	lib_song(BN5_SONG_LIBERATION);
}
