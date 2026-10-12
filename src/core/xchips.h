/* Another game's chips as BN6's, by name (docs/MULTIROM.md, Guest
 * battles): the tables the translation runs on, worked out from the names
 * and codes it is handed, so the tests run it on made-up ones; and the
 * run's chips paired with BN5's, read from both ROMs as the game runs, on
 * every build (none where BN5's ROM is not read). */
#ifndef CW_XCHIPS_H
#define CW_XCHIPS_H

#include <stdint.h>

#define XCHIP_NAME 20   /* a name's room, as ChipInfo's */
#define XCHIPS_BN6 314           /* BN6's standard, Mega and Giga chips, those a folder holds (ids 1-313) */
#define XCHIPS_BN6_STANDARD 203  /* ... its standard ones, 1-202 */

/* BN6's chips paired with BN5's of the same name, the first time both ROMs
 * are read (BN5's names in BN6's text codes, rom.h's xrom_text_bn6): how
 * many of BN6's paired, 0 without BN5's ROM */
int xchips_paired(void);
/* BN5's chip of BN6's chip `id`'s name, 0 none */
int xchips_to_bn5(int id);
/* BN6's chip of BN5's chip `x`'s name, 0 none (its DarkChips) */
int xchips_bn6_of(int x);
/* BN6's standard chips whose namesake in BN5 is of kind `kind` (its chip
 * records' BN5_CHIP_KIND), their ids into `out`, in BN6's order; how many */
int xchips_kind(int kind, uint16_t *out, int max);
/* The chips of `folder` (30 BN6 entries) BN5 has none of by name: each
 * once, in the folder's order, the first `max` into `out` (BN6 ids); how
 * many there are, 0 without BN5's ROM */
int xchips_sitting_out(const uint16_t *folder, uint16_t *out, int max);
/* A reward entry of BN5's (its rows' and its Mystery Data's: BN6's
 * encoding, bits 14-15 0 a chip with its code << 9, 1 zenny, 2 HP, 3
 * BugFrags; 0xFFFF none) as BN6's: a chip as BN6's of its name, in its
 * code where BN6's has it, else its *, else its first (xchips_code);
 * XCHIPS_STAND_IN zenny where BN6 has none of its name (until BN5's chips
 * are registered in BN6, issue #82); the others as they are */
#define XCHIPS_STAND_IN 200
uint16_t xchips_reward_bn6(uint16_t v);

/* Pairs BN6's chips 1 to n6 - 1 with the other game's 1 to nx - 1 whose
 * names match (names6[i] and namesx[j], "" for none): to_x[i] the other
 * game's chip of BN6's chip i's name (its first), from_x[j] BN6's chip of
 * the other game's chip j's name (its first), 0 for none; how many of
 * BN6's chips paired. */
int xchips_pair(const char (*names6)[XCHIP_NAME], int n6, const char (*namesx)[XCHIP_NAME], int nx, uint16_t *to_x, uint16_t *from_x);

/* The code a chip whose record holds `codes` (its four: 0-25 A to Z, 26 *,
 * 0xFF none) takes for `code`, crossing between the games: `code` where
 * the record has it, else its *, else its first; `code` where it has none.
 * A chip in a code its record lacks is drawn as nothing in BN5's Custom
 * screen. */
int xchips_code(const uint8_t codes[4], int code);

/* The chips of a folder (n BN6 entries, chip | code << 9; 0 and 0xFFFF
 * empty) that sit out where to_x (n6 long) pairs none: each chip once, in
 * the folder's order, the first `max` into `out`; how many there are. */
int xchips_out(const uint16_t *folder, int n, const uint16_t *to_x, int n6, uint16_t *out, int max);

/* A reward in one of the folder's codes (folder: three, 1 + the letter, 0
 * none) where both games' records of the chip have it (bn6 and other: four
 * codes each, as xchips_code's), on every other `row` of a virus's rows (a
 * reward's row comes of the busting level): about half the chips a battle
 * gives, as BN6's rewards lean to the folder; `code` itself elsewhere. */
int xchips_fit(const uint8_t other[4], const uint8_t bn6[4], const uint8_t folder[3], int code, int row);

#endif
