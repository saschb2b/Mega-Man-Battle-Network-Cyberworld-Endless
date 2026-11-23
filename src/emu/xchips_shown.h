/* A moved chip's computed power as the PET shows it (issue 835;
 * docs/multirom/chips-code.md, Computed powers). BN6's PET reads a shown
 * power past 1000 as a Navi's by MegaMan's best time against him
 * (BN6_NAVI_POWER: the folder editor's card, the pack's Attack sort), its
 * tables indexed by the power less 1001. Another game's SP whose routine a
 * helper moved has a power past BN6's 45 entries, an entry of BN6's grown
 * table (XC_POWER_FIRST and XREG4_POWER_FIRST on), and those tables hold
 * none for it: CosmoMn SP's card read 6544 past them. Hooked: such a power
 * goes BN6's battle's way instead (BN6_CHIP_POWER_OF, side 0), the moved
 * routine itself, whose answer for a save without a deletion time is the
 * one BN5's and BN4's own cards show (the last of its row). */
#ifndef CW_XCHIPS_SHOWN_H
#define CW_XCHIPS_SHOWN_H

#include <stdbool.h>

/* The hooks, once a core as the chip registry installs (xchips_install.c);
 * false where one is refused. */
bool xchips_shown_install(void);

/* The power the folder editor's card last drew and its chip, as BN6 drew
 * them (BN6_CARD_POWER_DRAWN), for the state's lines (tests/scenarios);
 * false where it drew none since the core started. */
bool xchips_shown_card(int *chip, int *power);

#endif
