/* Phase 0's disposable Net Square, built and drawn by BN6's own map engine. */
#ifndef CW_ONLINE_SQUARE_H
#define CW_ONLINE_SQUARE_H

#include <stdbool.h>
#include <stdint.h>

#include "online_transport.h"

bool online_square_build(uint32_t seed, uint32_t player_id);
bool online_square_arrived(void);
uint32_t online_square_hash(void);
/* The scripts are changed between emulator frames; BN6 performs the move. */
void online_square_peers(const OnlineSnapshot *snapshot);
/* Where BN6 actually placed this peer's NPC last frame, for the spike's trace. */
bool online_square_peer_position(unsigned slot, int *x, int *y, int *z);

#endif
