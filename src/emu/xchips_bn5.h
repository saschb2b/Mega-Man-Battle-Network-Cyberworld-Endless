/* BN5's chips' code as units moved into BN6's battle engine (issue #82's
 * C3; docs/MULTIROM.md, The chips' code): a unit is one attack family of
 * BN5's (the routine a chip's CurAction runs), or one support or Navi
 * routine (the routine BN6's support family, or its Navi object, runs by a
 * chip's subfamily), with every routine and object it reaches that BN6 has
 * no twin of. Numbers alone, written by tools/xchiprecipe.py from the two
 * ROMs into xchips_bn5.c; src/emu/xchips_code.c installs them. */
#ifndef CW_XCHIPS_BN5_H
#define CW_XCHIPS_BN5_H

#include <stdint.h>

#include "xvirus_recipe.h"

/* What a unit is in BN6's tables */
enum {
	XCHIP_ATTACK,    /* an attack family: its entry of the grown attack families (xattack_grow.h), a chip's family */
	XCHIP_SUPPORT,   /* a support routine: its entry of the grown support routines, a chip's subfamily under family 0x15 */
	XCHIP_NAVI,      /* a Navi chip's routine: its entry of the grown Navi routines, a chip's subfamily under family 0x1B */
};

typedef struct {
	uint8_t kind;          /* XCHIP_* */
	uint8_t bn5;           /* BN5's family (an attack) or subfamily (a support or Navi routine) */
	uint8_t number;        /* BN6's: its family (XATTACK_CHIPS on), or its entry in the grown routines' table */
	uint32_t entry;        /* the routine the table names, BN5's address (Thumb): moved with the code */
	uint32_t home;         /* its veneers' place (XC_HOMES, XC_HOME_SIZE each) */
	XVirusFamily code;     /* its code alone: blocks, data, calls, words, immediates, numbers, the objects it spawns, what it draws */
} XChipUnit;

extern const XChipUnit xchips_bn5[];
extern const int xchips_bn5_count;

#endif
