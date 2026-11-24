/* Another game's NEW GAME folder as a run plays it (docs/game/meta/forms.md,
 * Each game's start): BN5 Team Colonel's or BN4 Red Sun's, the start a
 * SOUL BN5 or SOUL BN4 run brings where the setup's FOLDER row says
 * Standard. Read from that game's ROM as the run starts, where its
 * GiveFolder finds folder 0 (BN5_FOLDER_TABLE's first, BN4_FOLDERS'
 * first; docs/rom/chips.md, Other games' folders), never a copy of ours. */
#ifndef CW_XCHIPS_FOLDER_H
#define CW_XCHIPS_FOLDER_H

#include <stdbool.h>
#include <stdint.h>

#define XFOLDER_ENTRIES 30   /* a folder's chips, BN6's BN6_FOLDER_ENTRIES */

/* The code a chip in `code` takes in the run, its record's four codes
 * `codes` (0-25 A-Z, 26 *, 0xFF none): `code` where the record has it;
 * else the nearest letter it has (the earlier on a tie: BN4's Vulcan1 V
 * is S, its record B D S *); else * where it has *, as AirShot, whose
 * record has * alone; else `code`. Never a silent swap: the folder's
 * line names each (meta_text.c, meta_standard_about). */
int xfolder_code(const uint8_t codes[4], int code);

/* The pure part, which the tests run on made-up chips: `in`, the other
 * game's 30 entries in its nine-bit form (id | code << 9), into `out` as
 * the run's (chip_entry.h), `run_of` the run's chip of the other game's id
 * (0 none) and `codes_of` its record's four codes in the run; how many
 * entries changed their code into `swapped` (NULL to skip). False where an
 * entry has no chip in the run: the run then keeps BN6's own Standard. */
bool xfolder_as_run(const uint16_t in[XFOLDER_ENTRIES], int (*run_of)(int), void (*codes_of)(int, uint8_t[4]),
	uint16_t out[XFOLDER_ENTRIES], int *swapped);

/* The NEW GAME folder of game `game` (XROM_BN5_COLONEL_US or
 * XROM_BN4_RED_SUN_US, rom.h) as the run's into `out`, how many entries
 * changed their code into `swapped` (NULL to skip); false for another
 * game, where its ROM is not read, or where a chip of it has no run chip
 * (the run keeps BN6's own Standard) */
bool xfolder_start(int game, uint16_t out[XFOLDER_ENTRIES], int *swapped);

#endif
