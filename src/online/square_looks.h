/* The players' looks in the Net Square (issue #854, docs/online/square/looks.md):
 * MegaMan's own sent over the wire (protocol v30's LOOK) as it changes, and
 * the peers drawn in theirs. Every peer's puppet draws one copy of BN6's
 * field MegaMan whose frames name a palette block of the Square's own, a
 * palette a peer slot, its colours his look's or BN6's own; how many looks
 * the part of the Square has room for is square_looks_pick.h's. */
#ifndef CW_SQUARE_LOOKS_H
#define CW_SQUARE_LOOKS_H

#include <stdbool.h>
#include <stdint.h>

#include "online_square.h"

#define SQUARE_LOOKS_HOLD 90   /* frames a pick is held while the same peers stand in the same part */
#define SQUARE_LOOKS_KEEP 24   /* world units a peer drawn in his look keeps it against a nearer one */

/* The peers' sprite copied into the map's room as the Square is installed
 * (inside its claims) and listed at list 6's BN6_LOOK_FIELD_SPARE: the
 * list-6 number a peer's puppet takes, PUPPET_MEGAMAN where there was no
 * room (BN6's own colours then, as before looks). */
int square_looks_install(void);

/* The looks a part of the Square has room for besides MegaMan's own (its
 * free OBJ palette slots, docs/rom/megaman-colours.md, The Square's slots). */
int square_looks_cap(int part);

/* This frame's looks: `shown`'s peers by slot, their puppets' scripts,
 * MegaMan's place and the part of the Square he is in (SQ_PART_*, -1 none). */
void square_looks_frame(const OnlineSnapshot *shown, const uint32_t scripts[SQUARE_PEERS], int x, int y, int part);

/* MegaMan's own look over the wire where it changed since it was last
 * sent, or again (`again`: a visit begun or a resume's WELCOME). */
void square_looks_send(bool again);

/* For the state in words (online_describe.c): whether peer `peer` (his
 * session) is drawn in his own look (1), in BN6's own colours (0) or not
 * by the pick at all (-1); the peers BN6 drew in a stale slot's colours
 * this frame; the looks the part has room for now. */
int square_looks_shown(uint32_t peer);
int square_looks_stale(void);
int square_looks_room(void);

#endif
