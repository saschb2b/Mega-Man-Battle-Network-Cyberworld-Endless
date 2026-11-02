/* The game's music or effects quieter or silenced in the core
 * (sound_quiet.h; docs/engine/emulation/core-sound.md). */
#include "sound_quiet.h"

#include <stdbool.h>
#include <stdint.h>

#include "bn6.h"
#include "emu.h"
#include "hook.h"
#include "sound_heard.h"

/* the players' tracks, from BN6_MPLAYS: where, how many; the music's */
static uint32_t tracks[BN6_MPLAY_COUNT];
static uint8_t counts[BN6_MPLAY_COUNT];
static uint32_t music_lo, music_hi;
static bool installed;
/* the levels last taken in, the music's and the effects', and their
 * shares the tracks were told (SOUND_SHARE_ALL: BN6's own volume) */
static int levels[2] = { -1, -1 };
static volatile int shares[2] = { SOUND_SHARE_ALL, SOUND_SHARE_ALL };
/* (a state just loaded: every track told at the next frame) */
static bool loaded;

/* (on the core's thread where it has one) */
static HookAct vol_pit_set(HookRegs *r, void *user) {
	(void)user;
	uint32_t t = r->r[1];
	int share = shares[t >= music_lo && t < music_hi ? 0 : 1];
	if (share >= SOUND_SHARE_ALL) return HOOK_CONTINUE;
	uint8_t flags = hook_read8(t + BN6_TRACK_FLAGS);
	if (!(flags & BN6_TRACK_VOL_SET)) return HOOK_CONTINUE;
	/* the volume as BN6 works it out (TrkVolPitSet: docs/rom/sound.md),
	 * right and left, its share of it kept, and none left to work out:
	 * the pitch's part as BN6 works it */
	uint32_t x = (uint32_t)(hook_read8(t + BN6_TRACK_VOL) * hook_read8(t + BN6_TRACK_VOL_X)) >> 5;
	uint8_t mod_t = hook_read8(t + BN6_TRACK_MOD_T);
	int mod_m = (int8_t)hook_read8(t + BN6_TRACK_MOD_M);
	if (mod_t == 1) x = (x * (uint32_t)(mod_m + 128)) >> 7;
	int y = 2 * (int8_t)hook_read8(t + BN6_TRACK_PAN) + (int8_t)hook_read8(t + BN6_TRACK_PAN_X);
	if (mod_t == 2) y += mod_m;
	y = y < -128 ? -128 : y > 127 ? 127 : y;
	uint8_t right = (uint8_t)(((uint32_t)(y + 128) * x) >> 8), left = (uint8_t)(((uint32_t)(127 - y) * x) >> 8);
	hook_write8(t + BN6_TRACK_VOL_RIGHT, (uint8_t)((right * share + 128) >> 8));
	hook_write8(t + BN6_TRACK_VOL_LEFT, (uint8_t)((left * share + 128) >> 8));
	hook_write8(t + BN6_TRACK_FLAGS, (uint8_t)(flags & ~BN6_TRACK_VOL_SET));
	return HOOK_CONTINUE;
}

/* the shares of the levels heard now, true where either changed */
static bool take_levels(void) {
	int m = sound_heard_music(), e = sound_heard_effects();
	if (m == levels[0] && e == levels[1]) return false;
	levels[0] = m;
	levels[1] = e;
	int louder = m > e ? m : e;
	shares[0] = sound_heard_share(m, louder);
	shares[1] = sound_heard_share(e, louder);
	return true;
}

void sound_quiet_install(void) {
	if (installed) return;
	for (int p = 0; p < BN6_MPLAY_COUNT; ++p) {
		uint32_t row = BN6_MPLAYS + (uint32_t)p * BN6_MPLAY_ROW;
		tracks[p] = emu_read32(row + BN6_MPLAY_TRACKS);
		counts[p] = emu_read8(row + BN6_MPLAY_TRACK_COUNT);
		if (emu_read32(row) != BN6_MUSIC_PLAYER) continue;
		music_lo = tracks[p];
		music_hi = music_lo + counts[p] * BN6_TRACK_SIZE;
	}
	installed = music_hi > music_lo && emu_hook(BN6_M4A_TRK_VOL_PIT_SET, vol_pit_set, NULL);
	/* (before the boot, or a state's load: no track plays yet) */
	take_levels();
	loaded = false;
}

/* the tracks that play of the players `which` (bit 0 the music's, bit 1
 * the effects') told their volume changed: their player's next frame works
 * it out again, through the hook */
static void tell(int which) {
	for (int p = 0; p < BN6_MPLAY_COUNT; ++p) {
		if (!(which & (tracks[p] == music_lo ? 1 : 2))) continue;
		for (uint32_t i = 0, t = tracks[p]; i < counts[p]; ++i, t += BN6_TRACK_SIZE) {
			uint8_t flags = emu_read8(t + BN6_TRACK_FLAGS);
			if (flags & BN6_TRACK_EXISTS) emu_write8(t + BN6_TRACK_FLAGS, (uint8_t)(flags | BN6_TRACK_VOL_CHANGED));
		}
	}
}

void sound_quiet_frame(void) {
	if (!installed) return;
	int was[2] = { shares[0], shares[1] };
	bool changed = take_levels();
	int which = loaded ? 3 : changed ? (shares[0] != was[0] ? 1 : 0) | (shares[1] != was[1] ? 2 : 0) : 0;
	loaded = false;
	if (which) tell(which);
}

void sound_quiet_loaded(void) { loaded = true; }
