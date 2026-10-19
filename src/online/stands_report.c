/* A NetBattle's report read off its player objects (stands_report.h). */
#include "stands_report.h"

#include <stdbool.h>

#define CHIP_ACTION_FIRST 0x12     /* BN6_T1_ACTION: a chip's, 0x12-0x16 (a charged shot's among them, with no chip) */
#define CHIP_ACTION_LAST  0x16

void stands_report_side(ArenaReport *r, ReportSeen *seen, unsigned side, uint16_t hp, uint16_t max_hp, uint8_t action, uint16_t chip) {
	side &= 1u;
	r->hp[side] = hp <= max_hp ? hp : max_hp;
	r->max_hp[side] = max_hp > 9999 ? 9999 : max_hp;
	/* (the chip he uses next is the hand's first: as he starts using it, it gives way to the next, or to none,
	 * the frame his action turns to a chip's; a charged shot's action is one of them too, with no chip) */
	uint16_t next = seen->chip[side];
	bool using = action >= CHIP_ACTION_FIRST && action <= CHIP_ACTION_LAST;
	if (using && next && next <= ONLINE_CHIP_LAST && chip != next) {
		r->chip[side] = next;
		if (r->used[side] < 255) ++r->used[side];
	}
	seen->action[side] = action;
	seen->chip[side] = chip;
}
