/* Headphones' crossfeed (crossfeed.h): bs2b's init() and cross_feed_d(),
 * its numbers as libbs2b 3.1.0's src/bs2b.c has them (read 2 November
 * 2026), written here for one rate and doubles.
 *
 * bs2b is Copyright (c) 2005 Boris Mikhaylov, under the MIT licence:
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions: The above copyright notice and this
 * permission notice shall be included in all copies or substantial
 * portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT
 * WARRANTY OF ANY KIND. */
#include "crossfeed.h"

#include <math.h>
#include <string.h>

void crossfeed_init(Crossfeed *c, double rate, int fcut, int feed) {
	double level = feed / 10.0;
	/* (the share's gain and its own side's at the lowest frequencies, in
	 * dB; level is their difference) */
	double gb_lo = level * -5.0 / 6.0 - 3.0, gb_hi = level / 6.0 - 3.0;
	double g_lo = pow(10.0, gb_lo / 20.0), g_hi = 1.0 - pow(10.0, gb_hi / 20.0);
	double fc_hi = fcut * pow(2.0, (gb_lo - 20.0 * log10(g_hi)) / 12.0);
	double x = exp(-2.0 * M_PI * fcut / rate);
	memset(c, 0, sizeof *c);
	c->b1_lo = x;
	c->a0_lo = g_lo * (1.0 - x);
	x = exp(-2.0 * M_PI * fc_hi / rate);
	c->b1_hi = x;
	c->a0_hi = 1.0 - g_hi * (1.0 - x);
	c->a1_hi = -x;
	c->gain = 1.0 / (1.0 - g_hi + g_lo);
}

void crossfeed_run(Crossfeed *c, double *left, double *right) {
	double in[2] = { *left, *right };
	for (int ch = 0; ch < 2; ++ch) {
		c->lo[ch] = c->a0_lo * in[ch] + c->b1_lo * c->lo[ch];
		c->hi[ch] = c->a0_hi * in[ch] + c->a1_hi * c->asis[ch] + c->b1_hi * c->hi[ch];
		c->asis[ch] = in[ch];
	}
	*left = (c->hi[0] + c->lo[1]) * c->gain;
	*right = (c->hi[1] + c->lo[0]) * c->gain;
}
