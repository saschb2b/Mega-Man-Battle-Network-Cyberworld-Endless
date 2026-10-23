/* The core's sound kept in step with the device: the rate's factor from the
 * ring's averaged fill (audio_pace.h). */
#include "audio_pace.h"

#define PACE_AVG  (1.0 / 40)   /* the average's weight a frame: about 0.67 s */
#define PACE_GAIN 0.02         /* the rate's change at a ring twice its target */
#define PACE_MAX  0.03         /* and at most */

double audio_pace(AudioPace *p, int fill, int target) {
	if (!p->primed) audio_pace_reset(p, fill);
	p->fill += (fill - p->fill) * PACE_AVG;
	double adj = 1.0 - PACE_GAIN * (p->fill - target) / target;
	if (adj < 1.0 - PACE_MAX) adj = 1.0 - PACE_MAX;
	if (adj > 1.0 + PACE_MAX) adj = 1.0 + PACE_MAX;
	return adj;
}

void audio_pace_reset(AudioPace *p, int fill) {
	p->fill = fill;
	p->primed = true;
}
