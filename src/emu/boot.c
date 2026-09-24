/* Boot: the game's own title and NEW GAME, then straight into the net.
 *
 * Emulation is deterministic, so a fixed input script reaches the same
 * state every time: the title at frame 400, NEW GAME 100 frames after START,
 * and the intro's first line 200 frames later. From there the game's warp
 * routine takes MegaMan to Central Area 1, and the intro's cutscene and chat
 * box are closed the way their own end commands close them. The result is
 * cached in the data directory. */
#include "boot.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "emu.h"
#include "game.h"
#include "gamecall.h"

static void run(int frames, uint32_t keys) {
	for (int i = 0; i < frames; ++i) emu_frame(keys);
}

/* bump the number when the boot sequence changes */
static void state_path(char *out, size_t n) { snprintf(out, n, "%s/boot-3.state", g_data_dir); }

bool emu_boot(void) {
	char path[600];
	state_path(path, sizeof path);
	if (emu_load_state(path)) return true;
	emu_reset();
	run(400, 0);
	run(4, KEY_START);
	run(100, 0);
	run(4, KEY_A);             /* NEW GAME */
	run(200, 0);
	emu_warp(0x91, 0, 0, 0, 5);  /* Seaside Area 1: a layer loads as a new area group */
	run(120, 0);
	/* end the intro: no cutscene script, chat box closed (chatbox_E6_end) */
	emu_write32(BN6_CUTSCENE + 0x1C, 0);
	emu_write32(BN6_CUTSCENE + 0x40, 0);
	emu_write32(BN6_CHATBOX_FLAGS, emu_read32(BN6_CHATBOX_FLAGS) & ~0xC8u);
	emu_write8(BN6_CHATBOX + 0, 0);
	emu_write8(BN6_CHATBOX + 4, 0);
	run(30, 0);
	emu_save_state(path);
	return true;
}
