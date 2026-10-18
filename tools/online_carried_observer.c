/* Original carried-chip control only. There is no loadout installer here. */
#include "online_carried_observe.h"

#include <stdio.h>
#include <string.h>
#include "bn6.h"
#include "online_carried_read.h"
#include "online_carried_script.h"
#include "online_frame_pair_internal.h"
#include "online_link_script.h"
#include "online_replay_observe.h"

static FILE *output(const char *prefix, const char *suffix) {
	char path[1024];
	if (snprintf(path, sizeof path, "%s%s", prefix, suffix) >= (int)sizeof path) return NULL;
	return fopen(path, "w");
}

static bool seed(OnlineFramePair *p, const char *prefix, bool no_write) {
	static const uint32_t address[] = {BN6_RNG_PRIMARY, BN6_RNG_SECONDARY};
	static const uint32_t value[] = {2622147170u, 2129837214u};
	FILE *out = output(prefix, "-seed.json");
	if (!out) return false;
	fputc('[', out);
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *c = p->core[side];
		unsigned mode = c->rawRead8(c, c->rawRead32(c, BN6_TOOLKIT, -1), -1);
		if (mode != BN6_MODE_GAME) { fclose(out); return false; }
		fprintf(out, "%s{\"core\":%u,\"main\":%u,\"frame\":%u,\"before\":[%u,%u],", side ? "," : "", side, mode,
			c->frameCounter(c), c->rawRead32(c, address[0], -1), c->rawRead32(c, address[1], -1));
		if (!no_write) for (unsigned i = 0; i < 2; ++i) c->rawWrite32(c, address[i], -1, value[i]);
		fprintf(out, "\"after\":[%u,%u],\"writes\":%u}", c->rawRead32(c, address[0], -1),
			c->rawRead32(c, address[1], -1), no_write ? 0 : 1);
	}
	fputs("]\n", out);
	return !fclose(out);
}

static bool run(OnlineFramePair *p, const char *prefix, bool no_write) {
	OnlineLoadout loadout;
	if (!online_carried_read(p->core[0], &loadout)) return false;
	FILE *captured = output(prefix, "-loadout.json");
	if (!captured) return false;
	bool ok = online_carried_write(captured, &loadout);
	if (fclose(captured) || !ok || !seed(p, prefix, no_write)) return false;
	OnlineCarriedObserver *observer = online_carried_observer_open(p, prefix);
	FILE *full = output(prefix, "-full.jsonl");
	if (!observer || !full) {
		online_carried_observer_close(observer);
		if (full) fclose(full);
		return false;
	}
	ok = online_replay_observe(p, full);
	for (unsigned frame = 0; ok && frame < online_link_full_frames(); ++frame) {
		uint16_t keys[ONLINE_FRAME_LOOKAHEAD][2];
		for (unsigned ahead = 0; ahead < ONLINE_FRAME_LOOKAHEAD; ++ahead)
			for (unsigned side = 0; side < 2; ++side) keys[ahead][side] = (uint16_t)online_frame_script_key(frame + ahead, side);
		ok = online_frame_pair_step(p, keys) && online_carried_observer_flush(observer) && online_replay_observe(p, full);
		if (ok && (frame + 1 == online_link_entry_frames() || frame + 1 == online_link_entry_frames() + 340 ||
			frame + 1 == online_link_full_frames())) ok = online_frame_pair_shot(p, prefix, frame + 1);
	}
	if (fclose(full)) ok = false;
	online_carried_observer_close(observer);
	return ok;
}

int main(int argc, char **argv) {
	if ((argc != 4 && argc != 5) || (argc == 5 && strcmp(argv[4], "--no-seed") && strcmp(argv[4], "--no-chip"))) {
		fprintf(stderr, "usage: %s ROM NATIVE_MAP_STATE OUTPUT_PREFIX [--no-seed|--no-chip]\n", argv[0]); return 2;
	}
	if (argc == 5 && !strcmp(argv[4], "--no-chip")) online_carried_script_no_chip();
	OnlineFramePair *pair = online_frame_pair_open(argv[1], argv[2], argv[3]);
	bool ok = pair && run(pair, argv[3], argc == 5 && !strcmp(argv[4], "--no-seed"));
	online_frame_pair_close(pair);
	return ok ? 0 : 1;
}
