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

/* The battle `e` asked for, on background `backdrop` (-1 the map's own):
 * the caller has MegaMan free on a map. It begins with the next frame. */
void lab_battle_ask(const Encounter *e, int backdrop);
/* The director's update, first thing: true while a test battle has the
 * frame (it began, is fought, or is not yet back on the map); false where
 * none is on, and on the frame it ended, the game put back. */
bool lab_battle_frame(void);
const LabResult *lab_battle_result(void);

#endif
