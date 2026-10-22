/* The pair a NetBattle from the Net Square builds, and what of it is a
 * 2v2's (protocol v15, issue #181, docs/ONLINE_ARENA.md, Over the
 * server): the four seats as START names them, the MegaMen built into
 * the pair (seats 0 and 1's as the GBAs', seats 2 and 3's as the
 * partners', arena_seats.h), my seat's own stream and screen (a partner's
 * own Custom screen, arena_team_client.h), whose PET a stop names, and
 * the battle's end in the log. Part of square_arena.c's course, on its
 * state (square_arena_state.h). */
#ifndef CW_SQUARE_BOUT_H
#define CW_SQUARE_BOUT_H

#include <stdbool.h>
#include <stdint.h>

#include "arena.h"
#include "arena_wire.h"

/* START come: a 2v2's four seats (the names kept), or a battle of two's */
void square_bout_seats(const ArenaWireState *w);
/* my MegaMan as he goes in, the seats known: a partner's own folder made a
 * Program Advance's first under --dev teampa=1 (arena_seats_pa_folder) */
void square_bout_mine(OnlineLoadout *l);
/* the pair built from the MegaMen come (the way in begun); false where
 * memory ran out */
bool square_bout_build(void);
/* the way in done, the pair at its Custom screen: my seat's screen (a
 * partner's own Custom screen made now); false where it could not be */
bool square_bout_ready(void);
/* my keys this tick under --dev arenaboth (the bot's, side 0's lead's R
 * now and then side by side) */
uint32_t square_bout_bot(uint32_t keys, uint32_t tick);
/* the battle's stage after a tick: a 2v2's end said in the log */
void square_bout_after(ArenaStage stage);
/* the player a stop names: the other, or a 2v2's seat it waits on */
const char *square_bout_whose(void);
/* my seat's own client let go */
void square_bout_free(void);

#endif
