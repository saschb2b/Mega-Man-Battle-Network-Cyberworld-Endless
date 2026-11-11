/* The Battle Lab's keeper in Lan's HP (docs/game/battle-lab.md, How it is
 * earned, Where it lives): a Mr.Prog of Dad's lab, there once the profile
 * has it (or --dev lab says so). Talked to, he says his lines (his first
 * once a profile, lab_words.c) and the builder opens over the held game
 * as the chat box closes (lab.h). His course at home, beside the gate's
 * (director_gate.h). */
#ifndef CW_LAB_KEEPER_H
#define CW_LAB_KEEPER_H

#include <stdbool.h>

/* the flag his script sets to open the builder; the director clears it */
#define LAB_OPEN_FLAG 0x147D

/* Whether the Battle Lab is there for this profile */
bool lab_there(void);
/* (his visit's start and his frame at home, home_lab_visit and
 * home_lab_frame, are director_home.h's, beside the gate's) */

#endif
