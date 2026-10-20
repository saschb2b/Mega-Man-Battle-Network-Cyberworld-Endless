/* netpass.h. The NetPass (issue #131, docs/ONLINE.md, The NetPass): what a
 * game's ROMs open online, as stamps. The Net Square is every game's; the
 * Arena, Trade Central and the week's DeepNet dive need the full set, every
 * game this build supports for online play: netpass.c's table, BN6's and
 * BN5 Team Colonel's today, a future game one entry more. The server holds
 * the same rule (protocol.json's `roms_full`, NOTICE `notice_pass`). */
#ifndef CW_NETPASS_H
#define CW_NETPASS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum { STAMP_SQUARE, STAMP_ARENA, STAMP_TRADE, STAMP_DEEPNET, STAMP_COUNT } NetStamp;

/* A game of the set: its cartridge's label, its bit in HELLO's ROMs
 * (ONLINE_ROMS_*; 0 for BN6's, which every game has) and the extra ROM it
 * is (rom.h's XRomId; -1 for BN6's own) */
typedef struct {
	const char *label;
	uint8_t bit;
	int xrom;
} NetPassGame;

/* the set: how many games, each one, and whether this game has it */
int netpass_games(void);
const NetPassGame *netpass_game(int i);
bool netpass_game_in(int i);

/* every game of the set in: the full pass */
bool netpass_full(void);
/* a stamp this game's pass holds: the Square's always, the rest with the
 * full pass */
bool netpass_has(NetStamp stamp);
/* the stamps held, 1 to STAMP_COUNT */
int netpass_held(void);

#endif
