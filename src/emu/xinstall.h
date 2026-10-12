/* Another game's content moved into BN6's battle engine as a core starts
 * (issue #82; docs/MULTIROM.md), each track's install in its order: what
 * one installs first another may run. BN5's engine objects every track's
 * code reaches (xnavi_objects.h) where BN5's families or Navis come in,
 * then the Navis `navis` names (xnavi_install.h, XNAVI_*), then BN5's
 * chips' moved code (xchips_code.h: it spawns the moved objects' and the
 * Navis' copies of the types they share), then the virus families
 * `families` names (xvirus.h, XVIRUS_*: they spawn the chips' copies of
 * the types they share, as tools/bn5_twins.py's TYPES gives each moved
 * type one copy, the first track's in this order), then BN5's sea's
 * submerge (xsea.h) where its sea came in. A track whose copies another's
 * needs left out (--dev nonavis) leaves that other's install failing, its
 * reason logged. */
#ifndef CW_XINSTALL_H
#define CW_XINSTALL_H

#include "xnavi_install.h"
#include "xvirus.h"

void xinstall_core(int families, int navis);

#endif
