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

typedef enum { MUS_NONE, MUS_TITLE, MUS_NET, MUS_BATTLE, MUS_BOSS, MUS_WIN, MUS_UNDERNET, MUS_SHOP, MUS_GAMEOVER, MUS_COUNT } Music;

bool audio_init(void);
/* Replaces the engine's own sound with another source (48 kHz stereo frames);
 * NULL returns to it. */
typedef int (*AudioSource)(int16_t *out, int frames);
void audio_external(AudioSource src);
/* Without a device but with CYBERWORLD_AUDIO_DUMP, renders one frame of sound. */
void audio_frame(void);
bool audio_offline(void);
void audio_sfx(Sfx s);
void audio_music(Music m);
void audio_set_volume(int music, int sfx); /* 0-10 */
/* Play a song-table entry directly (music replaces, effects mix). */
void audio_play_song(int id, bool music);
void audio_music_id(int id);
bool audio_render_wav(int song, int seconds, const char *path);

#endif
