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

#include <SDL.h>

#include "bn6.h"
#include "bn6_chips.h"
#include "emu.h"
#include "game.h"
#include "gamecall.h"
#include "platform.h"

static int ran;   /* frames the boot has run */

static void run(int frames, uint32_t keys) {
	/* (the boot's frames are never shown: the core draws no picture) */
	bool shown = P.skip_present;
	P.skip_present = true;
	for (int i = 0; i < frames; ++i) emu_frame(keys);
	P.skip_present = shown;
	ran += frames;
}

/* Bump the number when the boot sequence changes, or what NEW GAME gives
 * in the core's copy. 4: ten-bit chip ids (xchips_wide.c), the preset
 * folders widened. A boot-3.state, kept by a build before them, holds the
 * Standard folder nine-bit, which ten bits read as other chips: BN4's
 * CloudMn for Cannon B, dealt blank where BN4's ROM is not read (a New
 * 3DS's run; own_folder_chips mends a run begun from one). */
#define BOOT_STATE "boot-4.state"
#define BOOT_STATE_OLD "boot-3.state"
static void state_path(char *out, size_t n, const char *name) { snprintf(out, n, "%s/%s", g_data_dir, name); }

/* The key items a run gains (bn6f's KeyItems counts): the RegUPs
 * (0x72-0x74, scripts.h's SCRIPTS_REG_UP1), the SubChips (from 0x80,
 * shop.h's SUB_MINI_ENERGY) and the NaviCust's programs (from
 * BN6_PROGRAM_ITEMS). BN6's NEW GAME gives none of them (it gives 0x00
 * once and 0x75 four times: measured on a fresh boot, 17 November 2026). */
#define KEY_REG_UPS   0x72
#define KEY_REG_UPS_END 0x75
#define KEY_SUBCHIPS  0x80
#define KEY_ITEMS     0x100

/* BN6 just past its NEW GAME, as the boot leaves it, in what a run keeps:
 * no zenny or BugFrags, MegaMan's 100 HP, no RegUP, SubChip or program,
 * and the pack holding the Standard folder's thirty chips and no other. A
 * kept state that differs is a played game's, and a new run begun from it
 * keeps that game's MegaMan (issue #688): it is made again. */
static bool new_game_held(void) {
	uint32_t keys = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS), pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	if (keys < BN6_EWRAM || keys + KEY_ITEMS > BN6_EWRAM_END || pack < BN6_EWRAM || pack >= BN6_EWRAM_END) return false;
	int items = 0, chips = 0;
	for (uint32_t i = KEY_REG_UPS; i < KEY_ITEMS; ++i)
		if (i < KEY_REG_UPS_END || i >= KEY_SUBCHIPS) items += emu_read8(keys + i);
	for (unsigned id = 1; id < BN6_PACK_IDS; ++id)
		for (uint32_t k = 0, e = bn6_pack_entry(pack, id); e && k < 4; ++k) chips += emu_read8(e + k);
	unsigned zenny = emu_read32(BN6_ZENNY), frags = emu_read32(BN6_BUGFRAGS), hp = emu_read16(BN6_NAVI_MAX_HP), base = emu_read16(BN6_NAVI_BASE_MAX_HP);
	bool ok = !zenny && !frags && hp == 100 && base == 100 && !items && chips == 30;
	if (!ok) printf("boot: the kept state is no new game's (zenny %u, BugFrags %u, max HP %u/%u, a run's key items %d, chips %d): made again\n",
		zenny, frags, hp, base, items, chips);
	return ok;
}

bool emu_boot(const char *from) {
	char path[600];
	state_path(path, sizeof path, BOOT_STATE);
	if (emu_load_state(from ? from : path) && new_game_held()) return true;
	uint32_t t0 = SDL_GetTicks();
	ran = 0;
	emu_reset();
	run(400, 0);
	run(4, KEY_START);
	run(100, 0);
	run(4, KEY_A);             /* NEW GAME */
	run(200, 0);
	emu_warp(0x91, 0, 0, 0, 5);  /* Seaside Area 1: a layer loads as a new area group */
	run(120, 0);
	/* end the intro: no cutscene script, chat box closed (chatbox_E6_end) */
	emu_write32(BN6_CUTSCENE_POS, 0);
	emu_write32(BN6_CUTSCENE_POS0, 0);
	emu_write32(BN6_CHATBOX_FLAGS, emu_read32(BN6_CHATBOX_FLAGS) & ~0xC8u);
	emu_write8(BN6_CHATBOX, 0);
	emu_write8(BN6_CHATBOX_STATE, 0);
	run(30, 0);
	bool kept = emu_save_state(path);
	/* (the old number's, no build reads it now: its 420 KB back) */
	char old[600];
	state_path(old, sizeof old, BOOT_STATE_OLD);
	if (kept) remove(old);
	/* (once per install: a slow machine's first start is told apart) */
	printf("boot: %d frames from power-on in %.2f s, %s\n", ran, (SDL_GetTicks() - t0) / 1000.0,
		kept ? "kept for the next start" : "not kept: the state could not be written");
	return true;
}
