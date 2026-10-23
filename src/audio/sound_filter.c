/* The sound's shape on its way to the device (sound_filter.h).
 *
 * BN6 mixes its music at 10512 Hz (MP2K's rate 3: docs/ROM_DATA.md, the
 * sound's mix) and the GBA holds each sample until the next, so above
 * 5256 Hz the mix holds only its own images: a flat hiss some 20 dB under
 * the music, up to 22 kHz. The Legacy Collection plays CD recordings in
 * the game's place, and against ours on the same music (its net and virus
 * battle themes, aligned by their notes in 35 chunks of 6 s of a video of
 * it, docs/SOURCES.md) it is flat from 300 Hz to 5 kHz and falls away on
 * either side: -4.5 dB at 8 kHz, -10 at 10, -18 at 13, -29 at 16, and
 * -3.5 from 80 to 160 Hz, -12 at 40. Soft is three sections fitted to that
 * curve, within 0.8 dB of it from 35 Hz to 17 kHz: a low-pass at 6750 Hz
 * (Q 0.88), a high-pass at 68 Hz and a low shelf of -2.5 dB at 320 Hz. */
#include "sound_filter.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>

#define RATE      48000.0
#define SECTIONS  3

typedef struct { double b0, b1, b2, a1, a2; } Biquad;

static volatile int wanted = SOUND_GBA;   /* (set by the main thread) */
static int applied = SOUND_GBA;
static Biquad sec[SECTIONS];
static double z[SECTIONS][2][2];   /* each section's state, per channel */

/* The sections, by the bilinear transform prewarped at their frequency
 * (Robert Bristow-Johnson's cookbook): a pass of quality q, low or high */
static Biquad pass(double hz, double q, bool high) {
	double w = 2.0 * M_PI * hz / RATE, c = cos(w), alpha = sin(w) / (2.0 * q), a0 = 1.0 + alpha;
	double b0 = (high ? 1.0 + c : 1.0 - c) / 2.0;
	return (Biquad){ b0 / a0, (high ? -2.0 : 2.0) * b0 / a0, b0 / a0, -2.0 * c / a0, (1.0 - alpha) / a0 };
}

/* ... and a shelf of `db` below hz (its slope 1) */
static Biquad low_shelf(double hz, double db) {
	double A = pow(10.0, db / 40.0), w = 2.0 * M_PI * hz / RATE, c = cos(w), s = sin(w);
	double alpha = s / 2.0 * sqrt(2.0), r = 2.0 * sqrt(A) * alpha;
	double a0 = (A + 1.0) + (A - 1.0) * c + r;
	return (Biquad){ A * ((A + 1.0) - (A - 1.0) * c + r) / a0, 2.0 * A * ((A - 1.0) - (A + 1.0) * c) / a0,
		A * ((A + 1.0) - (A - 1.0) * c - r) / a0, -2.0 * ((A - 1.0) + (A + 1.0) * c) / a0,
		((A + 1.0) + (A - 1.0) * c - r) / a0 };
}

void sound_filter_set(SoundFilter f) { wanted = f; }
SoundFilter sound_filter(void) { return (SoundFilter)wanted; }

void sound_filter_run(int16_t *frames, int n) {
	int want = wanted;
	if (want != applied) {
		sec[0] = pass(6750.0, 0.88, false);
		sec[1] = pass(68.0, 0.70710678, true);
		sec[2] = low_shelf(320.0, -2.5);
		memset(z, 0, sizeof z);
		applied = want;
	}
	if (applied == SOUND_GBA) return;
	for (int i = 0; i < n; ++i) {
		for (int ch = 0; ch < 2; ++ch) {
			double x = frames[i * 2 + ch];
			for (int s = 0; s < SECTIONS; ++s) {
				const Biquad *b = &sec[s];
				double *st = z[s][ch], y = b->b0 * x + st[0];
				st[0] = b->b1 * x - b->a1 * y + st[1];
				st[1] = b->b2 * x - b->a2 * y;
				x = y;
			}
			frames[i * 2 + ch] = (int16_t)(x > 32767.0 ? 32767 : x < -32768.0 ? -32768 : lrint(x));
		}
	}
}
