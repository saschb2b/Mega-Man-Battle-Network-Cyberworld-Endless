/* Graphics decoded from the ROM, plus drawing helpers on the logical canvas. */
#ifndef GFX_H
#define GFX_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

enum {
	SPR_BATTLE = 0,   /* MegaMan, crosses, beast forms */
	SPR_VIRUS = 1,
	SPR_NAVI = 2,
	SPR_ATTACK = 3,   /* chip and buster effects */
	SPR_EFFECT = 4,
	SPR_HIT = 5,
	SPR_NPC = 6,
	SPR_OBJECT = 7,   /* overworld objects */
	SPR_MUGSHOT = 8,
	SPR_GUI = 9,
};

typedef struct Sprite Sprite;

typedef struct {
	Sprite *spr;
	int anim, frame;
	int timer;
	bool done;       /* reached the last frame of a non-looping animation */
	bool fresh;      /* started this frame: the next update leaves it alone */
} Anim;

enum {
	FX_FLASH = 1,       /* white hit flash */
	FX_GHOST = 2,       /* half transparent */
	FX_DARK = 4,
};

bool gfx_init(void);
Sprite *sprite_get(int category, int index);
int sprite_anim_count(const Sprite *s);
int sprite_frame_count(const Sprite *s, int anim);

void anim_play(Anim *a, Sprite *s, int anim);
void anim_update(Anim *a);
/* Draws with the sprite origin at (x, y). pal selects a palette bank
 * (virus versions use banks 1-5). */
void anim_draw(const Anim *a, int x, int y, bool flip, int pal, int fx);
/* pal = SPRITE_ROM_PAL + offset draws every object with that ROM palette
 * (the game loads some effects in palettes of their own). */
#define SPRITE_ROM_PAL 0x1000000
/* A plain white silhouette, as the game flashes hit and deleted sprites. */
#define SPRITE_WHITE 0x2000000
/* OBJ mosaic block size (0/1 off) and alpha (255 opaque) for sprite draws,
 * as the GBA's MOSAIC and BLDALPHA registers; callers restore them. */
extern int gfx_obj_mosaic;
extern int gfx_obj_alpha;
void sprite_draw_frame(Sprite *s, int anim, int frame, int x, int y, bool flip, int pal, int fx);

/* A tile reference for LZ77 block `lz` (add the offset into its data,
 * which starts with the game's 4-byte size word) for rom_tile(s). */
uint32_t gfx_lz_ref(uint32_t lz);
/* One 8x8 ROM tile with a ROM palette. flip: bit 0 horizontal, bit 1 vertical. */
void rom_tile(uint32_t tile, uint32_t pal, int x, int y, int flip);
/* A block of consecutive tiles (w x h tiles, row-major, as sprites use). */
void rom_tiles(uint32_t first, uint32_t pal, int x, int y, int w, int h, int flip);
/* Text in the game's bold battle font. Height 12 on screen. */
enum { TEXT_LEFT = 0, TEXT_CENTER = 1, TEXT_RIGHT = 2 };
int text_width(const char *s);
void text_draw(int x, int y, const char *s, SDL_Color c, int align);
void text_drawf(int x, int y, SDL_Color c, int align, const char *fmt, ...);
#define TEXT_H 12

/* Primitives. */
void fill_rect(int x, int y, int w, int h, SDL_Color c);

static inline SDL_Color rgba(int r, int g, int b, int a) { SDL_Color c = { (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a }; return c; }
#define WHITE rgba(255, 255, 255, 255)
#define BLACK rgba(0, 0, 0, 255)

static inline uint32_t bgr555(uint16_t c) {
	uint32_t r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
	r = (r << 3) | (r >> 2); g = (g << 3) | (g >> 2); b = (b << 3) | (b >> 2);
	return 0xFF000000u | r << 16 | g << 8 | b;
}

#endif
