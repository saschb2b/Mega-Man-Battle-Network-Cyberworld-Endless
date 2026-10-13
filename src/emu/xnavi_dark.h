/* The Dark Soul in BN6's engine (issue #82; docs/MULTIROM.md, Transplanted
 * Navis, The Dark Soul): what BN5's engine does for it by its id, the
 * chip records its chip AI reads, and its lists. BN5's chip AI, moved
 * with the Dark Soul (BN5_CHIP_AI_DARK), plays the chips of its foe's
 * side's list, as BN5 keeps them at BN5_DARK_LISTS and BN6's engine at
 * BN6_DARK_LISTS (RAM_DARK_LISTS, zeroed as each battle is set up: no
 * chips, so the Dark Soul moves and fires its buster). BN5 makes
 * MegaMan's side's list from the save's record of the chips he uses most
 * (0x0802C7C2), which BN6 keeps none of; a list is given here from BN5's,
 * its chip ids made BN6's by the chip registry (xchips_registry.h). */
#ifndef CW_XNAVI_DARK_H
#define CW_XNAVI_DARK_H

#include <stdbool.h>
#include <stdint.h>

#include "bn5.h"

#define XNAVI_DARK_CHIPS     0x2A   /* a list's chips (u16 each, BN5_DARK_LIST_COUNT their count's place) */
#define XNAVI_DARK_SEQUENCES 8      /* its sequences (BN5_DARK_SEQUENCES on, 16 bytes each: two offsets, then seven chips) */
#define XNAVI_DARK_SEQ_CHIPS 7

/* What BN5's engine does for the Dark Soul by its id, which BN6's twins
 * do for none (BN5_DARK_REGION, BN5_DARK_PLATE): made for the moved one's
 * id `id` by hooks on BN6's twins (BN6_NAVI_REGION, BN6_NAVI_PLATE), each
 * site checked to be BN6's own first; and the chip records its chip AI
 * reads (XN_DARK_RECORDS, through XN_STUB_CHIPDATA): BN6's, by BN6's id,
 * with the four bytes BN5's records give a meaning of their own
 * (BN5_CHIP_DS_*) BN5's for each chip of the registry. False, none set,
 * where not */
bool xnavi_dark_install(int id);

/* `len` bytes of BN5's lists (BN5_DARK_LIST_SIZE a side) into `out` as
 * BN6's engine keeps them: 0xFFFF and a sequence's number (0x8000 on) as
 * they are, a chip id BN6's for it by the registry, 0xFFFF (none) for one
 * it does not hold, the rest the same; how many chips were BN5's own and
 * none in BN6 */
int xnavi_dark_lists_from(const uint8_t *bn5, uint8_t *out, uint32_t len);

#endif
