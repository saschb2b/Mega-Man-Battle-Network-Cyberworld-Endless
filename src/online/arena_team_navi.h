/* --dev arena2v2: a partner's own battle NaviStats (epic #167, docs/
 * ONLINE_ARENA.md, The partner's own NaviStats). BN6 keeps one battle
 * NaviStats a side (BN6_BATTLE_NAVI), every reader and writer going
 * through BN6_NAVI_STATS_ADDR with a side; the five routines that take
 * the side from the object in r5 (BN6_NAVI_BY_OBJECT: his HP as he is set
 * up, his forms, his mood, his buster's levels) are answered by object
 * instead: side 0's partner, and every object he spawns and they spawn in
 * turn (marked as BN6_SPAWN_COMMON makes them, the marks kept a core for
 * a rollback), read and write his own record (RAM_TEAM_NAVI), a copy of
 * his side's made as the battle spawns its field, his own max HP in it
 * where `teamhp` gives one. Side 1's partner keeps his side's until the
 * RAM for a second record is found. */
#ifndef CW_ARENA_TEAM_NAVI_H
#define CW_ARENA_TEAM_NAVI_H

#include <stdbool.h>
#include <stdint.h>

struct HookCore;

/* the three hooks on one of the pair's cores' table (side its GBA) */
bool arena_team_navi_hooks(struct HookCore *core, unsigned side, int slot);
/* from the spawn's hook, as the field is spawned: the record made from his
 * side's, the core's marks cleared */
void arena_team_navi_begin(void);
/* the record's HP fields (MaxBaseHP, CurHP, MaxHP) folded into a digest
 * both GBAs must agree on, from a hook */
uint32_t arena_team_navi_digest(uint32_t h);
/* how often the record answered, by whom, said in the output */
void arena_team_navi_report(void);

#endif
