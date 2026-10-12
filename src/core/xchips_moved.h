/* The chips whose code moved from BN5's into BN6's battle engine (issue
 * #82's C3; docs/MULTIROM.md, The chips' code): the registry's XREG_MOVED
 * rows, each with the family and subfamily its record takes in BN6's
 * tables (a moved attack family's number, src/emu/xattack_grow.h, its
 * subfamily BN5's; or the support or Navi family with the moved routine's
 * entry, src/emu/xchips_code.h); and the computed powers whose routines
 * moved. Numbers alone, written by
 * tools/xchiprecipe.py with src/emu/xchips_bn5.c, whose units they run. */
#ifndef CW_XCHIPS_MOVED_H
#define CW_XCHIPS_MOVED_H

#include <stdint.h>

typedef struct {
	uint16_t bn6;          /* its id in BN6's tables (the registry's own) */
	uint8_t family, sub;   /* what its record takes */
} XChipMoved;

extern const XChipMoved xchips_moved[];
extern const int xchips_moved_count;

/* The moved chip of BN6's id `bn6`, NULL for one whose code did not move */
const XChipMoved *xchips_moved_of(int bn6);

/* A computed power whose routine moved (a chip's power of 1000 and up
 * names its routine by power - 1000 in its game's table; BN6's grown,
 * src/emu/xchips_code.h, XC_POWERS): BN5's entry and BN6's */
typedef struct {
	uint8_t bn5, bn6;
} XChipPower;

extern const XChipPower xchips_powers[];
extern const int xchips_powers_count;

/* BN6's power for BN5's computed power `bn5_power` (1000 + its entry):
 * 1000 + BN6's entry where its routine moved, else 0 */
int xchips_power_of(int bn5_power);

#endif
