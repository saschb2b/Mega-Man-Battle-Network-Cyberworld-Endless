#ifndef ONLINE_LINK_IPC_H
#define ONLINE_LINK_IPC_H

#include <stdbool.h>
#include <stdint.h>

struct GBASIOLockstep;
struct mCore;

enum OnlineLinkIpcStatus {
	ONLINE_LINK_IPC_COMPLETE, ONLINE_LINK_IPC_DEADLINE,
	ONLINE_LINK_IPC_DISCONNECTED, ONLINE_LINK_IPC_DESYNC,
	ONLINE_LINK_IPC_PROTOCOL, ONLINE_LINK_IPC_TIMEOUT
};

struct OnlineLinkIpcConfig {
	int32_t initial_credit[2];
	unsigned initial_waiting;
	unsigned frames, delay_ms, delay_after_frames, timeout_ms, wall_limit_ms;
	unsigned disconnect_after, corrupt_after;
	const char *trace_path;
	void (*frame)(struct mCore *, unsigned side, unsigned frame, void *context);
	void (*sample)(struct mCore *, unsigned side, uint32_t out[8], void *context);
	bool (*delay_ready)(const uint32_t left[8], const uint32_t right[8]);
	void *context;
};

struct OnlineLinkIpcResult {
	enum OnlineLinkIpcStatus status;
	int pids[2];
	unsigned frames[2], transfers[2], sample[2][8];
	uint64_t transcript[2], elapsed_ms, delayed_transfers, applied_delay_ms;
	unsigned delay_start_frames[2], delay_start_transfers[2];
	uint64_t delay_start_elapsed_ms;
};

/* POSIX research harness: fork one independently executing core per process.
 * Scheduler lock/credit messages are local IPC. delay_ms is inserted before
 * releasing each serial transfer's master words to the slave, exactly once
 * per transfer. No RAM, ROM or emulator state is serialized over the channel.
 * The parent's cores are unchanged, so callers may repeat from the same point.
 */
bool online_link_ipc_run(struct GBASIOLockstep *cable, struct mCore *cores[2],
	const struct OnlineLinkIpcConfig *config, struct OnlineLinkIpcResult *result);
const char *online_link_ipc_status(enum OnlineLinkIpcStatus status);

#endif
