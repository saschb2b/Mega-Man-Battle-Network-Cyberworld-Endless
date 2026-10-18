/* Trade Central's board as the game holds it (online_board.c, docs/
 * ONLINE.md, Trade Central: the board): the channel's offers as the
 * server last said (BOARD), each a chip its poster has and the kind of
 * chip he wants for it, one a player; MegaMan's own posted and taken
 * down (OFFER); and whether a chip meets a want, read from its record. */
#ifndef CW_ONLINE_BOARD_H
#define CW_ONLINE_BOARD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_trade.h"

/* The wants, a fixed list (protocol.json's want_last): the words are
 * trade_words.c's, the test online_board_meets. */
enum { WANT_ANY, WANT_FIRE, WANT_AQUA, WANT_ELEC, WANT_WOOD, WANT_SWORD, WANT_WIND, WANT_BREAK, WANT_CURSOR, WANT_MEGA, WANT_COUNT };

typedef struct {
	uint32_t player;   /* the poster's session */
	TradeChip have;
	uint8_t want;
} BoardOffer;

/* As MegaMan arrives in the Square: none. */
void online_board_reset(void);
/* A BOARD message come in; false where it is malformed. */
bool online_board_mail(const uint8_t *data, size_t size);
/* The board as last said, the poster's own among them; how many. */
int online_board_offers(const BoardOffer **out);
/* The offer of `player` up on the board, NULL none. */
const BoardOffer *online_board_of(uint32_t player);

/* OFFER: `have` up for `want`; taken down. */
bool online_board_post(TradeChip have, int want);
bool online_board_clear(void);

/* Whether `chip` is of the kind `want` names: its record's icon element,
 * or a Mega by its library type; anything meets WANT_ANY. */
bool online_board_meets(int want, TradeChip chip);

#endif
