/* --dev arena2v2: a partner's own Cross (epic #167, docs/ONLINE_ARENA.md,
 * His Cross). BN6's crossing controller (BN6_CROSS_SIDES) keeps a record
 * a side, copied from the turn's link packets, and crosses those records'
 * objects alone: each side's player. A partner chooses his Cross on his
 * own Custom screen (BN6's CROSSSELECT on his client's copy of the pair);
 * the form it committed crosses in his stream before his picks
 * (TEAM_CROSS, arena_team_custom.h) and lives in both GBAs' RAM as his
 * pending form for the turn. Three hooks on each of the pair's cores then
 * run BN6's own crossing for his object too: the turn's form check sends
 * the controller to its crossing where a partner has a Cross (as where a
 * side's record has one), the crossing's step asks his object to cross as
 * it asks the records' (BN6_AI_CROSS_ASK) and waits for him as for them,
 * and the crossing's phases, which read the pending form by the side,
 * are answered his own where the crossing object is his. His form is then
 * his own record's (the NaviStats answered by object, arena_team_navi.h),
 * on both GBAs alike. His Beast Out commits a form as his Cross does and
 * enters so; its turns and its end are not counted for him yet (BN6 runs
 * them for each side's player alone). Research, never a
 * player's battle. */
#ifndef CW_ARENA_TEAM_CROSS_H
#define CW_ARENA_TEAM_CROSS_H

#include <stdbool.h>

struct HookCore;

/* the three hooks on one of the pair's cores' table (side its GBA, slot
 * its hook slot: teamcross=-N's mutation on the second GBA's alone) */
bool arena_team_cross_hooks(struct HookCore *core, unsigned side, int slot);
/* how the partners crossed, every core's, said in the output */
void arena_team_cross_report(void);

#endif
