/* Plain-ROM experiment only: two local cores, no production Arena hooks. */
#include "online_frame_pair.h"
#include "online_frame_wire.h"
#include "online_link_script.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool report_frame(int fd, unsigned side, unsigned frame, OnlineFramePair *pair, const char *prefix) {
	OnlineFramePacket p;
	online_frame_packet(&p, ONLINE_FRAME_STATE, frame, side);
	uint32_t samples[2][8];
	uint64_t hash;
	if (!online_frame_pair_sample(pair, samples, &hash)) return false;
	p.words[FRAME_VALUE] = online_frame_pair_transfers(pair);
	p.words[FRAME_HASH_LOW] = (uint32_t)hash;
	p.words[FRAME_HASH_HIGH] = (uint32_t)(hash >> 32);
	for (unsigned i = 0; i < 2; ++i)
		for (unsigned j = 0; j < 8; ++j) p.words[FRAME_SAMPLE + i * 8 + j] = samples[i][j];
	p.words[FRAME_AGREES] = (unsigned)online_link_battle_agrees(samples[0], samples[1]);
	if (!online_frame_send(fd, &p, 5000)) return false;
	unsigned entry = online_link_entry_frames();
	if (frame + 1 == entry || frame + 1 == entry + 900 || frame + 1 == online_link_full_frames())
		return online_frame_pair_shot(pair, prefix, frame + 1);
	return true;
}

static bool worker(int fd, unsigned side, OnlineFramePair *pair, const char *prefix) {
	OnlineFrameQueue queue = {0};
	uint16_t held_keys[ONLINE_FRAME_LOOKAHEAD][2] = {{0}};
	unsigned held = 0;
	OnlineFramePacket p;
	online_frame_packet(&p, ONLINE_FRAME_HELLO, online_link_full_frames(), side);
	p.words[FRAME_VALUE] = (unsigned)getpid();
	p.words[FRAME_HASH_LOW] = ONLINE_FRAME_LOOKAHEAD;
	if (!online_frame_send(fd, &p, 5000)) return false;
	while (queue.consumed < online_link_full_frames()) {
		if (!online_frame_receive(fd, &p, 5000)) return false;
		unsigned kind = p.words[FRAME_KIND], frame = p.words[FRAME_SEQUENCE], owner = p.words[FRAME_SIDE];
		if (kind == ONLINE_FRAME_STOP) return true;
		if (frame >= online_link_full_frames()) return false;
		if (kind == ONLINE_FRAME_TICK && owner == side) {
			unsigned keys = online_frame_script_key(frame, side);
			if (!online_frame_queue_push(&queue, side, frame, keys)) return false;
			online_frame_packet(&p, ONLINE_FRAME_INPUT, frame, side);
			p.words[FRAME_VALUE] = keys;
			if (!online_frame_send(fd, &p, 5000)) return false;
		} else if (kind == ONLINE_FRAME_INPUT && owner == (side ^ 1u)) {
			if (!online_frame_queue_push(&queue, owner, frame, p.words[FRAME_VALUE])) return false;
		} else return false;
		uint16_t keys[2];
		while (online_frame_queue_pop(&queue, keys)) {
			memcpy(held_keys[held++], keys, sizeof keys);
			if (held < ONLINE_FRAME_LOOKAHEAD) continue;
			if (!online_frame_pair_step(pair, held_keys) ||
				!report_frame(fd, side, queue.consumed - held, pair, prefix)) return false;
			--held;
			memmove(held_keys, held_keys + 1, held * sizeof *held_keys);
		}
	}
	while (held) {
		memset(held_keys + held, 0, (ONLINE_FRAME_LOOKAHEAD - held) * sizeof *held_keys);
		if (!online_frame_pair_step(pair, held_keys) ||
			!report_frame(fd, side, queue.consumed - held, pair, prefix)) return false;
		--held;
		memmove(held_keys, held_keys + 1, held * sizeof *held_keys);
	}
	return true;
}

static bool unsigned_arg(const char *text, unsigned limit, unsigned *value) {
	char *end;
	errno = 0;
	unsigned long number = strtoul(text, &end, 10);
	if (errno || !*text || *end || number > limit) return false;
	*value = (unsigned)number;
	return true;
}

int main(int argc, char **argv) {
	unsigned side, fd;
	if (argc != 6 || !unsigned_arg(argv[3], 1, &side) || !unsigned_arg(argv[4], INT_MAX, &fd)) {
		fprintf(stderr, "usage: %s ROM BOOT_STATE SIDE SOCKET_FD OUTPUT_PREFIX\n", argv[0]);
		return 2;
	}
	OnlineFramePair *pair = online_frame_pair_open(argv[1], argv[2], argv[5]);
	bool ok = pair && worker((int)fd, side, pair, argv[5]);
	if (!ok) fprintf(stderr, "frame worker %u: invalid/closed/stalled input or core failure (errno=%d)\n", side, errno);
	online_frame_pair_close(pair);
	close((int)fd);
	return ok ? 0 : 1;
}
