/* BN6's attack families grown in the core's copy (xattack_grow.h). */
#include "xattack_grow.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"

#define BUS 0x08000000u

/* the stub (r5 the object, lr the runner's way back): a tail call on to
 * object_exitAttackState, which returns to the runner itself:
 *   ldr r0,[pc,#0]; bx r0; .word BN6_EXIT_ATTACK | 1 */
static void stub_make(uint8_t out[8]) {
	put16(out, 0x4800);
	put16(out + 2, 0x4700);
	put32(out + 4, BN6_EXIT_ATTACK | 1u);
}

uint32_t xattack_entry(int family) {
	return family >= XATTACK_FIRST && family < XATTACK_FIRST + XATTACK_COUNT ? XATTACK_TABLE + 4u * (uint32_t)(family - XATTACK_FIRST) : 0;
}

/* a word of BN6's as the player's ROM and the core's copy both have it */
static bool bn6_word(uint32_t at, uint32_t want) { return emu_read32(at) == want && rom_u32(at - BUS) == want; }

bool xattack_grow(void) {
	if (!emu_ready() || !R.data) return false;
	if (emu_read32(BN6_ATTACK_FAMILIES_REF) == XATTACK_TABLE && emu_read32(BN6_ATTACK_FAMILIES_REF2) == XATTACK_TABLE) return true;
	const char *why = NULL;
	if (!bn6_word(BN6_ATTACK_FAMILIES_REF, BN6_ATTACK_FAMILIES) || !bn6_word(BN6_ATTACK_FAMILIES_REF2, BN6_ATTACK_FAMILIES)) why = "a word naming the table";
	for (uint32_t i = 0; !why && i < BN6_ATTACK_FAMILY_COUNT; ++i)
		if (!bn6_word(BN6_ATTACK_FAMILIES + 4 * i, rom_u32(BN6_ATTACK_FAMILIES - BUS + 4 * i))) why = "an entry of BN6's";
	for (uint32_t a = FREE_ATTACK_FAMILIES; !why && a < FREE_ATTACK_FAMILIES_END; a += 4)
		if (emu_read32(a) != 0xFFFFFFFFu) why = "the place (not free)";
	if (why) {
		fprintf(stderr, "xattack: BN6's attack families not grown: %s\n", why);
		return false;
	}
	static uint8_t img[FREE_ATTACK_FAMILIES_END - FREE_ATTACK_FAMILIES];
	memset(img, 0xFF, sizeof img);
	for (uint32_t i = 0; i < XATTACK_COUNT; ++i)
		put32(img + 4 * i, i < BN6_ATTACK_FAMILY_COUNT ? rom_u32(BN6_ATTACK_FAMILIES - BUS + 4 * i) : XATTACK_STUB | 1u);
	stub_make(img + (XATTACK_STUB - XATTACK_TABLE));
	uint8_t ref[4];
	put32(ref, XATTACK_TABLE);
	claims_begin(XATTACK_CLAIMS);
	emu_write(XATTACK_TABLE, img, sizeof img);
	emu_write(BN6_ATTACK_FAMILIES_REF, ref, sizeof ref);
	emu_write(BN6_ATTACK_FAMILIES_REF2, ref, sizeof ref);
	claims_end();
	return true;
}

bool xattack_set(int family, uint32_t routine) {
	uint32_t at = xattack_entry(family);
	if (family < XATTACK_BN6_END || !at || !(routine & 1) || emu_read32(BN6_ATTACK_FAMILIES_REF) != XATTACK_TABLE) return false;
	uint32_t was = emu_read32(at);
	if (was == routine) return true;
	if (was != (XATTACK_STUB | 1u)) return false;
	/* (the table's bytes stay its own, whoever fills an entry: claims.h) */
	claims_begin(XATTACK_CLAIMS);
	emu_write32(at, routine);
	claims_end();
	return true;
}
