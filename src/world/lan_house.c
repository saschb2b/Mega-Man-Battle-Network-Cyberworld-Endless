/* lan_house.h. Lan's house (Central Town's map 0x01:01) and his room
 * (0x01:02) as BN6 has them (indoors.c): the house's front door to Central
 * Town and its stairs up (warp entries 1 and 3; 2 the bathroom's), the
 * room's stairs down (1) and the PC before the desk, jack-in point 0x40,
 * which BN6's own table sends to Lan's HP (docs/ROM_DATA.md, Lan's HP),
 * with a line of its story's. */
#include "lan_house.h"

#include <stdint.h>

#include "indoors.h"
#include "mapslot.h"
#include "town.h"

#define HOME_SONG 0x04       /* BN6's for the house and its rooms */
#define FRONT_DOOR 1
#define STAIRS_UP 3          /* the house's */
#define STAIRS_DOWN 1        /* the room's */
#define PC_POINT 0x40
#define JACK_IN_FACE 1       /* +x in Lan's HP: out of the blue pad's alcove, the way on (BN6's faced the camera, session 73) */
#define HOUSE_DOORS (1u << FRONT_DOOR | 1u << STAIRS_UP)   /* (the bathroom's shut) */

static struct {
	bool read;
	uint32_t warps[3];        /* BN6's own lists: the town's, the house's, the room's */
	int pc_x, pc_y;           /* the middle of the PC's jack-in cells */
	int pc_box[4];            /* ... and their bounds: x0, y0, x1, y1 */
	int up_x, up_y;           /* ... of the house's stairs up */
	int door_x, door_y;       /* ... of its front door */
} L;

static bool read_spots(void) {
	if (L.read) return true;
	/* (BN6's lists, read before the run's first install takes the maps
	 * over) */
	for (int n = 0; n < 3; ++n)
		if (!(L.warps[n] = indoors_bn6_warps(LAN_HOUSE_GROUP, n))) return false;
	if (!indoors_spot(LAN_HOUSE_GROUP, LAN_ROOM, PC_POINT, &L.pc_x, &L.pc_y, L.pc_box) ||
		!indoors_spot(LAN_HOUSE_GROUP, LAN_HOUSE, STAIRS_UP, &L.up_x, &L.up_y, NULL) ||
		!indoors_spot(LAN_HOUSE_GROUP, LAN_HOUSE, FRONT_DOOR, &L.door_x, &L.door_y, NULL)) return false;
	L.read = true;
	return true;
}

bool lan_house_install(int to_group, int to_number, int x, int y) {
	if (!read_spots()) return false;
	int hx, hy, rx, ry, face;
	indoors_dest(L.warps[0], FRONT_DOOR, &hx, &hy, &face);   /* (in at the front door) */
	indoors_dest(L.warps[LAN_HOUSE], STAIRS_UP, &rx, &ry, &face);   /* (up the stairs) */
	mapslot_house(true);
	/* (the PC through the town's destination, Lan's plain "Jack in!": BN6's
	 * own, destination 1, runs a line of its story's that turns him back,
	 * "Lan,let's check out the town first!") */
	uint32_t house = indoors_take_over(LAN_HOUSE_GROUP, LAN_HOUSE, hx, hy, HOUSE_DOORS, 1, HOME_SONG);
	bool ok = house && indoors_take_over(LAN_HOUSE_GROUP, LAN_ROOM, rx, ry, 1u << STAIRS_DOWN, 2, HOME_SONG) &&
		mapslot_jack_in(LAN_HOUSE_GROUP, LAN_ROOM, to_group, to_number, x, y, JACK_IN_FACE);
	mapslot_house(false);
	/* (out of the front door where the planned town has it: BN6's place is
	 * the original's) */
	const TownInfo *ti = town_info();
	if (ok && town_is_home()) indoors_door_to(house, FRONT_DOOR, ti->start_x, ti->start_y);
	return ok;
}

bool lan_house_map(int group, int number) { return group == LAN_HOUSE_GROUP && (number == LAN_HOUSE || number == LAN_ROOM); }

void lan_room_start(int *x, int *y, int *face) {
	*x = *y = *face = 0;
	if (read_spots()) indoors_dest(L.warps[LAN_HOUSE], STAIRS_UP, x, y, face);
}

bool lan_house_goal(int number, int *x, int *y) {
	if (!read_spots()) return false;
	if (number == LAN_ROOM) { *x = L.pc_x; *y = L.pc_y; return true; }
	if (number == LAN_HOUSE) { *x = L.up_x; *y = L.up_y; return true; }
	return false;
}

bool lan_house_door(int *x, int *y) {
	if (!read_spots()) return false;
	*x = L.door_x;
	*y = L.door_y;
	return true;
}

bool lan_room_on_pc(int x, int y) {
	return read_spots() && x >= L.pc_box[0] && y >= L.pc_box[1] && x < L.pc_box[2] && y < L.pc_box[3];
}

void lan_house_frame(int number) {
	if (number == LAN_HOUSE) indoors_shut(HOUSE_DOORS);
	else if (number == LAN_ROOM) indoors_shut(1u << STAIRS_DOWN);
}
