/* Another game's chips as BN6's, by name (docs/MULTIROM.md, Guest
 * battles): the tables the guest's translation runs on, worked out from
 * the names and codes it is handed (guest.c reads them from both ROMs), so
 * the tests run it on made-up ones. */
#ifndef CW_XCHIPS_H
#define CW_XCHIPS_H

#include <stdint.h>

#define XCHIP_NAME 20   /* a name's room, as ChipInfo's */

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
