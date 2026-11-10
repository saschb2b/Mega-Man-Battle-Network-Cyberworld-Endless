/* The chip registry (issue #82, docs/multirom/one-net.md):
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

#define XREG_IDS 1024   /* BN6's chip ids: ten bits (chip_entry.h), 512-1023 free */
extern const XChipReg xreg_rows[];
extern const int xreg_count;

/* Whether row `r` has an id of its own (a record of the registry's in
 * BN6's tables), not one of BN6's chips */
bool xreg_own(const XChipReg *r);
/* The row of BN5's chip `bn5`, NULL where it is not registered */
const XChipReg *xreg_of_bn5(int bn5);
/* The row whose own id is `bn6`, NULL where BN6's id `bn6` is BN6's own */
const XChipReg *xreg_at(int bn6);
/* The row of BN5's chip paired by name with BN6's own chip `bn6` (XREG_SAME
 * or XREG_ALIAS: the first, where two names share it), NULL where none is */
const XChipReg *xreg_named(int bn6);

/* The ids a row of its own may take: BN6's records no chip of the game
 * uses (blank-named, named by none of its tables, docs/rom/) and
 * 411-510, past its 411 records, and those from 512; 511 held, no chip's:
 * the id whose flag of BN6's Library a chip past 511 sets as it is given
 * (xchips_wide.c; docs/rom/chips-code.md, The pack past 511). */
#define XREG_HELD 511
bool xreg_free_id(int bn6);

/* `n` rows checked: every BN5 id once, every own id once and free, every
 * BN6 id of XREG_SAME and XREG_ALIAS a chip of BN6's folders (1-313) or
 * Program Advances (320-349), every twin one of BN6's 411 records. 0 where
 * they hold, else the first row's index + 1 that does not, `why` saying
 * what. */
int xreg_check(const XChipReg *rows, int n, char *why, size_t len);

/* ---- BN4 Red Sun's rows (docs/multirom/bn4-chips.md, The build plan's unit 2; written by tools/xreg_bn4.py) ----
 * The chips of Battle Network 4 a run has none of, by name or by Navi and
 * version: each with its id in BN6's tables, fixed as BN5's are. A table of
 * their own, so that what walks BN5's rows meets none of them. */
typedef struct {
	uint16_t bn4, bn6;   /* BN4's chip id; its id in BN6's tables */
	uint8_t how;         /* XREG_PENDING: its code is not in BN6's engine yet, it sits out; XREG_PA: a Program Advance, unformed until its recipe
	                        is listed; XREG_RECORD: its record over a BN6 chip's family (`twin`), BN6's own code: it plays where BN4's ROM is read;
	                        XREG_MOVED: its code moved into BN6's engine (src/emu/xchips_bn4.h's units): it plays where BN4's ROM is read */
	uint16_t twin;       /* (XREG_RECORD) the BN6 chip whose family, lock-on and lock-on type its record takes; 0 none; (XREG_MOVED) the BN6
	                        chip whose family alone its record takes, 0 BN6's support or misc family (a Navi chip's: BN6's Roll, the Navi family) */
	uint8_t sub;         /* ... and the subfamily, 0xFF the twin's; (XREG_MOVED) its unit's routine in BN6's grown support routines,
	                        XC4_FIRST_SUPPORT + BN4's subfamily, its record's subfamily under BN6's support family (a Navi chip's: in BN6's
	                        grown Navi routines, under its Navi family) */
	uint8_t codes[4];    /* its codes as BN4's record has them */
	uint8_t type, mb;    /* its class (BN6's library type: 0 standard, 1 Mega, 2 Giga, 4 a Program Advance) and its MB */
} XChipReg4;

#define XREG4_PACK_FIRST 527   /* its first id: 527-602, the ids with a pack entry (bn6_pack_entry), its chips that play first; */
#define XREG4_PA_FIRST 605     /* 605-634 its Program Advances, BN6's bits for one formed the engine's there (after BN5's two from BN6_PA_WIDE_FIRST); */
#define XREG4_REST_FIRST 635   /* 635 on the rest of its folder chips */
extern const XChipReg4 xreg4_rows[];
extern const int xreg4_count;
/* BN4's chips that are a chip of BN6's under another spelling (LifeAura, BN6's LifeAur): BN4's id and BN6's, no row */
extern const uint16_t xreg4_alias[][2];
extern const int xreg4_alias_count;
/* BN4's dark chips among the rows, by their id in BN6's tables (docs/game/meta/darkchips.md, the owner's call of 10 November
 * 2026): a record BN4 marks dark (its alignment, BN4_CHIP_ALIGN: played only for a dark MegaMan there) or a DarkChip
 * (BN4_CHIP_DARK); a battle one is used in costs what every dark chip's does */
extern const uint16_t xreg4_dark_ids[];
extern const int xreg4_dark_count;

/* The row of BN4's chip `bn4`, NULL where it has none (the run's chip by
 * name or Navi: xchips_bn4_paid; or none at all) */
const XChipReg4 *xreg4_of(int bn4);
/* The row whose id is `bn6`, NULL where that id is none of BN4's */
const XChipReg4 *xreg4_at(int bn6);
/* Whether id `bn6` is a dark chip of BN4's (xreg4_dark_ids) */
bool xreg4_dark(int bn6);
/* `n` rows checked against BN5's rows `rows5` (`n5`): every BN4 id once,
 * every id once, free (xreg_free_id), none of BN5's rows' and from
 * XREG4_PACK_FIRST; a folder chip's with a pack entry or from
 * XREG4_REST_FIRST, a Program Advance's in XREG4_PA_FIRST to
 * XREG4_REST_FIRST - 1. 0 where they hold, else the first row's index + 1
 * that does not, `why` saying what. */
int xreg4_check(const XChipReg4 *rows, int n, const XChipReg *rows5, int n5, char *why, size_t len);

#endif
