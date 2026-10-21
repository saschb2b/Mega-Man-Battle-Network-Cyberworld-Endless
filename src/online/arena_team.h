/* --dev arena2v2 (epic #167, docs/ONLINE_ARENA.md, Two MegaMen a side):
 * the pair's Practice NetBattle with two MegaMen a side, BN6's own engine
 * fighting them. Two hooks on each of the pair's cores: the Practice list
 * traded for one of four MegaMen as the battle spawns its field, and at
 * the battle loop's copy of the sides' keys (BN6_KEY_COPY) the partners'
 * keys written into their AI data by the loop's iteration, which both
 * GBAs count alike (their frame counters do not: the second GBA's runs a
 * frame or two ahead of the first's at the same iteration). A partner's
 * keys are a stream of the Arena's (players 2 and 3, arena_input.h),
 * sampled on his own side's GBA's frames as its pad is, queued into both
 * GBAs' rings (RAM_TEAM_RING) at that GBA's iteration plus BN6's own link
 * delay, and taken there at each GBA's own iteration. The count and the
 * rings live in the cores' RAM, so a rollback puts them back with the
 * state. Research, never a player's battle. */
#ifndef CW_ARENA_TEAM_H
#define CW_ARENA_TEAM_H

#include <stdbool.h>
#include <stdint.h>

struct mCore;

/* the hooks on one of the pair's cores (side its GBA), on its own table,
 * after its state is loaded; false where the table took none */
bool arena_team_hooks(struct mCore *core, unsigned side);
/* side's GBA (core[side]) in a frame it has not sampled in: its partner's
 * `keys` for that frame queued in both cores' rings, for the iteration
 * its battle loop is at plus the delay (between run loops, by the pair) */
void arena_team_sample(struct mCore *core[2], unsigned side, uint16_t keys);
/* a scripted partner's keys (player 2 or 3) by the frame's number alone,
 * so that a run that rolls back and one that does not hand him the same:
 * side 0's steps up to row 1 and fires charged shots along it, side 1's
 * fires along row 1 and steps down and back between; each uses a chip of
 * his hand now and then */
uint16_t arena_team_script(unsigned player, uint32_t frame);
/* the two GBAs compared iteration by iteration (the four MegaMen's
 * panels, HP and action, their AI data's keys, the random number), said
 * in the output with the partners' keys' takes and each one's HP as it
 * went: false where they came apart or never ran */
bool arena_team_report(void);

#endif
