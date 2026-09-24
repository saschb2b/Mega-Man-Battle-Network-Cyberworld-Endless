/* Random encounters on the game's own battle engine, with the run's enemies. */
#ifndef CW_ENCOUNTER_H
#define CW_ENCOUNTER_H

#include <stdbool.h>

#include "foes.h"

/* Makes the game's encounter roll return the engine's battle settings. */
void emu_encounters_install(void);
/* The battle the next encounter starts: enemies, area background and music. */
void emu_encounter_set(const Encounter *e);
/* Starts this battle at once (a boss); release it when the battle is on. */
void emu_battle_force(const Encounter *e);
void emu_battle_release(void);
bool emu_battle_forcing(void);

#endif
