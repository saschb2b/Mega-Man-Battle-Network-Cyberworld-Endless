/* place_lines.h. Who is in home's indoor places on a day without class
 * (docs/HOME.md, pieces 6 and 10; docs/VOICE.md): AsterLand's clerk,
 * BN6's own, behind the counter where BN6 stands him, the Order Service's,
 * and the SubChip seller beside him, a NetBattler at the request board and
 * a shopper at the rare chips; the Academy's NetBattle club in
 * class 6-1, where BN6 stands Lan's classmates, and a first grader on the
 * first floor. Of the game's people in sprite list 5 none of Central
 * Town's own (town_lines.c), who are out in the town. The NetBattler and
 * the club's member post requests (jobs.h, job_words.c). */
#include "place_lines.h"

#include "academy.h"
#include "aster_land.h"
#include "jobs.h"
#include "shop.h"

const PlaceFolk place_folk[] = {
	/* (BN6's own clerk, its list 6 0x33, list 5 0x53 here: the Order
	 * Service's window has his face; its SubChip sellers' the man in the
	 * white coat's, list 5 0x30) */
	{ ASTER_GROUP, ASTER_LAND, { 28, -70, FACE_SE, 5, 0x53,
		"Welcome to AsterLand!|Any chip you've had,I can order!|One order a visit,OK?" }, 10, 0, SHOP_ORDER + 1 },
	{ ASTER_GROUP, ASTER_LAND, { -4, -70, FACE_SE, 5, 0x30, "SubChips! Energy and keys!|Take a look!" }, 10, 0, SHOP_SUBS_HOME + 1 },
	{ ASTER_GROUP, ASTER_LAND, { -92, -40, FACE_SE, 5, 0x35,
		"Hmm... The request board's empty.|Doesn't anybody need a NetBattler?" }, 0, JOB_BOARD + 1 },
	/* (at the NEW case beside the Chip Trader: before the counter she
	 * stood in the SubChip seller's way, session 69) */
	{ ASTER_GROUP, ASTER_LAND, { 60, -20, FACE_NW, 5, 0x40,
		"Ooh... Look at these rare chips!|...And look at these prices! *gulp*" }, 0 },
	{ ACADEMY_GROUP, ACADEMY_CLASS_6_1, { -148, 2, FACE_NE, 5, 0x3B,
		"Lan! Welcome to the club!|Tip of the day! Match your codes!|Same code,and you can send more chips!" }, 0, JOB_CLUB + 1 },
	{ ACADEMY_GROUP, ACADEMY_CLASS_6_1, { -130, -2, FACE_SW, 5, 0x42,
		"My Navi lost to a Mettaur again...|*sigh* Don't tell anyone,OK?" }, 0 },
	{ ACADEMY_GROUP, ACADEMY_HALL_1F, { -150, -42, FACE_SE, 5, 0x37,
		"The big kids have club today!|I'm gonna join when I'm big!" }, 0 },
};
const int place_nfolk = (int)(sizeof place_folk / sizeof *place_folk);

const char *const place_number_trader = "The Number Trader...|There's an \"Out of order\" sign on it.";

const char *const place_order_closed = "That's your order for today!|Come back next time,Lan!";
