/* lcd_canvas.h */
#include "lcd_canvas.h"

#include "lcd_frame.h"
#include "platform.h"

static bool was;
static unsigned gen;

/* whether the engine's colours are corrected now */
static bool on_now(void) {
	bool on = lcd_colour() == LCD_COLOUR_GBA && !platform_own_screen();
	if (on != was) {
		was = on;
		++gen;
	}
	return on;
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
