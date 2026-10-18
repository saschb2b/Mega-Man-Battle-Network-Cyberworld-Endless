/* BN6's Practice NetBattle on one linked pair of cores, as each client of the
 * Arena will run it (docs/ONLINE_LINK.md): the cable stays inside the pair,
 * and only the two players' inputs drive it. At SNAPSHOT_FRAME it takes a
 * complete snapshot of the pair, plays HORIZON frames, restores and plays
 * them again, and checks the replay's every frame against the first. The
 * player's ROM and a boot-3.state are only read; the flash is in memory. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "online_frame_pair_internal.h"
#include "online_link_script.h"
#include "online_replay_execute.h"
#include "online_replay_snapshot.h"

#define MAX_HORIZON 64

static bool frame_hash(OnlineFramePair *pair, uint64_t *hash) {
	uint32_t samples[2][8];
	return online_frame_pair_sample(pair, samples, hash);
}

static bool step(OnlineFramePair *pair) {
	uint16_t keys[ONLINE_FRAME_LOOKAHEAD][2];
	for (unsigned ahead = 0; ahead < ONLINE_FRAME_LOOKAHEAD; ++ahead)
		for (unsigned side = 0; side < 2; ++side)
			keys[ahead][side] = (uint16_t)online_frame_script_key(pair->frame + ahead, side);
	return online_frame_pair_step(pair, keys);
}

/* Plays HORIZON frames, rewinds to the snapshot and plays them again. */
static bool rewind_check(OnlineFramePair *pair, unsigned horizon, unsigned *mismatches, size_t *bytes) {
	OnlineReplayExecutor executor;
	online_replay_executor_init(&executor, pair);
	OnlineReplaySnapshot *snapshot = online_replay_snapshot_take(&executor);
	if (!snapshot) return false;
	*bytes = online_replay_snapshot_bytes(snapshot);
	uint64_t first[MAX_HORIZON], again;
	bool ok = true;
	for (unsigned i = 0; ok && i < horizon; ++i) ok = step(pair) && frame_hash(pair, &first[i]);
	ok = ok && online_replay_snapshot_restore(&executor, snapshot, REPLAY_RESTORE_ALL);
	for (unsigned i = 0; ok && i < horizon; ++i) {
		ok = step(pair) && frame_hash(pair, &again);
		if (ok && again != first[i]) ++*mismatches;
	}
	online_replay_snapshot_free(snapshot);
	return ok;
}

static bool number(const char *text, unsigned limit, unsigned *value) {
	char *end;
	errno = 0;
	unsigned long n = strtoul(text, &end, 10);
	if (errno || !*text || *end || n > limit) return false;
	*value = (unsigned)n;
	return true;
}

int main(int argc, char **argv) {
	unsigned at = 1150, horizon = 8, total = online_link_full_frames();
	if (argc < 4 || argc > 6 || (argc > 4 && !number(argv[4], total - MAX_HORIZON, &at)) ||
		(argc > 5 && (!number(argv[5], MAX_HORIZON, &horizon) || !horizon))) {
		fprintf(stderr, "usage: %s ROM BOOT_STATE PREFIX [SNAPSHOT_FRAME [HORIZON]]\n", argv[0]);
		return 2;
	}
	OnlineFramePair *pair = online_frame_pair_open(argv[1], argv[2], argv[3]);
	if (!pair) { fprintf(stderr, "cannot open the pair from %s and %s\n", argv[1], argv[2]); return 1; }
	unsigned mismatches = 0;
	size_t bytes = 0;
	bool ok = true;
	while (ok && pair->frame < total) {
		if (pair->frame == at) ok = rewind_check(pair, horizon, &mismatches, &bytes);
		else ok = step(pair);
	}
	uint32_t samples[2][8];
	uint64_t hash = 0;
	bool done = ok && online_frame_pair_sample(pair, samples, &hash);
	bool won = done && online_link_battle_agrees(samples[0], samples[1]);
	printf("frames %u, serial transfers %u\n", pair->frame, online_frame_pair_transfers(pair));
	printf("snapshot at frame %u: %zu bytes, %u of %u replayed frames differ\n", at, bytes, mismatches, horizon);
	printf("result: %s\n", won ? "side 0 won 100 to 0, both back at Comm" : "Practice did not finish as scripted");
	online_frame_pair_close(pair);
	return ok && won && !mismatches ? 0 : 1;
}
