/* battle_rng.h: the step both games' RNG words take, and a seed's word (the
 * guest's battles', guest.c; the differential's, src/dev/xdiff.c). */
#include "battle_rng.h"

#include "bn5.h"

uint32_t battle_rng_step(uint32_t x) { return ((x << 1 | x >> 31) + 1u) ^ BN5_RNG_XOR; }

/* The step parts the 2^32 states into 21 cycles: two of one state and one
 * of nine, the next shortest of 983, SeedRNG's own of 2,794,329,874. Those
 * eleven states, their lowest bit flipped, lie on cycles of 170 million
 * states or more. */
uint32_t battle_rng_word(uint32_t s) {
	s ^= s >> 16;
	s *= 0x85EBCA6Bu;
	s ^= s >> 13;
	s *= 0xC2B2AE35u;
	s ^= s >> 16;
	uint32_t x = s;
	for (int k = 0; k < 9; ++k)
		if ((x = battle_rng_step(x)) == s) return s ^ 1u;
	return s;
}
