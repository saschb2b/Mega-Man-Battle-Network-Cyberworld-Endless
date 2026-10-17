#ifndef CW_ONLINE_REPLAY_OBSERVE_H
#define CW_ONLINE_REPLAY_OBSERVE_H

#include <stdbool.h>
#include <stdio.h>
#include "online_frame_pair.h"

/* Observe a stopped pair; no restoration or target mutation. */
bool online_replay_observe(OnlineFramePair *pair, FILE *output);

#endif
