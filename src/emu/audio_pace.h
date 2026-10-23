/* The core's sound kept in step with the device (emu.c's ring): the GBA
 * makes 59.73 frames of sound a second and the loop runs 60, so the rate
 * the core's sound is resampled to is nudged by how full the ring is. */
#ifndef AUDIO_PACE_H
#define AUDIO_PACE_H

#include <stdbool.h>

typedef struct {
	double fill;   /* the ring's fill, averaged over about two thirds of a second */
	bool primed;
} AudioPace;

/* The ring's fill after a frame's sound in: the factor on the core's output
 * rate (1 the device's own). From the average, not the fill itself, as the
 * device reads in blocks of up to a few frames: the fill seen after each
 * frame swings by a block, and the pitch followed it frame to frame (10 to
 * 40 cents, a warble on every held note). */
double audio_pace(AudioPace *p, int fill, int target);
/* The ring cut to `fill`, too full: the average starts there again. */
void audio_pace_reset(AudioPace *p, int fill);

#endif
