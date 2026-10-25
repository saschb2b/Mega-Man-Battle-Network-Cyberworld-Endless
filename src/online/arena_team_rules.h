/* --dev arena2v2 under full rules (epic #167's 2v2; docs/online/arena/two-a-side/full-rules.md,
 * What a partner meets under full rules): a partner's own battle NaviStats
 * (arena_team_navi.h) told to the Souls' and the mood's hooks
 * (soul_navi_stats_by), which write a BN5 DarkChip's after-effect and his
 * dark there, and read his side's Navi there for BN5's dark gate. */
#ifndef CW_ARENA_TEAM_RULES_H
#define CW_ARENA_TEAM_RULES_H

#include <stdint.h>

/* the resolver told to the Souls' hooks, as arena_team's hooks go on; and the
 * audit's mutations, chipaibreak=N and partnerbreak=N (docs/devtools/) */
void arena_team_rules_begin(void);
/* side's partner's own battle NaviStats where `obj` is a side's partner and
 * his room is taken (arena_team_navi.h), else 0. From a hook. */
uint32_t arena_team_rules_stats(uint32_t obj);

#endif
