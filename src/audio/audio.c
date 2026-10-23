/* The engine's own sound: BN6's songs and effects on BN6's own engine
 * (MP2K, mp2k.h) and the GBA's sound after it (gba_sound.h), as mGBA plays
 * the game, for the screens before and beside it; the embedded game's own
 * sound while it plays (audio_external), the engine's effects over it. */
#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "gba_sound.h"
#include "intro_chime.h"
#include "mp2k.h"
#include "rom.h"
#include "sound_filter.h"
#include "touch.h"

#define OUT_RATE 48000

static SDL_AudioDeviceID dev;
static int music_volume = 8, sfx_volume = 8;
static int current_music = -1;

/* ---- the engine's frames ---- */

/* the ring's part the engine mixed the frame before, which the GBA plays
 * this frame */
static int8_t prev_right[704], prev_left[704];

static void engine_frame(void) {
	mp2k_frame();
	int n = mp2k_per_frame();
	gba_sound_frame(prev_right, prev_left, n);
	memcpy(prev_right, mp2k_mix_right(), (size_t)n);
	memcpy(prev_left, mp2k_mix_left(), (size_t)n);
}

/* `frames` of the engine's sound, at the music's volume (8, the default,
 * the game's own level) */
static void render(int16_t *o, int frames) {
	int done = 0;
	while (done < frames) {
		while (gba_sound_avail() < frames - done) engine_frame();
		done += gba_sound_read(o + done * 2, frames - done);
	}
	if (music_volume == 8) return;
	for (int i = 0; i < frames * 2; ++i) {
		int v = o[i] * music_volume / 8;
		o[i] = (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
	}
}

/* The start's chime (intro_chime.c, a sound of Pixabay's), played once
 * beside the songs at the effects' volume */
static long chime_at = -1;   /* the sample next, -1 silent */

static void chime_mix(int16_t *out, int frames) {
	int g = sfx_volume * 3;   /* (sfx_volume / 10 x 0.75, in 40ths) */
	for (int f = 0; f < frames && chime_at >= 0; ++f) {
		int s = intro_chime[chime_at] * g / 40;
		for (int k = 0; k < 2; ++k) {
			int v = out[f * 2 + k] + s;
			out[f * 2 + k] = (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
		}
		if (++chime_at >= INTRO_CHIME_LEN) chime_at = -1;
	}
}

static FILE *dump; /* CYBERWORLD_AUDIO_DUMP: raw 48 kHz s16 stereo of everything played */
static bool offline; /* no device: audio_frame() renders the dump a frame at a time */

static AudioSource external; /* the embedded game, when it plays */

void audio_external(AudioSource src) { external = src; }

static void mix_sources(int16_t *out, int frames) {
	if (!external) {
		render(out, frames);
		if (chime_at >= 0) chime_mix(out, frames);
		return;
	}
	external(out, frames);
	/* (a PET screen's sounds over the held game: the Arena's way in and its card, the NetPass, Trade Central;
	 * the engine's music is never playing then) */
	if (!mp2k_busy()) return;
	static int16_t fx[4096 * 2];
	for (int done = 0; done < frames;) {
		int n = frames - done < 4096 ? frames - done : 4096;
		render(fx, n);
		for (int i = 0; i < n * 2; ++i) {
			int v = out[done * 2 + i] + fx[i];
			out[done * 2 + i] = (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
		}
		done += n;
	}
}

/* (everything heard, shaped as settings.ini's sound says) */
static void mix_out(int16_t *out, int frames) {
	mix_sources(out, frames);
	sound_filter_run(out, frames);
}

static void callback(void *ud, Uint8 *stream, int len) {
	(void)ud;
	mix_out((int16_t *)stream, len / 4);
	if (dump) fwrite(stream, 1, (size_t)len, dump);
}

/* ------------------------------------------------------------------ */
/* Public API */

static const int sfx_ids[SFX_COUNT] = {
	[SFX_CURSOR] = 0x66, [SFX_SELECT] = 0x67, [SFX_CONFIRM] = 0x81, [SFX_CANCEL] = 0x68, [SFX_ERROR] = 0x69,
	[SFX_BUSTER] = 0x6A, [SFX_CHARGED] = 0x72, [SFX_CHARGE_SHOT] = 0x6A, [SFX_CANNON] = 0xAE, [SFX_SWORD] = 0xB0,
	[SFX_THROW] = 0xB2, [SFX_EXPLODE] = 0x6C, [SFX_HIT] = 0x6D, [SFX_HURT] = 0x6B, [SFX_GUARD] = 0xBA,
	[SFX_RECOVER] = 0xC7, [SFX_WAVE] = 0x97, [SFX_FLAME] = 0xE5, [SFX_THUNDER] = 0xF8, [SFX_WIND] = 0xFC,
	[SFX_GRAB] = 0x10E, [SFX_WARP] = 0x76, [SFX_DASH] = 0xE6, [SFX_CUSTOM_OPEN] = 0x79,
	[SFX_ITEM] = 0x73, [SFX_JACK_IN] = 0x77, [SFX_STEP] = 0x98,
	[SFX_ENCOUNTER] = 0x78, [SFX_MATERIALIZE] = 0x94, [SFX_CHIP_CURSOR] = 0x7F, [SFX_CHIP_SELECT] = 0x81,
	[SFX_CHIP_OK] = 0x82, [SFX_DESC_OPEN] = 0x9C, [SFX_DESC_CLOSE] = 0x9E, [SFX_GAUGE_FULL] = 0x8F,
	[SFX_DELETE] = 0x6F, [SFX_REVEAL] = 0x7E, [SFX_GOT] = 0x95, [SFX_CHARGE_START] = 0x71,
	[SFX_SHOCKWAVE] = 0xA6, [SFX_PICKAXE] = 0x183, [SFX_LOW_HP] = 0x84, [SFX_PAUSE] = 0x9F, [SFX_BOMB] = 0x70,
};

static const int music_ids[MUS_COUNT] = {
	[MUS_NONE] = 0, [MUS_TITLE] = 0x01, [MUS_NET] = 0x13, [MUS_BATTLE] = 0x15, [MUS_BOSS] = 0x16,
	[MUS_WIN] = 0x19, [MUS_UNDERNET] = 0x14, [MUS_SHOP] = 0x1E, [MUS_GAMEOVER] = 0x1B,
	[MUS_CREDITS] = 0x1D,   /* (bn6f SONG_CREDITS, the staff roll's) */
};

bool audio_init(void) {
	if (!R.data || !mp2k_init()) return false;
	if (SDL_WasInit(SDL_INIT_AUDIO) == 0 && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return false;
	SDL_AudioSpec want = { 0 }, have;
	want.freq = OUT_RATE;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	/* (a desktop's half as long: sound a frame and a half sooner after
	 * what makes it; a handheld and a phone keep the longer, safer one. A
	 * browser played by keys takes the half too, its menus' sounds heard
	 * late; the core's ring stays at 32 ms there (emu.c), as two of these
	 * reads can come between two frames on the page's one thread; a phone's
	 * page keeps the longer.) */
#if defined(CW_DESKTOP)
	want.samples = 512;
#elif defined(__EMSCRIPTEN__)
	want.samples = touch_shown() ? 1024 : 512;
#else
	want.samples = 1024;
#endif
	want.callback = callback;
	const char *dp = getenv("CYBERWORLD_AUDIO_DUMP");
	if (dp) dump = fopen(dp, "wb");
	/* frame-exact dumps for comparisons: no device, one frame of sound per frame */
	if (dump && getenv("CYBERWORLD_AUDIO_OFFLINE")) { offline = true; return true; }
	dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (!dev) {
		fprintf(stderr, "audio: %s\n", SDL_GetError());
		offline = dump != NULL;
		return offline;
	}
	SDL_PauseAudioDevice(dev, 0);
	printf("audio %d Hz, %d channels, %d-sample buffer\n", have.freq, have.channels, have.samples);
	return true;
}

bool audio_offline(void) { return offline; }

void audio_frame(void) {
	if (!offline) return;
	static int16_t buf[OUT_RATE / 60 * 2];
	mix_out(buf, OUT_RATE / 60);
	fwrite(buf, sizeof buf, 1, dump);
}

static void lock(void) { if (dev) SDL_LockAudioDevice(dev); }
static void unlock(void) { if (dev) SDL_UnlockAudioDevice(dev); }

/* (each song on the player BN6's song table names, as m4aSongNumStart
 * plays it: an effect over another on its player as its priority lets it) */
void audio_play_song(int id, bool music) {
	lock();
	if (music) {
		printf("music: song %#x\n", id);
		current_music = id;
	}
	mp2k_song_start(id);
	unlock();
}

void audio_sfx(Sfx s) {
	if (s < SFX_COUNT && getenv("CYBERWORLD_SFX_LOG")) {
		fprintf(stderr, "sfx %llu %#x\n", (unsigned long long)audio_log_frame, sfx_ids[s]);
	}
	if ((!dev && !offline) || s >= SFX_COUNT || !sfx_ids[s]) return;
	audio_play_song(sfx_ids[s], false);
}

uint64_t audio_log_frame;

void audio_chime(void) {
	if (!dev && !offline) return;
	lock();
	chime_at = 0;
	unlock();
}

void audio_music_id(int id) {
	if (getenv("CYBERWORLD_SFX_LOG")) fprintf(stderr, "music %llu %#x\n", (unsigned long long)audio_log_frame, id);
	if (!dev && !offline) return;
	if (id == current_music && mp2k_song_playing(id)) return;
	if (id == 0) {
		lock();
		/* (m4aMPlayFadeOut, 16 steps of two frames) */
		if (current_music > 0) mp2k_song_fade(current_music, 2);
		current_music = 0;
		unlock();
		return;
	}
	audio_play_song(id, true);
}

void audio_song_start(int rom, int song) {
	if (!dev && !offline) return;
	lock();
	printf("music: song %#x of rom %d\n", song, rom);
	mp2k_song_start_in(rom, song);
	unlock();
}

void audio_song_stop(int rom, int song) {
	if (!dev && !offline) return;
	lock();
	mp2k_song_stop_in(rom, song);
	unlock();
}

void audio_song_fade(int rom, int song, int speed) {
	if (!dev && !offline) return;
	lock();
	mp2k_song_fade_in(rom, song, speed);
	unlock();
}

int audio_song_state(int rom, int song, uint8_t *levels, int max, int *tracks) {
	*tracks = 0;
	/* (no sound to play it on: as if it played, quiet, and never went round) */
	if (!dev && !offline) return 0;
	lock();
	int loops = mp2k_song_loops(rom, song);
	*tracks = mp2k_song_levels(rom, song, levels, max);
	unlock();
	return loops;
}

void audio_music(Music m) {
	if (m >= MUS_COUNT) return;
	audio_music_id(music_ids[m]);
}

#ifdef __EMSCRIPTEN__
void audio_pause(bool on) {
	if (dev) SDL_PauseAudioDevice(dev, on ? 1 : 0);
}
#endif

void audio_set_volume(int music, int sfx) {
	music_volume = music;
	sfx_volume = sfx;
}

/* Offline render for tests: writes a 16-bit stereo WAV. */
bool audio_render_wav(int rom, int song, int seconds, const char *path) {
	if (!mp2k_init()) return false;
	mp2k_song_start_in(rom, song);
	FILE *f = fopen(path, "wb");
	if (!f) return false;
	uint32_t frames = (uint32_t)(OUT_RATE * seconds), bytes = frames * 4;
	uint8_t hdr[44] = { 'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ', 16, 0, 0, 0, 1, 0, 2, 0 };
	uint32_t v;
	v = 36 + bytes; memcpy(hdr + 4, &v, 4);
	v = OUT_RATE; memcpy(hdr + 24, &v, 4);
	v = OUT_RATE * 4; memcpy(hdr + 28, &v, 4);
	hdr[32] = 4; hdr[34] = 16;
	memcpy(hdr + 36, "data", 4);
	memcpy(hdr + 40, &bytes, 4);
	fwrite(hdr, 1, 44, f);
	int16_t buf[2048 * 2];
	for (uint32_t done = 0; done < frames;) {
		int n = frames - done > 2048 ? 2048 : (int)(frames - done);
		render(buf, n);
		fwrite(buf, 4, (size_t)n, f);
		done += (uint32_t)n;
	}
	fclose(f);
	return true;
}
