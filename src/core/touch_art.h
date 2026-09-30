/* The touch controls' art, drawn smooth at the screen's own resolution in
 * the PET's colours: glass plates (a see-through fill, a light edge and a
 * dark rim that keeps them apart from any picture under them), the D-pad
 * as a cross, the letters in the engine's 3x5 font at a whole scale. Each
 * picture is made once for its size and look and kept as a texture. */
#ifndef CW_TOUCH_ART_H
#define CW_TOUCH_ART_H

#include <SDL.h>
#include <stdbool.h>

enum {
	ART_PLATE,   /* a box with rounded corners (radius); a pill when it is half the height */
	ART_DISC,    /* a round button (w its diameter) */
	ART_DPAD,    /* the D-pad's cross, its arrows and its middle */
	ART_ARM,     /* one arm of the D-pad lit (arm: 0 up, 1 right, 2 down, 3 left), its glow and arrow */
	ART_DASHED,  /* a dashed frame round a round control (round) or a box (the editor's choice) */
};

/* What a picture shows; zeroed before it is filled, as it is the key the
 * pictures are kept by. Sizes and widths in screen pixels. */
typedef struct {
	int kind;
	float w, h, radius;
	float edge_w, rim_w;          /* the light edge's width; the dark rim's round it */
	SDL_Color edge, fill;         /* (alpha included) */
	float shrink;                 /* pressed: drawn this much smaller (1 or 0 as it is) */
	float glow;                   /* pressed: a glow this far out (a fraction of the half size) */
	SDL_Color glow_color;
	char label[16];
	int scale;                    /* the letters' pixels (0: none) */
	SDL_Color ink, ink_edge;
	int arm;
	bool round;
} ArtSpec;

/* The picture for s: its texture and where its middle is on it. */
typedef struct { SDL_Texture *tex; int w, h; float mx, my; } Art;
Art art_get(const ArtSpec *s);
/* Draws s with its middle at (cx, cy), `alpha` (0-1) of its opacity. */
void art_draw(const ArtSpec *s, float cx, float cy, float alpha);
/* Letters straight on the screen (the menus' words): `align` -1 from x, 0
 * centred on it, 1 ending at it; y their middle. Returns their width. */
int art_text(float x, float y, const char *s, int scale, SDL_Color ink, SDL_Color outline, int align);
int art_text_width(const char *s, int scale);
/* A frame drawn: pictures unused for a while are let go. */
void art_tick(void);
/* All let go (the renderer lost its textures, or the controls went away). */
void art_flush(void);

#endif
