/* Encounters, rewards and prices, scaled by depth and biome. */
#ifndef LOOT_H
#define LOOT_H

#include "foes.h"

/* A random battle: plain, from the lower half of the act's band (the run's
 * first battles, the first after a guardian), or a challenge (the next act's
 * band, one version up). */
enum { ENC_NORMAL, ENC_EASY, ENC_CHALLENGE };
Encounter make_encounter(int depth, int biome, int kind);
Encounter make_boss(int depth, int biome, int navi);
/* Random chip for rewards/shops; code chosen from the chip's own codes. */
int roll_chip(int depth, int bonus_tier, char *code);
int chip_price(int id);

#endif
