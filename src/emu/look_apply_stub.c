/* STUB until looks-core (issue #854) lands look_apply.c on dev: a look's
 * channels onto BN6's ramps by hue, BN6's saturation and value kept, from
 * the player's ROM file's bytes. Dropped as dev is brought in. */
#include "look_apply.h"

#include <math.h>

#include "bn6.h"
#include "bn6_forms.h"
#include "bn6_look.h"
#include "rom.h"

static void hsv_of(uint16_t c, float *h, float *s, float *v) {
	float r = (float)(c & 31) / 31.f, g = (float)((c >> 5) & 31) / 31.f, b = (float)((c >> 10) & 31) / 31.f;
	float mx = fmaxf(r, fmaxf(g, b)), mn = fminf(r, fminf(g, b)), d = mx - mn;
	*v = mx;
	*s = mx > 0 ? d / mx : 0;
	if (d <= 0) *h = 0;
	else if (mx == r) *h = fmodf((g - b) / d + 6.f, 6.f);
	else if (mx == g) *h = (b - r) / d + 2.f;
	else *h = (r - g) / d + 4.f;
}

static uint16_t of_hsv(float h, float s, float v) {
	float c = v * s, x = c * (1.f - fabsf(fmodf(h, 2.f) - 1.f)), m = v - c, r = 0, g = 0, b = 0;
	int k = (int)h % 6;
	if (k == 0) { r = c; g = x; } else if (k == 1) { r = x; g = c; } else if (k == 2) { g = c; b = x; }
	else if (k == 3) { g = x; b = c; } else if (k == 4) { r = x; b = c; } else { r = c; b = x; }
	return (uint16_t)((int)lroundf((r + m) * 31) | (int)lroundf((g + m) * 31) << 5 | (int)lroundf((b + m) * 31) << 10);
}

static void channel(const Look *look, int ch, int first, int last, const uint16_t *bn6, uint16_t *out) {
	if (look->palette[ch] == LOOK_NONE) return;
	float th, ts, tv, h, s, v;
	hsv_of(looks_colour(look->palette[ch], look->colour[ch]), &th, &ts, &tv);
	for (int i = first; i <= last; ++i) {
		hsv_of(bn6[i], &h, &s, &v);
		out[i] = of_hsv(th, s * fminf(1.f, ts / 0.6f), v);
	}
}

void look_apply(const Look *look) { (void)look; }
void look_apply_worn(void) {}

bool look_palette_of(const Look *look, int which, uint16_t out16[16]) {
	static const uint32_t at[LOOK_SPRITE_COUNT] = {
		[LOOK_SPRITE_BATTLE] = BN6_LOOK_BATTLE_PALS, [LOOK_SPRITE_FIELD] = BN6_LOOK_FIELD_PALS,
		[LOOK_SPRITE_MUGSHOT] = BN6_LOOK_MUGSHOT_PAL, [LOOK_SPRITE_FACE] = BN6_FACE_PALS,
		[LOOK_SPRITE_BATTLE_PALE] = BN6_LOOK_BATTLE_PALS + BN6_LOOK_PAL_PALE * BN6_LOOK_PAL_SIZE,
		[LOOK_SPRITE_BATTLE_SYNCHRO] = BN6_LOOK_BATTLE_PALS + BN6_LOOK_PAL_SYNCHRO * BN6_LOOK_PAL_SIZE,
	};
	if (which < 0 || which >= LOOK_SPRITE_COUNT || !R.data || at[which] - 0x08000000u + 32 > ROM_SIZE) return false;
	const uint8_t *p = R.data + (at[which] - 0x08000000u);
	uint16_t bn6[16];
	for (int i = 0; i < 16; ++i) bn6[i] = (uint16_t)(p[2 * i] | p[2 * i + 1] << 8);
	for (int i = 0; i < 16; ++i) out16[i] = bn6[i];
	if (!look) return true;
	if (which == LOOK_SPRITE_FACE) channel(look, LOOK_ARMOUR, BN6_LOOK_FACE_BLUE_FIRST, BN6_LOOK_FACE_BLUE_LAST, bn6, out16);
	else if (which != LOOK_SPRITE_FIELD && which != LOOK_SPRITE_MUGSHOT) {
		channel(look, LOOK_ARMOUR, BN6_LOOK_ARMOUR_FIRST, BN6_LOOK_ARMOUR_LAST, bn6, out16);
		channel(look, LOOK_SUIT, BN6_LOOK_SUIT_FIRST, BN6_LOOK_SUIT_LAST, bn6, out16);
		channel(look, LOOK_ACCENT, BN6_LOOK_GOLD, BN6_LOOK_GOLD, bn6, out16);
	}
	return true;
}
