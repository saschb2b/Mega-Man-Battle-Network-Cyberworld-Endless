/* The Battle Lab's builder (docs/game/battle-lab.md, The builder's screen): a
 * screen of ours in BN6's PET look (docs/engine/ui.md), BN6 having none
 * for it, where a test battle is set up: its foes of the families whose
 * battle data is complete (battle_data.h), each in a version MegaMan has
 * deleted, set down on the foes' nine panels; the field's eighteen panels
 * painted; the background and the song. BATTLE! fights it inside a
 * snapshot of the game (lab_battle.h) and the screen is back with the
 * result. Its course here, its picture in lab_draw.c, its words in
 * lab_text.c; the game's scene drives it through director.h
 * (director_lab_*). */
#ifndef CW_LAB_H
#define CW_LAB_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"
#include "lab_set.h"

/* The set as a battle of BN6's */
void lab_set_encounter(const LabSet *s, Encounter *e);

#endif
