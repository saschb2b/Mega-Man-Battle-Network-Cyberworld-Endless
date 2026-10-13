/* Virus families moved into BN6's own battle engine (issue #82; docs/
 * MULTIROM.md, Transplanted families; docs/EMULATION.md, its free space):
 * a family's own code copied whole from its game's ROM into the core's copy
 * past EMU_FREE, its calls out aimed again at BN6's routines through
 * veneers, the pointers in it moved, a state table laid out BN6's way, and
 * its entries in BN6's tables, grown past BN6's own once a core so every
 * family has room at fixed numbers (xvirus_grow.h): an AI index from 32,
 * six enemy ids from 0x200, its names, records, sprites, effects, attacks
 * and reward rows.
 *
 * A family is a recipe of numbers alone (xvirus_families.c): addresses,
 * ids, counts and the CRC32s of the ranges it copies. The code and the
 * records are the player's ROMs', copied as the core starts, and nothing of
 * them is in the repository. The install is all or nothing per family: a
 * source range whose CRC32 is not the recipe's, a word of BN6's that is not
 * the one expected, an instruction of a form not expected, a call or a
 * pointer the recipe does not name, and nothing is written. */
#ifndef CW_XVIRUS_H
#define CW_XVIRUS_H

#include <stdbool.h>
#include <stdint.h>

#include "xvirus_recipe.h"

/* What xvirus_install puts in */
enum {
	XVIRUS_FAMILIES,   /* the families whose game's ROM is read: BN5's CanGard */
	XVIRUS_CONTROL,    /* --dev xcontrol: the control instead, BN6's own Mettaur (AI 1) moved through the same steps as AI 32,
	                      which must play frame for frame as Mettaur does (docs/MULTIROM.md, Transplanted families) */
	XVIRUS_NONE,       /* --dev nofamilies: none (BN6's battles as the player's ROM has them, to compare) */
};
/* The families a run may meet (XVIRUS_*), installed into the core's ROM
 * copy once a core, as it starts (patches are not in a state: the same at
 * every boot with the same ROMs), with BN5's battlefields (xfields.h); BN6's
 * tables grown first (xvirus_grow.h), wherever a family is installed.
 * Their enemy ids then spawn them, and data.c reads those ids as the
 * engine does (EnemyMoved). Where BN5's ROM is not read yet nothing is
 * done, and a later call (a core's next start, the ROM put in on the ROMs
 * screen meanwhile) installs them. */
void xvirus_install(int which);
/* One family into the core's ROM copy (BN6's tables grown first where they
 * are not yet): true once it is there (now or before); false, nothing of it
 * written, where any check failed (the log says which). Its reward rows are
 * its game's own for its versions (another game's, each entry as BN6's:
 * xchips.h, xchips_reward_bn6) in the rows of its ids, which data.c's
 * enemy_drop then reads. */
bool xvirus_install_family(const XVirusFamily *f);
/* The family of game `game` (an XRomId) whose AI index there is `src_ai`,
 * installed in this core: its recipe (its AI index and first enemy id in
 * BN6, XVirusFamily.ai and .first_id); NULL where it is not. */
const XVirusFamily *xvirus_installed(int game, int src_ai);
/* Where enemy `id`'s reward row is in the core's copy, which the battle's
 * results read (BN6_DROP_ROWS, or its grown copy once the families' room
 * is made: ids to XV_IDS); 0 for an id past them. Read only what was set
 * as the core started, so a hook may ask (encounter.c, the reward pick). */
uint32_t xvirus_drop_row(int id);

/* Code alone of another game's moved into the core's copy (a chip's:
 * xchips_code.c): its blocks and clones copied and fixed up as a family's
 * are, their calls out through veneers from `veneers` (room for `nveneers`,
 * word-aligned), its sprites, effects and object types for the dispatch set
 * in BN6's grown tables, all within region `r` (xvirus_code_ok); staged into
 * the step open (xvirus_tx.h), false (the step failed) where a check did not
 * pass. */
bool xvirus_code_stage(const XVirusFamily *f, const XvRegion *r, uint32_t veneers, int nveneers);
/* Where an address of a recipe's blocks is moved to (its Thumb bit kept), 0
 * for one outside them. */
uint32_t xvirus_code_at(const XVirusFamily *f, uint32_t a);

/* What was moved where, for a trace's reader (battle_trace.c, xdiff.c): an
 * address range of a family's game, or of one of BN6's tables, and its
 * copy */
typedef struct { uint32_t from, to, len; } XvMoved;
#define XV_MOVED_MAX 1024
/* the ranges moved so far (at most max), how many */
int xvirus_moved(XvMoved *out, int max);

#endif
