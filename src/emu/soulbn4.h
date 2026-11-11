/* BN4 Red Sun's Souls in BN6's engine (docs/multirom/bn4-souls.md, ThunSoul
 * in BN6's engine): on the proxy form as BN5's six are, united on BN6's
 * Custom screen by the run's rule (BN5's), each with its charged shot's
 * power attack and attack family moved with the BN4 chips' units (its
 * family an attack unit's, its power attack copied with it:
 * tools/xchiprecipe_bn4.py SOUL_POWERS), its loadout, charge times and
 * element read from BN4's tables by its form. BN6's numbers for them: a
 * Soul's number in BN6_SOULS' fields past BN5's 1-12, its bit in a
 * MegaMan's Souls held past BN5's six. ThunSoul first; which of them a run
 * gives is the owner's call (none yet: the differential's souls6 alone). */
#ifndef CW_SOULBN4_H
#define CW_SOULBN4_H

#include <stdbool.h>
#include <stdint.h>

#define SOUL_BN4_FIRST 13   /* BN6's Soul number (BN6_SOUL_ID, the element's +BN6_ELEMENT_SOUL, a used bit) of BN4's first moved */
#define SOULS_BN4      1    /* BN4's Souls moved: ThunSoul (BN4's 6) */
#define SOULS_HELD_BN4 6    /* the bit of the first in the Souls held (soulunite_held), past BN5's six */

/* A BN4 Soul's rows as BN6's engine takes them, read from BN4's ROM */
typedef struct {
	uint8_t form;      /* BN4's Soul, its form (1-12) */
	uint8_t kind;      /* the chip kind it unites by (BN4_SOUL_KINDS: BN5's numbering, soulkinds.h) */
	uint8_t element;   /* MegaMan's element in it (BN4_FORM_ELEMENTS: BN6's numbering) */
	uint8_t power;     /* its charged shot's power attack, BN4's id (BN4_FORM_LOADOUTS +3) */
	uint32_t routine;  /* ... its routine moved with its family's unit (xchips_bn4_moved_at), 0 none */
	uint8_t charge[10];/* ... its charge times by his Charge (BN4_CHARGE_TIMES, BN6_CHARGE_ROW's shape) */
} SoulBn4;

/* BN6's k-th BN4 Soul (0 to SOULS_BN4 - 1) read: false where BN4's ROM is
 * not read, its rows are not the shape read, or its charged shot's code is
 * not moved (BN4's chips' units installed first: xchips_code.h) */
bool soulbn4_read(int k, SoulBn4 *out);
/* k of BN6's Soul number `soul`, -1 for none of BN4's */
int soulbn4_k(int soul);

#endif
