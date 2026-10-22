/* Two MegaMen a side: the turn's one limit for all four (the owner's
 * call, docs/ONLINE_ARENA.md, The turn's limit). Once any player of the
 * match has pressed OK on a turn, every player still picking has
 * TEAM_TURN_LIMIT frames; at it his picks so far are taken: a partner's
 * by his own client (arena_team_client.c) or, where it has gone quiet,
 * by his side's GBA saying none (arena_team_custom.c); a lead's by his
 * own game pressing OK for him, his own keys in his own stream, so that
 * every game takes them alike. Nobody's game presses for anyone else.
 * The clock each game reads is the turn's as the pair's two cores keep
 * it (arena_team_custom_clock_of: frames since the first OK each core
 * has seen), the later of the two; what it decides goes into this
 * game's own stream alone, so the games need not read it alike. */
#ifndef CW_TEAM_HURRY_H
#define CW_TEAM_HURRY_H

#include <stdbool.h>
#include <stdint.h>

#include "arena.h"

/* the turn's frames since the first OK of it, as this game's pair holds them (0: no one has pressed OK yet, or no turn) */
unsigned team_hurry_clock(Arena *a);
/* the hurry sign's due for `seat`: still picking in the turn's last TEAM_TURN_HURRY frames */
bool team_hurry_shown(Arena *a, unsigned seat);
/* side's lead's keys from his own game: `keys` as they are, but past the
 * turn's limit, his screen still open, the keys that take its cursor to
 * OK and press it (`tick` this game's frame count, for the presses'
 * rhythm); said in the output once a turn */
uint16_t team_hurry_lead(Arena *a, unsigned side, uint16_t keys, uint32_t tick);

#endif
