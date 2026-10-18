/* The Arena from the Net Square (issue #120, docs/ONLINE_ARENA.md): a
 * Practice NetBattle asked of another player's MegaMan in BN6's chat boxes
 * (online_chat.c's Battle), his player asked in one ("... wants a Practice
 * NetBattle! Take it?", the cursor on No), and both games then off the
 * Square's map into the Arena's two linked cores over the network's link
 * (arena_link_net.h). The Square's core holds still meanwhile, its
 * connection kept, MegaMan standing where he stood on the server's view;
 * after the battle he is back there, and says how it went. Not offered
 * where the Arena is not built (CW_ARENA: the 3DS, the browser). */
#ifndef CW_SQUARE_ARENA_H
#define CW_SQUARE_ARENA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_types.h"

/* the Battle choice offered at all in this build */
bool square_arena_offered(void);
/* The last battle fought from the Square in this game: 1 won, 2 lost, 3
 * over otherwise, 0 none; `name` the other's. */
unsigned square_arena_last(const char **name);
/* As MegaMan arrives in the Square, and as he leaves it: any battle left */
void square_arena_reset(void);
void square_arena_end(void);
/* Battle chosen for `peer`, my latency in ms */
void square_arena_challenge(const OnlinePeer *peer, uint32_t latency_ms);
/* An Arena message come in (ARENA_STATE and the relayed three) */
void square_arena_mail(const uint8_t *data, size_t size);
/* After each frame in the Square: the question asked, the word said */
void square_arena_update(const OnlineSnapshot *s);
/* The battle under way (the Square's core held still): a frame of it
 * with the pad's keys, and its picture, instead of the Square's */
bool square_arena_on(void);
void square_arena_frame(uint32_t keys);
void square_arena_draw(void);

#endif
