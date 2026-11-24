/* pet_menu.h. BN6's cursor follows ours, set to the entry's own row each
 * frame the menu takes input (Forms and Colours hold MegaMan's, EXIT 8), so BN6 lights
 * that row and pulses EXIT itself; where BN6 moves its cursor (the menu
 * opened on ChipFolder, a screen left on its row) ours follows it. */
#include "pet_menu.h"

#include <string.h>

#include "audio.h"
#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "free_space.h"
#include "looks_screen.h"
#include "pet_forms.h"
#include "platform.h"
#include "rom.h"

#define PET_STATE_INPUT 4          /* ePETMenuData +0: taking input */
#define BN6_EXIT 8                 /* BN6_PET_CURSOR on EXIT */
#define DPAD (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)

/* the entries' BN6 rows, ChipFolder to Save; Forms and Colours none */
static const signed char bn6_rows[PM_ENTRIES] = { 0, 1, 2, 3, -1, -1, 4, 5, 6, 7 };

static int cur = PM_FOLDER;        /* our cursor */
static int written = -1;           /* BN6's cursor as we last set it */

#define BUS 0x08000000u
#define ICONS_AT     FREE_PET_MENU                 /* the icons' list as the menu opens, the Status icon's added */
#define BACK_AT      (FREE_PET_MENU + 0x40u)       /* ... as a screen gives it back */
#define HEIGHTS_AT   (FREE_PET_MENU + 0x80u)       /* the heights, nine */
#define LIST_MOST    16                            /* a list's entries, its end among them */
#define ICON_HIDDEN  0xC0                          /* the ninth icon's height: under the screen */

_Static_assert(HEIGHTS_AT + 2u * BN6_PET_ICONS_GROWN <= FREE_PET_MENU_END, "the menu's tables past their region");

/* a list with the Status icon after row 7's (type 2, the next slot, row 8, the entry's back byte); its bytes */
static size_t grown(uint32_t list, uint8_t *out) {
	size_t n = 0;
	for (uint32_t at = list - BUS; n + 2 < LIST_MOST && R.data[at] != 0xFF; at += 4) {
		memcpy(out + 4 * n++, R.data + at, 4);
		if (R.data[at] == BN6_PET_ICON_TYPE && R.data[at + 2] == BN6_PET_ICON_LAST) {
			memcpy(out + 4 * n, R.data + at, 4);
			out[4 * n + 1] = (uint8_t)(R.data[at + 1] + 1);
			out[4 * n + 2] = BN6_PET_ICON_STATUS;
			++n;
		}
	}
	memset(out + 4 * n++, 0xFF, 4);
	return 4 * n;
}

bool pet_menu_install(void) {
	if (!R.data || rom_u32(BN6_PET_ICONS_LIT - BUS) != BN6_PET_ICONS || rom_u32(BN6_PET_ICONS_BACK_LIT - BUS) != BN6_PET_ICONS_BACK ||
		rom_u32(BN6_PET_ICON_HEIGHTS_LIT - BUS) != BN6_PET_ICON_HEIGHTS || rom_u16(BN6_PET_ICON_HEIGHTS - BUS) != 0x20)
		return false;
	uint8_t list[4 * LIST_MOST], heights[2 * BN6_PET_ICONS_GROWN];
	for (int k = 0; k < BN6_PET_ICON_ROWS; ++k) put16(heights + 2 * k, rom_u16(BN6_PET_ICON_HEIGHTS - BUS + 2u * (uint32_t)k));
	put16(heights + 2 * BN6_PET_ICON_STATUS, ICON_HIDDEN);
	claims_begin("the PET start menu's Status icon (pet_menu.c)");
	emu_write(ICONS_AT, list, grown(BN6_PET_ICONS, list));
	emu_write(BACK_AT, list, grown(BN6_PET_ICONS_BACK, list));
	emu_write(HEIGHTS_AT, heights, sizeof heights);
	emu_write32(BN6_PET_ICONS_LIT, ICONS_AT);
	emu_write32(BN6_PET_ICONS_BACK_LIT, BACK_AT);
	emu_write32(BN6_PET_ICON_HEIGHTS_LIT, HEIGHTS_AT);
	claims_end();
	return true;
}

int pet_menu_bn6_row(int entry) { return entry >= 0 && entry < PM_ENTRIES ? bn6_rows[entry] : -1; }

void pet_menu_place(int entry, int *col, int *row) {
	*col = entry >= PM_LEFT;
	*row = entry >= PM_LEFT ? entry - PM_LEFT : entry;
}

/* the entry at a place (EXIT's after the right column's last), -1 where it is empty */
static int at(int col, int row) {
	int e = col ? PM_LEFT + row : row;
	if (row < 0 || row >= PM_ROWS || (col == 0 && row >= PM_LEFT) || e > PM_EXIT) return -1;
	return e;
}

bool pet_menu_up(void) {
	return emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) == BN6_SUB_PET &&
		(emu_read8(BN6_PET_MENU_OPEN) & 1);
}

int pet_menu_cursor(void) { return cur; }

static bool taking(void) { return pet_menu_up() && emu_read8(BN6_PET_MENU) == PET_STATE_INPUT && !emu_read8(BN6_PET_HOLD); }

/* BN6's cursor moved by BN6 (the menu opened, a screen left): ours to that entry */
static void follow(void) {
	int b = emu_read8(BN6_PET_CURSOR);
	if (b == written) return;
	if (b == BN6_EXIT) cur = PM_EXIT;
	else
		for (int e = 0; e < PM_ENTRIES; ++e)
			if (bn6_rows[e] == b) cur = e;
	written = b;
}

static void to(int e) {
	if (e < 0 || e == cur) return;
	cur = e;
	audio_sfx(SFX_CURSOR);
}

/* Up and Down walk the folded list and wrap, EXIT left out of it as BN6
 * leaves it out of its column's (Save to ChipFolder, ChipFolder to Save);
 * Left and Right hop columns on the same row, Right to EXIT from the right
 * column and from Forms, beside it, as BN6's Right reaches EXIT from any
 * row; EXIT's Up to the entry over it, its Left to the one beside it */
static void move(void) {
	int col, row;
	pet_menu_place(cur, &col, &row);
	if (cur == PM_EXIT) {
		if (btn_repeat(BTN_UP)) to(at(1, row - 1));
		else if (btn_pressed(BTN_LEFT)) to(at(0, row));
		return;
	}
	if (btn_repeat(BTN_DOWN)) to((cur + 1) % PM_ENTRIES);
	else if (btn_repeat(BTN_UP)) to((cur + PM_ENTRIES - 1) % PM_ENTRIES);
	else if (btn_pressed(BTN_RIGHT)) to(col == 0 && at(1, row) >= 0 ? at(1, row) : PM_EXIT);
	else if (btn_pressed(BTN_LEFT) && col == 1) to(at(0, row));
}

uint32_t pet_menu_keys(uint32_t keys) {
	if (!pet_menu_up()) {
		written = -1;
		return keys;
	}
	if (!taking()) return keys;
	follow();
	move();
	int b = cur == PM_EXIT ? BN6_EXIT : bn6_rows[cur] < 0 ? bn6_rows[PM_MEGAMAN] : bn6_rows[cur];
	emu_write8(BN6_PET_CURSOR, (uint8_t)b);
	written = b;
	/* (A on Forms: ours; on the rest BN6's, on its own row, as BN6 opens it) */
	if (cur == PM_FORMS && (keys & KEY_A) && btn_pressed(BTN_A)) {
		pet_forms_open();
		return keys & ~(uint32_t)(DPAD | KEY_A);
	}
	/* (A on Colours: ours too) */
	if (cur == PM_LOOKS && (keys & KEY_A) && btn_pressed(BTN_A)) {
		looks_screen_open();
		return keys & ~(uint32_t)(DPAD | KEY_A);
	}
	return keys & ~(uint32_t)DPAD;
}

void pet_menu_describe(FILE *f) {
	static const char *const names[PM_ENTRIES + 1] = { "ChipFolder", "SubChip", "Library", "MegaMan", "Forms", "Colours", "E-Mail", "KeyItem", "Options", "Save", "EXIT" };
	if (pet_menu_up()) fprintf(f, "petmenu at %s\n", names[cur]);
}
