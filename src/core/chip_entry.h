/* chip_entry.h: a chip as BN6's folders, deck and hand hold it, one u16:
 * its id in the low CHIP_ID_BITS and its code above (0-25 A-Z, 26 *; 27
 * and 28 BN6's pseudo-chips'), 0xFFFF none (docs/rom/chips.md, Chip
 * folders). BN6 has nine bits, ids to 511; ten-bit chip ids (unit 1 of
 * docs/multirom/bn4-chips.md, Past 511) make it ten, the code from bit
 * 10, once the census of BN6's own code (tools/chip_ids.py,
 * docs/rom/chips-code.md, Ten-bit chip ids) is patched. What stays nine
 * bits whatever the folders hold: a battle's reward (its kind from bit
 * 14: xchips.h), and BN5's and BN4's own folders in their own games. */
#ifndef CHIP_ENTRY_H
#define CHIP_ENTRY_H

#include <stdbool.h>
#include <stdint.h>

#define CHIP_ID_BITS 10
#define CHIP_ID_MASK ((1u << CHIP_ID_BITS) - 1u)
#define CHIP_ENTRY_NONE 0xFFFFu

/* an entry's id; its code with whatever lies above it (none in a folder) */
static inline unsigned chip_entry_id(uint16_t e) { return e & CHIP_ID_MASK; }
static inline unsigned chip_entry_code(uint16_t e) { return (unsigned)e >> CHIP_ID_BITS; }
static inline uint16_t chip_entry(unsigned id, unsigned code) { return (uint16_t)(id | code << CHIP_ID_BITS); }
/* (the same as a constant, for a table's initializer) */
#define CHIP_ENTRY(id, code) ((uint16_t)((unsigned)(id) | (unsigned)(code) << CHIP_ID_BITS))

/* The nine-bit form: a reward's chip, BN5's and BN4's folders, and BN6's
 * folders as a run saved before ten bits holds them */
#define CHIP9_MASK 0x1FFu
static inline unsigned chip9_id(uint16_t e) { return e & CHIP9_MASK; }
static inline unsigned chip9_code(uint16_t e) { return (unsigned)e >> 9; }
static inline uint16_t chip9_entry(unsigned id, unsigned code) { return (uint16_t)(id | code << 9); }

/* A battle's reward (a reward row's entry, the results' u16; 0xFFFF none):
 * its kind in bits 14-15, 0 a chip in the nine-bit form, 1 zenny, 2 HP, 3
 * BugFrags, the amount in the low 14. A chip past 511 has the wide form:
 * kind 2 or 3 with bit 13, which no row's HP or BugFrags reach (BN6's rows
 * give 800 HP at most: docs/rom/chips-code.md, The pack past 511), the
 * kind its id's block (512-767, 768-1023), the id's low byte below and the
 * code from bit 8; a hook on BN6's split of a reward reads it
 * (xchips_wide.c). */
#define REWARD_NONE 0xFFFFu
#define REWARD_WIDE 0x2000u
enum { REWARD_CHIP, REWARD_ZENNY, REWARD_HP, REWARD_BUGFRAGS };
static inline bool reward_is_chip(uint16_t r) { return r != REWARD_NONE && (r >> 14 == 0 || (r >> 14 >= 2 && (r & REWARD_WIDE))); }
/* (a reward's kind, the wide form a chip's; its amount where it is none) */
static inline unsigned reward_kind(uint16_t r) { return reward_is_chip(r) ? (unsigned)REWARD_CHIP : (unsigned)r >> 14; }
static inline unsigned reward_amount(uint16_t r) { return r & 0x3FFFu; }
static inline unsigned reward_chip_id(uint16_t r) {
	return r >> 14 == 0 ? chip9_id(r) : (CHIP9_MASK + 1u) + (((unsigned)r >> 14) - 2u) * 256u + (r & 0xFFu);
}
static inline unsigned reward_chip_code(uint16_t r) { return (r >> 14 == 0 ? chip9_code(r) : (unsigned)r >> 8) & 0x1Fu; }
static inline uint16_t reward_chip(unsigned id, unsigned code) {
	if (id <= CHIP9_MASK) return chip9_entry(id, code);
	return (uint16_t)((2u + ((id - (CHIP9_MASK + 1u)) >> 8)) << 14 | REWARD_WIDE | code << 8 | (id & 0xFFu));
}

/* A nine-bit entry with its code moved to bit `bits` (9 or 10; 0xFFFF
 * none and 0 empty as they are): the converter a run saved with nine-bit
 * folders goes through once, and a spec's or a table's nine-bit entry
 * written into this build's game. A folder's entry has nothing at bit 15,
 * so nothing is lost. */
static inline uint16_t chip_entry_widen(uint16_t e, unsigned bits) {
	return e == CHIP_ENTRY_NONE ? e : (uint16_t)(chip9_id(e) | chip9_code(e) << bits);
}
static inline uint16_t chip_entry_from9(uint16_t e) { return chip_entry_widen(e, CHIP_ID_BITS); }

/* `n` nine-bit entries in this build's form, in place */
static inline void chip_entries_from9(uint16_t *e, int n) {
	for (int i = 0; i < n; ++i) e[i] = chip_entry_from9(e[i]);
}

#endif
