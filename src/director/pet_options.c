/* pet_options.h. BN6's menu labels are pictures, a strip of 9 x 2 tiles
 * each, and none says Options: the word is set from their own letters
 * (letters.h), read from the player's ROM as the core starts, into a copy
 * of the labels' tiles that the PET's load list points at, one tile
 * column more than Comm's after them (docs/ROM_DATA.md, the PET menu's
 * labels). */
#include "pet_options.h"

#include <string.h>

#include "audio.h"
#include "bn6.h"
#include "claims.h"
#include "emu.h"
#include "letters.h"
#include "options.h"
#include "rom.h"

#define TILE 32
#define LW (BN6_PET_LABEL_W * 8)   /* a label's pixels across */
#define LH 16                      /* ... and down */
#define COLS 5                     /* the label's tile columns the word takes, from its second (its first the edge) */
#define ADDED 2                    /* the tiles past BN6's: the word's fifth column */
#define VRAM_LABELS 0x0600D340u    /* where the load list puts them: BN6_PET_LABEL_FIRST in BG character block 2 */
#define VRAM_FREE_END 0x0600E000u  /* BG0's map from here; 0x0600DE80 on, zero in the PET and on the map (ROM_DATA.md) */
#define BLOCK ((BN6_PET_LABEL_COUNT + ADDED) * TILE)
#define CHIPFOLDER 0
#define MEGAMAN 3

_Static_assert(VRAM_LABELS + BLOCK <= VRAM_FREE_END, "the added tiles stop short of BG0's map");
_Static_assert(FREE_PET_OPTIONS + BLOCK <= FREE_PET_OPTIONS_END, "the labels' copy fits its region");

/* Options in the labels' letters, Comm's C closed into the O; and the
 * letters each label is cut into, as Gregar's are */
static const struct { uint8_t entry, letter; bool closed; } picks[] = {
	{ BN6_PET_ENTRY_COMM, 0, true },     /* (Comm's C) */
	{ CHIPFOLDER, 3, false },            /* p */
	{ BN6_PET_ENTRY_STATUS, 1, false },  /* t */
	{ CHIPFOLDER, 2, false },            /* i */
	{ BN6_PET_ENTRY_COMM, 1, false },    /* o */
	{ MEGAMAN, 6, false },               /* n */
	{ BN6_PET_ENTRY_STATUS, 5, false },  /* s */
};
static const struct { uint8_t entry, letters; } cuts[] = {
	{ CHIPFOLDER, 10 }, { MEGAMAN, 7 }, { BN6_PET_ENTRY_COMM, 4 }, { BN6_PET_ENTRY_STATUS, 6 },
};

static bool installed;
static bool shown;      /* Options opened from the PET, until it closes */
static uint32_t hush;   /* the keys held as it closed, kept from the game until let go */

static uint32_t entry_map(int entry) { return rom_u32(rom_off(BN6_PET_LABEL_MAPS) + 4u * (uint32_t)entry); }

/* Label `entry`'s pixels through its tile entries; false where they are
 * not the labels' tiles */
static bool label_px(int entry, uint8_t px[LH][LW]) {
	uint32_t map = entry_map(entry);
	if (!rom_is_ptr(map)) return false;
	for (int k = 0; k < 2 * BN6_PET_LABEL_W; ++k) {
		uint16_t e = rom_u16(rom_off(map) + 2u * (uint32_t)k);
		int t = (e & 0x3FF) - BN6_PET_LABEL_FIRST;
		if (t < 0 || t >= BN6_PET_LABEL_COUNT || (e & 0xC00)) return false;
		const uint8_t *src = R.data + rom_off(BN6_PET_LABEL_TILES) + (uint32_t)t * TILE;
		int cx = k % BN6_PET_LABEL_W, cy = k / BN6_PET_LABEL_W;
		for (int y = 0; y < 8; ++y)
			for (int x = 0; x < 8; ++x) px[cy * 8 + y][cx * 8 + x] = (uint8_t)(src[y * 4 + x / 2] >> (x & 1 ? 4 : 0) & 15);
	}
	return true;
}

static uint8_t ink(int entry) { return (uint8_t)(BN6_PET_LABEL_INK + (entry == BN6_PET_ENTRY_STATUS ? MEGAMAN : entry)); }

/* The word on Comm's blank columns, its letters in Comm's colour; false
 * where a label is not cut into the letters read */
static bool set_word(uint8_t labels[][LH][LW], uint8_t word[LH][COLS * 8]) {
	for (size_t i = 0; i < sizeof cuts / sizeof *cuts; ++i) {
		int at[16][2];
		LetterStrip s = { &labels[cuts[i].entry][0][0], LW, LH, ink(cuts[i].entry) };
		if (letters_cut(&s, at, 16) != cuts[i].letters) return false;
	}
	for (int y = 0; y < LH; ++y)
		for (int x = 0; x < COLS * 8; ++x) word[y][x] = labels[BN6_PET_ENTRY_COMM][y][BN6_PET_LABEL_BLANK * 8 + x % 8];
	LetterWord w = { &word[0][0], COLS * 8, LH };
	int x = 0;
	for (size_t i = 0; i < sizeof picks / sizeof *picks && x >= 0; ++i) {
		int e = picks[i].entry;
		LetterStrip s = { &labels[e][0][0], LW, LH, ink(e) };
		uint8_t map[16] = { 0 };
		map[ink(e)] = ink(BN6_PET_ENTRY_COMM);
		map[BN6_PET_LABEL_SHADE] = BN6_PET_LABEL_SHADE;
		x = picks[i].closed ? letters_put_closed(&s, picks[i].letter, &w, x, ink(BN6_PET_ENTRY_COMM), BN6_PET_LABEL_SHADE)
		                    : letters_put(&s, picks[i].letter, &w, x, 0, map);
	}
	return x >= 0;
}

/* a tile of the word's column c, row r, as the GBA's 4bpp */
static void word_tile(uint8_t word[LH][COLS * 8], int c, int r, uint8_t *out) {
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; x += 2)
			out[y * 4 + x / 2] = (uint8_t)(word[r * 8 + y][c * 8 + x] | word[r * 8 + y][c * 8 + x + 1] << 4);
}

static bool bytes_are(uint32_t at, uint8_t a, uint8_t b) { return R.data[rom_off(at)] == a && R.data[rom_off(at) + 1] == b; }

bool pet_options_install(void) {
	uint32_t load = rom_off(BN6_PET_LABEL_LOAD);
	if (rom_u32(load) != BN6_PET_LABEL_TILES || rom_u32(rom_off(BN6_PET_LABEL_DEST)) != VRAM_LABELS || rom_u32(rom_off(BN6_PET_LABEL_SIZE)) != BN6_PET_LABEL_COUNT * TILE)
		return false;
	/* (the grey's store to Comm's colour, after its load of Comm's) */
	if (!bytes_are(BN6_PET_GREY_COMM - 2, 0xA2, 0x69) || !bytes_are(BN6_PET_GREY_COMM, 0x11, 0x80)) return false;
	static uint8_t labels[BN6_PET_LABELS][LH][LW];
	for (int e = 0; e < BN6_PET_LABELS; ++e)
		if (!label_px(e, labels[e])) return false;
	/* Comm's tile entries: its columns 1-4 its own, 5 the blank's as 7 is */
	uint32_t comm = rom_off(entry_map(BN6_PET_ENTRY_COMM));
	uint16_t top[COLS], bottom[COLS];
	for (int c = 0; c < COLS; ++c) {
		top[c] = rom_u16(comm + 2u * (uint32_t)(c + 1));
		bottom[c] = rom_u16(comm + 2u * (uint32_t)(BN6_PET_LABEL_W + c + 1));
	}
	if (top[COLS - 1] != rom_u16(comm + 2u * BN6_PET_LABEL_BLANK) || bottom[COLS - 1] != rom_u16(comm + 2u * (BN6_PET_LABEL_W + BN6_PET_LABEL_BLANK)))
		return false;
	static uint8_t word[LH][COLS * 8];
	if (!set_word(labels, word)) return false;

	/* the labels' tiles copied, Comm's four columns the word's, its fifth two tiles more */
	static uint8_t block[BLOCK];
	memcpy(block, R.data + rom_off(BN6_PET_LABEL_TILES), BN6_PET_LABEL_COUNT * TILE);
	for (int c = 0; c < COLS; ++c)
		for (int r = 0; r < 2; ++r) {
			uint16_t e = r ? bottom[c] : top[c];
			int t = c < COLS - 1 ? (e & 0x3FF) - BN6_PET_LABEL_FIRST : BN6_PET_LABEL_COUNT + r;
			word_tile(word, c, r, block + t * TILE);
		}
	claims_begin("the PET's Options (pet_options.c)");
	emu_write(FREE_PET_OPTIONS, block, sizeof block);
	emu_write32(BN6_PET_LABEL_LOAD, FREE_PET_OPTIONS);
	emu_write32(BN6_PET_LABEL_SIZE, BLOCK);
	/* (Comm's fifth column, above and below: the two tiles added) */
	for (int r = 0; r < 2; ++r) {
		uint16_t e = (uint16_t)(((r ? bottom : top)[COLS - 1] & ~0x3FF) | (BN6_PET_LABEL_FIRST + BN6_PET_LABEL_COUNT + r));
		emu_write(entry_map(BN6_PET_ENTRY_COMM) + 2u * (uint32_t)(r * BN6_PET_LABEL_W + COLS), &e, 2);
	}
	/* (lit: the grey's store to its colour a no-op, as Save's is) */
	static const uint8_t nop[] = { 0xC0, 0x46 };
	emu_write(BN6_PET_GREY_COMM, nop, sizeof nop);
	claims_end();
	installed = true;
	return true;
}

bool pet_options_here(void) { return installed; }

void pet_options_update(void) {
	if (emu_read8(BN6_PET_MENU_TAKEN) != BN6_PET_ENTRY_COMM) return;
	emu_write8(BN6_PET_MENU_TAKEN, 0);
	options_open();
	audio_sfx(SFX_SELECT);
	shown = true;
}

uint32_t pet_options_keys(uint32_t keys) {
	if (shown && !options_shown()) {
		shown = false;
		hush = keys;
	}
	hush &= keys;
	return keys & ~hush;
}
