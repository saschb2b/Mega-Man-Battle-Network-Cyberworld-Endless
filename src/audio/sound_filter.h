/* The sound on its way to the device: shaped as settings.ini's `sound`
 * says (as the GBA makes it, soft, or through mGBA's Audio Filter at
 * `low_pass`), laid out for where it is heard as `output` says
 * (speakers, headphones or mono), at the louder of the music's and the
 * effects' levels (sound_heard.h). With the defaults (gba, speakers, the
 * game's own level) every sample leaves as it came. */
#ifndef SOUND_FILTER_H
#define SOUND_FILTER_H

#include <stdint.h>

typedef enum {
	SOUND_GBA,    /* untouched: BN6's 10512 Hz mix and the bright hiss above it */
	SOUND_SOFT,   /* the Legacy Collection's balance, measured: the highs above 5 kHz and the bass under 300 Hz taken down */
	SOUND_MGBA,   /* mGBA's libretro core's Audio Filter: a low-pass of one pole, at the level sound_low_pass says */
} SoundFilter;
#define SOUND_FILTER_COUNT 3

typedef enum {
	OUTPUT_SPEAKERS,     /* the stereo as the game makes it */
	OUTPUT_HEADPHONES,   /* each ear a little of the other side's (crossfeed.h) */
	OUTPUT_MONO,         /* both sides the same, left and right together */
} SoundOutput;
#define SOUND_OUTPUT_COUNT 3

/* mGBA's Audio Filter Level (mgba_audio_low_pass_range): 5 to 95 %, 5 a step */
#define SOUND_LOW_PASS_MIN     5
#define SOUND_LOW_PASS_MAX     95
#define SOUND_LOW_PASS_DEFAULT 60

/* settings.ini's words: gba, soft, mgba; speakers, headphones, mono (an
 * unknown word the default) */
SoundFilter sound_filter_read(const char *word);
const char *sound_filter_word(SoundFilter f);
SoundOutput sound_output_read(const char *word);
const char *sound_output_word(SoundOutput o);

void sound_filter_set(SoundFilter f);
SoundFilter sound_filter(void);
void sound_low_pass_set(int level);
int sound_low_pass(void);
void sound_output_set(SoundOutput o);
SoundOutput sound_output(void);
/* 48 kHz stereo frames, filtered in place at the louder `level` of the
 * music's and the effects' (sound_heard.h) (the device's thread) */
void sound_filter_run(int16_t *frames, int n, int level);

#endif
