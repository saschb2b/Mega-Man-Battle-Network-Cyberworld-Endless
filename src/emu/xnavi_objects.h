/* BN5's engine objects that BN6's engine runs otherwise, moved into BN6
 * once for every track whose code spawns one (issue #82's plan for its
 * tracks; docs/MULTIROM.md, The engine's objects moved once;
 * tools/bn5_objects.py): BN5's image object, the first kind's type 5, its
 * module whole (its kinds table, its routine, the spawner every attack
 * calls, its helpers) at XN_OBJECTS, its type moved through the
 * second-level dispatch (xvirus_dispatch.h), its kinds' sprites as BN6
 * lists them where a kind is fixed (xnavi_bn5.c, xnavi_objects_bn5). One
 * install, all or nothing, before any family's or Navi's (xnavi_install.c,
 * xnavi_install_core: a family's code calls into them too, WuNote's image,
 * and its type is run as theirs, XV_TYPE_SHARED). */
#ifndef CW_XNAVI_OBJECTS_H
#define CW_XNAVI_OBJECTS_H

#include <stdbool.h>

/* The moved objects into the core's ROM copy (the rooms grown first): true
 * once they are there (now or before); false, nothing written, where a
 * check failed (the log says which). */
bool xnavi_objects_install(void);
/* Whether the dispatch runs its game's type `type` of kind `kind` as a
 * moved copy in the core's ROM copy (a Navi's need, XNavi.needs). */
bool xnavi_objects_have(int kind, int type);

#endif
