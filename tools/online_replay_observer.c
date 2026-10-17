/* Original, uninterrupted linked Practice pair with a full numeric trace.
 * This executable has no snapshot or restore implementation. */
#include <stdio.h>
#include "online_frame_pair.h"
#include "online_link_script.h"
#include "online_replay_observe.h"

int main(int argc, char **argv) {
	if (argc != 4) {
		fprintf(stderr, "usage: %s ROM BOOT_STATE PREFIX\n", argv[0]);
		return 2;
	}
	OnlineFramePair *pair = online_frame_pair_open(argv[1], argv[2], argv[3]);
	if (!pair) return 1;
	char path[1024];
	FILE *output = NULL;
	if (snprintf(path, sizeof path, "%s.jsonl", argv[3]) < (int)sizeof path) output = fopen(path, "w");
	bool ok = output && online_replay_observe(pair, output);
	for (unsigned frame = 0; ok && frame < online_link_full_frames(); ++frame) {
		uint16_t keys[ONLINE_FRAME_LOOKAHEAD][2];
		for (unsigned ahead = 0; ahead < ONLINE_FRAME_LOOKAHEAD; ++ahead)
			for (unsigned side = 0; side < 2; ++side)
				keys[ahead][side] = (uint16_t)online_frame_script_key(frame + ahead, side);
		ok = online_frame_pair_step(pair, keys) && online_replay_observe(pair, output);
	}
	if (output && fclose(output)) ok = false;
	online_frame_pair_close(pair);
	return ok ? 0 : 1;
}
