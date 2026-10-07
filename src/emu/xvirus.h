/* Virus families moved into BN6's own battle engine (issue #82; docs/
 * MULTIROM.md, Transplanted families; docs/EMULATION.md, its free space):
 * a family's own code copied whole from its game's ROM into the core's copy
 * past EMU_FREE, its calls out aimed again at BN6's routines through
 * veneers, the pointers in it moved, a state table laid out BN6's way, and
 * BN6's eleven per-family tables grown past their 32 AI indexes so the
 * family has one of its own, with its enemy ids, names and records.
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
 * every boot with the same ROMs). Their enemy ids then spawn them, and
 * data.c reads those ids as the engine does (EnemyMoved). */
void xvirus_install(int which);
/* One family into the core's ROM copy: true once it is there (now or
 * before); false, nothing written, where any check failed (the log says
 * which). */
bool xvirus_install_family(const XVirusFamily *f);

/* What was moved where, for a trace's reader (battle_trace.c): an
 * address range of the family's game and BN6's copy of it. */
typedef struct { uint32_t from, to, len; } XvMoved;
/* the ranges moved so far (at most max), how many */
int xvirus_moved(XvMoved *out, int max);

#endif
