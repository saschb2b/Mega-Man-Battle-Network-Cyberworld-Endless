/* A NetBattle's report read off its player objects (issue #128,
 * arena_report.h): each side's HP and max HP, and a chip counted as he
 * starts using it. ROM-free: tests/test_arena.c. */
#ifndef CW_STANDS_REPORT_H
#define CW_STANDS_REPORT_H

#include <stdint.h>

#include "stands_wire.h"

/* What the last frame read of each side: his action and the chip he uses next */
typedef struct { uint8_t action[2]; uint16_t chip[2]; } ReportSeen;
/* One side's player object read into the report: HP, max HP, and his
 * action (BN6_T1_ACTION) and the chip he uses next (BN6_T1_CHIP, the
 * hand's first, 0xFFFF none) this frame: a chip counted used as his action
 * turns to a chip's and the next chip gives way (seen in a NetBattle: the
 * same frame); ROM-free (tests/test_arena.c) */
void stands_report_side(ArenaReport *r, ReportSeen *seen, unsigned side, uint16_t hp, uint16_t max_hp, uint8_t action, uint16_t chip);

#endif
