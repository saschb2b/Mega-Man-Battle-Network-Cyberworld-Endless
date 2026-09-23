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
	uint32_t enemy_ids;       /* (version, actor type, ai) triples */
	struct {                  /* the title screen (docs/ROM_DATA.md) */
		uint32_t bg_tiles;       /* LZ77: 8bpp tiles as loaded to 0x06000000 */
		uint32_t bg_map;         /* 32x20 map entries */
		uint32_t bg_pal;         /* banks 0-13; bank 15 is the text box palette */
		uint32_t bg_pal15;
		uint32_t bg_anims;       /* palette animation scripts (the logo's glow) */
		uint32_t text_tiles;     /* LZ77: PRESS START, NEW GAME, CONTINUE (OBJ tile 1 first) */
		uint32_t text_pal, menu_pal;
		uint32_t copy_tiles;     /* LZ77: the copyright line, 8 OBJs of 32x32 */
		uint32_t copy_pal;
		uint32_t arrow, arrow_pal; /* menu cursor: 3 frames of 16x16 */
	} title;
	struct {                  /* the original area each net biome borrows (docs/ROM_DATA.md) */
		uint8_t group, number;   /* map whose floor panels are learned */
		uint16_t styles;         /* hue buckets (bit 0-11, 12 grey) of the panels to learn */
		bool bg_in_map;          /* the background is drawn in the map's own tiles: other styles count as empty */
	} net_area[8];
	uint32_t song_table;       /* MP2K songs: (header, player, player) */
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

void sha1_hex(const uint8_t *data, size_t len, char out[41]);

#endif
