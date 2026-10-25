/* The battle engine's random numbers, BN5's and BN6's alike (docs/
 * docs/rom/bn5-battles.md, BN5's random numbers): both games keep two u32 words
 * (BN5_RNG_PRIMARY and BN5_RNG_SECONDARY, BN6_RNG_PRIMARY and
 * BN6_RNG_SECONDARY) and step them the same way. */
#ifndef CW_BATTLE_RNG_H
#define CW_BATTLE_RNG_H

#include <stdint.h>

/* One step of a word (BN5's step at 0x08001490, BN6's at 0x0800151C): x
 * rotated left once, one added, XORed with BN5_RNG_XOR */
uint32_t battle_rng_step(uint32_t x);
/* A state of a word from seed s: s mixed (murmur3's finaliser), and off
 * the step's shortest cycles, where every roll would come out alike */
uint32_t battle_rng_word(uint32_t s);

#endif
