/* --dev arena2v2 on a seat's screen (epic #167, docs/ONLINE_ARENA.md, Two
 * MegaMen a side): over the picture of the GBA a seat sees (seats 0 and 1
 * the GBAs' players, 2 and 3 side 0's and side 1's partners, who see
 * their side's GBA, their own Custom screen BN6's on a core of their own:
 * arena_team_client.h), a partner's own HP box over BN6's (which shows
 * his lead's), and the four MegaMen told apart: each one's seat marked by
 * his feet, the ally BN6 leaves unlabelled given his HP as BN6 gives the
 * enemies theirs. Our own drawing in the PET's colours (second_frame.h).
 * Reads the core, changes nothing. */
#ifndef CW_ARENA_TEAM_VIEW_H
#define CW_ARENA_TEAM_VIEW_H

#include <stdbool.h>

struct mCore;

/* the seat that GBA's picture shows: its side's GBA */
unsigned arena_team_view_side(unsigned seat);
/* drawn over the picture (at P.core_x, P.core_y) of `core`, the GBA `seat`
 * sees; `hurry`: the seat still picking in the turn's last ten seconds
 * (team_hurry_shown), his own plate blinking fast, no number */
void arena_team_view(struct mCore *core, unsigned seat, bool hurry);

#endif
