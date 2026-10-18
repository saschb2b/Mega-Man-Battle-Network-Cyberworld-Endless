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
bool online_transport_enabled(void);
void online_transport_poll(int32_t x, int32_t y, int32_t z, uint8_t facing);
void online_transport_snapshot(OnlineSnapshot *out);
bool online_transport_profile(const OnlineProfile *profile);
bool online_transport_inspect(uint32_t player_id);
bool online_transport_social(uint8_t kind, uint8_t value);
bool online_transport_channel(uint16_t channel);
void online_transport_end(void);

#endif
