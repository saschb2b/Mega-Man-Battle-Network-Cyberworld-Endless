/* The network's ArenaLink (arena_link_net.h). ROM-free. */
#include "arena_link_net.h"

#include <stdio.h>
#include <string.h>

#include "arena_wire.h"

static ArenaNetLink *of(ArenaLink *link) { return (ArenaNetLink *)(void *)link; }

static void breaks(ArenaNetLink *l, const char *why) {
	if (!l->broken[0]) snprintf(l->broken, sizeof l->broken, "%s", why);
}

bool arena_link_net_flush(ArenaNetLink *l) {
	if (!l->batched) return true;
	uint8_t data[ARENA_INPUT_HEAD + 2 * ONLINE_ARENA_INPUT_MAX];
	size_t size = arena_wire_input(data, sizeof data, l->match, l->batch_first, l->batch, l->batched);
	if (!size || !l->out(l->context, data, size)) return false;
	l->batch_first += l->batched;
	l->batched = 0;
	return true;
}

static bool net_send(ArenaLink *link, uint32_t frame, uint16_t keys) {
	ArenaNetLink *l = of(link);
	if (frame != l->sent || l->batched == ONLINE_ARENA_INPUT_MAX) return false;
	if (!l->batched) l->batch_first = frame;
	l->batch[l->batched++] = keys;
	++l->sent;
	/* (the outbox full: kept, and tried again with the next frame's) */
	if (l->batched >= ARENA_NET_FLUSH && !arena_link_net_flush(l) && l->batched == ONLINE_ARENA_INPUT_MAX)
		breaks(l, "my keys could not be sent");
	return true;
}

static bool net_poll(ArenaLink *link, uint32_t frame, uint16_t *keys) {
	ArenaNetLink *l = of(link);
	if (frame >= l->peer_next || frame < l->polled || l->peer_next - frame > ARENA_NET_PEER) return false;
	*keys = l->peer[frame & (ARENA_NET_PEER - 1)];
	l->polled = frame + 1;
	return true;
}

static void net_send_hash(ArenaLink *link, uint32_t frame, uint64_t hash) {
	ArenaNetLink *l = of(link);
	uint8_t data[ARENA_HASH_BYTES];
	arena_link_net_flush(l);
	/* (a hash lost to a full outbox is no difference: the engine gives it up) */
	if (arena_wire_hash(data, l->match, frame, hash)) l->out(l->context, data, sizeof data);
}

static bool net_poll_hash(ArenaLink *link, uint32_t frame, uint64_t *hash) {
	ArenaNetLink *l = of(link);
	for (unsigned i = 0; i < l->nhashes; ++i)
		if (l->hashes[i].frame == frame) { *hash = l->hashes[i].hash; return true; }
	return false;
}

static void net_tick(ArenaLink *link) {
	ArenaNetLink *l = of(link);
	if (l->ended) ++l->since_end;
	/* (a batch the outbox refused, tried again) */
	if (l->batched >= ARENA_NET_FLUSH) arena_link_net_flush(l);
}

/* the peer's end: its keys already come are spent first, as its battle
 * was over on the same frame as mine will be */
static const char *net_broken(ArenaLink *link) {
	ArenaNetLink *l = of(link);
	if (!l->broken[0] && l->ended && l->since_end > ARENA_NET_GRACE) breaks(l, "the other player left the NetBattle");
	return l->broken[0] ? l->broken : NULL;
}

static void net_close(ArenaLink *link) { (void)link; }

ArenaLink *arena_link_net(ArenaNetLink *l, uint32_t match, unsigned side, unsigned delay, ArenaSend out, void *context) {
	memset(l, 0, sizeof *l);
	if (delay < ARENA_DELAY_MIN) delay = ARENA_DELAY_MIN;
	if (delay > ARENA_DELAY_MAX) delay = ARENA_DELAY_MAX;
	l->link = (ArenaLink){ net_send, net_poll, net_send_hash, net_poll_hash, net_tick, net_broken, net_close, side & 1u, delay };
	l->out = out;
	l->context = context;
	l->match = match;
	/* (the first D frames are released keys on both sides: each stream begins at D) */
	l->sent = l->peer_next = l->polled = delay;
	return &l->link;
}

static void keep_hash(ArenaNetLink *l, uint32_t frame, uint64_t hash) {
	if (frame == ARENA_NET_READY) { l->peer_ready = true; l->peer_ready_hash = hash; return; }
	if (l->nhashes == ARENA_NET_HASHES) {
		memmove(l->hashes, l->hashes + 1, sizeof l->hashes - sizeof *l->hashes);
		--l->nhashes;
	}
	l->hashes[l->nhashes].frame = frame;
	l->hashes[l->nhashes].hash = hash;
	++l->nhashes;
}

bool arena_link_net_take(ArenaNetLink *l, const uint8_t *data, size_t size) {
	uint32_t match, frame;
	uint64_t hash;
	uint16_t keys[ONLINE_ARENA_INPUT_MAX];
	unsigned count;
	if (arena_wire_read_hash(data, size, &match, &frame, &hash)) {
		if (match != l->match) return false;
		keep_hash(l, frame, hash);
		return true;
	}
	if (!arena_wire_read_input(data, size, &match, &frame, keys, &count) || match != l->match) return false;
	/* (each stream in order, its frames once: a gap or a frame past what
	 * the link holds breaks it) */
	if (frame != l->peer_next) { breaks(l, "the other player's keys came out of order"); return true; }
	if (l->peer_next + count - l->polled > ARENA_NET_PEER) { breaks(l, "the other player's keys came too far ahead"); return true; }
	for (unsigned i = 0; i < count; ++i) l->peer[(frame + i) & (ARENA_NET_PEER - 1)] = keys[i] & ARENA_KEYS_MASK;
	l->peer_next += count;
	return true;
}

bool arena_link_net_ready(ArenaNetLink *l, uint64_t hash) {
	uint8_t data[ARENA_HASH_BYTES];
	return arena_wire_hash(data, l->match, ARENA_NET_READY, hash) && l->out(l->context, data, sizeof data);
}

bool arena_link_net_peer_ready(const ArenaNetLink *l, uint64_t *hash) {
	if (l->peer_ready) *hash = l->peer_ready_hash;
	return l->peer_ready;
}

void arena_link_net_ended(ArenaNetLink *l) { l->ended = true; }

void arena_link_net_lost(ArenaNetLink *l, const char *why) { breaks(l, why); }
