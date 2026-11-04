/* BN6's own object types copied for BN4's families, one changed
 * instruction apart (tools/xrecipe_bn4_copy.py writes the recipe into
 * xvirus_bn4.c; docs/rom/bn4.md, The thunder's copy): the thunder, BN6's
 * third-kind 0x2A, whose init gives its collision setup r3 = 1 where BN4's
 * gives 0, so a struck the paralysis does not take starts its struck
 * reaction in BN6 and not in BN4. BN6's own spawns of it keep its 1, so
 * BN6's type is never patched: its copy, at the families' code room's
 * start, its init's 1 made 0, runs as a moved type of the second-level
 * dispatch (xvirus_dispatch.h) at a number of its own, and Billy's call of
 * BN6's spawner goes to the copy's. One install, all or nothing, before
 * BN4's families (xvirus.c), only where BN4's ROM is read for research. */
#ifndef CW_XVIRUS_BN4_COPY_H
#define CW_XVIRUS_BN4_COPY_H

#include <stdbool.h>
#include <stdint.h>

#include "xvirus_recipe.h"

/* the recipe of code alone (game XV_GAME_BN6), its veneers' place and
 * count (xvirus_bn4.c, the generator's) */
extern const XVirusFamily xvirus_bn4_bn6copy;
extern const uint32_t xvirus_bn4_bn6copy_veneers;
extern const int xvirus_bn4_bn6copy_nveneers;
/* the room it may take: the families' code room, its calls to BN6's
 * routines and the dispatch's spawners */
#define XV_BN4_COPY_REGION { XV_CODE, XV_ART, XV_ART, XV_ART_END, XV_STUBS, XV_STUBS_END }

/* The copy into the core's ROM copy (BN6's tables grown first): true once
 * it is there (now or before); false, nothing written, where a check
 * failed (the log says which). */
bool xvirus_bn4_copy_install(void);

#endif
