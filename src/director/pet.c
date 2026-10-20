/* The PET's Save as the run's (docs/PET.md). */
#include "pet.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "director_gate.h"
#include "emu.h"
#include "netpass_screen.h"
#include "pet_text.h"
#include "platform.h"
#include "rom.h"

/* A Save under way: the PET closing, then the checkpoint. */
static int saving;

/* The PET's input handler (its state 4, run each frame the menu takes
 * input), hooked: A on Save, disabled in a run, is taken from the game (its
 * bit in the keys just pressed cleared) and left in the menu's spare byte
 * for the engine, so the handler sees no A there; Comm keeps BN6's grey and
 * buzz. */
static HookAct pet_input(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	if (hook_read8(BN6_PET_CURSOR) != 7 || hook_read8(BN6_PET_HOLD)) return HOOK_CONTINUE;
	uint16_t keys = hook_read16(BN6_KEYS_PRESSED);
	if (!(keys & KEY_A)) return HOOK_CONTINUE;
	hook_write16(BN6_KEYS_PRESSED, (uint16_t)(keys & ~KEY_A));
	hook_write8(BN6_PET_MENU_TAKEN, 7);
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
	/* Save lit, Comm greyed as BN6 has it without a link: the grey's
	 * store to Save's colour (0x03001B58) made a no-op */
	uint32_t grey = BN6_PET_GREY_SAVE - 0x08000000u;
	if (R.data[grey - 4] == 0xE2 && R.data[grey - 3] == 0x69 && R.data[grey] == 0x11 && R.data[grey + 1] == 0x80) {
		static const uint8_t nop[] = { 0xC0, 0x46 };
		emu_write(BN6_PET_GREY_SAVE, nop, sizeof nop);
	}
}

void pet_update(void) {
	if (emu_read8(BN6_PET_MENU_TAKEN) == 7) {
		emu_write8(BN6_PET_MENU_TAKEN, 0);
		saving = 1;
	}
	/* a Save: the PET closed, then the checkpoint on the map, as a
	 * layer's arrival takes one (it waits for MegaMan free to move) */
	if (saving && !(emu_read8(BN6_PET_MENU_OPEN) & 1)) {
		saving = 0;
		director_save_here();
	}
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

void pet_draw(void) {
	if (home_gate_asking()) netpass_screen_strip();
	netpass_screen_draw();
}

uint32_t pet_keys(uint32_t keys) {
	/* (the NetPass up: the game held under it) */
	if (netpass_screen_update()) return 0;
	/* (A on the NetPass, newly pressed and come to the game: the pass opens over the KeyItem screen) */
	if (!saving && (keys & KEY_A) && btn_pressed(BTN_A) && !netpass_screen_shown() && pet_key_item() == PET_ITEM_NETPASS) {
		netpass_screen_open();
		return keys & ~(uint32_t)KEY_A;
	}
	if (!saving) return keys;
	/* (B, pressed and let go, till the PET has closed) */
	++saving;
	return (saving / 6) % 2 ? KEY_B : 0;
}
