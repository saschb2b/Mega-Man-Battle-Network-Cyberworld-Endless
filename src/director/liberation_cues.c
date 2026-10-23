/* BN5's songs at a liberation's moments (liberation.h; docs/MULTIROM.md,
 * BN5's music in a run): copied by src/audio/xsong.c with the board's
 * songs and begun by BN6's own player. A first turn's win answers BN6's
 * win through a hook on its song, as BN5's win picks its own (0x08007A82);
 * the hook only reads what the director armed between frames and the
 * battle's own count of its Custom screens. */
#include "liberation.h"

#include <stdio.h>

#include "bn5.h"
#include "bn6.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "run.h"
#include "xsong.h"

/* (the director's, between frames) BN6's slot of BN5's one-turn jingle, -1
 * where no counted battle of a liberation is armed */
static volatile int one_turn_song = -1;

/* The board won: its own song again once BN5's for the win has ended, on
 * the layer it was won on */
static struct {
	bool due;
	uint32_t layer;
} resume;

/* BN6's win, its song in r2: BN5's jingle where the battle was won in its
 * first turn (BN6_BATTLE_SCREENS, BN5's BattleState +7, one at the first
 * Custom screen) */
static HookAct win_song(HookRegs *r, void *user) {
	(void)user;
	if (one_turn_song >= 0 && hook_read8(BN6_BATTLE_SCREENS) <= 1) r->r[2] = (uint32_t)one_turn_song;
	return HOOK_CONTINUE;
}

void lib_cue_battle(bool counted) {
	static bool hooked;
	int slot = counted && lib_look_on() ? xsong_install(XROM_BN5_COLONEL_US, BN5_SONG_ONE_TURN_WIN, -1) : -1;
	if (slot >= 0 && !hooked && !(hooked = emu_hook(BN6_WIN_SONG, win_song, NULL))) fprintf(stderr, "liberation: the win's song hook did not fit\n");
	one_turn_song = hooked ? slot : -1;
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
