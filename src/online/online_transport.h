/* Network ownership stays off the game thread. Snapshots are copies. */
#ifndef CW_ONLINE_TRANSPORT_H
#define CW_ONLINE_TRANSPORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_types.h"

/* URL must be ws(s)://host[:port]/presence. No implicit retry or fallback.
 * enabled remains true after a refusal, until end, so the online square
 * cannot accidentally become a normal run. */
bool online_transport_start(const char *url, const char *name, const uint8_t identity[ONLINE_IDENTITY_BYTES]);
void online_transport_poll(int32_t x, int32_t y, int32_t z, uint8_t facing);
void online_transport_snapshot(OnlineSnapshot *out);
bool online_transport_profile(const OnlineProfile *profile);
bool online_transport_inspect(uint32_t player_id);
bool online_transport_social(uint8_t kind, uint8_t value);
bool online_transport_channel(uint16_t channel);
/* Trade Central's and the Arena's messages (TRADE, ARENA and its relayed
 * LOADOUT, INPUT and HASH; shared/online/README.md): sent within the
 * worker's next pass (about 2 ms), not a frame's queue; false where the
 * outbox is full or the kind is none of them. */
bool online_transport_send(const uint8_t *data, size_t size);
/* ... and theirs come in (TRADE_STATE, ARENA_STATE and the opponent's
 * LOADOUT, INPUT and HASH): the next one's bytes into `out`, its size;
 * 0 none. */
size_t online_transport_receive(uint8_t *out, size_t cap);
void online_transport_end(void);

#endif
