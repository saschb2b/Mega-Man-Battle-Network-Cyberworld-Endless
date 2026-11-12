/* The Battle Lab's own folder (lab_folder.h). */
#include "lab_folder.h"

#include <stdio.h>

#include "bn6.h"
#include "bn6_chips.h"
#include "bn6_pet.h"
#include "chip_entry.h"
#include "chips.h"
#include "debug.h"
#include "director_folder.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "lab_battle.h"
#include "meta.h"
#include "save.h"

#define OPEN_MOST 900    /* frames to BN6's editor at most: past them the game is put back with nothing changed */
#define KEY_GAP 24       /* frames between two keys pressed for the player on the way in */
#define KEY_HELD 3       /* ... and each one's */
#define LEFT_FRAMES 6    /* frames run on once the editor is left, BN6 writing the folder back, before it is read */
#define SPARES 5         /* copies of a chip in each of its codes: the most a folder takes of one */

static struct {
	bool on, seen;       /* under way; BN6's editor has been up */
	int t, since, held;  /* frames since it began, since the last key pressed for the player, and that key's frames left */
	uint32_t key;        /* ... the key */
	int left;            /* frames since the editor was left, 0 while it is up */
} F;

/* Where the game is: its map, the PET's menu, the folders' list, the editor, another screen of the PET's, or between two */
enum { AT_MAP, AT_PET, AT_FOLDERS, AT_EDIT, AT_WRONG, AT_OTHER };
static int where(void) {
	int mode = main_mode();
	if (mode == BN6_MODE_SUBMENU) {
		int screen = emu_read8(emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU));
		return screen == BN6_SUBMENU_EDIT ? AT_EDIT : screen == BN6_SUBMENU_FOLDERS ? AT_FOLDERS : AT_WRONG;
	}
	if (mode != BN6_MODE_GAME) return AT_OTHER;
	return emu_read8(BN6_GAMESTATE) == BN6_SUB_PET ? AT_PET : on_map() ? AT_MAP : AT_OTHER;
}

static uint32_t folder_at(void) {
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	return data >= BN6_EWRAM && data < BN6_EWRAM_END ? data : 0;
}

bool lab_folder_used(void) { return profile.lab_folder_on && profile.lab_folder[0]; }

void lab_folder_into_game(void) {
	uint32_t data = folder_at();
	if (!data || !profile.lab_folder[0]) return;
	int other = 0;
	for (uint32_t i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		const uint8_t b[2] = { (uint8_t)profile.lab_folder[i], (uint8_t)(profile.lab_folder[i] >> 8) };
		other += emu_read16(data + 2 * i) != profile.lab_folder[i];
		emu_write(data + 2 * i, b, sizeof b);
	}
	own_folder_chips();
	if (emu_debug_on()) fprintf(stderr, "lab: the Lab's folder in the game's first, %d of its %d entries other than MegaMan's\n", other, BN6_FOLDER_ENTRIES);
}

/* The pack as the Lab's: every chip the profile has held in any run, and the run's own so far, SPARES of it in each code
 * its record has, owned as BN6 marks a chip it gives (own_folder_chips) */
static void pack_fill(void) {
	uint32_t marks = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIP_MARKS);
	for (int id = 1; id < PACK_CHIPS; ++id) {
		bool held = meta_library_has(id) || (id < BN6_FLAG_LIBRARY_IDS && flag_get(BN6_FLAG_LIBRARY + id));
		const uint8_t *rec = held ? chips_record(id) : NULL;
		uint32_t at = rec && pack_listed(id) ? pack_entry(id) : 0;
		if (!at) continue;
		for (uint32_t k = 0; k < 4; ++k)
			if (rec[BN6_CHIP_CODES + k] != 0xFF && emu_read8(at + k) < SPARES) emu_write8(at + k, SPARES);
		if (marks >= BN6_EWRAM && marks < BN6_EWRAM_END) emu_write8(marks + (uint32_t)id, (uint8_t)(emu_read8(BN6_CHIP_KEYS + (uint32_t)id) ^ BN6_CHIP_KEY_XOR));
	}
}

bool lab_folder_edit(void) {
	if (F.on || !lab_battle_keep()) return false;
	F = (__typeof__(F)){ .on = true, .since = KEY_GAP };
	/* (the Lab's folder where one is made; else the one MegaMan holds, the Lab's to begin from) */
	lab_folder_into_game();
	pack_fill();
	return true;
}

bool lab_folder_on(void) { return F.on; }
bool lab_folder_opening(void) { return F.on && !F.seen; }

/* The editor left, or never reached: the folder kept where it was, then the game put back */
static void end(bool kept) {
	uint32_t data = folder_at();
	if (kept && data) {
		for (uint32_t i = 0; i < BN6_FOLDER_ENTRIES; ++i) profile.lab_folder[i] = emu_read16(data + 2 * i);
		profile_save();
	}
	F.on = false;
	lab_battle_put(kept ? "the Lab's folder kept" : "BN6's folder editor not reached, nothing kept");
}

bool lab_folder_frame(void) {
	if (!F.on) return false;
	++F.t;
	++F.since;
	int at = where();
	if (at == AT_EDIT && !F.left) F.seen = true;
	else if (F.seen && ++F.left >= LEFT_FRAMES) end(true);
	else if (!F.seen && (F.t >= OPEN_MOST || at == AT_WRONG)) end(false);
	return true;
}

uint32_t lab_folder_keys(uint32_t keys) {
	if (!F.on || F.left) return 0;
	if (F.seen) return keys;
	/* (on the way in: START on the map, A on the PET's menu, whose cursor a PET just opened has on ChipFolder, and A
	 * twice on the folders' list, its folder and then EDIT) */
	if (F.held > 0) { --F.held; return F.key; }
	int at = where();
	if (F.since < KEY_GAP || at == AT_OTHER || at == AT_WRONG) return 0;
	F.key = at == AT_MAP ? KEY_START : KEY_A;
	F.held = KEY_HELD - 1;
	F.since = 0;
	return F.key;
}
