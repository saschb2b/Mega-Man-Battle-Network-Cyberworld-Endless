/* BN6's battle engine given the room other games' virus families take
 * (issue #82, the engine's capacity; docs/ROM_DATA.md, The engine's room
 * for moved families; docs/EMULATION.md, the free-space map): its tables by
 * AI index, sprite lists, effects, the kinds' types, enemy ids and reward
 * rows grown past BN6's own, at fixed places past EMU_FREE (XV_*), the one
 * word that names each pointed at its copy; the names of the ids from
 * XV_FIRST_ID in an archive of their own, which a hook on the name lookup
 * (BN6_NAME_OF) hands out; the battle's sprite preload reading as many bits
 * of an id as its spawn does; the delete counts, by AI index, kept to BN6's
 * 32; and the second-level dispatch (xvirus_dispatch.h), its code, its
 * tables and its three types' entries in the grown type tables. Made once a
 * core, all or nothing, before any family's install (xvirus.c), which then
 * stages its entries into them. */
#ifndef CW_XVIRUS_GROW_H
#define CW_XVIRUS_GROW_H

#include <stdbool.h>
#include <stdint.h>

#include "xvirus_recipe.h"

/* The grown tables, by what a family's install names them */
enum {
	XG_FAMILY,                     /* BN6's 11 per-family tables: XG_FAMILY + k for table k, a word an AI index */
	XG_ICE = XG_FAMILY + XV_TABLES, /* the ice blocks' sizes, a byte an AI index */
	XG_AI_HOOKS,                   /* the three 35-word tables, a word an AI index: BN6_AI_HOOKS, */
	XG_AI_HOOKS2,                  /* ... BN6_AI_HOOKS2, */
	XG_HELD,                       /* ... BN6_HELD_CHIPS */
	XG_LIST,                       /* sprite lists 1, 2, 4 and 5: XG_LIST + xvirus_list_slot(list), a word a number */
	XG_EFFECTS = XG_LIST + XV_LISTS_GROWN,   /* the effects, a word an effect id */
	XG_T1,                         /* the first, third and fourth kinds' types, a word a type */
	XG_T3,
	XG_T4,
	XG_IDS,                        /* the enemy ids' triples, 3 bytes an id */
	XG_ROWS,                       /* the reward rows, 2 x ENEMY_DROPS bytes an id */
	XG_ARMS,                       /* the viruses' arm points' records, BN6_ARM_VIRUS_SIZE bytes a number of sprite list 1 */
	XG_TABLES
};

/* Who the families' room's and the families' bytes are (claims.h) */
#define XV_CLAIMS "the families and their room (xvirus.c, xvirus_grow.c)"

/* The names of the ids from XV_FIRST_ID (XV_NAMES): a text archive, entry
 * id - XV_FIRST_ID each, which holds this offset (an empty name) until a
 * family names it */
#define XG_NAME_NONE (2u * (XV_IDS - XV_FIRST_ID))

/* BN6's tables grown: true once they are, now or before; false, nothing
 * written, where a check failed (the log says which): a word of BN6's not
 * the one expected, an entry of a table not as the player's ROM has it, an
 * instruction not BN6's, the room past EMU_FREE not free. */
bool xg_grow(void);
/* Entry i of grown table t (XG_*) given a family's bytes (the table's
 * entry size), staged (xvirus_tx.h): the core's copy must still hold the
 * table's fill there, or BN6's own entry where BN6 has one (a placeholder
 * of a sprite list's, a record of the arm points'), no other family's. */
void xg_set(int t, uint32_t i, const uint8_t *entry);
void xg_set_word(int t, uint32_t i, uint32_t v);
/* A family's object type for the second-level dispatch (xvirus_dispatch.h),
 * staged: its game's type `from` of the kind (1, 3, 4) spawned as BN6's twin
 * `to`, or moved (to XV_TYPE_MOVED, its routine `routine` the moved copy's
 * address). Each entry must be blank in the core's copy, or set to the same
 * by a family before (a type two families' code spawns). */
void xg_type(int kind, uint8_t from, uint8_t to, uint32_t routine);
/* The names' archive's bytes in use in the core's copy: where a family's
 * names go on. */
uint32_t xg_names_end(void);
/* Entry i of one of BN6's own tables the room grows (XG_*, i one of BN6's
 * entries) changed in the copy the engine reads: its grown copy where the
 * tables are grown, else BN6's own table in place; it must hold `was`
 * (the table's entry size) there first, then it holds `now`, written as
 * the room's own bytes where they are (claims.h). The Souls' proxy form
 * gives form 8's object id so. False, nothing written, where it is not
 * `was` or is none of BN6's. */
bool xg_bn6_entry(int t, uint32_t i, const uint8_t *was, const uint8_t *now);

#endif
