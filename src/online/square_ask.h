/* The Net Square's open challenge and the link before one (epic #167's
 * fourth step, protocol v13, docs/online/arena/from-square/square-before.md, The Square before a
 * NetBattle): the challenger's strip over the Square while his challenge
 * is open (whom it asks, his PET ringing until he reads the question, then
 * his time to answer running down, the way to call it off), and the link
 * between two players as bars beside his card and beside the question of
 * his challenge. Bars only, never a number (the owner's word). Draws only. */
#ifndef CW_SQUARE_ASK_H
#define CW_SQUARE_ASK_H

#include <stdint.h>

#include "online_types.h"

/* Over the Square's picture: the strip, and the bars beside a card or a
 * question open; `shown` the snapshot as drawn (its peers' bars). The link
 * between this game and a player is the fewer of the two bars, 1-3 (0
 * unknown), his as the server says it, this game's from its own round
 * trip by the same steps (protocol.json's link_good_ms and link_fair_ms):
 * the longer round trip decides a battle's delay. */
void square_ask_draw(const OnlineSnapshot *shown, uint32_t now);

#endif
