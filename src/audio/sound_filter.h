/* The sound on its way to the device, shaped as settings.ini's `sound`
 * says: as the GBA makes it, or soft. */
#ifndef SOUND_FILTER_H
#define SOUND_FILTER_H

#include <stdint.h>

typedef enum {
	SOUND_GBA,    /* untouched: BN6's 10512 Hz mix and the bright hiss above it */
	SOUND_SOFT,   /* the Legacy Collection's balance, measured: the highs above 5 kHz and the bass under 300 Hz taken down */
} SoundFilter;

void sound_filter_set(SoundFilter f);
SoundFilter sound_filter(void);
/* 48 kHz stereo frames, filtered in place (the device's thread) */
void sound_filter_run(int16_t *frames, int n);

#endif
