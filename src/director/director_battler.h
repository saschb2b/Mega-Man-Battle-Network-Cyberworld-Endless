/* A Virus Battler's series on a layer (director_battler.c; docs/
 * docs/world/level-design/virus-battler.md), for the director's frame. */
#ifndef CW_DIRECTOR_BATTLER_H
#define CW_DIRECTOR_BATTLER_H

#include <stdbool.h>
#include <stdio.h>

/* A run begun or continued: the layer's Battler looked for again on its map */
void battler_forget(void);
/* His stake paid (his choice's Yes): the series' first round, BN6's own
 * record in battle mode 6 */
void battler_begin(void);
/* Back on the map from a round: BN6_BATTLE_RESULT's `result`, said once a
 * talk can start */
void battler_round_over(int result);
/* Each frame on the layer's map: the layer's Battler and the profile's card
 * in the game as MegaMan comes onto it; his word after a round, then the
 * next round where FIGHT ON was chosen; the card a chat gave, for the
 * profile */
void battler_update(void);
/* After any battle: the families BN6 put on the card (a Rare deleted),
 * into the profile's (btlr_card.h) */
void battler_card_note(void);
/* The Battler in a state's words (director_describe; tests/scenarios'
 * extras_battler): the profile's card and the families on it (`btlrcard`
 * held or none, `virusdata` N); the layer's `battler` (1-5, none), his
 * `stake` and `vbrounds`, his `series` (none, on, over), the round under
 * way and the rounds won (`vbround`, `vbwins`), the `pot` as it stands,
 * the last round's result (`vblast` none, won, lost, draw) and his first
 * prize in this run (`vbprize` open, taken); after a round won, whether
 * his choice shows (`potchoice` up, none); in a round's battle, where
 * BN6's screen stands (`vbscreen` list, ok, placing, confirm, fight) and
 * the viruses picked and placed (`vbpicked`, `vbplaced`) */
void battler_describe(FILE *f);

#endif
