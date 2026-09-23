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

/* Engine hooks in the free ROM space */
#define WARP_DATA (EMU_FREE + 0x000)
#define WARP_STUB (EMU_FREE + 0x100)

static void put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

static void run(int frames, uint32_t keys) {
	for (int i = 0; i < frames; ++i) emu_frame(keys);
}

void emu_warp(int group, int number, int x, int y, int facing) {
	/* the warp record the routine copies to Warp2011bb0, then a warp list
	 * (index 1) pointing back at it */
	uint8_t data[32] = { (uint8_t)group, (uint8_t)number, 0, (uint8_t)facing };
	put32(data + 4, (uint32_t)x << 16);
	put32(data + 8, (uint32_t)y << 16);
	data[17] = 1;
	put32(data + 20, WARP_DATA);
	emu_write(WARP_DATA, data, sizeof data);
	/* push {r4-r7,lr}; ldr r2,=mark; mov r3,#1; strb r3,[r2]; ldr r0,=src;
	 * ldr r1,=dst; ldmia/stmia 32 bytes; ldr r3,=routine; bl 1f;
	 * pop {r4-r7,pc}; 1: bx r3 */
	static const uint8_t code[] = {
		0xF0, 0xB5, 0x07, 0x4A, 0x01, 0x23, 0x13, 0x70, 0x06, 0x48, 0x07, 0x49,
		0x3C, 0xC8, 0x3C, 0xC1, 0x3C, 0xC8, 0x3C, 0xC1, 0x05, 0x4B, 0x00, 0xF0,
		0x01, 0xF8, 0xF0, 0xBD, 0x18, 0x47, 0x00, 0x00,
	};
	uint8_t stub[48];
	memcpy(stub, code, sizeof code);
	put32(stub + 32, BN6_ENGINE_MARK);
	put32(stub + 36, WARP_DATA);
	put32(stub + 40, BN6_WARP);
	put32(stub + 44, BN6_ENTER_MAP_ON_WARP);
	emu_write(WARP_STUB, stub, sizeof stub);
	/* borrow the overworld hook until the stub has run (the game skips it
	 * on some frames): ldr r0,[pc]; bx r0; .word stub+1 */
	uint8_t saved[8], jump[8] = { 0x00, 0x48, 0x00, 0x47 };
	put32(jump + 4, WARP_STUB + 1);
	for (int i = 0; i < 8; ++i) saved[i] = emu_read8(BN6_OW_HOOK + (uint32_t)i);
	emu_write8(BN6_ENGINE_MARK, 0);
	emu_write(BN6_OW_HOOK, jump, sizeof jump);
	for (int i = 0; i < 120 && !emu_read8(BN6_ENGINE_MARK); ++i) run(1, 0);
	emu_write(BN6_OW_HOOK, saved, sizeof saved);
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
