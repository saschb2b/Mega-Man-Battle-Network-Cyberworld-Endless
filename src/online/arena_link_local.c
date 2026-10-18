/* The local link (arena_link.h): a peer on this machine whose messages
 * take a set number of ticks, as a network's would. ROM-free. */
#include "arena_link.h"

#include <string.h>

static ArenaLocalLink *of(ArenaLink *link) { return (ArenaLocalLink *)(void *)link; }

static bool local_send(ArenaLink *link, uint32_t frame, uint16_t keys) {
	ArenaLocalLink *l = of(link);
	(void)keys;   /* (the peer is a script: what I send goes nowhere) */
	if (frame != l->sent) return false;
	++l->sent;
	return true;
}

static bool local_poll(ArenaLink *link, uint32_t frame, uint16_t *keys) {
	ArenaLocalLink *l = of(link);
	if (l->head == l->tail) return false;
	uint32_t i = l->head & (ARENA_LOCAL_QUEUE - 1);
	if (l->queue[i].at > l->now || l->queue[i].frame != frame) return false;
	*keys = l->queue[i].keys;
	++l->head;
	return true;
}

static void local_send_hash(ArenaLink *link, uint32_t frame, uint64_t hash) {
	ArenaLocalLink *l = of(link);
	if (l->nhashes == ARENA_LOCAL_HASHES) {
		memmove(l->hashes, l->hashes + 1, sizeof l->hashes - sizeof *l->hashes);
		--l->nhashes;
	}
	l->hashes[l->nhashes].at = l->now + l->latency;
	l->hashes[l->nhashes].frame = frame;
	l->hashes[l->nhashes].hash = hash;
	++l->nhashes;
}

static bool local_poll_hash(ArenaLink *link, uint32_t frame, uint64_t *hash) {
	ArenaLocalLink *l = of(link);
	for (unsigned i = 0; i < l->nhashes; ++i)
		if (l->hashes[i].frame == frame && l->hashes[i].at <= l->now) { *hash = l->hashes[i].hash; return true; }
	return false;
}

/* the peer samples its keys for frame now + delay, and they set out */
static void local_tick(ArenaLink *link) {
	ArenaLocalLink *l = of(link);
	if (l->tail - l->head >= ARENA_LOCAL_QUEUE) { l->lost = true; return; }
	uint32_t frame = l->now + l->link.delay;
	uint32_t i = l->tail++ & (ARENA_LOCAL_QUEUE - 1);
	l->queue[i].at = l->now + l->latency;
	l->queue[i].frame = frame;
	l->queue[i].keys = l->peer ? (uint16_t)(l->peer(l->peer_context, frame) & ARENA_KEYS_MASK) : 0;
	++l->now;
}

static const char *local_broken(ArenaLink *link) {
	return of(link)->lost ? "the local link's queue overflowed" : NULL;
}

static void local_close(ArenaLink *link) { (void)link; }

ArenaLink *arena_link_local(ArenaLocalLink *l, unsigned delay, unsigned latency, ArenaPeerKeys peer, void *context) {
	memset(l, 0, sizeof *l);
	if (delay < ARENA_DELAY_MIN) delay = ARENA_DELAY_MIN;
	if (delay > ARENA_DELAY_MAX) delay = ARENA_DELAY_MAX;
	l->link = (ArenaLink){ local_send, local_poll, local_send_hash, local_poll_hash, local_tick, local_broken, local_close, 0, delay };
	l->latency = latency;
	l->peer = peer;
	l->peer_context = context;
	l->sent = delay;
	return &l->link;
}
