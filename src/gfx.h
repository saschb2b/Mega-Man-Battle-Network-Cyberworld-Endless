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
/* One object of a frame on its own (the game moves a bomb and its shadow apart). */
void sprite_draw_part(Sprite *s, int anim, int frame, int part, int x, int y, bool flip, int pal);
/* Bounding box of a frame relative to its origin. */
SDL_Rect sprite_frame_bounds(Sprite *s, int anim, int frame, int pal);

/* Battle panels: type 1-12 (see battle.h), row 0-2, side 0 red / 1 blue. */
void panel_draw(int type, int row, int side, int x, int y);
void panel_edge_draw(int side, int x, int y);
/* A panel in the yellow palette the game flashes where an attack will land. */
void panel_draw_warn(int type, int row, int side, int x, int y);

/* Chip artwork (56x48) and icons (16x16). */
SDL_Texture *chip_image(int chip);
SDL_Texture *chip_icon(int chip);
/* The Zenny picture the RESULT window shows for money. */
SDL_Texture *zenny_image(void);

/* A palette assembled at runtime (the game builds some from several tables).
 * Returns a handle usable wherever a ROM palette offset is expected. */
uint32_t gfx_palette(const uint16_t colors[16]);
/* One 8x8 ROM tile with a ROM palette. flip: bit 0 horizontal, bit 1 vertical. */
void rom_tile(uint32_t tile, uint32_t pal, int x, int y, int flip);
/* A block of consecutive tiles (w x h tiles, row-major, as sprites use). */
void rom_tiles(uint32_t first, uint32_t pal, int x, int y, int w, int h, int flip);
/* The same block rotated (degrees, clockwise) and scaled about its centre. */
void rom_tiles_affine(uint32_t first, uint32_t pal, int x, int y, int w, int h, double angle, double scale);
/* The original battle UI windows, rebuilt from recorded ROM tile layouts. */
enum { HUD_CUSTOM, HUD_RESULT, HUD_TEXT, HUD_FOLDER, HUD_GAMEOVER_BG, HUD_GAMEOVER_TEXT };
void hud_window(int which, int x, int y);
void hud_banner(int which, int x, int y); /* 0 BATTLE START, 1 ENEMY DELETED, 2 MEGAMAN DELETED */
void hud_banner_scaled(int which, int x, int y, int pd);
/* The PET menu with button `selected` (0-7) highlighted. */
void hud_pet_menu(int x, int y, int selected);

/* The game's battle background `id` (0x00-0x15) at `frame` frames into the
 * battle: animated and scrolled as the original. */
void battle_bg_draw(int id, int frame);

/* Text in the game's bold battle font. Height 12 on screen. */
enum { TEXT_LEFT = 0, TEXT_CENTER = 1, TEXT_RIGHT = 2 };
int text_width(const char *s);
void text_draw(int x, int y, const char *s, SDL_Color c, int align);
void text_drawf(int x, int y, SDL_Color c, int align, const char *fmt, ...);
/* Multi-line text split on '\n'; returns the number of lines. */
int text_lines(int x, int y, int line_h, const char *s, SDL_Color c, int align);
/* The same font in fixed 8x16 cells with a ROM palette, as on the Custom screen. */
void text_draw_cells(int x, int y, const char *s, uint32_t pal);
#define TEXT_H 12

/* The dialogue font of the original text window: thin, proportional, dark
 * with a light shadow. Lines are 16 pixels apart. */
int chat_width(const char *s);
void chat_draw(int x, int y, const char *s, int max_chars);
/* The same font in fixed 8x16 cells, as the chip card uses it; y = cell top. */
void chat_draw_cells(int x, int y, const char *s);
/* Proportional chat text in another ROM palette (e.g. white on the PET). */
void chat_draw_pal(int x, int y, const char *s, uint32_t pal);

/* Primitives. */
void fill_rect(int x, int y, int w, int h, SDL_Color c);
void draw_rect(int x, int y, int w, int h, SDL_Color c);
void draw_frame(int x, int y, int w, int h, SDL_Color fill, SDL_Color edge);

static inline SDL_Color rgba(int r, int g, int b, int a) { SDL_Color c = { (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a }; return c; }
#define WHITE rgba(255, 255, 255, 255)
#define BLACK rgba(0, 0, 0, 255)

static inline uint32_t bgr555(uint16_t c) {
	uint32_t r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
	r = (r << 3) | (r >> 2); g = (g << 3) | (g >> 2); b = (b << 3) | (b >> 2);
	return 0xFF000000u | r << 16 | g << 8 | b;
}

#endif
