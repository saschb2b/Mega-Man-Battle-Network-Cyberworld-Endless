/* The PET's Save as the run's, and its Options (docs/game/pet.md). */
#include "pet.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "cinema.h"
#include "director.h"
#include "director_folder.h"
#include "director_see.h"
#include "emu.h"
#include "jukebox.h"
#include "library.h"
#include "netpass_screen.h"
#include "online_visit.h"
#include "pet_menu.h"
#include "pet_options.h"
#include "pet_screens.h"
#include "pet_text.h"
#include "platform.h"
#include "rom.h"
#include "shop.h"
#include "square_pet.h"
#include "talk.h"

/* A Save under way: the PET closing, then the checkpoint. */
static int saving;

/* The frames from the PET closed after a Save until its box is up, A kept
 * from the map: the checkpoint waits for MegaMan free, and an A meant for
 * a box took the Guardian Data he faced (session 78); at most a second */
#define SAVE_BOX_WAIT 60
static int box_wait;

/* The PET's input handler (its state 4, run each frame the menu takes
 * input), hooked: A on Save, disabled in a run, and on Comm, the run's
 * Options (pet_options.h), and on Library, the run's own over the entries
 * (library.h), is taken from the game (its bit in the keys just pressed
 * cleared) and left in the menu's spare byte for the engine, so the
 * handler sees no A there. */
static HookAct pet_input(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	uint8_t at = hook_read8(BN6_PET_CURSOR);
	if (at != BN6_PET_ENTRY_SAVE && at != BN6_PET_ENTRY_LIBRARY && !(at == BN6_PET_ENTRY_COMM && pet_options_here())) return HOOK_CONTINUE;
	if (hook_read8(BN6_PET_HOLD)) return HOOK_CONTINUE;
	uint16_t keys = hook_read16(BN6_KEYS_PRESSED);
	if (!(keys & KEY_A)) return HOOK_CONTINUE;
	hook_write16(BN6_KEYS_PRESSED, (uint16_t)(keys & ~KEY_A));
	hook_write8(BN6_PET_MENU_TAKEN, at);
	return HOOK_CONTINUE;
}

static uint32_t rom32(uint32_t at) {
	uint32_t o = at - 0x08000000u;
	return (uint32_t)R.data[o] | (uint32_t)R.data[o + 1] << 8 | (uint32_t)R.data[o + 2] << 16 | (uint32_t)R.data[o + 3] << 24;
}

void pet_install(void) {
	/* (only where the game is the one we read: its input handler's
	 * pointer in the PET's state table, the grey's store) */
	if (rom32(BN6_PET_INPUT_PTR) == BN6_PET_INPUT) emu_hook(BN6_PET_INPUT, pet_input, NULL);
	/* Save lit: the grey's store to Save's colour (0x03001B58) made a
	 * no-op */
	uint32_t grey = BN6_PET_GREY_SAVE - 0x08000000u;
	if (R.data[grey - 4] == 0xE2 && R.data[grey - 3] == 0x69 && R.data[grey] == 0x11 && R.data[grey + 1] == 0x80) {
		static const uint8_t nop[] = { 0xC0, 0x46 };
		emu_write(BN6_PET_GREY_SAVE, nop, sizeof nop);
	}
	/* Comm made Options and lit, else greyed as BN6 has it without a link */
	pet_options_install();
	pet_menu_install();
}

void pet_update(void) {
	/* at home, where a layer's refresh (director.c) never runs: the
	 * profile's key items wherever the game's state lacks them (a NEW
	 * GAME's, the week's dive's), so KeyItem holds the NetPass before the
	 * first layer (session 74) */
	if ((director_in_town() || director_in_hp()) && emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_GAME && !pet_text_items_held())
		pet_text_items();
	pet_options_update();
	pet_screens_update();
	/* (A on Library: the Library opens over the PET's menu, the game held under it) */
	if (emu_read8(BN6_PET_MENU_TAKEN) == BN6_PET_ENTRY_LIBRARY) {
		emu_write8(BN6_PET_MENU_TAKEN, 0);
		/* (the chips the game gave since the run's last save first: one to id
		 * 511 waits in BN6's own flags for it, and the page opened before then
		 * left a chip just found out of its count, while one past 511, told at
		 * once, was in: tests/scenarios/xrom_library.py) */
		library_from_game();
		library_open();
	}
	if (emu_read8(BN6_PET_MENU_TAKEN) == BN6_PET_ENTRY_SAVE) {
		emu_write8(BN6_PET_MENU_TAKEN, 0);
		saving = 1;
	}
	/* a Save: the PET closed, then the checkpoint on the map, as a
	 * layer's arrival takes one (it waits for MegaMan free to move) */
	if (saving && !(emu_read8(BN6_PET_MENU_OPEN) & 1)) {
		saving = 0;
		/* (in the Net Square the run was saved at the gate, and a note says so: square_pet.h) */
		if (online_visit_away()) square_pet_saved();
		else if (director_save_here()) box_wait = SAVE_BOX_WAIT;
	}
	/* the checkpoint taken: the PET's box says so, and takes the next A,
	 * as BN6's own Save ends on its words (HandleSaveMenu8132B88, bn6f) */
	if (director_pet_saved() && !talk_start(pet_saved_text(), FACE_NONE)) cinema_note("Run saved", 150);
}

int pet_key_item(void) {
	if (emu_read8(emu_read32(BN6_TOOLKIT)) != BN6_MODE_SUBMENU) return -1;
	uint32_t m = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU);
	if (m < BN6_EWRAM || m >= BN6_EWRAM_END || emu_read8(m) != BN6_SUBMENU_KEYITEM) return -1;
	if (emu_read8(m + BN6_SUBMENU_STATE) != 4 || emu_read8(m + BN6_SUBMENU_STEP) != 4) return -1;
	unsigned count = emu_read16(m + BN6_KEYITEM_COUNT);
	unsigned at = (emu_read16(m + BN6_KEYITEM_SCROLL) + emu_read16(m + BN6_KEYITEM_ROW)) * 2u + emu_read8(m + BN6_KEYITEM_COLUMN);
	if (!count || at >= count || at >= 0x6F) return -1;
	return emu_read8(BN6_KEYITEM_LIST + 2 * at);
}

bool pet_holds(void) {
	bool juke = jukebox_update(), lib = library_update(), forms = pet_screens_holds();
	return juke || lib || forms;
}

void pet_describe(FILE *f) {
	int mails = (int)emu_read32(BN6_MAIL_COUNT), unread = 0;
	mails = mails < 0 || mails > BN6_MAIL_MAX ? 0 : mails;
	for (int i = 0; i < mails; ++i) unread += flag_get(BN6_FLAG_MAIL_NEW + emu_read8(BN6_MAIL_LIST + (uint32_t)i));
	folder_describe(f);
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	if (items >= BN6_EWRAM && items < BN6_EWRAM_END)
		fprintf(f, "subchips minienrg %d fullenrg %d sneakrun %d untrap %d\n", emu_read8(items + SUB_MINI_ENERGY),
			emu_read8(items + SUB_FULL_ENERGY), emu_read8(items + SUB_SNEAK_RUN), emu_read8(items + SUB_UNTRAP));
	fprintf(f, "mail %d unread %d\n", mails, unread);
	jukebox_describe(f);
	library_describe(f);
	pet_menu_describe(f);
	pet_options_describe(f);
	pet_screens_describe(f);
}

bool pet_second_follows(void) { return library_shown() || pet_screens_second(); }

bool pet_plays_music(void) { return jukebox_shown(); }

void pet_leave(void) {
	jukebox_shut();
	library_shut();
	pet_screens_leave();
}

void pet_draw(void) {
	uint64_t t = frame_part_begin();
	if (seen.gate_strip) netpass_screen_strip();
	netpass_screen_draw();
	frame_part_end("netpass", t);
	t = frame_part_begin();
	jukebox_draw();
	frame_part_end("jukebox", t);
	t = frame_part_begin();
	library_draw();
	frame_part_end("library", t);
	t = frame_part_begin();
	pet_screens_draw();
	frame_part_end("forms", t);
}

uint32_t pet_keys(uint32_t keys) {
	keys = pet_options_keys(keys);
	/* (the PET's menu: ours over BN6's, but in the Net Square, where the menu is BN6's own) */
	if (!online_visit_away()) keys = pet_menu_keys(keys);
	/* (the NetPass up: the game held under it) */
	if (netpass_screen_update()) return 0;
	/* (A on the NetPass, newly pressed and come to the game: the pass opens over the KeyItem screen) */
	if (!saving && (keys & KEY_A) && btn_pressed(BTN_A) && !netpass_screen_shown() && pet_key_item() == PET_ITEM_NETPASS) {
		netpass_screen_open();
		return keys & ~(uint32_t)KEY_A;
	}
	/* (A on the Jukebox: it opens over the KeyItem screen, the game held under it, scene_emu.c) */
	if (!saving && (keys & KEY_A) && btn_pressed(BTN_A) && !jukebox_shown() && pet_key_item() == PET_ITEM_JUKEBOX) {
		jukebox_open();
		return keys & ~(uint32_t)KEY_A;
	}
	if (box_wait > 0 && (talk_busy() || --box_wait == 0)) box_wait = 0;
	if (box_wait > 0) return keys & ~(uint32_t)KEY_A;
	if (!saving) return keys;
	/* (B, pressed and let go, till the PET has closed) */
	++saving;
	return (saving / 6) % 2 ? KEY_B : 0;
}
