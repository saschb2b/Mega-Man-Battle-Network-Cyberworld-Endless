/* place_lines.h. Who is in home's indoor places on a day without class
 * (docs/HOME.md, piece 10; docs/VOICE.md): AsterLand's clerk behind the
 * counter where BN6 stands its own, a NetBattler at the request board and
 * a shopper at the rare chips; the Academy's NetBattle club in
 * class 6-1, where BN6 stands Lan's classmates, and a first grader on the
 * first floor. Of the game's people in sprite list 5 none of Central
 * Town's own (town_lines.c), who are out in the town. */
#include "place_lines.h"

#include "academy.h"
#include "aster_land.h"

const PlaceFolk place_folk[] = {
	{ ASTER_GROUP, ASTER_LAND, { 28, -70, FACE_SE, 5, 0x33,
		"Welcome to AsterLand!|Got chips you don't need?|Three in the Chip Trader,one new one out!" }, 10 },
	{ ASTER_GROUP, ASTER_LAND, { -92, -40, FACE_SE, 5, 0x35,
		"Hmm... The request board's empty.|Doesn't anybody need a NetBattler?" }, 0 },
	{ ASTER_GROUP, ASTER_LAND, { -20, -34, FACE_NW, 5, 0x40,
		"Ooh... Look at these rare chips!|...And look at these prices! *gulp*" }, 0 },
	{ ACADEMY_GROUP, ACADEMY_CLASS_6_1, { -148, 2, FACE_NE, 5, 0x3B,
		"Lan! Welcome to the club!|Tip of the day! Match your codes!|Same code,and you can send more chips!" }, 0 },
	{ ACADEMY_GROUP, ACADEMY_CLASS_6_1, { -130, -2, FACE_SW, 5, 0x42,
		"My Navi lost to a Mettaur again...|*sigh* Don't tell anyone,OK?" }, 0 },
	{ ACADEMY_GROUP, ACADEMY_HALL_1F, { -150, -42, FACE_SE, 5, 0x37,
		"The big kids have club today!|I'm gonna join when I'm big!" }, 0 },
};
const int place_nfolk = (int)(sizeof place_folk / sizeof *place_folk);

const char *const place_number_trader = "The Number Trader...|There's an \"Out of order\" sign on it.";
