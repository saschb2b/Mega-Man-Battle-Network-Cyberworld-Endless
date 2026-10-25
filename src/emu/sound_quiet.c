/* The game's music or effects silenced in the core (sound_quiet.h;
 * docs/engine/emulation/core-sound.md). */
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
/* what the tracks were last told: bit 0 the music heard, bit 1 the
 * effects; -1 none (a state just loaded) */
static int told;

static int heard(void) { return (sound_heard_music() ? 1 : 0) | (sound_heard_effects() ? 2 : 0); }

/* (on the core's thread where it has one: the settings read there) */
static HookAct vol_pit_set(HookRegs *r, void *user) {
	(void)user;
	uint32_t t = r->r[1];
	bool music = t >= music_lo && t < music_hi;
	if (music ? sound_heard_music() : sound_heard_effects()) return HOOK_CONTINUE;
	uint8_t flags = hook_read8(t + BN6_TRACK_FLAGS);
	if (!(flags & BN6_TRACK_VOL_SET)) return HOOK_CONTINUE;
	/* (none, right and left, and none left to work out: the pitch's part
	 * as BN6 works it) */
	hook_write8(t + BN6_TRACK_VOL_RIGHT, 0);
	hook_write8(t + BN6_TRACK_VOL_LEFT, 0);
	hook_write8(t + BN6_TRACK_FLAGS, (uint8_t)(flags & ~BN6_TRACK_VOL_SET));
	return HOOK_CONTINUE;
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
	told = heard();
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
	int now = heard(), changed = told < 0 ? 3 : now ^ told;
	if (!changed) return;
	told = now;
	tell(changed);
}

void sound_quiet_loaded(void) { told = -1; }
