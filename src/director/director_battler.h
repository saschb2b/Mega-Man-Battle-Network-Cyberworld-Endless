/* A Virus Battler's series on a layer (director_battler.c; docs/
 * docs/world/level-design/virus-battler.md), for the director's frame. */
#ifndef CW_DIRECTOR_BATTLER_H
#define CW_DIRECTOR_BATTLER_H

#include <stdbool.h>

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

#endif
