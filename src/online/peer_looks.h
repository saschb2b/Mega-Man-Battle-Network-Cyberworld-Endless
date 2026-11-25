/* The look each player of the Square is known by (issue #854, protocol
 * v30): the latest SNAPSHOT's, kept by session and by public identity for
 * as long as he was last seen, so the Arena's pairing reads the look of a
 * player it pairs with (a NetBattle's two sides, docs/game/looks.md). */
#ifndef CW_PEER_LOOKS_H
#define CW_PEER_LOOKS_H

#include <stdbool.h>
#include <stdint.h>

#include "looks.h"
#include "online_types.h"

/* The peers of a SNAPSHOT noted (the visit's every frame). */
void peer_looks_note(const OnlineSnapshot *snapshot);
/* Player `session`'s look as the Square last showed him: false, and
 * `out` BN6's own (look_plain), where he was never seen. */
bool peer_looks_of(uint32_t session, Look *out);
/* Forgotten: a visit begun. */
void peer_looks_reset(void);

#endif
