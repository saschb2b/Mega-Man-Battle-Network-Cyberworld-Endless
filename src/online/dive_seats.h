/* Who of a layer's divers gets one of the puppets it has room for (the
 * shared dive, issue #755; docs/online/shared-dive.md, Room for the
 * crew): MegaMan's crew first, then the nearest. A diver keeps his puppet
 * while he stays among those chosen, so no MegaMan jumps from one diver
 * to another as they walk. Pure, no game: tests/test_online.c. */
#ifndef CW_DIVE_SEATS_H
#define CW_DIVE_SEATS_H

#include <stdbool.h>
#include <stdint.h>

/* One diver in the layer's room: his session, where he stands (world
 * units) and whether he is in MegaMan's crew. */
typedef struct {
	uint32_t id;
	int x, y;
	bool crew;
} SeatDiver;

/* `seat[k]`: the session puppet k shows (0 none), as it was; rewritten
 * for the room's `n` divers, MegaMan at (mx, my). How many of them are
 * left without one. */
int dive_seats(uint32_t *seat, int seats, const SeatDiver *d, int n, int mx, int my);

#endif
