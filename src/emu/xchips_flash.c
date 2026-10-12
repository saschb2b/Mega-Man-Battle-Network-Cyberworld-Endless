/* BN5's green flash in BN6's engine (xchips_flash.h). */
#include "xchips_flash.h"

#include <stdio.h>

#include "bn5.h"
#include "bn5_chips.h"
#include "bn6.h"
#include "bn6_chips.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "xchips_bn5.h"
#include "xchips_code.h"
#include "xvirus.h"
#include "xvirus_recipe.h"

#define BUS 0x08000000u
#define TINTS 8   /* BN5's table of the flash's colour shaders (BN5_FLASH_TINTS), by the timer's low three bits */

static uint8_t tints[TINTS];

static int side_of(uint32_t obj) { return hook_read8(obj + BN6_T1_ALLIANCE) & 1; }
static uint32_t flash_at(uint32_t obj) { return XC_FLASH + XC_FLASH_STEP * (uint32_t)side_of(obj); }

/* (the moved code's strh r0,[r1,#0x3c], r0 the frames, r5 the chip's user) to the side's flash, on past it */
static HookAct store(HookRegs *r, void *user) {
	(void)user;
	hook_write16(flash_at(r->r[5]), (uint16_t)r->r[0]);
	r->r[12] = (r->pc + 2u) | 1u;
	return HOOK_JUMP;
}

/* (BN6_FLASH_COUNT: ldr r4,[r5,#0x58], past the time stop's test, r5 the player) one frame less, as BN5's count-down of its
 * four timers (0x0800E53E-0x0800E554) takes one from the first */
static HookAct count(HookRegs *r, void *user) {
	(void)user;
	uint32_t at = flash_at(r->r[5]);
	uint16_t v = hook_read16(at);
	if (v) hook_write16(at, (uint16_t)(v - 1));
	return HOOK_CONTINUE;
}

/* (BN6_FLASH_TINT: the bl to 0x080168C8 in an object's chain of colours, r5 it) BN5's tint before it while the flash lasts:
 * sprite_setColorShader's store (BN6 0x08002ED0: the shader at the sprite's +6, its block by the object's +2's high bits) of
 * BN5's shader by the timer's low bits, shifted up five (0x080136BA-0x080136C4) */
static HookAct tint(HookRegs *r, void *user) {
	(void)user;
	uint32_t obj = r->r[5];
	uint16_t v = hook_read16(flash_at(obj));
	if (!v) return HOOK_CONTINUE;
	uint32_t block = (uint32_t)(hook_read8(obj + BN6_OBJ_SPRITE_BLOCK) >> 4) << 4;
	hook_write16(obj + block + BN6_SPRITE_SHADER, (uint16_t)(tints[v & (TINTS - 1)] << 5));
	return HOOK_CONTINUE;
}

/* the moved code's address of BN5's `bn5` (a unit's or a helper's copy), 0 where none holds it */
static uint32_t moved(uint32_t bn5) {
	for (int i = 0; i < xchips_bn5_count; ++i)
		if (xvirus_is_code(&xchips_bn5[i].code, bn5)) return xvirus_code_at(&xchips_bn5[i].code, bn5 | 1u) & ~1u;
	return 0;
}

/* every byte it stands on: BN5's stores, count-down and tint (their CRCs: BN5_FLASH_*), BN6's two places */
static bool expected(const uint8_t *rom5) {
	static const uint32_t stores[] = { BN5_FLASH_STORE, BN5_FLASH_STORE_GATE };
	for (size_t i = 0; i < sizeof stores / sizeof *stores; ++i)
		if (get16(rom5 + (stores[i] - BUS)) != BN5_FLASH_STRH || get16(rom5 + (stores[i] - 2 - BUS)) != BN5_FLASH_FRAMES_OP) return false;
	return xvirus_crc32(rom5 + (BN5_FLASH_COUNT - BUS), BN5_FLASH_COUNT_END - BN5_FLASH_COUNT) == BN5_FLASH_COUNT_CRC &&
		xvirus_crc32(rom5 + (BN5_FLASH_TINT - BUS), BN5_FLASH_TINT_END - BN5_FLASH_TINT) == BN5_FLASH_TINT_CRC &&
		rom_u16(BN6_FLASH_COUNT - BUS) == BN6_FLASH_COUNT_OP && rom_u32(BN6_FLASH_TINT - BUS) == BN6_FLASH_TINT_BL;
}

bool xchips_flash_install(void) {
	static bool done;
	if (done) return true;
	const uint8_t *rom5 = XR[XROM_BN5_COLONEL_US].data;
	if (!rom5 || !R.data || !emu_ready()) return false;
	uint32_t at = moved(BN5_FLASH_STORE), gate = moved(BN5_FLASH_STORE_GATE);
	if (!expected(rom5) || !at || !gate) {
		fprintf(stderr, "xchips_flash: BN5's green flash NOT hooked: %s\n", at && gate ? "a byte it stands on is not as expected" :
			"the moved code holds no store of it");
		return false;
	}
	for (int k = 0; k < TINTS; ++k) tints[k] = rom5[BN5_FLASH_TINTS - BUS + (uint32_t)k];
	/* (its stores' hooks the moved code's own: they stand on its bytes; BN6's two the flash's) */
	claims_begin(XC_CLAIMS);
	bool ok = emu_hook(at, store, NULL) && emu_hook(gate, store, NULL);
	claims_end();
	claims_begin(XC_FLASH_CLAIMS);
	ok = ok && emu_hook(BN6_FLASH_COUNT, count, NULL) && emu_hook(BN6_FLASH_TINT, tint, NULL);
	claims_end();
	done = ok;
	printf("xchips_flash: BN5's green flash%s\n", ok ? ": its stores, count-down and tint hooked" : " NOT hooked whole");
	return ok;
}
