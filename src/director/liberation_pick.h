/* A liberation's team picked on arrival (docs/LIBERATION.md, The team
 * picked): after the board's opening look, before PLAYER PHASE 01, where
 * the run holds more Navis (BN5's Souls, Gregar's Crosses) than BN5's team
 * of the stage has places beside MegaMan. Our own screen in BN6's PET look
 * (docs/UI.md), as neither BN6 nor BN5 has a team screen: each held Navi's
 * face, HP, command and its shape, and the form his battles take with its
 * weakness. It opens on the run's last pick where its Navis are held. Its
 * course here, its picture in liberation_pick_draw.c, its words in
 * liberation_pick_text.c. */
#ifndef CW_LIBERATION_PICK_H
#define CW_LIBERATION_PICK_H

#include <stdbool.h>
#include <stdint.h>

#include "liberation_board.h"

/* The run's last pick, kept in the board's note (liberation_save.c): the
 * run it was made in (run.seed), its Navis */
typedef struct {
	uint32_t run;
	uint8_t n;
	uint8_t navi[LIB_TEAM - 1];
} LibPickMemory;
extern LibPickMemory lib_pick_memory;

/* Whether the board's arrival asks: more Navis held (LB.b.team.held) than
 * the board has places beside MegaMan (LB.b.team.n - 1) */
bool lib_pick_needed(void);
/* The screen opened: the last pick marked where its Navis are held, the
 * board's own team filling the places left */
void lib_pick_open(void);
/* A frame of its keys (the pad's new presses): true once the pick is made,
 * its Navis into navi in the order picked, how many into *n (the board's
 * places, LB.b.team.n - 1) */
bool lib_pick_keys(uint32_t pressed, uint8_t navi[LIB_TEAM], int *n);
/* Its picture, over the board, in BN6's PET look */
void lib_pick_draw(void);

#endif
