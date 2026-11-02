/* Headphones' crossfeed, as bs2b (Boris Mikhaylov's Bauer
 * stereophonic-to-binaural DSP, MIT) makes it: each ear given a low-passed
 * share of the other side's sound, the share's filter delaying it as a
 * head does, and its own side's highs lifted to keep the balance. For
 * Output's Headphones (sound_filter.h). */
#ifndef CW_CROSSFEED_H
#define CW_CROSSFEED_H

/* bs2b's default level: the share's cut at 700 Hz, 4.5 dB under its own side */
#define CROSSFEED_FCUT 700
#define CROSSFEED_FEED 45   /* in tenths of a dB */

typedef struct {
	double a0_lo, b1_lo;         /* the share's low-pass */
	double a0_hi, a1_hi, b1_hi;  /* its own side's high boost */
	double gain;                 /* both together brought back to the level */
	double lo[2], hi[2], asis[2];
} Crossfeed;

/* `rate` Hz, the share cut at `fcut` Hz and `feed` tenths of a dB under its own side; its state cleared */
void crossfeed_init(Crossfeed *c, double rate, int fcut, int feed);
/* a frame, its left and right in place */
void crossfeed_run(Crossfeed *c, double *left, double *right);

#endif
