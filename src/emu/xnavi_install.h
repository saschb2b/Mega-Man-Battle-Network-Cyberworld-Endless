/* Navis moved into BN6's own battle engine (issue #82; docs/MULTIROM.md,
 * Transplanted Navis; docs/EMULATION.md, the free space): a Navi's own
 * code copied from its game's ROM by the families' code mover
 * (xvirus_move.h), its state table laid out BN6's way, its entries in
 * BN6's Navi tables grown for them (xnavi_room.h), its ids, names, reward
 * rows and battle sprite (xnavi_sprites.h). Each install all or nothing:
 * every write staged and made only once every check has passed. */
#ifndef CW_XNAVI_INSTALL_H
#define CW_XNAVI_INSTALL_H

#include <stdbool.h>
#include <stdint.h>

#include "xnavi_dark.h"
#include "xnavi_recipe.h"
#include "xvirus.h"

/* What xnavi_install_all puts in */
enum {
	XNAVI_BN5,       /* BN5's Navis whose code is proven against BN5's own (xnavi_bn5.h), where its ROM is read */
	XNAVI_CONTROL,   /* --dev xnavicontrol: the control instead, BN6's own HeatMan (AI 1) moved through the same steps as AI XN_FIRST_AI
	                    with ids XN_FIRST_ID on, which must play frame for frame as HeatMan does (tools/xcontrol.py --navi) */
	XNAVI_NONE,      /* --dev nonavis: none */
};
/* The Navis a battle may meet, installed once a core as it starts (after
 * the families, xvirus.h): their ids then spawn them (the foes step), and
 * data.c reads those ids as the engine does (EnemyMoved). */
void xnavi_install_all(int which);
/* Another game's content moved into BN6's engine as the core starts: the
 * virus families `families` names (xvirus_install's XVIRUS_*), then the
 * Navis `navis` names (XNAVI_*), then BN5's sea's submerge (xsea.h) where
 * its sea came in */
void xnavi_install_core(int families, int navis);
/* One Navi into the core's ROM copy (the rooms grown first): true once it
 * is there (now or before); false, nothing of it written, where any check
 * failed (the log says which). */
bool xnavi_install(const XNavi *n);
/* Enemy id `id` of BN5's (a Navi's version) as the engine has it once its
 * Navi is installed; -1 where it is not. */
int xnavi_bn6_id(int id);

#endif
