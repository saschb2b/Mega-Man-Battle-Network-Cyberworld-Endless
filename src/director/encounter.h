/* Random encounters on the game's own battle engine, with the run's enemies. */
#ifndef CW_ENCOUNTER_H
#define CW_ENCOUNTER_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"

/* Makes the game's encounter roll return the engine's battle settings, and
 * hooks its battles: their start (EV_BATTLE_START, events.h) and their
 * rewards, picked in the folder's codes half the time (docs/META.md). */
void emu_encounters_install(void);
/* The battle the next encounter starts: enemies, area background and music. */
void emu_encounter_set(const Encounter *e);
/* The songs a battle's record names, a random battle's and a boss's: BN6's
 * themes, or another game's where the layer is one of its areas
 * (docs/MULTIROM.md), as the layer sets them. */
#define ENCOUNTER_SONG_BATTLE 0x15
#define ENCOUNTER_SONG_BOSS   0x16
extern int encounter_song[2];
/* The background a battle's record names: -1 for its area's own, as BN6
 * rolls them, or another game's (xbackdrop_install), as the layer sets it. */
extern int encounter_backdrop;
/* Set by the director on a layer whose random battles are another game's,
 * on the guest core (guest.h): a battle the roll gives begins none in BN6
 * and queues EV_GUEST_BATTLE. */
extern bool encounter_guest;
/* The record the last emu_encounter_set wrote (0 or 1), and which one a
 * battle's BattleSettings pointer (EV_BATTLE_START's r0) is: -1 another,
 * a story or test battle's. */
int emu_encounter_slot(void);
int emu_encounter_record(uint32_t settings);
/* The battle StartBattle was given (EV_BATTLE_START's r0), told between
 * frames: a guardian's pays zenny where his row has his chip, which his
 * Guardian Data gives. */
void emu_encounter_started(uint32_t settings);
/* The next battle watched, until emu_battle_unwatch (the duel's: docs/
 * RIVAL.md): MegaMan's hits queued as EV_MEGAMAN_HIT, and the first enemy
 * to spawn given `hp_cap` HP at most (0: its own). */
void emu_battle_watch(int hp_cap);
void emu_battle_unwatch(void);
/* Starts this battle at once (a boss); release it when the battle is on. */
void emu_battle_force(const Encounter *e);
void emu_battle_release(void);
bool emu_battle_forcing(void);

#endif
