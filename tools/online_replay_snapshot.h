#ifndef CW_ONLINE_REPLAY_SNAPSHOT_H
#define CW_ONLINE_REPLAY_SNAPSHOT_H

#include <stddef.h>
#include "online_replay_execute.h"

typedef struct OnlineReplaySnapshot OnlineReplaySnapshot;
typedef enum {
	REPLAY_RESTORE_ALL, REPLAY_OMIT_CABLE, REPLAY_OMIT_FLASH,
	REPLAY_BAD_CREDIT, REPLAY_BAD_RECEIVE, REPLAY_BAD_EVENT
} OnlineReplayFault;

OnlineReplaySnapshot *online_replay_snapshot_take(OnlineReplayExecutor *executor);
bool online_replay_snapshot_restore(OnlineReplayExecutor *executor, const OnlineReplaySnapshot *snapshot, OnlineReplayFault fault);
void online_replay_snapshot_free(OnlineReplaySnapshot *snapshot);
size_t online_replay_snapshot_bytes(const OnlineReplaySnapshot *snapshot);

#endif
