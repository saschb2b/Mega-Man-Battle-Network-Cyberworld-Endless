/* The shared dive's join-in battles on the run's side (issue #589;
 * docs/online/shared-dive.md, As built): a crewmate's random battle that
 * others on the layer jump into is fought on the party's core
 * (party_play.h), every game making the same battle from the layer's seed
 * and the battle's index; its end comes back into each run as a battle of
 * its own: BN6's reward its results screen gave, MegaMan's HP as the field
 * left it, a deletion ending the run as alone. */
#ifndef CW_DIRECTOR_CREW_H
#define CW_DIRECTOR_CREW_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"

/* The index on the layer of MegaMan's next random battle: the battles he
 * has fought there */
int director_crew_index(void);
/* The layer's random battle `index` as every game on the layer makes it:
 * its kind by the index, rolled from the layer's seed and the index, the
 * run's own draws and its memory of the battles fought left aside (the
 * run's own roll draws on both, so two divers' n-th battles differ); no
 * Mystery Data on its field, whose row is a record's own */
Encounter director_crew_battle(int index);
/* `e` the run's next battle, as the director keeps it (the party's core
 * reads its record; the battle's owner fights it alone where the field
 * cannot be made) */
void director_crew_record(const Encounter *e);
/* BN6's walk toward its next random battle back to none, as a battle's
 * return to the map leaves it; the walk it had */
int director_crew_steps_reset(void);
/* The battle over on the party's core: MegaMan's HP as the field left it
 * written into the run, HP 0 the run over as a deletion ends it; else a
 * battle fought on the layer (its count, the next one rolled), and won,
 * its viruses counted and `reward` given, BN6's results' entry (a chip,
 * zenny, HP or BugFrags; REWARD_NONE nothing) */
void director_crew_over(const Encounter *e, bool won, int hp, uint16_t reward);

#endif
