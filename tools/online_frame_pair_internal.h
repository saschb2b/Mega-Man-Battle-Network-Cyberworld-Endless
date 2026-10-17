#ifndef CW_ONLINE_FRAME_PAIR_INTERNAL_H
#define CW_ONLINE_FRAME_PAIR_INTERNAL_H

#include "online_frame_pair.h"

#include <stdio.h>
#include <mgba/core/core.h>
#include <mgba/internal/gba/sio/lockstep.h>

/* Experiment-only state shared with the separately pinned replay observer.
 * Pointers are live wiring, never snapshot data or a network format. */
struct OnlineFramePair {
	struct GBASIOLockstep cable;
	struct GBASIOLockstepNode node[2];
	struct mCore *core[2];
	color_t video[2][240 * 160];
	uint8_t *state[2];
	size_t state_bytes[2];
	int32_t credit[2];
	unsigned waiting, transfers;
	int running;
	unsigned traced[2];
	unsigned start_frame[2], frame;
	unsigned entry_step, entry_frame;
	FILE *trace;
};

#endif
