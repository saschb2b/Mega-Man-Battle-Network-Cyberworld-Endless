/* The swap step (--input's "0:swap 101:300/107:300/10D:300", play.py's
 * "swap 101:300 107:300 10D:300"): a battle of one Navi swapped mid-fight
 * for the next (src/emu/battle_swap.h; docs/devtools/battle-on-demand.md). */
#ifndef CW_SWAP_DEV_H
#define CW_SWAP_DEV_H

#include <stdbool.h>

/* The forms parsed (each a Navi's enemy id in hex, then :HP in decimal, 0
 * or none its own, apart by '/', ' ' or ','), the plan armed and the
 * battle of its first form due as the foes step's (director_dev_foes, at
 * column 5, row 2); false, with what was wrong, where it cannot be */
bool swap_dev_step(const char *spec);

#endif
