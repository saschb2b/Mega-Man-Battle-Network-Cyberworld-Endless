/* The crew as it was (issue #778; docs/online/shared-dive.md, The door
 * waits for the crew): since a crew lives only on screen (#750, #770), one
 * who walks ahead to a guardian's door has none left when he gets there.
 * The door waits instead for those who were in MegaMan's crew within the
 * last CREW_RECENT_MS and are still on the layer (the room's snapshot) and
 * free. This keeps, for each crewmate the server named, when he was last
 * in the crew; it reads the server's word and changes nothing else. */
#ifndef CW_DIVE_CREW_RECENT_H
#define CW_DIVE_CREW_RECENT_H

#include <stdint.h>

#include "dive_wire.h"
#include "online_types.h"

#define CREW_RECENT_MS 30000   /* how long one who was in the crew is waited for at a door */

void dive_crew_recent_reset(void);
/* CREW_STATE: the crew before and after it, each of them in it until `now` */
void dive_crew_recent_note(const CrewState *before, const CrewState *after, uint32_t me, uint32_t now);
/* Whether `session` was in MegaMan's crew within CREW_RECENT_MS (in it now counts) */
int dive_crew_recent_was(uint32_t session, uint32_t now);
/* Those who were, still in the room `s` and free: their sessions into `out`
 * (at most `most`), their count */
int dive_crew_recent_here(const OnlineSnapshot *s, uint32_t now, uint32_t *out, int most);

#endif
