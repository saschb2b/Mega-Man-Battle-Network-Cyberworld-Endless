/* lcd_frame.h. Each step is mGBA's libretro core's (_initColorCorrection
 * and the videoPostProcess functions in libretro.c), its numbers and its
 * float arithmetic in the same order, on the same RGB565 frames (its
 * mColorFrom555: red at the top, green's sixth bit 0); tests/test_core.c
 * holds results of mGBA's own code. */
#include "lcd_frame.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static LcdColour colour;
static LcdBlend blend;

/* ---- Color Correction: Pokefan531's GBA matrix and gamma (libretro's
 * gba-color shader, in the public domain), as mGBA's GBA_CC_* have them ---- */

#define CC_TARGET_GAMMA 2.2f
#define CC_GAMMA_ADJ    1.0f   /* (GBA_CC_GAMMA_ADJ: the GBA's screen darker) */
#define CC_RGB_MAX      31.0f
#define CC_LUM          0.94f
#define CC_R            0.82f
#define CC_G            0.665f
#define CC_B            0.73f
#define CC_RG           0.125f
#define CC_RB           0.195f
#define CC_GR           0.24f
#define CC_GB           0.075f
#define CC_BR           -0.06f
#define CC_BG           0.21f

/* a channel's light back to its 5 bits: kept from below 0, gamma
 * compressed, kept from above 1, rounded */
static unsigned cc_out(float v) {
	const float display_gamma_inv = 1.0f / CC_TARGET_GAMMA;
	v = v > 0.0f ? v : 0.0f;
	v = (float)pow(v, display_gamma_inv);
	v = v > 1.0f ? 1.0f : v;
	return (unsigned)((v * CC_RGB_MAX) + 0.5f) & 0x1F;
}

uint16_t lcd_correct565(uint16_t c) {
	const float rgb_max_inv = 1.0f / CC_RGB_MAX, gamma = CC_TARGET_GAMMA + CC_GAMMA_ADJ;
	/* (green read at its top five bits, as mGBA reads it) */
	float r = (float)pow((float)(c >> 11 & 0x1F) * rgb_max_inv, gamma);
	float g = (float)pow((float)(c >> 6 & 0x1F) * rgb_max_inv, gamma);
	float b = (float)pow((float)(c & 0x1F) * rgb_max_inv, gamma);
	unsigned ro = cc_out(CC_LUM * ((CC_R * r) + (CC_GR * g) + (CC_BR * b)));
	unsigned go = cc_out(CC_LUM * ((CC_RG * r) + (CC_G * g) + (CC_BG * b)));
	unsigned bo = cc_out(CC_LUM * ((CC_RB * r) + (CC_GB * g) + (CC_B * b)));
	return (uint16_t)(ro << 11 | go << 6 | bo);
}

/* every colour corrected once (mGBA's ccLUT, by the five bits of each
 * channel it reads) */
static uint16_t *table;

static bool table_made(void) {
	if (table) return true;
	if (!(table = malloc(32768 * sizeof *table))) return false;
	for (unsigned i = 0; i < 32768; ++i) table[i] = lcd_correct565((uint16_t)((i >> 10) << 11 | (i >> 5 & 0x1F) << 6 | (i & 0x1F)));
	return true;
}

static uint16_t corrected(uint16_t c) { return table[(unsigned)(c >> 11) << 10 | (unsigned)(c >> 6 & 0x1F) << 5 | (c & 0x1F)]; }

LcdColour lcd_colour(void) { return colour; }

void lcd_set_colour(LcdColour c) { colour = c == LCD_COLOUR_GBA && table_made() ? LCD_COLOUR_GBA : LCD_COLOUR_OFF; }

/* ---- Interframe Blending ---- */

#define RESPONSE      0.333f   /* LCD_RESPONSE_TIME: a frame's share of the one before */
#define RESPONSE_FAST 0.5f     /* LCD_RESPONSE_TIME_FAKE: Fast's, raised to look as long */

static int frame_n;            /* the buffers' pixels */
static uint16_t *now, *made;   /* the picture as 565, the frame blended from it */
static uint16_t *before[4];    /* the pictures before, the latest first (outputBufferPrev1-4) */
static float *acc;             /* Fast's running frame, R, G and B a pixel */
static bool seed;              /* the next picture stands for the ones before it */
static float response[4];

static void release(void) {
	free(now);
	free(made);
	free(acc);
	now = made = NULL;
	acc = NULL;
	for (int k = 0; k < 4; ++k) { free(before[k]); before[k] = NULL; }
}

/* the buffers the blending needs, for pictures of `n` pixels */
static bool ready(int n) {
	if (n != frame_n) { release(); frame_n = n; seed = true; }
	int k = blend == LCD_BLEND_SMART ? 3 : blend == LCD_BLEND_LCD ? 4 : blend == LCD_BLEND_SIMPLE ? 1 : 0;
	/* (cleared: each is seeded before it is read, but never left unknown) */
	if (!now) now = calloc((size_t)n, sizeof *now);
	if (!made) made = calloc((size_t)n, sizeof *made);
	bool ok = now && made;
	for (int i = 0; i < k; ++i) {
		if (!before[i]) { before[i] = calloc((size_t)n, sizeof *before[i]); seed = true; }
		ok = ok && before[i];
	}
	if (blend == LCD_BLEND_LCD_FAST && !acc) { acc = calloc((size_t)n * 3, sizeof *acc); seed = true; }
	return ok && (blend != LCD_BLEND_LCD_FAST || acc);
}

LcdBlend lcd_blend(void) { return blend; }

void lcd_set_blend(LcdBlend b) {
	blend = (unsigned)b < LCD_BLEND_COUNT ? b : LCD_BLEND_OFF;
	seed = true;
	/* (mGBA's response factors for the four frames before) */
	response[0] = RESPONSE;
	for (int k = 1; k < 4; ++k) response[k] = (float)pow(RESPONSE, (double)(k + 1));
}

/* blargg's mix of packed pixels, mGBA's for Simple and Smart: each
 * channel half of the two, rounded up */
static uint16_t mix(uint16_t a, uint16_t b) { return (uint16_t)((a + b + ((a ^ b) & 0x821)) >> 1); }

static void blend_simple(void) {
	for (int i = 0; i < frame_n; ++i) {
		uint16_t c = now[i], p = before[0][i];
		before[0][i] = c;
		made[i] = mix(c, p);
	}
}

/* mixed only where the frames alternate: the same as two before, not as
 * the one before */
static void blend_smart(void) {
	for (int i = 0; i < frame_n; ++i) {
		uint16_t c = now[i], p1 = before[0][i], p2 = before[1][i], p3 = before[2][i];
		before[0][i] = c;
		before[1][i] = p1;
		before[2][i] = p2;
		made[i] = ((c == p2 || p1 == p3) && c != p1 && c != p3 && p1 != p2) ? mix(c, p1) : c;
	}
}

/* a channel (at bit `at`) of the picture, moved toward each of the four
 * before it by their response: Harlequin's exponential drop-off */
static unsigned ghost(int i, int at) {
	float v = (float)(now[i] >> at & 0x1F);
	for (int k = 0; k < 4; ++k) v += ((float)(before[k][i] >> at & 0x1F) - v) * response[k];
	return (unsigned)(v + 0.5f) & 0x1F;
}

static void blend_lcd(void) {
	for (int i = 0; i < frame_n; ++i) {
		made[i] = (uint16_t)(ghost(i, 11) << 11 | ghost(i, 6) << 6 | ghost(i, 0));
		for (int k = 3; k > 0; --k) before[k][i] = before[k - 1][i];
		before[0][i] = now[i];
	}
}

/* a channel mixed into the running frame, which keeps the mix */
static unsigned ghost_fast(float *keep, unsigned cur) {
	float v = ((float)cur * (1.0f - RESPONSE_FAST)) + (RESPONSE_FAST * *keep);
	*keep = v;
	return (unsigned)(v + 0.5f) & 0x1F;
}

static void blend_lcd_fast(void) {
	for (int i = 0; i < frame_n; ++i) {
		float *a = acc + i * 3;
		unsigned c = now[i];
		made[i] = (uint16_t)(ghost_fast(a, c >> 11 & 0x1F) << 11 | ghost_fast(a + 1, c >> 6 & 0x1F) << 6 | ghost_fast(a + 2, c & 0x1F));
	}
}

/* the pictures before, and Fast's running frame, all the picture's */
static void seed_from_now(void) {
	for (int k = 0; k < 4; ++k)
		if (before[k]) memcpy(before[k], now, (size_t)frame_n * sizeof *now);
	if (acc)
		for (int i = 0; i < frame_n; ++i) {
			acc[i * 3] = (float)(now[i] >> 11 & 0x1F);
			acc[i * 3 + 1] = (float)(now[i] >> 6 & 0x1F);
			acc[i * 3 + 2] = (float)(now[i] & 0x1F);
		}
	seed = false;
}

/* ---- the frame ---- */

/* a 5- and a 6-bit channel's 8 bits, as a GPU reads a 565 texture (c / 31, c / 63) */
static uint8_t five[32], six[64];

static void eight_made(void) {
	if (!five[31])
		for (int i = 0; i < 64; ++i) {
			if (i < 32) five[i] = (uint8_t)((i * 510 + 31) / 62);
			six[i] = (uint8_t)((i * 510 + 63) / 126);
		}
}

void lcd_frame(uint32_t *px, int n, bool fresh) {
	if ((colour == LCD_COLOUR_OFF && blend == LCD_BLEND_OFF) || !ready(n)) return;
	eight_made();
	if (fresh || seed || blend == LCD_BLEND_OFF) {
		/* (the 8-bit picture back to mGBA's 565: its top five bits a channel) */
		for (int i = 0; i < n; ++i) {
			uint32_t c = px[i];
			now[i] = (uint16_t)((c >> 3 & 0x1F) << 11 | (c >> 11 & 0x1F) << 6 | (c >> 19 & 0x1F));
		}
		if (seed) seed_from_now();
		switch (blend) {
		case LCD_BLEND_SIMPLE: blend_simple(); break;
		case LCD_BLEND_SMART: blend_smart(); break;
		case LCD_BLEND_LCD: blend_lcd(); break;
		case LCD_BLEND_LCD_FAST: blend_lcd_fast(); break;
		default: memcpy(made, now, (size_t)n * sizeof *now); break;
		}
	}
	for (int i = 0; i < n; ++i) {
		uint16_t c = colour == LCD_COLOUR_GBA ? corrected(made[i]) : made[i];
		px[i] = 0xFF000000u | (uint32_t)five[c & 0x1F] << 16 | (uint32_t)six[c >> 5 & 0x3F] << 8 | five[c >> 11];
	}
}

/* ---- colours drawn over the frame ---- */

/* (an ARGB8888 colour as the frame's: its channels' top five bits, through
 * the table, back as a GPU reads 565; its alpha kept) */
static uint32_t argb_corrected(uint32_t c) {
	uint16_t k = corrected((uint16_t)((c >> 19 & 0x1F) << 11 | (c >> 11 & 0x1F) << 6 | (c >> 3 & 0x1F)));
	return (c & 0xFF000000u) | (uint32_t)five[k >> 11] << 16 | (uint32_t)six[k >> 5 & 0x3F] << 8 | five[k & 0x1F];
}

uint32_t lcd_correct_argb(uint32_t c) {
	if (colour != LCD_COLOUR_GBA) return c;
	eight_made();
	return argb_corrected(c);
}

void lcd_correct_pixels(uint32_t *px, int n) {
	if (colour != LCD_COLOUR_GBA) return;
	eight_made();
	for (int i = 0; i < n; ++i) px[i] = argb_corrected(px[i]);
}
