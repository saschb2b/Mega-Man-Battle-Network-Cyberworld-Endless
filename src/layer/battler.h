/* BN6's Virus Battlers in a run (docs/LEVEL_DESIGN.md, The Virus Battler):
 * each one's series of BN6's own records, his stake, his pairs as the ROM
 * names them and his first prize; the flags his chats set on a layer. */
#ifndef CW_BATTLER_H
#define CW_BATTLER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "text.h"

/* Event flags a Battler's chats set on his layer (layer_objs.h's range):
 * fight on chosen after a win, his series over on this layer (cashed out,
 * won or lost), and the BtlrCard given (the profile's from then on) */
#define BATTLER_ON_FLAG   0x1477
#define BATTLER_DONE_FLAG 0x1478
#define BATTLER_CARD_FLAG 0x1479

/* Battler `k` (1-5, BN6's order: RoboDog's comp to Central Area 1): his
 * series' rounds (3, 5, 5, 10, 10), the stake to enter and what each win
 * adds to the pot (his repeat prize in BN6, 500 to 3000 zenny), and the
 * BattleSettings of round `round` (0-based) in BN6's list 0 */
int battler_rounds(int k);
int battler_stake(int k);
uint32_t battler_record(int k, int round);
/* ... that round's pair as BN6 names them, in capitals, "METTAUR AND
 * GUNNER" (a Mr.Prog says it); false where the record is not as BN6's */
bool battler_pair(int k, int round, char *out, size_t n);

/* His own Mr.Prog's sprite (sprite list 6), as BN6 dresses each Battler */
int battler_sprite(int k);
/* His first prize, paid with a run's first series won against him: a chip
 * (its id and code, 0-25, 26 '*') or a NaviCust program (its give id and
 * colour, as BN6's EF 1B takes them), and its name from the ROM */
typedef struct {
	bool chip;
	int id, code;
	char name[24];
} BattlerPrize;
void battler_prize(int k, BattlerPrize *out);

/* Battler `k`'s offer on the layer being installed (battler_words.h's
 * ta_battler: his first pair, his stake, the BtlrCard where the profile
 * holds none), `flag` his choice's; his chats' flags cleared. Its script. */
int battler_offer(TextArchive *t, int flag, int k);

#endif
