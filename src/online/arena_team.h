/* --dev arena2v2 (epic #167's 2v2 spike; docs/ONLINE_ARENA.md, Two
 * MegaMen a side): the pair's Practice NetBattle with two MegaMen a side,
 * BN6's own engine fighting them. Two hooks on each of the pair's cores:
 * the Practice list traded for one of four MegaMen as the battle spawns
 * its field, and at the battle loop's copy of the sides' keys
 * (BN6_KEY_COPY) the partners' keys written into their AI data, scripted
 * by the loop's iteration, which both GBAs count alike (their frame
 * counters do not: the second GBA's runs a frame or two ahead of the
 * first's at the same iteration, which is what the study's one-frame skew
 * was). The count lives in the core's RAM (RAM_TEAM_TICK), so a rollback
 * puts it back with the state. Research, never a player's battle. */
#ifndef CW_ARENA_TEAM_H
#define CW_ARENA_TEAM_H

#include <stdbool.h>

struct mCore;

/* the hooks on one of the pair's cores (side its GBA), on its own table,
 * after its state is loaded; false where the table took none */
bool arena_team_hooks(struct mCore *core, unsigned side);
/* the two GBAs compared iteration by iteration (the four MegaMen's
 * panels, HP and action, their AI data's keys, the random number), said
 * in the output: false where they came apart or never ran */
bool arena_team_report(void);

#endif
