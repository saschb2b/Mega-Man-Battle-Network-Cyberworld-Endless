/* The sound's shape on its way to the device (sound_filter.h).
 *
 * BN6 mixes its music at 10512 Hz (MP2K's rate 3: docs/rom/sound.md, the
 * sound's mix) and the GBA holds each sample until the next, so above
 * 5256 Hz the mix holds only its own images: a flat hiss some 20 dB under
 * the music, up to 22 kHz. The Legacy Collection plays CD recordings in
 * the game's place, and against ours on the same music (its net and virus
 * battle themes, aligned by their notes in 35 chunks of 6 s of a video of
 * it, docs/research/sources.md) it is flat from 300 Hz to 5 kHz and falls away on
 * either side: -4.5 dB at 8 kHz, -10 at 10, -18 at 13, -29 at 16, and
 * -3.5 from 80 to 160 Hz, -12 at 40. Soft is three sections fitted to that
 * curve, within 0.8 dB of it from 35 Hz to 17 kHz: a low-pass at 6750 Hz
 * (Q 0.88), a high-pass at 68 Hz and a low shelf of -2.5 dB at 320 Hz.
 *
 * mGBA's Audio Filter is its libretro core's _audioLowPassFilter
 * (src/platform/libretro/libretro.c, read 2 November 2026), every step of
 * it: one pole in 16.16 fixed point, its factor the level's share of
 * 0x10000, the sum shifted down 16 and kept as it falls. That core has run
 * it at 32768 Hz (until January 2025), at 65536 Hz, and at the frontend's
 * own rate (October 2025 to August 2026); here at the device's 48000, as
 * that last core did on a 48 kHz RetroArch: the same samples in give the
 * same samples out (docs/engine/sound-out.md). */
#include "sound_filter.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "crossfeed.h"
#include "sound_heard.h"

#define RATE      48000.0
#define SECTIONS  3

typedef struct { double b0, b1, b2, a1, a2; } Biquad;

/* (set by the main thread) */
static volatile int wanted = SOUND_GBA, wanted_low = SOUND_LOW_PASS_DEFAULT, wanted_out = OUTPUT_SPEAKERS;
static int applied = SOUND_GBA, applied_out = OUTPUT_SPEAKERS;
static Biquad sec[SECTIONS];
static double z[SECTIONS][2][2];   /* each section's state, per channel */
static int32_t low_prev[2];        /* mGBA's audioLowPassLeftPrev and ...RightPrev */
static Crossfeed feed;

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

/* mGBA's _audioLowPassFilter, `range` its audioLowPassRange (the level's
 * share of 0x10000). (The sums stay within 32 bits: the two factors add
 * to 0x10000.) */
static void mgba_low_pass(int16_t *out, int count, int32_t range) {
	int32_t left = low_prev[0], right = low_prev[1];
	int32_t factor_a = range, factor_b = 0x10000 - factor_a;
	for (int i = 0; i < count; ++i) {
		left = (left * factor_a) + (out[0] * factor_b);
		right = (right * factor_a) + (out[1] * factor_b);
		left >>= 16;
		right >>= 16;
		out[0] = (int16_t)left;
		out[1] = (int16_t)right;
		out += 2;
	}
	low_prev[0] = left;
	low_prev[1] = right;
}

static const char *const filter_words[] = { "gba", "soft", "mgba" };
static const char *const output_words[] = { "speakers", "headphones", "mono" };

SoundFilter sound_filter_read(const char *word) {
	for (int i = 0; i < SOUND_FILTER_COUNT; ++i)
		if (!strcmp(word, filter_words[i])) return (SoundFilter)i;
	return SOUND_GBA;
}
const char *sound_filter_word(SoundFilter f) { return filter_words[(unsigned)f < SOUND_FILTER_COUNT ? f : SOUND_GBA]; }

SoundOutput sound_output_read(const char *word) {
	for (int i = 0; i < SOUND_OUTPUT_COUNT; ++i)
		if (!strcmp(word, output_words[i])) return (SoundOutput)i;
	return OUTPUT_SPEAKERS;
}
const char *sound_output_word(SoundOutput o) { return output_words[(unsigned)o < SOUND_OUTPUT_COUNT ? o : OUTPUT_SPEAKERS]; }

void sound_filter_set(SoundFilter f) { wanted = f; }
SoundFilter sound_filter(void) { return (SoundFilter)wanted; }

void sound_low_pass_set(int level) {
	wanted_low = level < SOUND_LOW_PASS_MIN ? SOUND_LOW_PASS_MIN : level > SOUND_LOW_PASS_MAX ? SOUND_LOW_PASS_MAX : level;
}
int sound_low_pass(void) { return wanted_low; }

void sound_output_set(SoundOutput o) { wanted_out = o; }
SoundOutput sound_output(void) { return (SoundOutput)wanted_out; }

/* a frame's two sides through Soft's sections, where it is chosen; laid
 * out for the output; at the gain */
static void frame_run(int16_t *frame, double gain) {
	double s[2] = { frame[0], frame[1] };
	if (applied == SOUND_SOFT) {
		for (int ch = 0; ch < 2; ++ch) {
			double x = s[ch];
			for (int k = 0; k < SECTIONS; ++k) {
				const Biquad *b = &sec[k];
				double *st = z[k][ch], y = b->b0 * x + st[0];
				st[0] = b->b1 * x - b->a1 * y + st[1];
				st[1] = b->b2 * x - b->a2 * y;
				x = y;
			}
			s[ch] = x;
		}
	}
	if (applied_out == OUTPUT_HEADPHONES) crossfeed_run(&feed, &s[0], &s[1]);
	else if (applied_out == OUTPUT_MONO) s[0] = s[1] = (s[0] + s[1]) * 0.5;
	for (int ch = 0; ch < 2; ++ch) {
		double x = s[ch] * gain;
		frame[ch] = (int16_t)(x > 32767.0 ? 32767 : x < -32768.0 ? -32768 : lrint(x));
	}
}

void sound_filter_run(int16_t *frames, int n, int level) {
	int want = wanted, out = wanted_out;
	if (want != applied) {
		sec[0] = pass(6750.0, 0.88, false);
		sec[1] = pass(68.0, 0.70710678, true);
		sec[2] = low_shelf(320.0, -2.5);
		memset(z, 0, sizeof z);
		applied = want;
	}
	if (out != applied_out) {
		crossfeed_init(&feed, RATE, CROSSFEED_FCUT, CROSSFEED_FEED);
		applied_out = out;
	}
	double gain = sound_heard_gain(level);
	if (gain == 0.0) {
		memset(frames, 0, (size_t)n * 4);
		return;
	}
	if (applied == SOUND_MGBA) mgba_low_pass(frames, n, wanted_low * 0x10000 / 100);
	if (applied != SOUND_SOFT && applied_out == OUTPUT_SPEAKERS && gain == 1.0) return;
	for (int i = 0; i < n; ++i) frame_run(frames + i * 2, gain);
}
