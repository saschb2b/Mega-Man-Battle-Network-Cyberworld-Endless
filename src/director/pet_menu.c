/* pet_menu.h. BN6's cursor follows ours, set to the entry's own row each
 * frame the menu takes input (Forms holds MegaMan's, EXIT 8), so BN6 lights
 * that row and pulses EXIT itself; where BN6 moves its cursor (the menu
 * opened on ChipFolder, a screen left on its row) ours follows it. */
#include "pet_menu.h"

#include "audio.h"
#include "bn6.h"
#include "emu.h"
#include "pet_forms.h"
#include "platform.h"

#define PET_STATE_INPUT 4          /* ePETMenuData +0: taking input */
#define BN6_EXIT 8                 /* BN6_PET_CURSOR on EXIT */
#define DPAD (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)

/* the entries' BN6 rows, ChipFolder to Save; Forms none */
static const signed char bn6_rows[PM_ENTRIES] = { 0, 1, 2, 3, -1, 4, 5, 6, 7 };

static int cur = PM_FOLDER;        /* our cursor */
static int back = PM_FOLDER;       /* the entry EXIT returns to */
static int written = -1;           /* BN6's cursor as we last set it */

int pet_menu_bn6_row(int entry) { return entry >= 0 && entry < PM_ENTRIES ? bn6_rows[entry] : -1; }

void pet_menu_place(int entry, int *col, int *row) {
	*col = entry >= PM_LEFT;
	*row = entry >= PM_LEFT ? entry - PM_LEFT : entry;
}

/* the entry at a place, -1 where it is empty */
static int at(int col, int row) {
	int e = col ? PM_LEFT + row : row;
	if (row < 0 || row >= PM_ROWS || (col == 0 && row >= PM_LEFT) || e >= PM_ENTRIES) return -1;
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
	if (e == PM_EXIT) back = cur;
	cur = e;
	audio_sfx(SFX_CURSOR);
}

/* Up and Down walk the folded list and wrap; Left and Right hop columns on
 * the same row, Right to EXIT where nothing is beside it or from the right
 * column; EXIT's Left or Up back to where the cursor came from */
static void move(void) {
	if (cur == PM_EXIT) {
		if (btn_pressed(BTN_LEFT | BTN_UP)) to(back);
		return;
	}
	int col, row;
	pet_menu_place(cur, &col, &row);
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
	return keys & ~(uint32_t)DPAD;
}

void pet_menu_describe(FILE *f) {
	static const char *const names[PM_ENTRIES + 1] = { "ChipFolder", "SubChip", "Library", "MegaMan", "Forms", "E-Mail", "KeyItem", "Options", "Save", "EXIT" };
	if (pet_menu_up()) fprintf(f, "petmenu at %s\n", names[cur]);
}
