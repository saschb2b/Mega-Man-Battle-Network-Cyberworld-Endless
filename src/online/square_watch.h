/* The stands in the Net Square (issue #128, docs/ONLINE.md, The stands):
 * the referee Mr.Prog by the ring calls the channel's NetBattle from the
 * server's report (STANDS) on every game, and where this game can watch it
 * (the Arena built here, and BN5's ROM for a battle by full rules) asks
 * "WANNA WATCH?". Yes asks the server, and the screen turns to the
 * battle's pair, built and run here from both players' keys
 * (arena_watch.h), the Square's core held as it is in a battle of this
 * game's: R shows the other side's GBA, L cheers (the Square's Cheer
 * emote, which the players hear of only after their battle), B leaves,
 * back in the Square where MegaMan stood. The Board posts the report too. */
#ifndef CW_SQUARE_WATCH_H
#define CW_SQUARE_WATCH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_types.h"

/* As MegaMan arrives in the Square, and as he leaves it */
void square_watch_reset(void);
void square_watch_end(void);
/* A message of the stands come in (STANDS, WATCH_STATE, WATCH_RELAY, WATCH_KEYS) */
void square_watch_mail(const uint8_t *data, size_t size);
/* After each frame in the Square: the referee's Yes taken up */
void square_watch_update(const OnlineSnapshot *s);
/* The referee's archive: his own `words`, then the channel's battle and
 * his question where this game can watch it; false where it could not be
 * written. square_watch_said changes whenever what he says would. */
bool square_watch_referee(const char *words);
uint32_t square_watch_said(void);
/* A NetBattle under way in the channel, as the server last said */
bool square_watch_live(void);
/* The Board's posts on the channel's battles ("" none) */
const char *square_watch_board(void);
/* Watching (the Square's core held): a frame of it, and its picture */
bool square_watch_on(void);
void square_watch_frame(uint32_t keys);
void square_watch_draw(void);

#endif
