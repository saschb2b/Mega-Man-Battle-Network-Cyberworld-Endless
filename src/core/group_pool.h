/* The encounter groups' pool from the ROMs read (groups.h;
 * docs/game/progression/encounter-groups.md): every virus family in the
 * engine a battle may hold, BN6's own and the families moved in from
 * BN5's ROM and BN4's, each with its versions (the ROM's HP, hit and
 * element), where its own game's records set it (visitor_families.h's
 * tally: its lean, its stage) and its measured tags (group_tags.c). A
 * family moved for a summon alone, or one its game sets on special panels
 * alone (a sea's, lava's), is not in it. A family no record of its net
 * sets (seven of BN5's, whose maps' battle tables are not read) roams
 * from the first act.
 *
 * And a battle built from it: an act's arc's, in every run
 * (group_arc_battle), or any template's where `--dev groups` asks
 * (loot.h's loot_groups_always). */
#ifndef CW_GROUP_POOL_H
#define CW_GROUP_POOL_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"
#include "group_arc.h"
#include "groups.h"

/* A group battle's Encounter.from_pick: past any formation's number, the
 * template added */
#define GROUP_FROM_PICK 0x4000

/* The pool (into *out), how many families; made once for the ROMs read
 * and the families installed, again where they change */
int group_pool(const GroupFam **out);
/* The nets it holds families of (groups.h: a bit each) */
unsigned group_nets(void);

/* Battle `e` (a plain random battle at `depth` in area `biome`, as the
 * area rolled it) made a group's: a template the act's pool fills and one
 * of its fillings, by the layer's seed and the battle's index (no random
 * number drawn, so a checkpoint's layer rolls the same), on the panels
 * (group_place); its field one of the area's own battles' on which every
 * panel the group stands on is plain or cracked, else the plain field
 * (the owner's call 2). `only` a template (groups.h's GROUP_*), or -1
 * any. False, `e` as it was, where the pool fills none. */
bool group_battle(int depth, int biome, uint32_t layer_seed, int index, int only, Encounter *e);

/* The arc of the act `depth` lies in, in area `biome`, of the run of seed
 * `run_seed` (group_arc.h): its signature, its beats, its challenge, by
 * the seed and the act alone, so every layer of the act and a CONTINUE
 * have the same; kept for the act once made. Act 1 of the first cycle
 * holds every beat to the eased limits (the band's lower half). False where no family of
 * the pool can be a signature there. */
bool group_arc_at(uint32_t run_seed, int depth, int biome, GroupArc *out);
/* Battle `e` (as the area rolled it) made the arc's beat of the layer at
 * `depth` (its first layer the signature alone, its second the pair, its
 * third the constellation; the Nest's one layer its constellation), or
 * where `challenge` the Server's; Encounter.arc_beat and .signature say
 * which. False, `e` as it was, where the act has no arc (or no
 * challenge). */
bool group_arc_battle(uint32_t run_seed, int depth, int biome, bool challenge, Encounter *e);

#endif
