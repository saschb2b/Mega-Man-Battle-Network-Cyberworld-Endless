/* The shared dive's join-in battles (issue #589; docs/online/shared-dive.md,
 * As built): a random battle BN6 rolls for MegaMan with a crewmate near
 * (within a few panels, the room's SNAPSHOT) is held back (encounter.h's
 * emu_encounters_offer) and offered (CREW battle, its index on the layer);
 * a crewmate near whose game gets the offer shows it in the dive's band
 * for the window, and A jumps in (CREW join). The server's START takes
 * every game in into one field (party_play_crew), the battle made alike
 * from the layer's seed and its index (director_crew_battle), each
 * joiner's walk toward his next battle reset; nobody joining, the owner
 * fights it alone as ever (CREW_STATE alone). A game that cannot run the
 * one field (the New 3DS, the PSP) offers and joins nothing. */
#ifndef CW_DIVE_JOIN_H
#define CW_DIVE_JOIN_H

#include <stdbool.h>
#include <stdint.h>

#include "dive_wire.h"
#include "online_types.h"

void dive_join_reset(void);
/* Each frame of the dive: the rolls held while a crewmate stands near, a
 * held one offered, the offer's window, the battle's end. */
void dive_join_frame(const OnlineSnapshot *s);
/* The pad's keys: A on an offer; none while MegaMan waits on the answer. */
uint32_t dive_join_keys(uint32_t keys);
/* CREW_STATE's offer and alone. */
void dive_join_crew_state(const CrewState *c);
/* A party battle's START: true where it is the crew's battle this game
 * offered or joined (taken, or refused), else the door's. */
bool dive_join_start(const PartyState *p);
/* The crew's battle under way (its ARENA_STATE's battle id), 0 none */
uint32_t dive_join_battle(void);
/* The band's line while an offer stands or MegaMan waits, NULL none. */
const char *dive_join_band(void);

#endif
