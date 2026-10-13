/* BN6's attack families grown (issue #82; docs/ROM_DATA.md, The attack
 * families and chip routines grown; docs/EMULATION.md, its free space):
 * the battle runs an attack by its CurAction from the families' table
 * (BN6_ATTACK_FAMILIES, a word per family 0x10-0x5E), and CurAction is a
 * byte, so the table grows to every family a byte can name, 0x10-0xFF, at
 * FREE_ATTACK_FAMILIES, its two words pointed there. BN6's 79 entries are
 * copied as the player's ROM has them; every other is XATTACK_STUB, which
 * ends the attack as BN6 ends one (object_exitAttackState), so a CurAction
 * no family holds ends safely. One growth for every track that adds
 * families (the numbers below), each of which sets its own entries where
 * they still hold the stub. */
#ifndef CW_XATTACK_GROW_H
#define CW_XATTACK_GROW_H

#include <stdbool.h>
#include <stdint.h>

#include "free_space.h"

#define XATTACK_TABLE  FREE_ATTACK_FAMILIES             /* the grown table: a Thumb routine per family, from */
#define XATTACK_FIRST  0x10                             /* ... family 0x10 (BN6_ATTACK_FAMILY_FIRST) */
#define XATTACK_COUNT  240                              /* ... to 0xFF */
#define XATTACK_STUB   (FREE_ATTACK_FAMILIES + 0x3C0u)  /* the stub every entry no family holds names (8 bytes, the project's own Thumb) */
/* The families' numbers by track: BN6's own, then the chips' moved from
 * BN5 (xchips_code.c), then the Souls', the rest spare */
#define XATTACK_BN6_END   0x5F
#define XATTACK_CHIPS     0x60   /* ... to 0xBF */
#define XATTACK_CHIPS_END 0xC0
#define XATTACK_SOULS     0xC0   /* ... to 0xCF */
#define XATTACK_SOULS_END 0xD0

/* Who the growth's bytes are (claims.h) */
#define XATTACK_CLAIMS "the attack families grown (xattack_grow.c)"

/* The table grown, once a core (again it does nothing): true once both
 * words name the copy, now or before; false, nothing written, where a word
 * or an entry of BN6's is not the player's ROM's or the place is not free. */
bool xattack_grow(void);
/* The bus address of family `family`'s entry in the grown table, 0 for one
 * outside 0x10-0xFF. */
uint32_t xattack_entry(int family);
/* Family `family` (past BN6's own, XATTACK_BN6_END on) run as `routine` (a
 * Thumb address), outside a staged step (the Souls' install; the chips'
 * stage theirs through xattack_entry): true where its entry held the stub
 * or `routine` already; false, nothing written, where the table is not
 * grown or the entry is another's. */
bool xattack_set(int family, uint32_t routine);

#endif
