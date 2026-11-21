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

#include <stdio.h>

#include "crew_pull.h"
#include "foes.h"

/* The index on the layer of MegaMan's next random battle: the battles he
 * has fought there */
int director_crew_index(void);
/* The layer's random battle `index` as every game on the layer makes it:
 * its kind by the index, rolled from the layer's seed and the index, the
 * run's own draws and its memory of the battles fought left aside (the
 * run's own roll draws on both): a dive's own battle (dive_battle,
 * director_state.h) with no Mystery Data on its field, whose row is a
 * record's own */
Encounter director_crew_battle(int index);
/* `e` the run's next battle, as the director keeps it (the party's core
 * reads its record; the battle's owner fights it alone where the field
 * cannot be made) */
void director_crew_record(const Encounter *e);
/* BN6's walk toward its next random battle back to none, as a battle's
 * return to the map leaves it; the walk it had */
int director_crew_steps_reset(void);
/* A crew's battle's end as the field left it: won, the reward BN6's
 * results on the field picked, this game's seat and every seat's MegaMan
 * as the field ended (his HP and max, his dive's pull-back spent, his game
 * gone), from which each game works out the crew's gifts alike
 * (crew_pull.h) */
typedef struct {
	bool won;
	uint16_t reward;
	unsigned seat;
	CrewPullIn field;
} CrewOver;
/* The battle over on the party's core: MegaMan's HP as the field left it
 * written into the run, HP 0 the run over as a deletion ends it; else a
 * battle fought on the layer (its count, the next one rolled), and won,
 * its viruses counted and its reward given, BN6's results' entry (a chip,
 * zenny, HP or BugFrags; REWARD_NONE nothing). Pulled back by the crew
 * (issue #718): on the layer with his HP, the dive's pull-back spent and
 * marked, the battle's viruses and reward not his */
void director_crew_over(const Encounter *e, const CrewOver *o);
/* Where the crew's gifts are told: the dive's band (dive_band.c sets it) */
extern void (*director_crew_band)(const char *line);
/* Whether this dive's pull-back is spent (its record's: weekly.h) */
bool director_crew_pull_spent(void);
/* On a dive, its pull-back ("pullback free" or "spent") and the last
 * crew's battle's end, in the state's words ("crewend none", or "crewend
 * won hp 220 pulled" / "... gave 120") */
void director_crew_describe(FILE *f);

#endif
