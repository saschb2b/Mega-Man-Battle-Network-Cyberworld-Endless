/* Trade Central's trade as the game holds it (online_trade.c, docs/
 * ONLINE.md): its stage as the server last said (TRADE_STATE), the spare
 * chips MegaMan can offer, and a swap the server committed, kept in a file
 * of its own until the run has it. The chats around it are
 * trade_central.c's. */
#ifndef CW_ONLINE_TRADE_H
#define CW_ONLINE_TRADE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "protocol.h"

typedef struct { uint16_t id; uint8_t code; } TradeChip;   /* code 0-25 A-Z, 26 * */

/* The server's stages (server/cyberworld_server/trade.py) */
enum { TRADE_NONE, TRADE_OFFERED, TRADE_WAITING, TRADE_ANSWERED, TRADE_ACCEPTED, TRADE_DONE, TRADE_CANCELLED };

typedef struct {
	int stage;
	uint32_t id, other;          /* the trade's, the other player's session */
	uint8_t identity[ONLINE_IDENTITY_BYTES];
	TradeChip mine, theirs;      /* id 0: not chosen yet */
	bool taken;                  /* begun answered: an offer on the board taken, by either */
	uint32_t serial;             /* counts the stages said */
} TradeView;

/* As MegaMan arrives in the Square: no trade. */
void online_trade_reset(void);
/* A TRADE_STATE message come in; false where it is malformed. */
bool online_trade_mail(const uint8_t *data, size_t size);
const TradeView *online_trade_view(void);

/* Whether this game holds `c`: one of BN6's own, or one of BN5's where
 * its ROM is read; in a code of its record's. */
bool online_trade_holds(TradeChip c);

/* The wire (protocol.json's TRADE): to `other` with `give`; an answer
 * with `give` for `get` as shown; the pair accepted as shown; called off;
 * the board's offer of `poster`, his `get`, taken with `give` (the trade
 * comes back answered, both chips named). */
bool online_trade_propose(uint32_t other, TradeChip give);
bool online_trade_take(uint32_t poster, TradeChip give, TradeChip get);
bool online_trade_answer(TradeChip give);
bool online_trade_accept(void);
bool online_trade_cancel(void);

/* MegaMan's spare chips in the running game: the Pack's copies past those
 * in his three folders (the pack's extension's too), the chips the pair
 * can both hold: BN6's own, and BN5's where `bn5` (both games have its
 * ROM); at most `cap` into `out` (stock in `count`), how many; BN5's kept
 * back for want of `bn5` counted in `held_back`. */
int online_trade_spares(TradeChip *out, uint8_t *count, int cap, bool bn5, int *held_back);

/* A swap the server committed, written at once to the data folder
 * (trade.pending: what MegaMan gives and gets), there until the run has
 * it; online_trade_settle gives it to the run: its chip taken from the
 * Pack and the other's given, BN6's own TakeChips and GiveChips, on the
 * checkpoint just loaded, which the caller then saves. True where a swap
 * was settled (the caller saves, then online_trade_settled). */
bool online_trade_keep(const TradeView *done);
bool online_trade_pending(TradeChip *give, TradeChip *get);
/* ... MegaMan's word on it where it was settled, else NULL */
const char *online_trade_settle(void);
void online_trade_settled(void);

#endif
