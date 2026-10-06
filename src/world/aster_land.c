/* aster_land.h. AsterLand (Central Town's map 0x01:04) as BN6 has it: its
 * door out to the town (its warp entry 1), the counter, the Request BBS on
 * the wall (BN6's, with none of its requests in a run), its checks, and
 * BN6's Chip Trader, whose trade screen holds AsterLand's own prizes
 * (docs/ROM_DATA.md, Chip Traders); in from the town's warp 4. Its Number
 * Trader is off: its codes are no secret, and every run would take BN6's
 * prizes from it (docs/HOME.md, piece 10). */
#include "aster_land.h"

#include <stdint.h>

#include "emu.h"
#include "indoors.h"
#include "mapslot.h"
#include "town.h"
#include "trader.h"

#define SONG 0x04          /* BN6's for AsterLand, as for Lan's house */
#define DOOR_OUT 1
#define TOWN_DOOR 4        /* the town's warp entry into it */
#define MUSIC_SLOT 3       /* home's music slot (mapslot.h) */
#define NUMBER_TRADER 0xF9 /* its check */
#define NUMBER_TRADER_MAPS 0x08034E80u /* the map ids (u16, 0xFFFF ending) whose check 0xF9 BN6's Number Trader is: AsterLand's (bn6f dword_8034E80) */

static int numbers[4];     /* the Number Trader's check cells' bounds: x0, y0, x1, y1 */

bool aster_land_install(void) {
	uint32_t town = indoors_bn6_warps(ASTER_GROUP, 0);
	if (!town || !indoors_bn6_warps(ASTER_GROUP, ASTER_LAND)) return false;
	int x, y, face;
	indoors_dest(town, TOWN_DOOR, &x, &y, &face);   /* (in at its door) */
	mapslot_indoors(1);
	uint32_t list = indoors_take_over(ASTER_GROUP, ASTER_LAND, x, y, 1u << DOOR_OUT, MUSIC_SLOT, SONG);
	mapslot_indoors(0);
	if (!list) return false;
	trader_home();
	/* (BN6's Number Trader on no map: its check then reads none, and the
	 * director says place_lines.c's word there) */
	emu_write32(NUMBER_TRADER_MAPS, 0xFFFFFFFFu);
	int nx, ny;
	if (!indoors_spot(ASTER_GROUP, ASTER_LAND, NUMBER_TRADER, &nx, &ny, numbers)) numbers[0] = numbers[2] = 0;
	/* (out where the planned town has its door) */
	int ox, oy, tx, ty;
	indoors_dest(indoors_bn6_warps(ASTER_GROUP, ASTER_LAND), DOOR_OUT, &ox, &oy, &face);
	if (town_is_home() && town_moved(ox, oy, &tx, &ty)) indoors_door_to(list, DOOR_OUT, tx, ty);
	return true;
}

bool aster_land_map(int group, int number) { return group == ASTER_GROUP && number == ASTER_LAND; }

bool aster_land_door(int *x, int *y) { return indoors_spot(ASTER_GROUP, ASTER_LAND, DOOR_OUT, x, y, NULL); }

bool aster_land_number_trader(int x, int y) { return x >= numbers[0] && y >= numbers[1] && x < numbers[2] && y < numbers[3]; }

void aster_land_frame(void) { indoors_shut(1u << DOOR_OUT); }
