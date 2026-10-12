/* BN6's attack families grown once a core (issue #82; docs/ROM_DATA.md,
 * BN6's attack tables grown; docs/EMULATION.md, the free-space map): the
 * table of routines by CurAction (bn6_attacks.h), 79 entries for BN6's
 * 0x10-0x5E, copied to FREE_ATTACK_FAMILIES with room for every CurAction
 * a byte holds (0x10-0xFF), its two words pointed at the copy, every entry
 * past BN6's the routine that ends an attack at once. One numbering for
 * every track that grows it: BN6's own 0x10-0x5E as they are, BN5's
 * chips' families 0x60-0xBF, the Souls' 0xC0-0xCF, 0xD0-0xFF spare. */
#ifndef CW_XATTACK_GROW_H
#define CW_XATTACK_GROW_H

#include <stdbool.h>
#include <stdint.h>

#include "free_space.h"

#define XATTACK_CLAIMS "BN6's attack families grown (xattack_grow.c)"   /* whose bytes they are (claims.h) */
#define XATTACK_LAST   0xFF          /* the last CurAction the grown table holds */
#define XATTACK_ENTRIES (XATTACK_LAST + 1 - 0x10)   /* its entries: 240 */
#define XATTACK_CHIPS  0x60          /* BN5's chips' families from here, */
#define XATTACK_SOULS  0xC0          /* ... the Souls' from here, */
#define XATTACK_SPARE  0xD0          /* ... spare from here */
#define XATTACK_EMPTY  (FREE_ATTACK_FAMILIES + 4u * XATTACK_ENTRIES)   /* the routine of an empty entry: a tail call of BN6_EXIT_ATTACK */

/* The table grown in the core's copy: true once it is (now, or by another
 * installer before: its words already the copy's); false, nothing written,
 * where a word or an entry of BN6's is not as the player's ROM has it. */
bool xattack_grow(void);
/* CurAction `action` (past BN6's own, BN6_ATTACK_FAMILY_FIRST + BN6_ATTACK_FAMILY_COUNT, to XATTACK_LAST) run
 * as `routine` (a Thumb address): true where its entry was empty or held
 * `routine` already; false, nothing written, where the table is not grown
 * or the entry is another's. */
bool xattack_set(int action, uint32_t routine);

#endif
