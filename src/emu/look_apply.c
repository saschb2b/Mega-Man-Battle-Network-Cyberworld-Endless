/* STUB of looks-core's apply (issue #854, step 2): a look's field palette
 * for the Square's peers, a channel's ramp turned to a hue of the look's,
 * BN6's saturation and value kept. Dropped for looks-core's own when dev
 * brings it in. */
#include "look_apply.h"

#include <math.h>
#include <string.h>

#include "bn6.h"
#include "rom.h"

static uint16_t hue_to(uint16_t c, double hue) {
	double r = (c & 31) / 31.0, g = (c >> 5 & 31) / 31.0, b = (c >> 10 & 31) / 31.0;
	double mx = fmax(r, fmax(g, b)), mn = fmin(r, fmin(g, b)), v = mx, s = mx > 0 ? (mx - mn) / mx : 0;
	double h = fmod(hue, 360.0) / 60.0, f = h - floor(h), p = v * (1 - s), q = v * (1 - s * f), t = v * (1 - s * (1 - f));
	double o[3];
	switch ((int)floor(h) % 6) {
	case 0: o[0] = v; o[1] = t; o[2] = p; break;
	case 1: o[0] = q; o[1] = v; o[2] = p; break;
	case 2: o[0] = p; o[1] = v; o[2] = t; break;
	case 3: o[0] = p; o[1] = q; o[2] = v; break;
	case 4: o[0] = t; o[1] = p; o[2] = v; break;
	default: o[0] = v; o[1] = p; o[2] = q; break;
	}
	return (uint16_t)((int)lround(o[0] * 31) | (int)lround(o[1] * 31) << 5 | (int)lround(o[2] * 31) << 10);
}

bool look_palette_of(const Look *look, int which, uint16_t out16[16]) {
	if (which != LOOK_SPRITE_FIELD || !R.data) return false;
	const uint8_t *pal = R.data + (BN6_LOOK_FIELD_PALS - 0x08000000u);
	for (int i = 0; i < 16; ++i) out16[i] = (uint16_t)(pal[2 * i] | pal[2 * i + 1] << 8);
	if (!look) return true;
	static const int first[2] = { 2, 6 }, last[2] = { 5, 8 };
	for (int c = 0; c < 2; ++c) {
		if (look->palette[c] == LOOK_NONE) continue;
		double hue = (look->palette[c] * 29 + look->colour[c] * 23) % 360;
		for (int i = first[c]; i <= last[c]; ++i) out16[i] = hue_to(out16[i], hue);
	}
	return true;
}
