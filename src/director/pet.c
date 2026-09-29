/* The PET's Save as the run's (docs/PET.md). */
#include "pet.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "emu.h"
#include "rom.h"

/* A Save under way: the PET closing, then the checkpoint. */
static int saving;

/* Our part of the PET's input (state 4, EMU_FREE + 0x300): A on Save,
 * disabled in a run, taken from the game (its A bit cleared) and left in
 * the menu's spare byte for the engine, then on to the game's own input
 * handler, which sees no A there; Comm keeps BN6's grey and buzz.
 *   ldrb r0,[r5,#4]; cmp r0,#7; bne 1f        (the cursor on Save)
 *   ldrb r1,[r5,#9]; cmp r1,#0; bne 1f        (input not held off)
 *   ldr r2,=0x0200A272; ldrh r1,[r2]; movs r3,#1; tst r1,r3; beq 1f
 *   bics r1,r3; strh r1,[r2]; strb r0,[r5,#15]
 * 1: ldr r0,=0x08120B91; bx r0 */
#define PET_INPUT_AT (EMU_FREE + 0x300)

static uint32_t rom32(uint32_t at) {
	uint32_t o = at - 0x08000000u;
	return (uint32_t)R.data[o] | (uint32_t)R.data[o + 1] << 8 | (uint32_t)R.data[o + 2] << 16 | (uint32_t)R.data[o + 3] << 24;
}

void pet_install(void) {
	static const uint8_t input[] = {
		0x28, 0x79, 0x07, 0x28, 0x0A, 0xD1, 0x69, 0x7A, 0x00, 0x29, 0x07, 0xD1, 0x04, 0x4A, 0x11, 0x88,
		0x01, 0x23, 0x19, 0x42, 0x02, 0xD0, 0x99, 0x43, 0x11, 0x80, 0xE8, 0x73, 0x01, 0x48, 0x00, 0x47,
		0x72, 0xA2, 0x00, 0x02, 0x91, 0x0B, 0x12, 0x08,
	};
	/* (only where the game is the one we read: its input handler's
	 * pointer in the PET's state table, the grey's store) */
	if (rom32(BN6_PET_INPUT_PTR) == BN6_PET_INPUT) {
		emu_write(PET_INPUT_AT, input, sizeof input);
		emu_write32(BN6_PET_INPUT_PTR, PET_INPUT_AT | 1);
	}
	/* Save lit, Comm greyed as BN6 has it without a link: the grey's
	 * store to Save's colour (0x03001B58) made a no-op */
	uint32_t grey = BN6_PET_GREY_SAVE - 0x08000000u;
	if (R.data[grey - 4] == 0xE2 && R.data[grey - 3] == 0x69 && R.data[grey] == 0x11 && R.data[grey + 1] == 0x80) {
		static const uint8_t nop[] = { 0xC0, 0x46 };
		emu_write(BN6_PET_GREY_SAVE, nop, sizeof nop);
	}
}

void pet_update(void) {
	if (emu_read8(BN6_PET_MENU + 0xF) == 7) {
		emu_write8(BN6_PET_MENU + 0xF, 0);
		saving = 1;
	}
	/* a Save: the PET closed, then the checkpoint on the map, as a
	 * layer's arrival takes one (it waits for MegaMan free to move) */
	if (saving && !(emu_read8(BN6_PET_MENU + 5) & 1)) {
		saving = 0;
		director_save_here();
	}
}

uint32_t pet_keys(uint32_t keys) {
	if (!saving) return keys;
	/* (B, pressed and let go, till the PET has closed) */
	++saving;
	return (saving / 6) % 2 ? KEY_B : 0;
}
