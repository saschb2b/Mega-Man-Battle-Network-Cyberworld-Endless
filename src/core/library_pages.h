/* The Library, a page per entry (docs/game/meta/library.md): BN6's chips,
 * BN5's and BN4's, each game's own Library in its own order, its standard
 * chips, Megas and Gigas a run can hand out, each slot filled by the run's
 * chip of its name where the profile's Library holds it (meta.h: one bit a
 * chip id, so a chip two games name fills its slot on each one's page).
 * BN6's slots are read from its records as the game runs; the others' are
 * library_pages_data.c's, written by tools/library_pages.py, so a page
 * keeps its slots and its count where its game's ROM is not read. */
#ifndef CW_LIBRARY_PAGES_H
#define CW_LIBRARY_PAGES_H

#include <stdbool.h>
#include <stdint.h>

enum { LIB_BN6, LIB_BN5, LIB_BN4, LIB_ENTRIES };
enum { LIB_STD, LIB_MEGA, LIB_GIGA, LIB_CLASSES };

/* A slot of a page: the game's own chip id, the run's chip that fills it
 * (the Library's bit), its number in its class as the game's Library
 * counts it, and its class (LIB_STD .. LIB_GIGA) */
typedef struct {
	uint16_t own, chip, number;
	uint8_t cls;
} LibSlot;

#define LIB_SLOTS_MOST 256   /* a class's slots at most (BN6's StdChip: 200) */

/* BN5's and BN4's slots, by class and then number */
extern const LibSlot library_bn5_rows[], library_bn4_rows[];
extern const int library_bn5_count, library_bn4_count;

/* `rows`' slots of class `cls` (-1 every class) whose chip `plays` takes,
 * in their order, into `out` (`most` at most; NULL to count): how many */
int library_page_of(const LibSlot *rows, int n, int cls, bool (*plays)(int chip), LibSlot *out, int most);
/* ... counted: `of` of them, `held` those whose chip `has` takes */
void library_tally(const LibSlot *rows, int n, int cls, bool (*plays)(int chip), bool (*has)(int chip), int *held, int *of);
/* `n` rows checked: by class and number, rising, no slot twice, no chip of
 * the game twice, every class one of the three, every chip an id of the
 * tables'. 0 where they hold, else the first row's index + 1 that does not */
int library_rows_check(const LibSlot *rows, int n);

/* Whether a run can hand out chip `chip` once its game's ROM is read: one
 * of BN6's the pool holds (chip_pool.h), or one of the registry's own
 * that plays (xchips_registry.h: over BN6's code or moved, BN5's rows and
 * BN4's alike); a chip that sits out until its code is moved, a DarkChip
 * and a Program Advance not */
bool library_plays(int chip);
/* Entry `entry`'s page of class `cls`: its slots a run can hand out, in
 * the game's order, into `out` (`most` at most; NULL to count): how many */
int library_page(int entry, int cls, LibSlot *out, int most);
/* ... counted against the profile's Library: `of` slots, `held` filled
 * (`cls` -1: the entry's three classes together) */
void library_count(int entry, int cls, int *held, int *of);
/* Whether a class of an entry is whole: it has slots, each filled */
bool library_whole(int entry, int cls);
/* Whether entry `entry`'s ROM is read (BN6's: the game's own) */
bool library_entry_read(int entry);

#endif
