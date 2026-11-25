/* pet_options.h. BN6's menu labels are pictures, a strip of 9 x 2 tiles
 * each, and none says Options or Forms: each word is set from their own
 * letters (letters.h), read from the player's ROM as the core starts, into
 * a copy of the labels' tiles that the PET's load list points at, Options'
 * one tile column more than Comm's after them; Forms' and Colours' five
 * columns each for the rows BN6 never draws, kept here for the menu's picture alone
 * (pet_menu_frame.c), out of the video memory, where tile 0x2FF is BN6's
 * blank (docs/rom/pet-and-second-screen.md, the PET menu's labels). */
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
#define ADDED 2                    /* the tiles past BN6's: Options' fifth column */
#define FORMS_TILES (2 * COLS)     /* Forms' word's tiles, its top row then its bottom: ours, never loaded */
#define FORMS_FIRST (BN6_PET_LABEL_FIRST + BN6_PET_LABEL_COUNT + ADDED)   /* the numbers Forms' map gives them, past the copy's (pet_options_forms_tile) */
#define LOOKS_FIRST (FORMS_FIRST + FORMS_TILES)   /* ... and Colours', after them */
#define FORMS_INK BN6_PET_LABEL_INK_NEXT   /* its letters' colour: the one after the eight rows' (the menu sets it, pet_menu_frame.c) */
#define SAVE 7                     /* the label Forms' is made on: Save's, its last row's */
#define VRAM_LABELS 0x0600D340u    /* where the load list puts them: BN6_PET_LABEL_FIRST in BG character block 2 */
#define VRAM_BLANK 0x0600DFE0u     /* tile 0x2FF, zero: BG0's every entry on the Chip Folder's screens (docs/rom/), so no added tile reaches it */
#define BLOCK ((BN6_PET_LABEL_COUNT + ADDED) * TILE)
#define CHIPFOLDER 0
#define MEGAMAN 3

_Static_assert(VRAM_LABELS + BLOCK <= VRAM_BLANK, "the added tiles stop short of BN6's blank tile");
_Static_assert(FREE_PET_OPTIONS + BLOCK <= FREE_PET_OPTIONS_END, "the labels' copy fits its region");

/* Options in the labels' letters, Comm's C closed into the O; Forms in
 * them; and the letters each label is cut into, as Gregar's are */
typedef struct { uint8_t entry, letter; bool closed; } Pick;
static const Pick picks[] = {
	{ BN6_PET_ENTRY_COMM, 0, true },     /* (Comm's C) */
	{ CHIPFOLDER, 3, false },            /* p */
	{ BN6_PET_ENTRY_STATUS, 1, false },  /* t */
	{ CHIPFOLDER, 2, false },            /* i */
	{ BN6_PET_ENTRY_COMM, 1, false },    /* o */
	{ MEGAMAN, 6, false },               /* n */
	{ BN6_PET_ENTRY_STATUS, 5, false },  /* s */
};
static const Pick forms_picks[] = {
	{ CHIPFOLDER, 4, false },            /* F */
	{ BN6_PET_ENTRY_COMM, 1, false },    /* o */
	{ CHIPFOLDER, 9, false },            /* r */
	{ BN6_PET_ENTRY_COMM, 2, false },    /* m */
	{ BN6_PET_ENTRY_STATUS, 5, false },  /* s */
};
static const Pick looks_picks[] = {
	{ CHIPFOLDER, 0, false },            /* C */
	{ BN6_PET_ENTRY_COMM, 1, false },    /* o */
	{ CHIPFOLDER, 6, false },            /* l */
	{ BN6_PET_ENTRY_COMM, 1, false },    /* o */
	{ BN6_PET_ENTRY_STATUS, 4, false },  /* u */
	{ CHIPFOLDER, 9, false },            /* r */
	{ BN6_PET_ENTRY_STATUS, 5, false },  /* s */
};
static const struct { uint8_t entry, letters; } cuts[] = {
	{ CHIPFOLDER, 10 }, { MEGAMAN, 7 }, { BN6_PET_ENTRY_COMM, 4 }, { BN6_PET_ENTRY_STATUS, 6 },
};

static bool installed;
static uint16_t forms_map[2 * BN6_PET_LABEL_W];   /* Forms' tile entries, Save's with the word's columns */
static uint8_t forms_tiles[FORMS_TILES][TILE];     /* ... the word's tiles they name */
static bool looks_set;                             /* Colours' word set (it fits Save's columns) */
static uint16_t looks_map[2 * BN6_PET_LABEL_W];    /* Colours' tile entries, as Forms' */
static uint8_t looks_tiles[FORMS_TILES][TILE];
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

/* Whether each label is cut into the letters read */
static bool cut_as_read(uint8_t labels[][LH][LW]) {
	for (size_t i = 0; i < sizeof cuts / sizeof *cuts; ++i) {
		int at[16][2];
		LetterStrip s = { &labels[cuts[i].entry][0][0], LW, LH, ink(cuts[i].entry) };
		if (letters_cut(&s, at, 16) != cuts[i].letters) return false;
	}
	return true;
}

/* A word of `n` picks on label `on`'s blank columns, its letters in
 * colour `to`; false where a letter does not fit */
static bool set_word(uint8_t labels[][LH][LW], const Pick *p, size_t n, int on, uint8_t to, uint8_t word[LH][COLS * 8]) {
	for (int y = 0; y < LH; ++y)
		for (int x = 0; x < COLS * 8; ++x) word[y][x] = labels[on][y][BN6_PET_LABEL_BLANK * 8 + x % 8];
	LetterWord w = { &word[0][0], COLS * 8, LH };
	int x = 0;
	for (size_t i = 0; i < n && x >= 0; ++i) {
		int e = p[i].entry;
		LetterStrip s = { &labels[e][0][0], LW, LH, ink(e) };
		uint8_t map[16] = { 0 };
		map[ink(e)] = to;
		map[BN6_PET_LABEL_SHADE] = BN6_PET_LABEL_SHADE;
		x = p[i].closed ? letters_put_closed(&s, p[i].letter, &w, x, to, BN6_PET_LABEL_SHADE) : letters_put(&s, p[i].letter, &w, x, 0, map);
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
	static uint8_t word[LH][COLS * 8], forms[LH][COLS * 8], looks[LH][COLS * 8];
	if (!cut_as_read(labels) || !set_word(labels, picks, sizeof picks / sizeof *picks, BN6_PET_ENTRY_COMM, ink(BN6_PET_ENTRY_COMM), word) ||
		!set_word(labels, forms_picks, sizeof forms_picks / sizeof *forms_picks, SAVE, FORMS_INK, forms))
		return false;
	looks_set = set_word(labels, looks_picks, sizeof looks_picks / sizeof *looks_picks, SAVE, FORMS_INK, looks);

	/* the labels' tiles copied, Comm's four columns the word's, its fifth two tiles more */
	static uint8_t block[BLOCK];
	memcpy(block, R.data + rom_off(BN6_PET_LABEL_TILES), BN6_PET_LABEL_COUNT * TILE);
	for (int c = 0; c < COLS; ++c)
		for (int r = 0; r < 2; ++r) {
			uint16_t e = r ? bottom[c] : top[c];
			int t = c < COLS - 1 ? (e & 0x3FF) - BN6_PET_LABEL_FIRST : BN6_PET_LABEL_COUNT + r;
			word_tile(word, c, r, block + t * TILE);
			/* (Forms' on Save's map, its columns 1-5 the word's) */
			word_tile(forms, c, r, forms_tiles[r * COLS + c]);
			uint32_t se = rom_u16(rom_off(entry_map(SAVE)) + 2u * (uint32_t)(r * BN6_PET_LABEL_W + c + 1));
			forms_map[r * BN6_PET_LABEL_W + c + 1] = (uint16_t)((se & ~0x3FFu) | (FORMS_FIRST + r * COLS + c));
			/* (and Colours', as Forms') */
			word_tile(looks, c, r, looks_tiles[r * COLS + c]);
			looks_map[r * BN6_PET_LABEL_W + c + 1] = (uint16_t)((se & ~0x3FFu) | (LOOKS_FIRST + r * COLS + c));
		}
	for (int k = 0; k < 2 * BN6_PET_LABEL_W; ++k)
		if (k % BN6_PET_LABEL_W == 0 || k % BN6_PET_LABEL_W > COLS) looks_map[k] = forms_map[k] = rom_u16(rom_off(entry_map(SAVE)) + 2u * (uint32_t)k);
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

bool pet_options_forms_map(uint16_t out[2 * BN6_PET_LABEL_W]) {
	if (installed) memcpy(out, forms_map, sizeof forms_map);
	return installed;
}

bool pet_options_looks_map(uint16_t out[2 * BN6_PET_LABEL_W]) {
	if (installed && looks_set) memcpy(out, looks_map, sizeof looks_map);
	return installed && looks_set;
}

void pet_options_describe(FILE *f) {
	/* (a PET screen of BN6's, or its menu over the map) */
	int mode = emu_read8(emu_read32(BN6_TOOLKIT));
	if (mode != BN6_MODE_SUBMENU && (mode != BN6_MODE_GAME || emu_read8(BN6_GAMESTATE) != BN6_SUB_PET)) return;
	const uint8_t *t = emu_memory(VRAM_BLANK, TILE);
	if (!t) return;
	bool zero = true;
	for (int i = 0; i < TILE; ++i) zero &= !t[i];
	fprintf(f, "blanktile %s\n", zero ? "zero" : "drawn");
}

const uint8_t *pet_options_forms_tile(int n) {
	if (installed && looks_set && n >= LOOKS_FIRST && n < LOOKS_FIRST + FORMS_TILES) return looks_tiles[n - LOOKS_FIRST];
	return installed && n >= FORMS_FIRST && n < FORMS_FIRST + FORMS_TILES ? forms_tiles[n - FORMS_FIRST] : NULL;
}

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
