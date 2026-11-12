/* netpass.h. The NetPass (issue #131, docs/online/netpass.md): what a
 * game's ROMs open online, as stamps. The Net Square is every game's; the
 * Arena, Trade Central and the week's DeepNet dive need the full set, every
 * game this build supports for online play: netpass.c's table, BN6's,
 * BN5 Team Colonel's and BN4 Red Sun's (v17), a future game one entry more. The server holds
 * the same rule (protocol.json's `roms_full`, NOTICE `notice_pass`). A
 * system that cannot read the whole set (v18: the New 3DS, one extra ROM)
 * holds a full pass with the games it can read, which trades on the
 * pair's common ground (netpass_peer_holds) and neither battles nor dives. */
#ifndef CW_NETPASS_H
#define CW_NETPASS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum { STAMP_SQUARE, STAMP_ARENA, STAMP_TRADE, STAMP_DEEPNET, STAMP_COUNT } NetStamp;

/* A game of the set: its cartridge's label and its title as the pass's
 * PET text names it, its bit in HELLO's ROMs (ONLINE_ROMS_*; 0 for BN6's,
 * which every game has) and the extra ROM it is (rom.h's XRomId; -1 for
 * BN6's own) */
typedef struct {
	const char *label, *name;
	uint8_t bit;
	int xrom;
} NetPassGame;

/* the set: how many games, each one, and whether this game has it */
int netpass_games(void);
const NetPassGame *netpass_game(int i);
bool netpass_game_in(int i);

/* A system that reads fewer extra ROMs than the set has games beside
 * BN6's (rom.h's xroms_most: the New 3DS, one): HELLO says so, and the
 * pass's rule goes by it */
bool netpass_one_extra(void);
/* ... whether game `i` is in, or the system has room for it yet (a slot
 * the pass shows; false for one this system could not read beside those in) */
bool netpass_game_fits(int i);

/* The full pass: every game of the set this system can read is in. All of
 * them; or, on a system of one extra ROM, one beside BN6's (protocol v18;
 * docs/online/netpass.md, A system with room for one game) */
bool netpass_full(void);
/* a stamp this game's pass holds: the Square's always; Trade's with the
 * full pass; the Arena's and the DeepNet's with every game of the set in,
 * as a NetBattle's two games build the same installs and every diver
 * dives the same net */
bool netpass_has(NetStamp stamp);
/* ... and how many of them */
int netpass_stamps(void);

/* another game's pass full, and a stamp it holds, by its ROMs as SNAPSHOT
 * says them (ONLINE_ROMS_ONE_EXTRA among them) */
bool netpass_peer_full(uint8_t roms);
bool netpass_peer_has(uint8_t roms, NetStamp stamp);

/* Whether a game with `roms` holds chip `id`, by the game whose ROM the
 * chip takes: BN4's from protocol.json's bn4_chip_first, else BN5's past
 * BN6's own ids and on its blank records, else one of BN6's, every game's
 * (the wire's numbers, as the server counts it: protocol.py's chip_game).
 * Two full passes need not hold the same games since protocol v18 (a
 * system of one extra ROM), so a trade keeps to the pair's common ground:
 * a chip both hold. The server refuses any other (NOTICE notice_ground) */
bool netpass_peer_holds(uint8_t roms, unsigned id);

#endif
