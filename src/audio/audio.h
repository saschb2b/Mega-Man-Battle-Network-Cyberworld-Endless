/* Sound: music and effects played from the ROM's own sound engine data. */
#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	SFX_CURSOR, SFX_SELECT, SFX_CONFIRM, SFX_CANCEL, SFX_ERROR,
	SFX_BUSTER, SFX_CHARGED, SFX_CHARGE_SHOT, SFX_CANNON, SFX_SWORD, SFX_THROW,
	SFX_EXPLODE, SFX_HIT, SFX_HURT, SFX_GUARD, SFX_RECOVER, SFX_WAVE, SFX_FLAME,
	SFX_THUNDER, SFX_WIND, SFX_GRAB, SFX_WARP, SFX_DASH, SFX_CUSTOM_OPEN,
	SFX_ITEM, SFX_JACK_IN, SFX_STEP,
	/* battle flow, as recorded from the original */
	SFX_ENCOUNTER, SFX_MATERIALIZE, SFX_CHIP_CURSOR, SFX_CHIP_SELECT, SFX_CHIP_OK,
	SFX_DESC_OPEN, SFX_DESC_CLOSE, SFX_GAUGE_FULL, SFX_DELETE, SFX_REVEAL, SFX_GOT, SFX_CHARGE_START,
	SFX_SHOCKWAVE, SFX_PICKAXE, SFX_LOW_HP, SFX_PAUSE, SFX_BOMB,
	SFX_COUNT
} Sfx;

typedef enum { MUS_NONE, MUS_TITLE, MUS_NET, MUS_BATTLE, MUS_BOSS, MUS_WIN, MUS_UNDERNET, MUS_SHOP, MUS_GAMEOVER, MUS_CREDITS, MUS_SORROW, MUS_COUNT } Music;

bool audio_init(void);
/* The frame CYBERWORLD_SFX_LOG's lines name, set by the main loop. */
extern uint64_t audio_log_frame;
/* Replaces the engine's own sound with another source (48 kHz stereo frames);
 * NULL returns to it. */
typedef int (*AudioSource)(int16_t *out, int frames);
void audio_external(AudioSource src);
/* Without a device but with CYBERWORLD_AUDIO_DUMP, renders one frame of sound. */
void audio_frame(void);
bool audio_offline(void);
/* Whether there is sound to play on: a device, or the offline dump */
bool audio_on(void);
/* The engine's sound runs on the device's thread: a change of its state
 * from the frame's thread is made between these (music_carry.c) */
void audio_lock(void);
void audio_unlock(void);
void audio_sfx(Sfx s);
/* The start's chime (intro_chime.h), once, beside the songs at the
 * effects' volume. */
void audio_chime(void);
void audio_music(Music m);
void audio_set_volume(int music, int sfx); /* 0-10 */
#ifdef __EMSCRIPTEN__
/* The device held silent (true) or playing again: the browser page's menu. */
void audio_pause(bool on);
#endif
/* Play a song-table entry directly (music replaces, effects mix). */
void audio_play_song(int id, bool music);
void audio_music_id(int id);
/* The Jukebox's songs (jukebox.h): song `song` of ROM `rom` (0 BN6's, 1 +
 * XRomId another's: mp2k.h) begun on the player its table names, stopped,
 * faded over 16 x `speed` frames; while it plays, how often it has gone
 * round (-1 where it does not play; 0 where there is no sound to play it
 * on, as if it played quiet) and its tracks' loudness, 0-255, into
 * `levels` (`max` at most, how many in *tracks) */
void audio_song_start(int rom, int song);
void audio_song_stop(int rom, int song);
void audio_song_fade(int rom, int song, int speed);
int audio_song_state(int rom, int song, uint8_t *levels, int max, int *tracks);
/* ... into a WAV file */
bool audio_render_wav(int rom, int song, int seconds, const char *path);

/* The game's music under the engine's own screens over a run (Options, the
 * touch controls' menu, the Library, a reward pick, the Battle Lab's
 * builder, the dev menu; music_carry.c): where one holds the game's core
 * still, BN6's song plays on on the engine's MP2K from where the game was
 * held, and goes back into the game, where the engine has brought it, as
 * the core runs again (mp2k_carry.h); where the game plays none, BN6's
 * NaviCust theme, the PET's own screen's song
 * (docs/engine/emulation/engine-sound.md, The game's music under the
 * engine's screens). Nothing where there is no sound to play on.
 * music_carry_held: a frame the core is held under such a screen, the song
 * taken at the first; music_carry_resume: the core about to run again, the
 * song given back; music_carry_drop: the game left (its scene, a soft
 * reset), the engine's song stopped, nothing given back. */
void music_carry_held(void);
void music_carry_resume(void);
void music_carry_drop(void);

/* Fast forward's music (music_fast.c; docs/engine/emulation/fast-forward.md,
 * The music): while the game runs fast, its song plays on at the GBA's pace on the
 * engine's MP2K, from where the game had it, and the game's own music
 * player runs on unheard (sound_quiet.h's music away), its effects heard
 * as fast forward hears them. The engine follows what the game's player
 * does meanwhile (a song begun, a fade, a stop: mp2k_carry_follow), and
 * gives the song back where it has brought it as the game runs at its
 * pace again (mp2k_carry.h, the same hand-over as under the engine's
 * screens). Its sound goes through the core's ring, a frame of the
 * engine's to each frame of the game's heard (emu_sound_mixer), so the
 * two hand over on a frame's edge. Nothing of the game's own changes but
 * its music's place.
 * Where settings.ini's `fast_music` is off (fast_forward.h), fast
 * forward's music plays faster, as its grains (emu.h). */
/* Before an update's frames of the game: whether they run fast (the song
 * taken, followed) or at the GBA's pace (given back) */
void music_fast_update(bool fast);
/* The game held under a screen of the engine's, or left to the title or
 * a quit: the song given back where the engine has it, its sound beside
 * the core's again */
void music_fast_held(void);
/* The game left (its scene, a soft reset): nothing given back */
void music_fast_drop(void);

/* The engine's sound heard through the core's ring rather than beside it
 * (true): audio_engine_add's alone, a frame of the core's at a time, in
 * step with the game's own sound (fast forward's music, music_fast_update) */
void audio_engine_in_ring(bool on);
/* ... its next `frames` added to a frame of the core's `out` (48 kHz
 * stereo, 1024 at most), as late in the frame as the core's own sound */
void audio_engine_add(int16_t *out, int frames);
/* ... its first frame the game's mix of the frame before (`n` samples of
 * each side, BN6's ring's), but for what the core plays of it itself */
void audio_engine_prime(const int8_t *right, const int8_t *left, int n);

#endif
