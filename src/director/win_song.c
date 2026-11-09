/* The song a won battle plays (win_song.h; docs/multirom/music.md). BN6's win
 * (0x080081A4) picks 0x19 or 0x1F and begins it at BN6_WIN_SONG; BN5's twin
 * (BN5_BATTLE_WIN_SONG) picks the same numbers the same way, so on BN5's
 * areas the hook swaps BN6's number for BN6's slot of BN5's song of that
 * number. The hook only reads what the director set between frames and the
 * battle's own count of its Custom screens. */
#include "win_song.h"

#include <stdbool.h>
#include <stdio.h>

#include "bn6.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "xsong.h"

/* (the director's, between frames) BN6's number of each win song and the
 * slot of the area's game's copy, -1 where BN6's plays */
static volatile int theirs[2] = { -1, -1 };
static volatile int ours[2];
static volatile int one_turn = -1;
static bool hooked;

static HookAct win_song(HookRegs *r, void *user) {
	(void)user;
	/* (a liberation's battle won in its first turn: BN5's BattleState +7, one at the first Custom screen) */
	if (one_turn >= 0 && hook_read8(BN6_BATTLE_SCREENS) <= 1) {
		r->r[2] = (uint32_t)one_turn;
		return HOOK_CONTINUE;
	}
	for (int k = 0; k < 2; ++k)
		if (theirs[k] >= 0 && r->r[2] == (uint32_t)ours[k]) {
			r->r[2] = (uint32_t)theirs[k];
			break;
		}
	return HOOK_CONTINUE;
}

/* the hook on BN6's win, once one is needed */
static bool hook_win(void) {
	if (!hooked && !(hooked = emu_hook(BN6_WIN_SONG, win_song, NULL))) fprintf(stderr, "win song: the hook on BN6's win did not fit\n");
	return hooked;
}

void win_song_area(int xrom) {
	const XRomLayout *x = xrom >= 0 && xrom < XROM_COUNT && XR[xrom].data ? XR[xrom].layout : NULL;
	int slot[2] = { -1, -1 }, song[2] = { 0, 0 };
	for (int k = 0; k < 2; ++k) {
		theirs[k] = -1;
		song[k] = x ? x->win_songs[k] : 0;
		if (song[k]) slot[k] = xsong_install_sound(xrom, song[k], -1);
	}
	if ((slot[0] >= 0 || slot[1] >= 0) && !hook_win()) slot[0] = slot[1] = -1;
	for (int k = 0; k < 2; ++k) {
		ours[k] = song[k];
		theirs[k] = slot[k];
	}
	static int said[3] = { -2, -2, -2 };
	if (emu_debug_on() && (slot[0] >= 0 || slot[1] >= 0) && (said[0] != xrom || said[1] != slot[0] || said[2] != slot[1]))
		fprintf(stderr, "win song: a won battle plays %s's 0x%02X as BN6's 0x%02X, its 0x%02X as 0x%02X\n", x->name, song[0], slot[0], song[1],
			slot[1]);
	said[0] = xrom;
	said[1] = slot[0];
	said[2] = slot[1];
}

void win_song_one_turn(int slot) {
	one_turn = slot >= 0 && hook_win() ? slot : -1;
}
