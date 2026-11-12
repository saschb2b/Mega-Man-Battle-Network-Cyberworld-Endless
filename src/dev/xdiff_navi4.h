/* The differential's logs and BN4's Navi chips (docs/devtools/differential.md,
 * A BN4 chip's spec; docs/multirom/bn4-chips.md, The Navi chips): a Navi chip
 * of BN4's whose Navi is moved (src/emu/xchips_bn4.c's XCHIP_NAVI units) runs
 * the chip's own Navi's code and the moved Navi's attack objects' code, which
 * the logs watch, in both games, where the spec's folder plays one, so a
 * draw his attack makes is keyed by BN4's place and replayed (xdiff_events.h). */
#ifndef CW_XDIFF_NAVI4_H
#define CW_XDIFF_NAVI4_H

#include <stdbool.h>
#include <stdint.h>

#include "xvirus_moved.h"

/* The units BN6's folder `folder` (n entries, this build's form) plays, noted
 * for xd_navi4_code; the last folder's forgotten */
void xd_navi4_units(const uint16_t *folder, int n);
/* Whether a moved range is a played Navi chip's unit's block, or a block of
 * the moved Navi's whose attack objects it runs */
bool xd_navi4_code(const XvMoved *m);

#endif
