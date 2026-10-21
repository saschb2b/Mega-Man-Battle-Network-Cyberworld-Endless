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

static bool local_poll_player(ArenaLink *link, unsigned player, uint32_t frame, uint16_t *keys) {
	ArenaLocalLink *l = of(link);
	if (player >= ARENA_PLAYERS || l->head[player] == l->tail[player]) return false;
	uint32_t i = l->head[player] & (ARENA_LOCAL_QUEUE - 1);
	if (l->queue[player][i].at > l->now || l->queue[player][i].frame != frame) return false;
	*keys = l->queue[player][i].keys;
	++l->head[player];
	return true;
}

static bool local_poll(ArenaLink *link, uint32_t frame, uint16_t *keys) { return local_poll_player(link, 1, frame, keys); }

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

/* a remote player's keys for `frame` set out, there from tick now + latency on */
static void local_queue(ArenaLocalLink *l, unsigned player, uint32_t frame, uint16_t keys) {
	if (l->tail[player] - l->head[player] >= ARENA_LOCAL_QUEUE) { l->lost = true; return; }
	uint32_t i = l->tail[player]++ & (ARENA_LOCAL_QUEUE - 1);
	l->queue[player][i].at = l->now + l->latency;
	l->queue[player][i].frame = frame;
	l->queue[player][i].keys = (uint16_t)(keys & (player < 2 ? ARENA_KEYS_MASK : ARENA_PARTNER_MASK));
}

/* tick `now` begun: the peer (and the partners, where they play) sample
 * their keys for frame now + delay, and they set out */
static void local_tick(ArenaLink *link) {
	ArenaLocalLink *l = of(link);
	if (l->started) ++l->now;
	l->started = true;
	uint32_t frame = l->now + l->link.delay;
	local_queue(l, 1, frame, l->peer ? l->peer(l->peer_context, frame) : 0);
	for (unsigned p = 2; l->partners && p < ARENA_PLAYERS; ++p) local_queue(l, p, frame, l->partners(l->partners_context, p, frame));
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
	l->link.poll_player = local_poll_player;
	l->latency = latency;
	l->peer = peer;
	l->peer_context = context;
	l->sent = delay;
	return &l->link;
}

void arena_link_local_partners(ArenaLocalLink *l, ArenaPartnerKeys partners, void *context) {
	l->partners = partners;
	l->partners_context = context;
}
