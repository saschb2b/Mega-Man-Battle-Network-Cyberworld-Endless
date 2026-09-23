/* Random encounters on the game's own battle engine, with the run's enemies. */
#ifndef CW_ENCOUNTER_H
#define CW_ENCOUNTER_H

#include "battle.h"

/* Makes the game's encounter roll return the engine's battle settings. */
void emu_encounters_install(void);
/* The battle the next encounter starts: enemies, area background and music. */
void emu_encounter_set(const Encounter *e);

#endif
