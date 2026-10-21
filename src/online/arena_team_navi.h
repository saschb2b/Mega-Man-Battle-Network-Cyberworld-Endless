/* --dev arena2v2: each partner's own battle NaviStats and hand (epic
 * #167, docs/ONLINE_ARENA.md, The partners' own NaviStats and hands). BN6
 * keeps one battle NaviStats a side (BN6_BATTLE_NAVI) and one hand
 * (BN6_BATTLE_HAND), every reader and writer going through one routine
 * with a side (BN6_NAVI_STATS_ADDR, BN6_HAND_ADDR). Where the side is
 * that of the object in r5 (the five routines BN6_NAVI_BY_OBJECT: his HP
 * as he is set up, his forms, his mood; the hand: the chip up next, a chip
 * used) they are answered by object instead: a partner, and every object
 * he spawns and they spawn in turn (marked as BN6_SPAWN_COMMON makes
 * them, the marks kept a core for a rollback), read and write his own
 * record and hand. His record is a copy of his side's made as the battle
 * spawns its field, his own max HP in it where `teamhp` gives one; his
 * hand is dealt from his lead's each time the Custom screen deals his
 * lead's anew (his own Custom screen is to come). Their room is BN6's
 * own: the fourth kind's last two object slots, taken in its pool
 * (BN6_T4_ACTIVE) as the field spawns, never run or freed since nothing
 * links them into the run order, cleared with the pool as the next
 * battle sets up. */
#ifndef CW_ARENA_TEAM_NAVI_H
#define CW_ARENA_TEAM_NAVI_H

#include <stdbool.h>
#include <stdint.h>

struct HookCore;

/* the four hooks on one of the pair's cores' table (side its GBA, slot its hook slot) */
bool arena_team_navi_hooks(struct HookCore *core, unsigned side, int slot);
/* from the spawn's hook, as the field is spawned: the rooms taken, the
 * records made from their sides', the core's marks cleared */
void arena_team_navi_begin(void);
/* at each iteration of the battle's loop, from a hook: a lead's hand
 * dealt anew by the Custom screen dealt to his partner too */
void arena_team_navi_deal(void);
/* a hand's chip up next, from a hook: side's lead's, or his partner's */
uint8_t arena_team_navi_at(unsigned side, bool partner);
/* the records' HP fields (MaxBaseHP, CurHP, MaxHP) and the hands' chips up
 * next folded into a digest both GBAs must agree on, from a hook */
uint32_t arena_team_navi_digest(uint32_t h);
/* how often each record and hand answered, by whom, said in the output */
void arena_team_navi_report(void);

#endif
