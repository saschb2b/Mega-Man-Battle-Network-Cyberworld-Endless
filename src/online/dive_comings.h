/* The shared dive's comings and goings (issue #758; docs/online/shared-dive.md,
 * Comings and goings): a crewmate whose signal is gone from the layer (he
 * quit, dropped, left the dive), his signal back as he comes onto the layer
 * again, and a crew near MegaMan that is full, each a short line in the
 * dive's band; strangers passing say nothing. A crewmate who walks off is
 * the plates' and the ring's to show, not a line. A crewmate's signal gone
 * is said at most once every COMINGS_EVERY_MS, his return once for each, so
 * a crew of four makes eighteen lines a minute at most (three links
 * flapping), and the band's queue drops its oldest past that; a full crew
 * is said once as MegaMan comes near it. */
#ifndef CW_DIVE_COMINGS_H
#define CW_DIVE_COMINGS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "dive_wire.h"
#include "online_types.h"

#define COMINGS_EVERY_MS 20000   /* a crewmate's line at most so often */

void dive_comings_reset(void);
/* The crew as the server said it, before and after: those who went, to be
 * told apart (gone from the layer, or walked off) as the snapshot shows. */
void dive_comings_crew(const CrewState *before, const CrewState *after, uint32_t me, uint32_t now);
/* CREW_STATE full: a crew near MegaMan that cannot take him. */
void dive_comings_full(const CrewState *full);
/* Whether `session` is in a crew MegaMan was told is full (no "about to
 * be" ring for him, dive_people.c). */
bool dive_comings_full_has(uint32_t session);
/* Each frame of the dive: the room's divers (the transport's snapshot) against those who went. */
void dive_comings_frame(uint32_t now);
/* `comings LINE` (the last said, `none`) */
void dive_comings_describe(FILE *f);

#endif
