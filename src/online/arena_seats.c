/* Two MegaMen a side over the network (arena_seats.h). */
#include "arena_seats.h"

#include <stdio.h>

#include "arena_team_custom.h"
#include "arena_team_navi.h"
#include "devtools.h"

static bool four;

bool arena_seats_on(void) { return four || dev.arena_2v2 > 0; }

void arena_seats_set(bool on) { four = on; }

void arena_seats_partners(const OnlineLoadout *partner[2]) {
	for (unsigned side = 0; side < 2; ++side) {
		const OnlineLoadout *l = partner[side];
		if (!l) {
			arena_team_custom_folder(side, NULL, 0, NULL);
			arena_team_navi_loadout(side, 0, 0, NULL);
			continue;
		}
		arena_team_custom_folder(side, l->folder, l->regular, l->tag);
		const uint8_t buster[3] = { l->attack, l->speed, l->charge };
		arena_team_navi_loadout(side, l->max_hp, l->base_hp, buster);
		printf("arena: side %u's partner: HP %u, buster %u %u %u, the folder's first chips %04x %04x %04x\n", side, l->max_hp,
			l->attack, l->speed, l->charge, l->folder[0], l->folder[1], l->folder[2]);
	}
}
