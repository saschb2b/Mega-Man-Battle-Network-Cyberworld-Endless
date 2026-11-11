/* pet_carry.h. */
#include "pet_carry.h"

#include <string.h>

const PetCarryRegion pet_carry_regions[PET_CARRY_REGIONS] = {
	{ BN6_EVENT_FLAGS, PET_CARRY_MAIN },
	{ BN6_NAVICUST_LINE, BN6_NAVICUST_LINE_BYTES },
	{ BN6_PACK_EXT, PET_CARRY_EXT },
	{ BN6_PACK_SAVE, BN6_PACK_SAVE_BYTES },
};

void pet_carry_clear(PetCarry *c, uint32_t seed, int32_t depth, uint32_t bits) {
	memset(c, 0, sizeof *c);
	c->seed = seed;
	c->depth = depth;
	c->bits = bits < PET_CARRY_BYTES ? bits : 0;
}

static unsigned counted(const PetCarry *c) {
	unsigned n = 0;
	for (unsigned k = 0; k < PET_CARRY_BYTES; ++k) n += c->mask[k] != 0;
	return n;
}

unsigned pet_carry_note(PetCarry *c, const uint8_t *open, const uint8_t *shut) {
	unsigned n = 0;
	for (unsigned k = 0; k < PET_CARRY_BYTES; ++k) {
		uint8_t m = open[k] ^ shut[k];
		if (!m) continue;
		++n;
		/* (a flag's byte: the bits this session changed, the others the Square's own; any other byte whole,
		 * as the first session found it) */
		if (k < c->bits) {
			uint8_t first = (uint8_t)(m & ~c->mask[k]);
			c->old[k] = (uint8_t)((c->old[k] & ~first) | (open[k] & first));
			c->now[k] = (uint8_t)((c->now[k] & ~m) | (shut[k] & m));
			c->mask[k] |= m;
		} else {
			if (!c->mask[k]) c->old[k] = open[k];
			c->now[k] = shut[k];
			c->mask[k] = 0xFF;
		}
		/* (changed and changed back: left alone) */
		if (!((c->old[k] ^ c->now[k]) & c->mask[k])) c->mask[k] = 0;
	}
	c->count = counted(c);
	return n;
}

bool pet_carry_fits(const PetCarry *c, const uint8_t *home) {
	for (unsigned k = 0; k < PET_CARRY_BYTES; ++k)
		if ((home[k] ^ c->old[k]) & c->mask[k]) return false;
	return true;
}

void pet_carry_apply(const PetCarry *c, uint8_t *home) {
	for (unsigned k = 0; k < PET_CARRY_BYTES; ++k)
		if (c->mask[k]) home[k] = (uint8_t)((home[k] & ~c->mask[k]) | (c->now[k] & c->mask[k]));
}

bool pet_carry_sound(const PetCarry *c) {
	if (c->bits >= PET_CARRY_BYTES || c->count != counted(c) || c->traded > 1) return false;
	for (unsigned k = c->bits; k < PET_CARRY_BYTES; ++k)
		if (c->mask[k] && c->mask[k] != 0xFF) return false;
	return true;
}
