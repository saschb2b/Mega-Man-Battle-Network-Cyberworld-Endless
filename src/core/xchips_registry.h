/* The chip registry (issue #82, docs/MULTIROM.md, One net: the design):
 * the chips of Battle Network 5: Team Colonel a run can meet, each with
 * its id in BN6's tables, fixed here so a saved folder, pack or Library
 * names the same chip at every boot, with BN5's ROM or without it.
 *
 * Numbers only (AGENTS.md): BN5's chip id, BN6's, how the chip comes in,
 * and for a chip of an id of its own the few numbers its placeholder
 * keeps where BN5's ROM is not read (its codes, which the pack counts by,
 * its library type and MB, which the folder's rules read). Its name, text,
 * art and other numbers are read from the player's BN5 ROM as the core
 * starts (src/core/chips.c). */
#ifndef CW_XCHIPS_REGISTRY_H
#define CW_XCHIPS_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* How a registered chip comes into BN6's tables */
enum {
	XREG_SAME,      /* BN6 has a chip of its name: BN6's id, BN6's record (one chip per name) */
	XREG_ALIAS,     /* BN6 has it under its US release's other name (Attck+10 is Atk+10): BN6's id and record */
	XREG_RECORD,    /* an id of its own, its record over a BN6 chip's family (`twin`): it plays now, where BN5's ROM is read */
	XREG_DARK,      /* a DarkChip of its own: its record ready, it sits out until BN5's dark rule comes into BN6's battles */
	XREG_PENDING,   /* an id of its own whose code BN6 lacks: its record shown, it sits out until its code is moved */
	XREG_PA,        /* a Program Advance of its own: its record, formed once BN6's lists hold its recipes */
	XREG_MOVED,     /* an id of its own whose code BN6 lacks, moved from BN5's into BN6's engine (src/core/xchips_moved.h: its family and
	                   subfamily there): it plays where BN5's ROM is read */
};

typedef struct {
	uint16_t bn5, bn6;   /* BN5's chip id; BN6's id for it */
	uint8_t how;         /* XREG_* */
	uint16_t twin;       /* (XREG_RECORD, XREG_DARK, XREG_PA) the BN6 chip whose family, lock-on and lock-on type its record takes; 0 none */
	uint8_t sub;         /* ... and the subfamily, 0xFF the twin's */
	uint8_t codes[4];    /* (an id of its own) its codes as BN5's record has them (0-25 A-Z, 26 *, 0xFF none) */
	uint8_t type, mb;    /* ... its library type (0 standard, 1 Mega, 2 Giga, 4 a Program Advance) and its MB */
} XChipReg;

#define XREG_IDS 512   /* BN6's chip ids: nine bits */
extern const XChipReg xreg_rows[];
extern const int xreg_count;

/* Whether row `r` has an id of its own (a record of the registry's in
 * BN6's tables), not one of BN6's chips */
bool xreg_own(const XChipReg *r);
/* The row of BN5's chip `bn5`, NULL where it is not registered */
const XChipReg *xreg_of_bn5(int bn5);
/* The row whose own id is `bn6`, NULL where BN6's id `bn6` is BN6's own */
const XChipReg *xreg_at(int bn6);

/* The ids a row of its own may take: BN6's records no chip of the game
 * uses (blank-named, named by none of its tables, docs/ROM_DATA.md) and
 * 411-511, past its 411 records. */
bool xreg_free_id(int bn6);

/* `n` rows checked: every BN5 id once, every own id once and free, every
 * BN6 id of XREG_SAME and XREG_ALIAS a chip of BN6's folders (1-313) or
 * Program Advances (320-349), every twin one of BN6's 411 records. 0 where
 * they hold, else the first row's index + 1 that does not, `why` saying
 * what. */
int xreg_check(const XChipReg *rows, int n, char *why, size_t len);

#endif
