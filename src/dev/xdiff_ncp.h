/* The differential's NaviCust items (docs/devtools/differential.md, A
 * program on each game's NaviCust): ncp5= and ncp6= place one program on
 * each game's own board, alone, and run each game's own compile before the
 * battle is asked, so that what the program sets is what each game's own
 * routine set. BN5's on the guest's core (guest_call), BN6's on BN6's
 * (game_call). Local only, as src/dev/xdiff.c. */
#ifndef CW_XDIFF_NCP_H
#define CW_XDIFF_NCP_H

#include <stdbool.h>

/* Variant `variant` (program x 4 + colour; 0: none asked, true) at `col`,
 * `row` of BN6's board, its shape's cells from its record, and BN6's
 * compile run; false where BN6 never ran it */
bool xd_ncp_subject(int variant, int col, int row);
/* ... of BN5's board on the guest's core, by BN5's compile, on its map
 * before its battle is asked */
bool xd_ncp_reference(int variant, int col, int row);

#endif
