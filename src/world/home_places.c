/* home_places.h. Home's indoor places side by side: Lan's house and room
 * (lan_house.c), AsterLand (aster_land.c), the Cyber Academy (academy.c),
 * each BN6's own maps taken over (indoors.c). */
#include "home_places.h"

#include <stddef.h>

#include "academy.h"
#include "aster_land.h"
#include "bn6_fields.h"
#include "indoors.h"
#include "job_words.h"
#include "lan_house.h"
#include "npc.h"
#include "place_lines.h"

int home_places_at(int group, int number) {
	if (lan_house_map(group, number)) return number == LAN_ROOM ? HOME_PLACE_ROOM : HOME_PLACE_HOUSE;
	if (aster_land_map(group, number)) return HOME_PLACE_ASTER;
	if (academy_map(group, number)) return HOME_PLACE_ACADEMY;
	return HOME_PLACE_NONE;
}

bool home_places_install(int to_group, int to_number, int x, int y) {
	return lan_house_install(to_group, to_number, x, y) && aster_land_install() && academy_install();
}

void home_places_shops(void) { aster_land_shops(); }

const char *home_places_name(int group, int number) {
	static const char *const names[] = { NULL, "Lan's Room", "Lan's House", "AsterLand", "Cyber Academy" };
	return names[home_places_at(group, number)];
}

bool home_places_way(int group, int number, int *x, int *y) {
	switch (home_places_at(group, number)) {
	case HOME_PLACE_ROOM: case HOME_PLACE_HOUSE: return lan_house_way(number, bn6_player_x(), bn6_player_y(), x, y);
	case HOME_PLACE_ASTER: return aster_land_door(x, y);
	case HOME_PLACE_ACADEMY: return academy_way_out(number, x, y);
	default: return false;
	}
}

void home_places_frame(int group, int number) {
	int at = home_places_at(group, number);
	if (at == HOME_PLACE_HOUSE || at == HOME_PLACE_ROOM) lan_house_frame(number);
	else if (at == HOME_PLACE_ASTER) aster_land_frame();
	else if (at == HOME_PLACE_ACADEMY) academy_frame(number);
}

bool home_places_aster_front(int *x, int *y) { return aster_land_front(x, y); }

bool home_places_door(int group, int number, int *x, int *y) {
	return home_places_at(group, number) == HOME_PLACE_HOUSE && lan_house_door(x, y);
}

bool home_places_counter(int group, int number, int x, int y, int face, uint32_t *archive, int *script) {
	int dx, dy;
	npc_probe(face, &dx, &dy);
	/* (facing the counter, its front towards +y: not along it) */
	return dy < 0 && home_places_at(group, number) == HOME_PLACE_ASTER && indoors_counter(group, number, x + dx, y + dy, archive, script);
}

const char *home_places_check(int group, int number, int x, int y, int face) {
	int dx, dy;
	npc_probe(face, &dx, &dy);
	if (!aster_land_map(group, number)) return NULL;
	if (aster_land_number_trader(x + dx, y + dy)) return place_number_trader;
	return aster_land_board(x + dx, y + dy) ? job_board_words() : NULL;
}
