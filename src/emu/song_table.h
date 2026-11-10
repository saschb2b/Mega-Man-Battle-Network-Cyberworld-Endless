/* BN6's song table grown (the owner's call 6 of docs/multirom/bn4-chips.md,
 * 11 November 2026; docs/rom/sound.md, The song table grown): BN6 has 474
 * songs and no empty slot left for other games' sounds, so the table is
 * copied into the free space (FREE_SONG_TABLE) with SONG_GROWN_SLOTS more
 * entries, each BN6's empty song, and the sound engine's five words that
 * name it are pointed at the copy. A song's number from SONG_GROWN_FIRST is
 * one of the new slots. Grown where BN4's ROM is read (its chips' sounds:
 * xchips_code.h), once a core; until then, and without it, BN6's own table
 * is the live one. Every reader and writer of an entry in the core goes
 * through song_entry_bus. */
#ifndef CW_SONG_TABLE_H
#define CW_SONG_TABLE_H

#include <stdbool.h>
#include <stdint.h>

#include "bn6_sound.h"
#include "free_space.h"

#define SONG_GROWN_FIRST BN6_SONG_COUNT    /* the first new slot's number, */
#define SONG_GROWN_SLOTS ((FREE_SONG_TABLE_END - FREE_SONG_TABLE) / BN6_SONG_ROW - BN6_SONG_COUNT)   /* ... and how many (550) */
#define SONG_GROWN_END   (SONG_GROWN_FIRST + SONG_GROWN_SLOTS)
#define SONG_TABLE_CLAIMS "BN6's song table grown (song_table.c)"

/* Song `slot`'s entry in the live table (the grown copy once the engine's
 * words name it, else BN6's own), BN6_SONG_ROW bytes: its header, its
 * player; 0 for a slot it does not hold */
uint32_t song_entry_bus(int slot);
/* Whether `slot` is one the live table holds */
bool song_slot_ok(int slot);
/* What a slot holds before anything is copied into it, as two words: BN6's
 * entry in the player's ROM, or for a grown slot BN6's empty song's */
void song_entry_empty(int slot, uint32_t out[2]);
/* The table grown (again it does nothing): false where the engine's words
 * do not name BN6's table, nothing written */
bool song_table_grow(void);

#endif
