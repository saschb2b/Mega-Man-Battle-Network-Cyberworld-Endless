/* Who a core's partners are, for the partners' own NaviStats, hands and
 * Custom screens (arena_team_navi.h, arena_team_custom.h): the Arena's two
 * a side (partner p on side p, the second slot of BN6_BATTLE_PLAYERS'
 * side), or a party's on the shared dive's one field (partners 0-2 on side
 * 0, its slots 1-3; docs/online/shared-dive.md, As built), each core's own
 * by its hook slot. A party's field trades side 0's first slot with the
 * MegaMan whose turn it is to draw the enemies' aim while an enemy's
 * routine runs (party_field.h): a partner's slot is read through that. */
#ifndef CW_TEAM_SHAPE_H
#define CW_TEAM_SHAPE_H

#include <stdbool.h>
#include <stdint.h>

#define TEAM_PARTNERS 3   /* partners on one core at most: the party's three (the Arena's two) */
#define TEAM_ARENA    2   /* ... the Arena's */

/* The core in hook slot `slot` a party's (its field's traded slot read
 * through `swapped`, 0 none), or the Arena's again (party false) */
void team_shape_party(int slot, bool party, const unsigned *swapped);
/* From a hook, or for a core between frames (team_shape_of): */
bool team_shape_is_party(void);
bool team_shape_is_party_of(int slot);
/* ... its partners, and partner p's side */
unsigned team_partners(void);
unsigned team_partners_of(int slot);
unsigned team_partner_side(unsigned p);
/* ... from a hook: the word of BN6_BATTLE_PLAYERS that holds partner p's object now */
uint32_t team_partner_slot(unsigned p);

#endif
