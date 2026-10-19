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
/* A layer's random battle (the step counter's): make_encounter's, with a
 * visitor of the other net where one is due (visitors.h: the layer's
 * seed, its battles fought so far `index`) */
Encounter loot_random_battle(int depth, int biome, int kind, uint32_t layer_seed, int index);
/* (dev: every battle the visitors' rules allow takes one, --dev visitors) */
extern bool loot_visitors_always;
/* The area (NET_AREAS + k, rom.h) whose own game's battle records a
 * layer's random battles are, fought in BN6's engine (xbattles.h, issue
 * #82), as make_encounter rolls them; -1 none (BN6's area's own) */
extern int loot_territory;
/* The deep (docs/MULTIROM.md, One net: the deep converges): the older
 * net's areas a deep act's random battles draw on beside BN6's own area's,
 * a coin a battle between the nets (and between the two where both are
 * named: the Nest's), as make_encounter rolls them; -1 none. Set by the
 * director where BN5's ROM is read and its records fought in BN6's
 * engine: the Undernet's with BN5's Undernet, the Graveyard's with Nebula
 * Area, the Nest's with both. */
extern int loot_deep[2];
/* The share (percent) of the chips the run finds and buys that come from
 * the older net's catalogue (roll_chip; chip_pool.h's CHIP_NET_BN5): half
 * on its areas' layers and in the deep where its ROM is read, else 0 */
extern int loot_older_share;
#define LOOT_OLDER_SHARE 50
/* The layer's nets, for a layer of BN6 area `biome` drawn in area `tiles`
 * (another game's where it dresses it, run_dress; -1 none: home): sets
 * loot_territory where every record of the area can be fought in BN6's
 * engine, loot_deep in the deep (taking the place of a territory), and
 * loot_older_share on the older net's areas and in the deep where BN5's
 * ROM is read; true where the layer's battles take the older net's
 * records so */
bool loot_layer_nets(int biome, int tiles);
/* When BN6's virus family `family` first meets MegaMan on the net: 0 the
 * surface areas, 1 the Undernet, 2 the Graveyard and the Underground (3
 * never); and the highest a battle at `depth` may hold (BN6's rule:
 * the comps and homepages keep to the families the run has reached) */
int loot_family_stage(int family);
int loot_stage_allowed(int depth);
enum { FAMILY_STARFISH = 15, FAMILY_SCARCROW = 17, FAMILY_DARKMECH = 25 };
Encounter make_boss(int depth, int biome, int navi);
/* The element that hits an act's guardian twice as hard (of either wheel),
 * else its area's viruses (ELEM_*, 0 for none): the Net Dealers stock a
 * chip of it. */
int counter_element(int depth, int biome, int navi);
/* Random chip for rewards/shops; code chosen from the chip's own codes. */
int roll_chip(int depth, int bonus_tier, char *code);
/* A chip of `tier` (chip_pool.h) from BN6's pool, or the older net's its
 * share of the time (loot_older_share); -1 without a ROM */
int loot_pool_pick(int tier);
/* An in-battle Mystery Data's four rewards, the game's encoding (bits
 * 14-15: 0 chip, 1 zenny, 3 BugFrags; a chip's id in bits 0-8, its code
 * in 9-13), one of which the game gives where it survives the battle
 * (docs/ROM_DATA.md): three chips a tier above a blue Mystery Data's, in
 * the folder's codes where they come in them, and zenny twice its. */
void loot_gem_rewards(int depth, uint16_t out[4]);
/* A Mystery Data on the enemies' side of `e`'s field, where it has none
 * and a panel is free (a test's, --dev gem). */
void loot_add_gem(Encounter *e);
int chip_price(int id);
/* A reward chip's code, leaned to the folder's (docs/META.md): where it
 * rolled a letter and has one of the codes the folder holds most, that one,
 * always or half the time; a * stays a *. */
char loot_fit_code(int id, char code, bool always);
/* A code the folder can play chip `id` in: the first of the folder's codes
 * the chip comes in, else (with `star`) its * if it has one, else 0. */
char loot_folder_code(int id, bool star);
/* The codes a folder holds most, three chips or more each, as run.codes
 * keeps them (code + 1, most held first, 0 for none). */
void loot_folder_codes(const uint16_t *folder, int n, uint8_t out[3]);
/* The folder's copies of each chip, as the layer is made (NULL: none), and
 * whether chip `id` has as many as BN6 lets a folder hold (by its memory:
 * 5 under 20 MB, 4, 3, 2, 1 from 50): a dealer's pick of it is of no use. */
void loot_folder_counts(const uint16_t *folder, int n);
bool loot_folder_full(int id);
/* How many of chip `id` the folder holds, as loot_folder_counts was given it. */
int loot_folder_copies(int id);
/* What the random battles remember of the last one (none of its virus
 * families next), kept beside the run's save: a CONTINUE had forgotten it
 * and brought the last session's pair back first thing. */
typedef struct { int32_t biome, pick; uint64_t families; uint32_t viruses; uint64_t families_before; } LootMemory;
void loot_memory(LootMemory *out);
/* The battle MegaMan fights: the next random battle keeps clear of it. */
void loot_battle_fought(const Encounter *e);
void loot_memory_set(const LootMemory *m);
/* ... the last one's area and battle (Encounter.from_biome, from_pick) */
void loot_last_fought(int *biome, int *pick);
/* A battle a random roll may pick, as the repeats are weighed: its own
 * weight (0: none it may pick), its virus families (a bit each) and its
 * viruses (a hash of their ids, whatever their panels) */
typedef struct { int weight; uint64_t families; uint32_t viruses; } LootCand;
/* The weights of `n` such battles of area `biome` against the last two
 * fought: none of either one's virus families where another fits, then
 * none of the last one's, then not its very viruses again, then anything;
 * a family the last battle brought a third as likely. Into weight[], their
 * sum (0: none at all). No random numbers drawn. */
int loot_repeat_weights(const LootCand *c, int n, int biome, int *weight);
/* One of `n` by those weights (their sum `total`, over 0): its index */
int loot_weighted_pick(const int *weight, int n, int total);

#endif
