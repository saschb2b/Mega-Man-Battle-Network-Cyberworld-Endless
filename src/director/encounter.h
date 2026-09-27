/* Random encounters on the game's own battle engine, with the run's enemies. */
#ifndef CW_ENCOUNTER_H
#define CW_ENCOUNTER_H

#include <stdbool.h>

#include "foes.h"

/* Makes the game's encounter roll return the engine's battle settings. */
void emu_encounters_install(void);
/* The battle the next encounter starts: enemies, area background and music. */
void emu_encounter_set(const Encounter *e);
/* The record the last emu_encounter_set wrote (0 or 1), and the one the
 * game's battle is set up from (-1: another, a story or test battle; -2:
 * not yet named, early in its setup). */
int emu_encounter_slot(void);
int emu_encounter_battle_slot(void);
/* Off the game's last battle's record: the pointer the next one's setup
 * names its record by keeps the last battle's until the setup writes it
 * (a CONTINUE's state or the last battle), so it is cleared on the map. */
void emu_encounter_battle_forget(void);
/* Starts this battle at once (a boss); release it when the battle is on. */
void emu_battle_force(const Encounter *e);
void emu_battle_release(void);
bool emu_battle_forcing(void);

#endif
