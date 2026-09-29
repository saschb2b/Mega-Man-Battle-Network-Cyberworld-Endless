/* Encounters, rewards and prices, scaled by depth and biome. */
#ifndef LOOT_H
#define LOOT_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"

/* A random battle: plain, from the lower half of the act's band (the run's
 * first battles, the first after a guardian), or a challenge (the next act's
 * band, one version up). */
/* ENC_FIRST: the run's very first battle, one virus where the area has
 * such battles (else as ENC_EASY) */
enum { ENC_NORMAL, ENC_EASY, ENC_CHALLENGE, ENC_FIRST };
Encounter make_encounter(int depth, int biome, int kind);
/* The virus families (a bit each) the area's plain random battles can hold
 * at `depth`; no random numbers drawn. */
uint32_t loot_families_here(int depth, int biome);
enum { FAMILY_SCARCROW = 17, FAMILY_DARKMECH = 25 };
Encounter make_boss(int depth, int biome, int navi);
/* The element strong against an act's guardian, else against its area's
 * viruses (ELEM_*, 0 for none): the Net Dealers stock a chip of it. */
int counter_element(int depth, int biome, int navi);
/* Random chip for rewards/shops; code chosen from the chip's own codes. */
int roll_chip(int depth, int bonus_tier, char *code);
int chip_price(int id);
/* A reward chip's code, leaned to the folder's (docs/META.md): where it
 * rolled a letter and has one of the codes the folder holds most, that one,
 * always or half the time; a * stays a *. */
char loot_fit_code(int id, char code, bool always);
/* The codes a folder holds most, three chips or more each, as run.codes
 * keeps them (code + 1, most held first, 0 for none). */
void loot_folder_codes(const uint16_t *folder, int n, uint8_t out[3]);
/* What the random battles remember of the last one (none of its virus
 * families next), kept beside the run's save: a CONTINUE had forgotten it
 * and brought the last session's pair back first thing. */
typedef struct { int32_t biome, pick; uint32_t families, viruses, families_before; } LootMemory;
void loot_memory(LootMemory *out);
/* The battle MegaMan fights: the next random battle keeps clear of it. */
void loot_battle_fought(const Encounter *e);
void loot_memory_set(const LootMemory *m);

#endif
