/* lan_house.h. Lan's house (Central Town's map 0x01:01) and his room
 * (0x01:02) as BN6 has them: the house's front door to Central Town and
 * its stairs up (warp entries 1 and 3; 2 the bathroom's), the room's
 * stairs down (1) and the PC before the desk, jack-in point 0x40, which
 * BN6's own table sends to Lan's HP (docs/ROM_DATA.md, Lan's HP), with a
 * line of its story's. */
#include "lan_house.h"

#include <stdint.h>
#include <string.h>

#include "area_src.h"
#include "bn6.h"
#include "emu.h"
#include "flags.h"
#include "mapslot.h"
#include "town.h"

#define HOME_SONG 0x04       /* BN6's for the house and its rooms */
#define FRONT_DOOR 1
#define BATHROOM_DOOR 2
#define STAIRS_UP 3          /* the house's */
#define STAIRS_DOWN 1        /* the room's */
#define PC_POINT 0x40

static struct {
	bool read;
	uint32_t warps[3];        /* BN6's own lists: the town's, the house's, the room's */
	int pc_x, pc_y;           /* the middle of the PC's jack-in cells */
	int pc_box[4];            /* ... and their bounds: x0, y0, x1, y1 */
	int up_x, up_y;           /* ... of the house's stairs up */
} L;

/* the middle of map `number`'s trigger cells of `value`, and their bounds
 * (box: x0, y0, x1, y1, or NULL); false none */
static bool spot(int number, int value, int *x, int *y, int *box) {
	AreaSrc a;
	if (!area_src_load(LAN_HOUSE_GROUP, number, &a)) return false;
	long sx = 0, sy = 0;
	int n = 0, b[4] = { 1 << 20, 1 << 20, -(1 << 20), -(1 << 20) };
	for (int i = 0; i < a.nsec[3]; ++i) {
		const CoordCell *c = &a.sec[3][i];
		if (c->value != value) continue;
		sx += c->x + 4; sy += c->y + 4; ++n;
		if (c->x < b[0]) b[0] = c->x;
		if (c->y < b[1]) b[1] = c->y;
		if (c->x + 8 > b[2]) b[2] = c->x + 8;
		if (c->y + 8 > b[3]) b[3] = c->y + 8;
	}
	area_src_free(&a);
	if (!n) return false;
	*x = (int)(sx / n);
	*y = (int)(sy / n);
	if (box) memcpy(box, b, sizeof b);
	return true;
}

static bool read_spots(void) {
	if (L.read) return true;
	/* (BN6's lists, read before the run's first install takes the maps
	 * over) */
	for (int n = 0; n < 3; ++n)
		if (!(L.warps[n] = mapslot_warps(LAN_HOUSE_GROUP, n))) return false;
	if (!spot(LAN_ROOM, PC_POINT, &L.pc_x, &L.pc_y, L.pc_box) || !spot(LAN_HOUSE, STAIRS_UP, &L.up_x, &L.up_y, NULL)) return false;
	L.read = true;
	return true;
}

/* where BN6's warp entry `entry` of list `list` sets Lan down */
static void dest(uint32_t list, int entry, int *x, int *y, int *face) {
	uint32_t e = list + 16u * (uint32_t)(entry - 1);
	*x = (int32_t)emu_read32(e + 4) >> 16;
	*y = (int32_t)emu_read32(e + 8) >> 16;
	*face = emu_read8(e + 3);
}

/* map `number` with no people or map scripts, BN6's objects, its own warp
 * list (every entry back to (x, y), where BN6 sets Lan down entering it)
 * but for BN6's doors `keep`; the list, 0 none */
static uint32_t take_over(int number, int x, int y, unsigned keep, int song_k) {
	NpcList npcs;
	memset(&npcs, 0, sizeof npcs);
	npcs.objects = mapslot_objects(LAN_HOUSE_GROUP, number);
	if (!mapslot_install(LAN_HOUSE_GROUP, number, &npcs, NULL, 0)) return 0;
	uint32_t list = mapslot_own_warps(LAN_HOUSE_GROUP, number, x, y, 1);
	if (!list) return 0;
	for (int e = 1; e <= 8; ++e) if (keep >> e & 1) mapslot_copy_warp(list, e, L.warps[number], e);
	return mapslot_music_home(song_k, LAN_HOUSE_GROUP, number, HOME_SONG) ? list : 0;
}

bool lan_house_install(int to_group, int to_number, int x, int y) {
	if (!read_spots()) return false;
	int hx, hy, rx, ry, face;
	dest(L.warps[0], FRONT_DOOR, &hx, &hy, &face);   /* (in at the front door) */
	dest(L.warps[LAN_HOUSE], STAIRS_UP, &rx, &ry, &face);   /* (up the stairs) */
	mapslot_house(true);
	/* (the PC through the town's destination, Lan's plain "Jack in!": BN6's
	 * own, destination 1, runs a line of its story's that turns him back,
	 * "Lan,let's check out the town first!") */
	uint32_t house = take_over(LAN_HOUSE, hx, hy, 1u << FRONT_DOOR | 1u << STAIRS_UP, 1);
	bool ok = house && take_over(LAN_ROOM, rx, ry, 1u << STAIRS_DOWN, 2) && mapslot_jack_in(LAN_HOUSE_GROUP, LAN_ROOM, to_group, to_number, x, y, 4);
	mapslot_house(false);
	/* (out of the front door where the planned town has it: BN6's place is
	 * the original's) */
	const TownInfo *ti = town_info();
	if (ok && town_is_home()) {
		emu_write32(house + 16u * (FRONT_DOOR - 1) + 4, (uint32_t)ti->start_x << 16);
		emu_write32(house + 16u * (FRONT_DOOR - 1) + 8, (uint32_t)ti->start_y << 16);
	}
	return ok;
}

uint32_t lan_house_bn6_warps(int number) { return number >= 0 && number <= LAN_ROOM && read_spots() ? L.warps[number] : 0; }

bool lan_house_map(int group, int number) { return group == LAN_HOUSE_GROUP && (number == LAN_HOUSE || number == LAN_ROOM); }

void lan_room_start(int *x, int *y, int *face) {
	*x = *y = *face = 0;
	if (read_spots()) dest(L.warps[LAN_HOUSE], STAIRS_UP, x, y, face);
}

bool lan_house_goal(int number, int *x, int *y) {
	if (!read_spots()) return false;
	if (number == LAN_ROOM) { *x = L.pc_x; *y = L.pc_y; return true; }
	if (number == LAN_HOUSE) { *x = L.up_x; *y = L.up_y; return true; }
	return false;
}

bool lan_room_on_pc(int x, int y) {
	return read_spots() && x >= L.pc_box[0] && y >= L.pc_box[1] && x < L.pc_box[2] && y < L.pc_box[3];
}

void lan_house_frame(int number) {
	if (number == LAN_HOUSE) flag_set(BN6_FLAG_WARP_OFF + BATHROOM_DOOR);
}
