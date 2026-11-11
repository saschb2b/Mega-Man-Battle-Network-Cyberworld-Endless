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

#define LAB_FOES 4       /* a battle's foes at most: BN6's spawn takes four */
#define LAB_COLS 6
#define LAB_ROWS 3
#define LAB_PANELS (LAB_COLS * LAB_ROWS)

/* A foe set down: its family (the engine's AI index), version and panel
 * (column 3-5 and row 0-2, from 0) */
typedef struct { uint8_t family, version, col, row; } LabFoe;

/* A set: what a test battle is made of */
typedef struct {
	uint8_t nfoes;
	LabFoe foes[LAB_FOES];
	uint8_t panels[LAB_PANELS];   /* BN6_PANEL_TYPE of each, row by row from the top left */
	uint8_t area;                 /* its background: an area of the net (BIOME_*) */
	uint8_t song;                 /* its song, BN6's number; 0 the battle's own */
} LabSet;

/* The set as a battle of BN6's */
void lab_set_encounter(const LabSet *s, Encounter *e);

#endif
