/* A registered chip's record in BN6's layout (bn6f ChipData, 0x2C bytes;
 * docs/rom/chips-code.md, The chip registry), made from BN5's record field by
 * field, or a placeholder's where BN5's ROM is not read; and the small
 * pieces of what the chips' tables need beside the records (the folder
 * editor's ABC place, the icons' colours, the battle deck with the chips
 * that sit out taken out). Pure: the tests run them on made-up records. */
#ifndef CW_XCHIPS_RECORD_H
#define CW_XCHIPS_RECORD_H

#include <stdbool.h>
#include <stdint.h>

#include "bn6.h"
#include "xchips_moved.h"
#include "xchips_registry.h"

#define XREC_SIZE 0x2C   /* a chip record (BN6_CHIP_RECORD_SIZE; BN5's the same, its fields named as BN6's in bn6.h) */
/* The behaviour a chip that sits out keeps, where it ran after all: BN6's
 * blank records' (family 0x1C, subfamily 3, as BN6's records 291-300) */
#define XREC_INERT_FAMILY 0x1C
#define XREC_INERT_SUB 0x03

/* BN5's icon element (its records' +6) as BN6's: the elements alike,
 * Recovery, Invisible and Null as Null, Plus, Sword, Cursor, Obstacle,
 * Wind, Break and a Program Advance one on (BN6 has no Recovery or
 * Invisible: their chips there show Null, as BN6's Recov10 and Invisibl
 * of the same names do) */
int xrec_icon_element(int bn5);

/* What a record of its own is made of */
typedef struct {
	const uint8_t *bn5;    /* BN5's record */
	const uint8_t *twin;   /* (XREG_RECORD, XREG_DARK, XREG_PA with one) BN6's record of the chip whose family it runs as, else NULL */
	const XChipReg *reg;
	uint16_t abc;          /* its place in the folder editor's ABC order (xrec_abc_place) */
	uint32_t art[3];       /* its icon's, picture's and picture palette's bus addresses in the core's copy */
	const XChipMoved *moved;   /* (XREG_MOVED) its code moved from BN5's: the family and subfamily its record takes there, else NULL */
} XRecordFrom;

/* BN5's record as BN6's: BN5's numbers (codes, elements, rarity, type, MB,
 * effect flags but the Library's, stamina, parameters, lockout, power,
 * gate), BN6's icon element, the twin's family, lock-on and lock-on type
 * (and subfamily, but the row's), out of BN6's Library screen (library
 * number and flags 0: docs/multirom/one-net.md, the Library call), its own id its
 * ID sort key, no DarkChipID. A chip whose code moved (XREG_MOVED) takes
 * its moved family and subfamily and BN5's parameters, no lock-on (BN5's
 * code knows none). A chip that sits out for want of its code keeps an
 * inert family, no lock-on and no parameters. A power BN5 computes (1000
 * and up, by BN5's own table) is a playing chip's (a twin's or a moved
 * one's) BN6 entry where a helper moved its routine (xchips_power_of), else
 * not BN6's to show: 0, unshown. */
void xrec_from_bn5(const XRecordFrom *in, uint8_t out[XREC_SIZE]);
/* BN6's own record of a chip BN5's of its name plays (a SAME row whose code
 * moved, tools/xchiprecipe.py's SAME_MOVED: DarkPlus), made BN5's chip in
 * what it does where BN5's ROM is read: the moved family and subfamily,
 * BN5's attack element, effect flags (but the Library's, BN6's), the
 * damage's upper half (+0xA) and the bytes after the subfamily, its
 * parameters and lockout, no lock-on and no DarkChipID (BN5's dark rule,
 * not BN6's BugFrag); BN6's codes, rarity, icon, MB, power, Library and
 * sort places and art kept. `bn5` BN5's record, `out` BN6's, changed. */
void xrec_same_moved(const uint8_t bn5[XREC_SIZE], const XChipMoved *moved, uint8_t out[XREC_SIZE]);
/* The placeholder where BN5's ROM is not read: the row's codes, type and
 * MB, a DarkChip's class, no power, Null, inert, BN6's blank art
 * (`blank`: icon, picture, palette). */
void xrec_placeholder(const XChipReg *reg, uint16_t abc, const uint32_t blank[3], uint8_t out[XREC_SIZE]);

/* The folder editor's ABC place for a chip named `name`: the place of the
 * chip of `n` whose name comes last before it (case aside; a tie the editor
 * breaks by id), skipping places of 0 (none: DarkChips, pseudo-chips); 1
 * where none comes before. As darkbn6.c places BN6's DarkChips. */
uint16_t xrec_abc_place(const char *name, const char *const *names, const uint16_t *places, int n);

/* BN5's icons in BN6's icons' palette: its colours 0-9 as they are (the
 * same in both, but 9, the outline, BN6's teal where BN5's is grey, as every
 * icon of BN6's has it), any of 10-15 as BN6's nearest of 1-9 */
void xrec_icon_colours(const uint8_t bn5_pal[32], const uint8_t bn6_pal[32], uint8_t map[16]);
void xrec_icon_recolour(uint8_t icon[0x80], const uint8_t map[16]);

/* The battle deck (`n` u16 entries, chip | code << 9, 0xFFFF after the
 * chips) with the chips whose bit `sits` has (a bit per id, 512) taken out,
 * the rest in their order, 0xFFFF after them; how many were taken, and in
 * `first` a bit for each of the deck's first `keep` entries (the Regular
 * chip's and the TagChips' places, 8 at most) taken. */
int xrec_deck_sit_out(uint16_t *deck, int n, const uint8_t sits[64], int keep, unsigned *first);

#endif
