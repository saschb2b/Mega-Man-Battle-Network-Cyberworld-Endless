/* The shared dive's crews (issues #589 and #714; docs/online/shared-dive.md,
 * Crews by nearness): who dives together, by where they walk, never by
 * asking. The server decides, one answer for every game (crews.py): two
 * divers of a layer within DIVE_CREW_NEAR of each other for a second are a
 * crew, which holds by chains within DIVE_CREW_FAR, four at most; this
 * keeps what it last said (CREW_STATE). Here is the one place the rest of
 * the game reads it: the ring at the radius (#716), the crew's display
 * (#717), its battles (dive_join.h, #715). */
#ifndef CW_DIVE_CREW_H
#define CW_DIVE_CREW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "dive_wire.h"
#include "protocol.h"

/* World units (the layer's, netmap.c's 32 a panel; SNAPSHOT's and STATE's
 * coordinates): a crew forms within DIVE_CREW_NEAR (four panels) and holds
 * within DIVE_CREW_FAR (six), the server's crew_near and crew_far. */
#define DIVE_CREW_NEAR ONLINE_CREW_NEAR
#define DIVE_CREW_FAR ONLINE_CREW_FAR

void dive_crew_reset(void);
/* The crew in the state's words (online_describe.h): `crew N: NAMES`, 0 alone */
void dive_crew_describe(FILE *f);
/* MegaMan's crew, him among them: 0 alone, else 2..ONLINE_PARTY_MOST. */
int dive_crew_count(void);
/* Its k-th member (0..count-1, in the order they came; MegaMan one of
 * them): session and name. NULL past them. */
const CrewMember *dive_crew_member(int k);
/* Whether `session` is in MegaMan's crew (MegaMan's own: whether he is in one). */
bool dive_crew_has(uint32_t session);
/* A crewmate's name ("" none). */
const char *dive_crew_name(uint32_t session);
/* CREW_STATE: the crew as it stands (formed), none (split_up); a battle's
 * offer and alone to dive_join.h. */
void dive_crew_mail(const uint8_t *m, size_t n);

#endif
