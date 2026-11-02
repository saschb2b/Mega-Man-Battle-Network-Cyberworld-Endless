/* lcd_frame.h. Each step is mGBA's libretro core's (_initColorCorrection
 * and the videoPostProcess functions in libretro.c), its numbers and its
 * float arithmetic in the same order, on the same RGB565 frames (its
 * mColorFrom555: red at the top, green's sixth bit 0); the handheld
 * shaders' pass is theirs, in a GPU's float; tests/test_core.c holds
 * results of mGBA's own code and of the shaders' math. */
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

/* ---- libretro's handheld colour shaders (glsl-shaders,
 * handheld/shaders/color: Pokefan531's numbers in hunterk's Color Mangler,
 * in the public domain), each with its #pragma parameter's default ---- */

typedef struct {
	float target_gamma;    /* (with its darken_screen: VBA's 1.0 times 1.7) */
	float display_gamma, lum;
	/* each channel's share of red, green and blue in the light out, the
	 * shader's mat4 by its names (column-major: red r, gr, br) */
	float r, g, b, rg, rb, gr, gb, br, bg;
} Shader;

static const Shader shaders[LCD_COLOUR_COUNT] = {
	/* vba-color: target and display gamma 1.45, darken_screen 1.0 */
	[LCD_COLOUR_VBA] = { 1.45f + (1.0f * 1.7f), 1.45f, 1.0f, 0.73f, 0.675f, 0.675f, 0.085f, 0.085f, 0.27f, 0.24f, 0.0f, 0.24f },
	/* nds-color (the DS Phat's) */
	[LCD_COLOUR_NDS] = { 1.91f, 1.91f, 0.89f, 0.87f, 0.645f, 0.73f, 0.10f, 0.10f, 0.255f, 0.17f, -0.125f, 0.255f },
	/* psp-color (the PSP 1000's and 2000's) */
	[LCD_COLOUR_PSP] = { 2.21f, 2.2f, 1.0f, 0.98f, 0.795f, 0.98f, 0.04f, 0.01f, 0.20f, 0.01f, -0.18f, 0.165f },
};

/* a channel's light out to its 8 bits, as a GPU writes its framebuffer:
 * at the display's gamma, kept within 0 and 1, to the nearest (a negative
 * one's pow is undefined in GLSL, a GPU's NaN written as 0: 0, as mGBA's) */
static unsigned shader_out(float v, float gamma_inv) {
	if (!(v > 0.0f)) return 0;
	v = powf(v, gamma_inv);
	return (unsigned)((v < 1.0f ? v : 1.0f) * 255.0f + 0.5f);
}

/* a channel of a 565 texel to light, read as a GPU reads one (a channel
 * over 31, green over 63): at the shader's gamma, times lum, kept within 0
 * and 1 (its contrast and saturation 1, its black levels 0, do nothing) */
static float shader_in(const Shader *s, unsigned v, float max) {
	float x = powf((float)v / max, s->target_gamma) * s->lum;
	return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x;
}

/* the light mixed by the matrix, and out at the display's gamma */
static uint32_t shader_mix(const Shader *s, float r, float g, float b) {
	const float gamma_inv = 1.0f / s->display_gamma;
	unsigned ro = shader_out(s->r * r + s->gr * g + s->br * b, gamma_inv);
	unsigned go = shader_out(s->rg * r + s->g * g + s->bg * b, gamma_inv);
	unsigned bo = shader_out(s->rb * r + s->gb * g + s->b * b, gamma_inv);
	return 0xFF000000u | bo << 16 | go << 8 | ro;
}

/* ---- every colour of the frame as the screen shows it ---- */

/* a 565 colour shown as a GPU shows a 565 texture (a channel over 31,
 * green over 63), R in the low byte */
static uint32_t shown(uint16_t c) {
	unsigned r = c >> 11, g = c >> 5 & 0x3F, b = c & 0x1F;
	return 0xFF000000u | ((b * 510 + 31) / 62) << 16 | ((g * 510 + 63) / 126) << 8 | (r * 510 + 31) / 62;
}

uint32_t lcd_shown565(LcdColour c, uint16_t px) {
	if (c == LCD_COLOUR_GBA) return shown(lcd_correct565(px));
	if ((unsigned)c >= LCD_COLOUR_COUNT || c == LCD_COLOUR_OFF) return shown(px);
	const Shader *s = &shaders[c];
	return shader_mix(s, shader_in(s, px >> 11, 31.0f), shader_in(s, px >> 5 & 0x3F, 63.0f), shader_in(s, px & 0x1F, 31.0f));
}

/* the colour's table, by the frame's 565 colour (mGBA's ccLUT for GBA,
 * which reads green's top five bits: an entry for each of the two), made
 * once each time a colour is chosen */
static uint32_t *table;

static bool table_made(LcdColour c) {
	if (!table && !(table = malloc(65536 * sizeof *table))) return false;
	if (c == LCD_COLOUR_GBA) {
		for (unsigned i = 0; i < 65536; ++i) table[i] = i & 0x20 ? table[i & ~0x20u] : shown(lcd_correct565((uint16_t)i));
		return true;
	}
	/* (each channel's light once: a shader's pass, value for value) */
	const Shader *s = &shaders[c];
	float five_in[32], six_in[64];
	for (unsigned v = 0; v < 64; ++v) {
		if (v < 32) five_in[v] = shader_in(s, v, 31.0f);
		six_in[v] = shader_in(s, v, 63.0f);
	}
	for (unsigned i = 0; i < 65536; ++i) table[i] = shader_mix(s, five_in[i >> 11], six_in[i >> 5 & 0x3F], five_in[i & 0x1F]);
	return true;
}

LcdColour lcd_colour(void) { return colour; }

void lcd_set_colour(LcdColour c) {
	if (c == colour && table) return;
	colour = (unsigned)c < LCD_COLOUR_COUNT && c != LCD_COLOUR_OFF && table_made(c) ? c : LCD_COLOUR_OFF;
}

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

void lcd_frame(uint32_t *px, int n, bool fresh) {
	if ((colour == LCD_COLOUR_OFF && blend == LCD_BLEND_OFF) || !ready(n)) return;
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
	if (colour == LCD_COLOUR_OFF)
		for (int i = 0; i < n; ++i) px[i] = shown(made[i]);
	else
		for (int i = 0; i < n; ++i) px[i] = table[made[i]];
}

/* ---- colours drawn over the frame ---- */

/* (an ARGB8888 colour as the frame's: its channels' top five bits a 565
 * colour, green's sixth bit 0, through the colour's table, red back at
 * bit 16; its alpha kept) */
static uint32_t argb_shown(uint32_t c) {
	uint32_t k = table[(c >> 19 & 0x1F) << 11 | (c >> 11 & 0x1F) << 6 | (c >> 3 & 0x1F)];
	return (c & 0xFF000000u) | (k & 0xFFu) << 16 | (k & 0xFF00u) | (k >> 16 & 0xFFu);
}

uint32_t lcd_correct_argb(uint32_t c) { return colour == LCD_COLOUR_OFF ? c : argb_shown(c); }

void lcd_correct_pixels(uint32_t *px, int n) {
	if (colour == LCD_COLOUR_OFF) return;
	for (int i = 0; i < n; ++i) px[i] = argb_shown(px[i]);
}
