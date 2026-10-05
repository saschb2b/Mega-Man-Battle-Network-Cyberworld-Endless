/* The game's own random numbers (game.h): xorshift, its seed mixed first. */

#include "game.h"

static uint32_t rng_s = 0x9E3779B9u;
/* A seed goes through a mixer first: xorshift from seeds one apart gave
 * nearly the same first numbers, so runs started with consecutive seeds
 * (and their layers) came out alike. */
void rng_seed(uint32_t s) {
	s += 0x9E3779B9u;
	s ^= s >> 16; s *= 0x85EBCA6Bu;
	s ^= s >> 13; s *= 0xC2B2AE35u;
	s ^= s >> 16;
	rng_s = s ? s : 0x9E3779B9u;
}
void rng_restore(uint32_t s) { rng_s = s ? s : 0x9E3779B9u; }
uint32_t rng_state(void) { return rng_s; }
uint32_t rng_next(void) {
	uint32_t x = rng_s;
	x ^= x << 13; x ^= x >> 17; x ^= x << 5;
	return rng_s = x;
}
int rng_range(int lo, int hi) {
	if (hi <= lo) return lo;
	return lo + (int)(rng_next() % (uint32_t)(hi - lo + 1));
}
