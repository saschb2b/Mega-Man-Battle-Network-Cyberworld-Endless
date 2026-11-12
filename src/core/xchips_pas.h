/* BN5's Program Advances in BN6's list (issue #82's final engine round;
 * docs/multirom/chip-registry.md; docs/rom/chips-code.md, BN5's Program
 * Advances): the recipes BN5's Custom screen forms its registered ones by
 * (its list 0x08028020), in BN6's chip ids, which the chips' code appends
 * to BN6's own 43 in a list of its own (src/emu/xchips_code.c), BN6's first
 * where two recipes are the same; and BN6's own CrosOver from the run's
 * Django chips, BN5's (BN6's Django 1-3 are cut: docs/multirom/bn6-gaps.md).
 * A recipe is BN6's and BN5's layout: n
 * parts, its kind (0: n of one chip, codes in a row, '*' standing in once;
 * 4: these ids in this order), the PA, then its part (kind 0) or parts.
 * Numbers alone, written by tools/xchiprecipe.py. */
#ifndef CW_XCHIPS_PAS_H
#define CW_XCHIPS_PAS_H

#include <stdint.h>

#define XCHIP_PA_PARTS 4   /* a recipe's parts at most (BN6's MstrCros's four) */

typedef struct {
	uint32_t bn5;                      /* BN5's entry (0x08027DE4 on); BN6's for its own recipe (0x0802BAD0, CrosOver's) */
	uint8_t n, kind;                   /* its parts' count and kind (0 or 4) */
	uint16_t pa;                       /* the Program Advance, BN6's id (the registry's own) */
	uint16_t parts[XCHIP_PA_PARTS];    /* its part (kind 0) or parts, BN6's ids */
} XChipPa;

extern const XChipPa xchips_pas[];
extern const int xchips_pas_count;
/* BN4 Red Sun's (docs/multirom/bn4-chips.md, The Program Advances): the recipes its Custom screen forms the registry's
 * rows by (BN4_PA_LIST; `bn5` its entry there), those whose code is moved and whose parts all play, BN4's listed kind
 * BN6's 4; listed after BN5's where BN4's chips' code is in. Written by tools/xreg_bn4.py (xchips_pas_bn4.c) */
extern const XChipPa xchips_pas4[];
extern const int xchips_pas4_count;

#endif
