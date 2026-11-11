/* BN6's PET in the Net Square (docs/online/square/pet.md). START opens it
 * there as everywhere, its screens BN6's own on the game's own core; the
 * Square's keys and chats rest while it is up. What it changes is noted
 * each time it closes (pet_carry.h) and carried home onto the gate's
 * checkpoint, so a folder made in the Square is the run's. Save stays the
 * gate's: a note says where the run was saved. On the PET's menu R opens
 * the NetPass, named on its band. */
#ifndef CW_SQUARE_PET_H
#define CW_SQUARE_PET_H

#include <stdbool.h>
#include <stdint.h>

/* A visit begins, the gate's checkpoint just saved and the run still in
 * place: nothing carried yet, and no record kept from a visit before */
void square_pet_begin(void);
/* BN6's PET up over the Square: its menu, or a screen of it */
bool square_pet_open(void);
/* Each frame of the game in the Square. True the frame the PET has closed
 * with something changed: MegaMan as carried is read again (online_visit.h),
 * and a chip up for trade that is no spare any more comes down (trade_central.h) */
bool square_pet_frame(void);
/* The keys while the PET is up: BN6's, but for the engine's own screens
 * over it (pet.h) and R on its menu, the NetPass */
uint32_t square_pet_keys(uint32_t keys);
/* Over the Square's picture: the NetPass while it is up; over the PET, the
 * Jukebox and the Library too, and the menu's key hint */
void square_pet_draw(void);
/* The PET shut and the map back for a few frames: a chat box may open */
bool square_pet_calm(void);
/* Save chosen in the Square's PET (pet.c): the game's note once the map is back, "Run saved at the gate" */
void square_pet_saved(void);
/* The Square left: the PET's own screens put away, a PET still open not noted */
void square_pet_leave(void);
/* A change the engine itself makes to the Square's game that home must have
 * too, noted as the PET's are: the regions read before it, and after.
 * `trade`: it is the visit's trade, the Pack's chips swapped, which home then
 * does not make again (online_trade.h). True where bytes changed. */
void square_pet_noting(void);
bool square_pet_noted(bool trade);
/* At home on the run's checkpoint just loaded (and at a CONTINUE, from the
 * record's file): the PET's changes written into the game where it stands
 * as the PET found it, the record another run's or layer's left behind (a
 * way home that loads no checkpoint, the week's dive's, leaves it in its
 * file). True where something was: the caller saves. `traded`: the
 * visit's trade was among them. */
bool square_pet_home(bool *traded);

#endif
