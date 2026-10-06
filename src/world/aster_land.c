/* aster_land.h. AsterLand (Central Town's map 0x01:04) as BN6 has it: its
 * door out to the town (its warp entry 1), the counter, the Request BBS on
 * the wall (its post the run's request, job_words.c: BN6's own Request
 * BBS holds none of its requests in a run), its checks, and
 * BN6's Chip Trader, whose trade screen holds AsterLand's own prizes
 * (docs/ROM_DATA.md, Chip Traders), and home's shops (docs/HOME.md, piece
 * 6): the counter's Order Service and the SubChip seller beside it; in
 * from the town's warp 4. Its Number
 * Trader is off: its codes are no secret, and every run would take BN6's
 * prizes from it (docs/HOME.md, piece 10). */
#include "aster_land.h"

#include <stdint.h>

#include "emu.h"
#include "indoors.h"
#include "flags.h"
#include "mapslot.h"
#include "run.h"
#include "shop.h"
#include "town.h"
#include "trader.h"

#define SONG 0x04          /* BN6's for AsterLand, as for Lan's house */
#define DOOR_OUT 1
#define TOWN_DOOR 4        /* the town's warp entry into it */
#define MUSIC_SLOT 3       /* home's music slot (mapslot.h) */
#define REQUEST_BOARD 0xF6 /* its check: BN6's Request BBS, its post ours (job_words.c) */
#define NUMBER_TRADER 0xF9 /* its check */
#define NUMBER_TRADER_MAPS 0x08034E80u /* the map ids (u16, 0xFFFF ending) whose check 0xF9 BN6's Number Trader is: AsterLand's (bn6f dword_8034E80) */

static int front[2];       /* where Lan comes out in the town, before its door */
static bool front_ok;
static int numbers[4];     /* the Number Trader's check cells' bounds: x0, y0, x1, y1 */
static int board[4];       /* ... and the request board's */
static int orders = -1;    /* the Order Service's copies to order as the visit began */

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
	aster_land_shops();
	/* (BN6's Number Trader on no map: its check then reads none, and the
	 * director says place_lines.c's word there) */
	emu_write32(NUMBER_TRADER_MAPS, 0xFFFFFFFFu);
	int nx, ny;
	if (!indoors_spot(ASTER_GROUP, ASTER_LAND, NUMBER_TRADER, &nx, &ny, numbers)) numbers[0] = numbers[2] = 0;
	if (!indoors_spot(ASTER_GROUP, ASTER_LAND, REQUEST_BOARD, &nx, &ny, board)) board[0] = board[2] = 0;
	/* (out where the planned town has its door) */
	int ox, oy, tx, ty;
	indoors_dest(indoors_bn6_warps(ASTER_GROUP, ASTER_LAND), DOOR_OUT, &ox, &oy, &face);
	if (town_is_home() && town_moved(ox, oy, &tx, &ty)) indoors_door_to(list, DOOR_OUT, tx, ty);
	else { tx = ox; ty = oy; }
	front[0] = tx;
	front[1] = ty;
	front_ok = true;
	return true;
}

void aster_land_shops(void) {
	ShopItem subs[SHOP_MAX_ITEMS];
	shop_install(SHOP_SUBS_HOME, subs, shop_home_subs(run.depth, subs), true);
	orders = shop_order_install();
}

bool aster_land_front(int *x, int *y) {
	if (!front_ok) return false;
	*x = front[0];
	*y = front[1];
	return true;
}

bool aster_land_map(int group, int number) { return group == ASTER_GROUP && number == ASTER_LAND; }

bool aster_land_door(int *x, int *y) { return indoors_spot(ASTER_GROUP, ASTER_LAND, DOOR_OUT, x, y, NULL); }

static bool in(const int b[4], int x, int y) { return x >= b[0] && y >= b[1] && x < b[2] && y < b[3]; }

bool aster_land_number_trader(int x, int y) { return in(numbers, x, y); }

bool aster_land_board(int x, int y) { return in(board, x, y); }

void aster_land_frame(void) {
	indoors_shut(1u << DOOR_OUT);
	/* (an order made: the counter's word for the rest of the visit; looked
	 * for a few times a second, the list being long) */
	static int tick;
	if (++tick % 8) return;
	int left = shop_order_left();
	if (orders >= 0 && left >= 0 && left < orders) flag_set(HOME_ORDER_FLAG);
}
