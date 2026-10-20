/* netpass.h. */
#include "netpass.h"

#include <stddef.h>

#include "protocol.h"
#include "rom.h"

/* the full set: every game this build supports for online play */
static const NetPassGame set[] = {
	{ "BN6", "Cybeast Gregar", 0, -1 },
	{ "BN5", "Team Colonel", ONLINE_ROMS_BN5, XROM_BN5_COLONEL_US },
};
#define SET_GAMES ((int)(sizeof set / sizeof *set))

int netpass_games(void) { return SET_GAMES; }

const NetPassGame *netpass_game(int i) { return i >= 0 && i < SET_GAMES ? &set[i] : NULL; }

bool netpass_game_in(int i) {
	const NetPassGame *g = netpass_game(i);
	if (!g) return false;
	return g->xrom < 0 || (g->xrom < XROM_COUNT && XR[g->xrom].data != NULL);
}

bool netpass_full(void) {
	for (int i = 0; i < SET_GAMES; ++i)
		if (!netpass_game_in(i)) return false;
	return true;
}

bool netpass_has(NetStamp stamp) { return stamp == STAMP_SQUARE || (stamp < STAMP_COUNT && netpass_full()); }

bool netpass_peer_full(uint8_t roms) { return (roms & ONLINE_ROMS_FULL) == ONLINE_ROMS_FULL; }

int netpass_held(void) {
	int n = 0;
	for (int s = 0; s < STAMP_COUNT; ++s) n += netpass_has((NetStamp)s);
	return n;
}
