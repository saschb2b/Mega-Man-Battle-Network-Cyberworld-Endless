/* Network ownership stays off the game thread. Snapshots are copies. */
#ifndef CW_ONLINE_TRANSPORT_H
#define CW_ONLINE_TRANSPORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "protocol.h"

typedef struct {
	uint32_t id;
	int32_t x, y, z;
	uint8_t facing;
	char name[ONLINE_NAME_BYTES + 1];
} OnlinePeer;

typedef struct {
	bool connected;
	uint32_t seed, player_id, latency_ms;
	size_t peer_count;
	OnlinePeer peers[ONLINE_MAX_PLAYERS];
	char error[128];
} OnlineSnapshot;

/* URL must be ws(s)://host[:port]/presence. No implicit retry or fallback.
 * enabled remains true after a refusal, until end, so the online square
 * cannot accidentally become a normal run. */
bool online_transport_start(const char *url, const char *name);
bool online_transport_enabled(void);
void online_transport_poll(int32_t x, int32_t y, int32_t z, uint8_t facing);
void online_transport_snapshot(OnlineSnapshot *out);
void online_transport_end(void);

#endif
