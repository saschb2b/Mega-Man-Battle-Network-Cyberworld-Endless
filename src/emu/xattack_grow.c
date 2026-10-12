/* BN6's attack families grown (xattack_grow.h). */
#include "xattack_grow.h"

#include <stdio.h>

#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"

#define BUS 0x08000000u
/* The empty entry's routine: ldr r0,[pc,#0]; bx r0; BN6_EXIT_ATTACK's address (a tail call: the attack ends, back to
 * the family's caller, r5 its object as BN6's own families leave it) */
#define LDR_R0_PC0 0x4800u
#define BX_R0      0x4700u
#define EMPTY_SIZE 8u

_Static_assert(XATTACK_EMPTY + EMPTY_SIZE <= FREE_ATTACK_FAMILIES_END && !(XATTACK_EMPTY & 3),
	"the grown attack families past their room (free_space.h)");

static bool grown_now(void) {
	return emu_read32(BN6_ATTACK_FAMILIES_REF) == FREE_ATTACK_FAMILIES && emu_read32(BN6_ATTACK_FAMILIES_REF2) == FREE_ATTACK_FAMILIES;
}

bool xattack_grow(void) {
	if (!R.data || !emu_ready()) return false;
	if (grown_now()) return true;
	/* (every word and entry of BN6's as the player's ROM has it, and the room free, before one byte is written) */
	bool ok = emu_read32(BN6_ATTACK_FAMILIES_REF) == BN6_ATTACK_FAMILIES && emu_read32(BN6_ATTACK_FAMILIES_REF2) == BN6_ATTACK_FAMILIES &&
		rom_u32(BN6_ATTACK_FAMILIES_REF - BUS) == BN6_ATTACK_FAMILIES && rom_u32(BN6_ATTACK_FAMILIES_REF2 - BUS) == BN6_ATTACK_FAMILIES;
	uint8_t table[4 * XATTACK_ENTRIES + EMPTY_SIZE];
	for (uint32_t i = 0; ok && i < BN6_ATTACK_FAMILY_COUNT; ++i) {
		uint32_t v = emu_read32(BN6_ATTACK_FAMILIES + 4 * i);
		ok = v == rom_u32(BN6_ATTACK_FAMILIES - BUS + 4 * i) && (v & 1);
		put32(table + 4 * i, v);
	}
	for (uint32_t i = 0; ok && i < sizeof table; i += 4) ok = emu_read32(FREE_ATTACK_FAMILIES + i) == 0xFFFFFFFFu;
	if (!ok) {
		fprintf(stderr, "xattack: BN6's attack families or their room are not as expected: the table left as it is\n");
		return false;
	}
	for (uint32_t i = BN6_ATTACK_FAMILY_COUNT; i < XATTACK_ENTRIES; ++i) put32(table + 4 * i, XATTACK_EMPTY | 1);
	uint8_t *e = table + 4 * XATTACK_ENTRIES;
	put16(e, LDR_R0_PC0);
	put16(e + 2, BX_R0);
	put32(e + 4, BN6_EXIT_ATTACK | 1);
	claims_begin(XATTACK_CLAIMS);
	emu_write(FREE_ATTACK_FAMILIES, table, sizeof table);
	emu_write32(BN6_ATTACK_FAMILIES_REF, FREE_ATTACK_FAMILIES);
	emu_write32(BN6_ATTACK_FAMILIES_REF2, FREE_ATTACK_FAMILIES);
	claims_end();
	printf("xattack: BN6's attack families grown to CurAction %#x\n", XATTACK_LAST);
	return true;
}

bool xattack_set(int action, uint32_t routine) {
	if (action < BN6_ATTACK_FAMILY_FIRST + BN6_ATTACK_FAMILY_COUNT || action > XATTACK_LAST || !(routine & 1) || !grown_now()) return false;
	uint32_t at = FREE_ATTACK_FAMILIES + 4u * (uint32_t)(action - BN6_ATTACK_FAMILY_FIRST), was = emu_read32(at);
	if (was == routine) return true;
	if (was != (XATTACK_EMPTY | 1)) return false;
	/* (the table's bytes stay its own, whoever fills an entry: claims.h) */
	claims_begin(XATTACK_CLAIMS);
	emu_write32(at, routine);
	claims_end();
	return true;
}
