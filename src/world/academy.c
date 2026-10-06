/* academy.h. Lan's school (the Cyber Academy, group 0x02) as BN6 has it,
 * four of its maps open on a club day: the foyer (0x02:06, in from the
 * town's warp 2), whose gate (its warp 1) leads out and whose way (2) to
 * the 1F hallway (0x02:04); its stairs (6) up to the 2F hallway (0x02:05),
 * whose two doors (2, 3) open into class 6-1 (0x02:00), Lan's, and whose
 * stairs (1) lead down. The teachers' room, the principal's office and
 * the other classrooms stay shut; the NetBattle club meets in class 6-1
 * (place_lines.c). Warp lists read in Gregar (docs/ROM_DATA.md, Lan's
 * house and room). */
#include "academy.h"

#include <stdint.h>

#include "indoors.h"
#include "mapslot.h"
#include "town.h"

#define SONG 0x05            /* BN6's for the Academy */
#define TOWN_GATE 2          /* the town's warp entry into the foyer */
#define FIRST_SLOT 4         /* home's music slots 4-7 (mapslot.h) */

/* each open map: its number, BN6's doors kept (the rest shut), the door
 * it is entered by from the one before (in the one before's list), and
 * its way out (its own warp entry) */
static const struct { int number; unsigned keep; int in_by, out; } maps[] = {
	{ ACADEMY_FOYER, 1u << 1 | 1u << 2, TOWN_GATE, 1 },
	{ ACADEMY_HALL_1F, 1u << 1 | 1u << 6, 2, 1 },
	{ ACADEMY_HALL_2F, 1u << 1 | 1u << 2 | 1u << 3, 6, 1 },
	{ ACADEMY_CLASS_6_1, 1u << 1 | 1u << 2, 2, 1 },
};
#define MAPS (int)(sizeof maps / sizeof *maps)

/* the list a map is entered from: the town's for the foyer, else the one
 * before's */
static uint32_t entered_from(int k) {
	return k == 0 ? indoors_bn6_warps(0x01, 0) : indoors_bn6_warps(ACADEMY_GROUP, maps[k - 1].number);
}

bool academy_install(void) {
	for (int k = 0; k < MAPS; ++k) if (!entered_from(k) || !indoors_bn6_warps(ACADEMY_GROUP, maps[k].number)) return false;
	uint32_t foyer = 0;
	bool ok = true;
	mapslot_indoors(2);
	for (int k = 0; k < MAPS && ok; ++k) {
		int x, y, face;
		indoors_dest(entered_from(k), maps[k].in_by, &x, &y, &face);
		uint32_t list = indoors_take_over(ACADEMY_GROUP, maps[k].number, x, y, maps[k].keep, FIRST_SLOT + k, SONG);
		ok = list != 0;
		if (k == 0) foyer = list;
	}
	mapslot_indoors(0);
	/* (out of the gate where the planned town has it) */
	int ox, oy, tx, ty, face;
	indoors_dest(indoors_bn6_warps(ACADEMY_GROUP, ACADEMY_FOYER), 1, &ox, &oy, &face);
	if (ok && town_is_home() && town_moved(ox, oy, &tx, &ty)) indoors_door_to(foyer, 1, tx, ty);
	return ok;
}

bool academy_map(int group, int number) {
	for (int k = 0; k < MAPS && group == ACADEMY_GROUP; ++k) if (maps[k].number == number) return true;
	return false;
}

bool academy_way_out(int number, int *x, int *y) {
	for (int k = 0; k < MAPS; ++k)
		if (maps[k].number == number) return indoors_spot(ACADEMY_GROUP, number, maps[k].out, x, y, NULL);
	return false;
}

void academy_frame(int number) {
	for (int k = 0; k < MAPS; ++k) if (maps[k].number == number) indoors_shut(maps[k].keep);
}
