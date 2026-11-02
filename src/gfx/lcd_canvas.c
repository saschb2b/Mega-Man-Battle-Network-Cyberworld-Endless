/* lcd_canvas.h */
#include "lcd_canvas.h"

#include "lcd_frame.h"
#include "platform.h"

static LcdColour was;
static unsigned gen;

/* whether the engine's colours are corrected now (and the count stepped
 * where the colour they take changed) */
static bool on_now(void) {
	LcdColour c = platform_own_screen() ? LCD_COLOUR_OFF : lcd_colour();
	if (c != was) {
		was = c;
		++gen;
	}
	return c != LCD_COLOUR_OFF;
}

unsigned lcd_canvas_gen(void) {
	on_now();
	return gen;
}

SDL_Color lcd_canvas_colour(SDL_Color c) {
	if (!on_now()) return c;
	uint32_t k = lcd_correct_argb((uint32_t)c.r << 16 | (uint32_t)c.g << 8 | c.b);
	return (SDL_Color){ (Uint8)(k >> 16), (Uint8)(k >> 8), (Uint8)k, c.a };
}

uint32_t lcd_canvas_argb(uint32_t c) { return on_now() ? lcd_correct_argb(c) : c; }

void lcd_canvas_pixels(uint32_t *px, int n) {
	if (on_now()) lcd_correct_pixels(px, n);
}
