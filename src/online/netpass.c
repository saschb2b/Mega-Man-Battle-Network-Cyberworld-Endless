/* netpass.h. */
#include "netpass.h"

#include <stddef.h>

#include "bn6_chips.h"
#include "protocol.h"
#include "rom.h"

/* the full set: every game this build supports for online play */
static const NetPassGame set[] = {
	{ "BN6", "Cybeast Gregar", 0, -1 },
	{ "BN5", "Team Colonel", ONLINE_ROMS_BN5, XROM_BN5_COLONEL_US },
	{ "BN4", "Red Sun", ONLINE_ROMS_BN4, XROM_BN4_RED_SUN_US },   /* (protocol v17, the owner's call of 9 November 2026) */
};
#define SET_GAMES ((int)(sizeof set / sizeof *set))

int netpass_games(void) { return SET_GAMES; }

const NetPassGame *netpass_game(int i) { return i >= 0 && i < SET_GAMES ? &set[i] : NULL; }

bool netpass_game_in(int i) {
	const NetPassGame *g = netpass_game(i);
	if (!g) return false;
	return g->xrom < 0 || (g->xrom < XROM_COUNT && XR[g->xrom].data != NULL);
}

/* every game of the set in */
static bool whole(void) {
	for (int i = 0; i < SET_GAMES; ++i)
		if (!netpass_game_in(i)) return false;
	return true;
}

/* the set's games beside BN6's that are in */
static int extras_in(void) {
	int n = 0;
	for (int i = 0; i < SET_GAMES; ++i) n += set[i].xrom >= 0 && netpass_game_in(i);
	return n;
}

bool netpass_one_extra(void) { return xroms_short(); }

bool netpass_game_fits(int i) {
	const NetPassGame *g = netpass_game(i);
	return g && (netpass_game_in(i) || extras_in() < xroms_most);
}

bool netpass_full(void) { return whole() || (netpass_one_extra() && extras_in() >= xroms_most && extras_in() > 0); }

bool netpass_has(NetStamp stamp) {
	if (stamp == STAMP_SQUARE) return true;
	/* (a trade keeps to the pair's common ground, so a full pass trades; a NetBattle's installs and the week's dive are the
	 * same on every game only with every game of the set in) */
	if (stamp == STAMP_TRADE) return netpass_full();
	return stamp < STAMP_COUNT && whole();
}

int netpass_stamps(void) {
	int n = 0;
	for (int s = 0; s < STAMP_COUNT; ++s) n += netpass_has((NetStamp)s);
	return n;
}

bool netpass_peer_full(uint8_t roms) {
	return (roms & ONLINE_ROMS_FULL) == ONLINE_ROMS_FULL || ((roms & ONLINE_ROMS_ONE_EXTRA) && (roms & ONLINE_ROMS_FULL));
}

bool netpass_peer_has(uint8_t roms, NetStamp stamp) {
	if (stamp == STAMP_SQUARE) return true;
	if (stamp == STAMP_TRADE) return netpass_peer_full(roms);
	return stamp < STAMP_COUNT && (roms & ONLINE_ROMS_FULL) == ONLINE_ROMS_FULL;
}

uint8_t netpass_chip_game(unsigned id) {
	if (id >= ONLINE_BN4_CHIP_FIRST) return ONLINE_ROMS_BN4;
	return id > ONLINE_BN6_CHIP_LAST || BN6_CHIP_BLANK(id) ? ONLINE_ROMS_BN5 : 0;
}

bool netpass_peer_holds(uint8_t roms, unsigned id) {
	uint8_t game = netpass_chip_game(id);
	return !game || (roms & game);
}
