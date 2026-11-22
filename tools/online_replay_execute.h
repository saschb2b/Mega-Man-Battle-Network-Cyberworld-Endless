#ifndef CW_ONLINE_REPLAY_EXECUTE_H
#define CW_ONLINE_REPLAY_EXECUTE_H

#include "online_frame_pair.h"

/* The pair stepped one scheduler round at a time, so a snapshot can be taken
 * inside a frame: a stopped boundary is after both cores' turns, with at_check
 * true and the frame-target check still due. */
typedef struct {
	OnlineFramePair *pair;
	unsigned target[2], relative, round;
	bool battle, in_frame, at_check;
} OnlineReplayExecutor;

typedef bool (*OnlineReplayStop)(OnlineReplayExecutor *executor, void *context);

void online_replay_executor_init(OnlineReplayExecutor *executor, OnlineFramePair *pair);
bool online_replay_execute_frame(OnlineReplayExecutor *executor, OnlineReplayStop stop, void *context);

#endif
