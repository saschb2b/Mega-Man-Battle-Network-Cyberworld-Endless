/* The GBA's screen over the core's picture (issue #265): Color Correction
 * and Interframe Blending as mGBA's libretro core makes them (libretro/mgba,
 * src/platform/libretro/libretro.c, the core RetroArch runs, whose options
 * these are), step for step: the picture as its RGB565 frame, blended with
 * the frames before it, then each colour through the correction's table,
 * and shown as a GPU shows a 565 frame. What the engine draws over the
 * picture takes the same correction from lcd_canvas.h.
 * docs/engine/lcd.md */
#ifndef CW_LCD_FRAME_H
#define CW_LCD_FRAME_H

#include <stdbool.h>
#include <stdint.h>

/* Color Correction: off, or a GBA's (mgba_color_correction's OFF and GBA;
 * its GBC and Auto are a Game Boy's, or the GBA's for a GBA game) */
typedef enum { LCD_COLOUR_OFF, LCD_COLOUR_GBA, LCD_COLOUR_COUNT } LcdColour;
/* Interframe Blending (mgba_interframe_blending): off; Simple (mix), half
 * the frame before; Smart (mix_smart), that only where a pixel flickers;
 * LCD Ghosting, Accurate (lcd_ghosting) the four frames before fading out,
 * Fast (lcd_ghosting_fast) a running half */
typedef enum { LCD_BLEND_OFF, LCD_BLEND_SIMPLE, LCD_BLEND_SMART, LCD_BLEND_LCD, LCD_BLEND_LCD_FAST, LCD_BLEND_COUNT } LcdBlend;

LcdColour lcd_colour(void);
LcdBlend lcd_blend(void);
/* Set; a blending turned on starts from the next picture, which stands for
 * the frames before it (mGBA's start from white: no flash here) */
void lcd_set_colour(LcdColour c);
void lcd_set_blend(LcdBlend b);

/* The core's picture, `n` pixels (240x160, R in the low byte, as
 * emu_video's) through them, in place: where `fresh` (a picture the core
 * made since the last), blended into the frames before it; else the frame
 * made from the last again (the game paused under Options), as a frontend
 * shows its last while the core runs no frame. Nothing where both are off. */
void lcd_frame(uint32_t *px, int n, bool fresh);

/* mGBA's correction of a 565 colour (ccLUT's entry, the tests) */
uint16_t lcd_correct565(uint16_t c);

/* An ARGB8888 colour (R at bit 16), and `n` pixels of them in place,
 * corrected as lcd_frame corrects the picture's (its alpha kept); as they
 * are where the correction is off. What the engine draws over the picture
 * takes them through lcd_canvas.h. */
uint32_t lcd_correct_argb(uint32_t c);
void lcd_correct_pixels(uint32_t *px, int n);

#endif
