/* The player's own ROM: discovery, verification and data access.
 * Nothing from the ROM is shipped; everything is read from it at runtime. */
#ifndef ROM_H
#define ROM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ROM_SIZE 0x800000

typedef enum { ROM_BN6_GREGAR_US } RomVersion;

/* Addresses of the data the engine reads (ROM offsets, not bus addresses). */
typedef struct {
	const char *name;
	const char *sha1;
	uint32_t sprite_lists;    /* SpritePointersList: 10 category tables */
	uint32_t chip_data;       /* 0x2C-byte chip records */
	uint32_t chip_names[2];   /* text archives: ids 0-255, 256+ */
	uint32_t enemy_names;     /* text archive indexed by enemy id */
	uint32_t navi_names;      /* text archive indexed by navi */
	uint32_t enemy_ids;       /* (version, actor type, ai) triples */
	uint32_t enemy_stats;     /* per actor type -> per ai -> 6-byte records */
	uint32_t panel_tiles;     /* LZ77 battle panel tileset */
	uint32_t panel_pals[8];
	uint32_t battle_bg_table; /* BGAnimData pointer per battle background id */
	uint32_t battle_bg_anims; /* GFX animation list pointer per id */
	uint32_t chip_icon_pal;
	uint32_t song_table;       /* MP2K songs: (header, player, player) */
	uint32_t chip_desc[2];     /* description archives, 3 short lines each */
	struct {                    /* battle UI tiles and palettes */
		uint32_t hp_digits, hp_pal;          /* 8x16 digits, 10 = blank, 11 = box edge */
		uint32_t gauge, gauge_pal;           /* caps, body, label and 1-pixel fill steps */
		uint32_t window_pal;                 /* Custom window text */
		uint32_t code_letters, grid_letters; /* 8x16 and 16x8, A-Z then '*', 27 = blank */
		uint32_t power_digits;
		uint32_t elem_icons;                 /* 16x16 per chip element */
		uint32_t icon_pal, icon_dim_pal, icon_obj_pal;
		uint32_t icon_elem_colors;           /* colours 10-12 of the icon palette */
		uint32_t enemy_hp_digits, enemy_hp_pal;
		uint32_t emotion, emotion_face, emotion_pal;
		uint32_t cursor, cursor_pal;
		uint32_t emblem;                     /* 16x16 above the picked column, cursor palette */
		uint32_t result_digits, result_pal;  /* 8x16, 11 = S rank */
		uint32_t chat_font, chat_pal;        /* 8x16 linear 4bpp cells by character code */
		uint32_t next_arrow;                 /* 16x16, chat palette */
		uint32_t card_sprite;                /* GUI sprite index of the chip card */
		uint32_t list_elem_icons, list_elem_pal, list_code_pal, list_icon_pal;
		uint32_t list_arrow, list_arrow_pal;  /* arrow tiles: LZ block ref (see hud_layout.inc) */
		/* battle flow (docs/BATTLE_FLOW.md) */
		uint32_t hp_hurt_pal;                /* HP box while MegaMan takes damage */
		uint32_t panel_warn_pal;             /* panel an attack is about to hit */
		uint32_t cursor_wide;                /* second frame of the pulsing Custom cursor */
		uint32_t ok_art, ok_art_pal;         /* 56x48 "CHIP DATA TRANSMISSION" shown on OK */
		uint32_t press_a;                    /* 10 tiles "PRESS A BUTTON", result palette */
		uint32_t box_corner, box_edge, box_side, box_side_top, box_fill, box_pal, box_arrow;
		uint32_t delete_sprite;              /* GUI sprite of the deletion explosion */
		uint32_t zenny_art, zenny_pal;       /* 56x48 reward picture for Zenny */
		uint32_t name_tab;                   /* enemy name tab: end cap, then fill (2 tiles each) */
		uint32_t charge_sprite, charge_full_pal; /* buster charge lines (GUI sprite) and their charged palette */
		uint32_t pause_text;                 /* PAUSE: 4x2 tiles then a 1x2 column, enemy HP palette */
	} ui;
} RomLayout;

typedef struct {
	uint8_t *data;
	RomVersion version;
	const RomLayout *layout;
	char path[512];
} Rom;

extern Rom R;

/* Looks for a supported ROM in dir (any *.gba). On failure, msg explains why. */
bool rom_find(const char *dir, char *msg, size_t msglen);
bool rom_load_file(const char *path, char *msg, size_t msglen);

static inline uint32_t rom_u32(uint32_t off) {
	return (uint32_t)R.data[off] | (uint32_t)R.data[off + 1] << 8 | (uint32_t)R.data[off + 2] << 16 | (uint32_t)R.data[off + 3] << 24;
}
static inline uint16_t rom_u16(uint32_t off) { return (uint16_t)(R.data[off] | R.data[off + 1] << 8); }
static inline bool rom_is_ptr(uint32_t v) { return v >= 0x08000000 && v < 0x08000000 + ROM_SIZE; }
static inline uint32_t rom_off(uint32_t ptr) { return ptr - 0x08000000; }

/* GBA BIOS LZ77 (type 0x10). Returns malloc'd data or NULL. */
uint8_t *lz77_decompress(const uint8_t *src, size_t avail, size_t *out_len);

/* Decode a text-archive entry into ASCII using the game's character table. */
void rom_text(uint32_t archive, int index, char *out, size_t outlen);
/* Like rom_text, but keeps line breaks as '\n' and skips script commands. */
void rom_script_text(uint32_t archive, int index, char *out, size_t outlen);

void sha1_hex(const uint8_t *data, size_t len, char out[41]);

#endif
