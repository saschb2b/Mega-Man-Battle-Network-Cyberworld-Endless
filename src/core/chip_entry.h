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

#include <stdint.h>

#define CHIP_ID_BITS 9
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
