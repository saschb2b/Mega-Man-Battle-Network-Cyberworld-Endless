/* A test battle (docs/game/battle-lab.md, The battle): the Battle Lab's, fought
 * inside a snapshot of the game. Asked for while MegaMan stands free on a
 * map, it starts as the next frame's update begins: the core's whole state
 * and the director's are kept, then BN6 fights the battle as any other,
 * with the blow that would delete MegaMan ending it (battle_spare.h). Its
 * frames are its own: nothing of the director's runs in them, so nothing
 * of the battle is counted. When BN6 is back on its map both are put back,
 * and the update the battle took the place of runs then: the run's game
 * is byte for byte what it was. */
#ifndef CW_LAB_BATTLE_H
#define CW_LAB_BATTLE_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"

enum { LAB_NONE, LAB_WON, LAB_DELETED, LAB_LEFT };

/* How the last test battle ended: its time in frames, BN6's busting level
 * (1-10, 11 S; 0 unless won), the HP MegaMan lost in it */
typedef struct {
	int end;                  /* LAB_* */
	int frames, bust, hp_lost;
	int took;                 /* the game's frames it took, start to end, all put back */
	uint32_t before, after;   /* the game's work RAM hashed as the battle began, and as it was put back */
} LabResult;

/* The battle `e` asked for, on background `backdrop` (BN6's number; -1 its
 * area's own, as BN6 rolls it):
 * the caller has MegaMan free on a map. It begins with the next frame. */
void lab_battle_ask(const Encounter *e, int backdrop);
/* ... on a field of these eighteen panels (BN6_PANEL_TYPE each, row by row
 * from the top left; NULL: the battle's own field): written over BN6's
 * last field in the core's copy while the battle is on, and put back */
void lab_battle_panels(const uint8_t *panels);
/* The builder's (lab.h): the battle `e` begun and held with its field up,
 * its foes in and BN6's Custom screen a frame from opening, so that the
 * builder shows BN6's own field and not a picture of one; `bare`: held
 * before the foe its record names comes in, a field with nobody set. One
 * shown already is put back first, so each stands on the frame the first
 * began on. Called between frames, after the director's update.
 * lab_battle_go lets the one held run on into its fight; when that ends,
 * all is put back and lab_battle_result has how. lab_battle_leave puts
 * all back without a fight. */
void lab_battle_show(const Encounter *e, int backdrop, bool bare);
bool lab_battle_held(void);
bool lab_battle_loading(void);
void lab_battle_go(void);
void lab_battle_leave(void);
/* Whether one is asked for or on */
bool lab_battle_on(void);
/* How the last one ended (LAB_NONE: none ended yet) */
const LabResult *lab_battle_result(void);
/* The director's update, first thing: true while a test battle has the
 * frame (it began, is fought, or is not yet back on the map); false where
 * none is on, and on the frame it ended, the game put back. */
bool lab_battle_frame(void);

#endif
