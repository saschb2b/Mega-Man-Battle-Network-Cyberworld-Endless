/* Two MegaMen a side (epic #167, issue #181; docs/online/arena/two-a-side/overview.md, Two
 * MegaMen a side): the pair's NetBattle with two MegaMen a side, BN6's
 * own engine fighting them, behind --dev arena2v2 and for a side-by-side
 * team match (arena_seats.h). Hooks on each of the pair's cores: the
 * NetBattle's list traded for one of four MegaMen as the battle spawns its
 * field, and at the battle loop's copy of the sides' keys (BN6_KEY_COPY)
 * the partners' keys written into their AI data as BN6 writes a player's,
 * the loop's iterations counted. A partner's keys are a stream of the
 * Arena's (players 2 and 3, arena_input.h), sampled on his own side's
 * GBA's frames as its pad is and carried in that GBA's own link packets
 * (arena_team_link.h), so that both GBAs take them at the same iteration
 * as they take his lead's. Everything lives in the cores' RAM, so a
 * rollback puts it back with the state. */
#ifndef CW_ARENA_TEAM_H
#define CW_ARENA_TEAM_H

#include <stdbool.h>
#include <stdint.h>

struct mCore;

/* the hooks on one of the pair's cores (side its GBA), on its own table,
 * after its state is loaded; false where the table took none */
bool arena_team_hooks(struct mCore *core, unsigned side);
/* side's GBA (core[side]) in a frame it has not sampled in: its partner's
 * `value` for that frame (keys, or a message of his Custom screen) handed
 * to it for its packets (team_link_put; between run loops, by the pair) */
void arena_team_sample(struct mCore *core[2], unsigned side, uint16_t value);
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
