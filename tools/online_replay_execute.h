#ifndef CW_ONLINE_REPLAY_EXECUTE_H
#define CW_ONLINE_REPLAY_EXECUTE_H

#include "online_frame_pair.h"
#include <stdio.h>

/* Replay-only cursor. A stopped instruction boundary is after both core
 * opportunities, with at_check true and the frame-target check still due. */
typedef struct {
	OnlineFramePair *pair;
	unsigned target[2], relative, round;
	bool battle, in_frame, at_check;
} OnlineReplayExecutor;

typedef bool (*OnlineReplayStop)(OnlineReplayExecutor *executor, void *context);

void online_replay_executor_init(OnlineReplayExecutor *executor, OnlineFramePair *pair);
bool online_replay_execute_frame(OnlineReplayExecutor *executor, OnlineReplayStop stop, void *context);
bool online_replay_write_boundary(OnlineReplayExecutor *executor, FILE *output);

#endif
