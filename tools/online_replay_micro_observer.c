/* No restore implementation: first prove the instruction-boundary executor
 * matches the separately controlled uninterrupted original pair. */
#include <stdio.h>
#include <mgba/internal/gba/gba.h>
#include "online_frame_pair_internal.h"
#include "online_link_script.h"
#include "online_replay_execute.h"
#include "online_replay_observe.h"

typedef struct { FILE *file; bool written, failed; } Boundary;

static bool observe_cable(OnlineReplayExecutor *e, void *context) {
	Boundary *boundary = context;
	OnlineFramePair *p = e->pair;
	if (boundary->written || p->cable.d.transferActive == TRANSFER_IDLE) return false;
	uint32_t samples[2][8];
	for (unsigned side = 0; side < 2; ++side) online_link_battle_sample(p->core[side], side, samples[side], NULL);
	if (!online_link_battle_active(samples[0], samples[1])) return false;
	bool scheduled = false;
	for (unsigned side = 0; side < 2; ++side)
		scheduled |= mTimingIsScheduled(&((struct GBA *)p->core[side]->board)->timing, &p->node[side].event);
	if (!scheduled) return false;
	boundary->written = true;
	boundary->failed = !online_replay_write_boundary(e, boundary->file);
	return false; /* The observer never pauses or restores the original. */
}

int main(int argc, char **argv) {
	if (argc != 4) { fprintf(stderr, "usage: %s ROM BOOT_STATE PREFIX\n", argv[0]); return 2; }
	OnlineFramePair *pair = online_frame_pair_open(argv[1], argv[2], argv[3]);
	if (!pair) return 1;
	char path[1024];
	FILE *out = NULL, *cable = NULL;
	if (snprintf(path, sizeof path, "%s.jsonl", argv[3]) < (int)sizeof path) out = fopen(path, "w");
	if (snprintf(path, sizeof path, "%s-cable.json", argv[3]) < (int)sizeof path) cable = fopen(path, "w");
	OnlineReplayExecutor executor;
	online_replay_executor_init(&executor, pair);
	Boundary boundary = {.file = cable};
	bool ok = out && cable && online_replay_observe(pair, out);
	while (ok && pair->frame < online_link_full_frames())
		ok = online_replay_execute_frame(&executor, observe_cable, &boundary) && online_replay_observe(pair, out);
	ok = ok && boundary.written && !boundary.failed;
	if (out && fclose(out)) ok = false;
	if (cable && fclose(cable)) ok = false;
	online_frame_pair_close(pair);
	return ok ? 0 : 1;
}
