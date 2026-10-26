/* The BtlrCard across runs (docs/world/level-design/virus-battler.md): the
 * profile's card and the virus data on it, kept like the Library, in each
 * run's game as BN6's own flags. */
#ifndef CW_BTLR_CARD_H
#define CW_BTLR_CARD_H

#include <stdbool.h>
#include <stdint.h>

/* The profile's card into the run's game: BN6_FLAG_BTLRCARD and each
 * family's data flag set as the profile holds them and cleared where it
 * does not (a boot's own leave none), its key item given; none without the
 * card. */
void card_to_game(void);
/* ... and back: the card a Battler's chat gave (BATTLER_CARD_FLAG, with
 * Mettaur's and Gunner's data), and the families the run's game has put
 * on it since (a Rare deleted: BN6's "You got new Virus Data"), the
 * profile saved where it changed. */
void card_from_game(void);
bool card_held(void);
/* The families on the card, BN6's VIRUS LIST's count of BN6_CARD_FAMILIES */
int card_families(void);

#endif
