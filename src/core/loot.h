/* Encounters, rewards and prices, scaled by depth and biome. */
#ifndef LOOT_H
#define LOOT_H

#include "foes.h"

Encounter make_encounter(int depth, int biome, bool corrupt, bool challenge);
Encounter make_boss(int depth, int biome, int navi);
/* Random chip for rewards/shops; code chosen from the chip's own codes. */
int roll_chip(int depth, int bonus_tier, char *code);
int chip_price(int id);
int virus_version(int depth, bool hard);

#endif
