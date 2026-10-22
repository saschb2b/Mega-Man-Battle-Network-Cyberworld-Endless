/* The network's ArenaLink (arena_link_net.h). ROM-free. */
#include "arena_link_net.h"

#include <stdio.h>
#include <string.h>

#include "arena_wire.h"

static ArenaNetLink *of(ArenaLink *link) { return (ArenaNetLink *)(void *)link; }

static void breaks(ArenaNetLink *l, const char *why) {
	if (!l->broken[0]) snprintf(l->broken, sizeof l->broken, "%s", why);
}

/* the peer: the other GBA's player (a partner's, the other side's lead) */
static unsigned peer_seat(const ArenaNetLink *l) { return l->link.side ^ 1u; }
static unsigned my_seat(const ArenaNetLink *l) { return arena_link_seat(&l->link); }

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

static bool poll_seat(ArenaNetLink *l, unsigned seat, uint32_t frame, uint16_t *keys) {
	if (seat >= l->seats || seat == my_seat(l)) return false;
	ArenaNetSeat *s = &l->from[seat];
	if (frame >= s->next || frame < s->polled || s->next - frame > ARENA_NET_PEER) return false;
	*keys = s->keys[frame & (ARENA_NET_PEER - 1)];
	s->polled = frame + 1;
	return true;
}

static bool net_poll(ArenaLink *link, uint32_t frame, uint16_t *keys) {
	ArenaNetLink *l = of(link);
	return poll_seat(l, peer_seat(l), frame, keys);
}

static bool net_poll_player(ArenaLink *link, unsigned player, uint32_t frame, uint16_t *keys) {
	return poll_seat(of(link), player, frame, keys);
}

static void keep(ArenaNetHash *list, unsigned *n, uint32_t frame, uint64_t hash) {
	if (*n == ARENA_NET_HASHES) {
		memmove(list, list + 1, (ARENA_NET_HASHES - 1) * sizeof *list);
		--*n;
	}
	list[*n] = (ArenaNetHash){ frame, hash };
	++*n;
}

static bool find(const ArenaNetHash *list, unsigned n, uint32_t frame, uint64_t *hash) {
	for (unsigned i = 0; i < n; ++i)
		if (list[i].frame == frame) { *hash = list[i].hash; return true; }
	return false;
}

static void net_send_hash(ArenaLink *link, uint32_t frame, uint64_t hash) {
	ArenaNetLink *l = of(link);
	uint8_t data[ARENA_HASH_BYTES];
	arena_link_net_flush(l);
	keep(l->mine, &l->nmine, frame, hash);
	/* (a hash lost to a full outbox is no difference: the engine gives it up) */
	if (arena_wire_hash(data, l->match, frame, hash)) l->out(l->context, data, sizeof data);
}

/* every other seat's word for `frame` come: mine where all agree with
 * `mine`, else the first apart (a battle of two: the peer's) */
static bool agreed(const ArenaNetLink *l, uint64_t mine, uint64_t *out, bool (*word)(const ArenaNetSeat *, uint64_t *)) {
	uint64_t first = mine;
	bool apart = false;
	for (unsigned k = 0; k < l->seats; ++k) {
		uint64_t theirs;
		if (k == my_seat(l)) continue;
		if (!word(&l->from[k], &theirs)) return false;
		if (theirs != mine && !apart) { first = theirs; apart = true; }
	}
	*out = first;
	return true;
}

static bool net_poll_hash(ArenaLink *link, uint32_t frame, uint64_t *hash) {
	ArenaNetLink *l = of(link);
	if (l->seats == 2) return find(l->from[peer_seat(l)].hashes, l->from[peer_seat(l)].nhashes, frame, hash);
	uint64_t mine, first = 0;
	bool apart = false;
	if (!find(l->mine, l->nmine, frame, &mine)) return false;
	for (unsigned k = 0; k < l->seats; ++k) {
		uint64_t theirs;
		if (k == my_seat(l)) continue;
		if (!find(l->from[k].hashes, l->from[k].nhashes, frame, &theirs)) return false;
		if (theirs != mine && !apart) { first = theirs; apart = true; }
	}
	*hash = apart ? first : mine;
	return true;
}

static void net_send_lag(ArenaLink *link, int lag) {
	ArenaNetLink *l = of(link);
	uint8_t data[ARENA_HASH_BYTES];
	if (lag < INT16_MIN) lag = INT16_MIN;
	if (lag > INT16_MAX) lag = INT16_MAX;
	/* (a note lost to a full outbox: the next, a second on) */
	if (arena_wire_hash(data, l->match, ARENA_NET_LAG, (uint16_t)(int16_t)lag)) l->out(l->context, data, sizeof data);
}

/* (a lag note is about its sender's peer's keys: in a 2v2 it is about mine
 * only where I am that peer, a GBA's player, and it comes from the other
 * side; a partner's sync keeps to its round trip) */
static bool net_poll_lag(ArenaLink *link, int *lag) {
	ArenaNetLink *l = of(link);
	for (unsigned k = 0; k < l->seats; ++k) {
		ArenaNetSeat *s = &l->from[k];
		if (k == my_seat(l) || !s->lagged) continue;
		s->lagged = false;
		if (l->seats > 2 && (my_seat(l) >= 2 || (k & 1u) == (my_seat(l) & 1u))) continue;
		*lag = s->lag;
		return true;
	}
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
	l->link = (ArenaLink){ net_send, net_poll, net_send_hash, net_poll_hash, net_tick, net_broken, net_close, side & 1u, delay,
		net_send_lag, net_poll_lag, NULL, 0 };
	l->out = out;
	l->context = context;
	l->match = match;
	l->seats = 2;
	/* (the first D frames are released keys on every side: each stream begins at D) */
	l->sent = delay;
	for (unsigned k = 0; k < ONLINE_SEATS; ++k) l->from[k].next = l->from[k].polled = delay;
	return &l->link;
}

void arena_link_net_seats(ArenaNetLink *l, unsigned seat) {
	l->seats = ONLINE_SEATS;
	l->link.seat = seat < ONLINE_SEATS ? seat : 0;
	l->link.side = seat & 1u;
	l->link.poll_player = net_poll_player;
}

static void keep_hash(ArenaNetSeat *s, uint32_t frame, uint64_t hash) {
	if (frame == ARENA_NET_READY) { s->ready = true; s->ready_hash = hash; return; }
	if (frame == ARENA_NET_RECORD) { s->recorded = true; s->record = hash; return; }
	if (frame == ARENA_NET_LAG) { s->lagged = true; s->lag = (int16_t)(uint16_t)hash; return; }
	keep(s->hashes, &s->nhashes, frame, hash);
}

/* a message of seat `seat`'s */
static bool take_from(ArenaNetLink *l, unsigned seat, const uint8_t *data, size_t size) {
	uint32_t match, frame;
	uint64_t hash;
	uint16_t keys[ONLINE_ARENA_INPUT_MAX];
	unsigned count;
	if (seat >= l->seats || seat == my_seat(l)) return false;
	ArenaNetSeat *s = &l->from[seat];
	if (arena_wire_read_hash(data, size, &match, &frame, &hash)) {
		if (match != l->match) return false;
		keep_hash(s, frame, hash);
		return true;
	}
	if (!arena_wire_read_input(data, size, &match, &frame, keys, &count) || match != l->match) return false;
	/* (each stream in order, its frames once: a gap or a frame past what
	 * the link holds breaks it) */
	if (frame != s->next) { breaks(l, "the other player's keys came out of order"); return true; }
	if (s->next + count - s->polled > ARENA_NET_PEER) { breaks(l, "the other player's keys came too far ahead"); return true; }
	/* (a partner's stream keeps its sixteen bits: his picks, arena_team_custom.h) */
	uint16_t mask = seat < 2 ? ARENA_KEYS_MASK : ARENA_PARTNER_MASK;
	for (unsigned i = 0; i < count; ++i) s->keys[(frame + i) & (ARENA_NET_PEER - 1)] = keys[i] & mask;
	s->next += count;
	return true;
}

bool arena_link_net_take(ArenaNetLink *l, const uint8_t *data, size_t size) {
	unsigned seat;
	const uint8_t *inner;
	size_t n;
	if (arena_wire_seat(data, size, &seat, &inner, &n)) return l->seats > 2 && take_from(l, seat, inner, n);
	/* (a battle of two: the peer's, as relayed) */
	return l->seats == 2 && take_from(l, peer_seat(l), data, size);
}

bool arena_link_net_ready(ArenaNetLink *l, uint64_t hash) {
	uint8_t data[ARENA_HASH_BYTES];
	l->readied = true;
	l->ready_hash = hash;
	return arena_wire_hash(data, l->match, ARENA_NET_READY, hash) && l->out(l->context, data, sizeof data);
}

static bool ready_of(const ArenaNetSeat *s, uint64_t *hash) { if (s->ready) *hash = s->ready_hash; return s->ready; }
static bool record_of(const ArenaNetSeat *s, uint64_t *record) { if (s->recorded) *record = s->record; return s->recorded; }

bool arena_link_net_peer_ready(const ArenaNetLink *l, uint64_t *hash) {
	if (l->seats == 2) return ready_of(&l->from[peer_seat(l)], hash);
	return l->readied && agreed(l, l->ready_hash, hash, ready_of);
}

bool arena_link_net_seat_ready(const ArenaNetLink *l, unsigned seat) {
	return seat < l->seats && (seat == my_seat(l) ? l->readied : l->from[seat].ready);
}

bool arena_link_net_record(ArenaNetLink *l, uint64_t record) {
	uint8_t data[ARENA_HASH_BYTES];
	l->recorded = true;
	l->record = record;
	return arena_wire_hash(data, l->match, ARENA_NET_RECORD, record) && l->out(l->context, data, sizeof data);
}

bool arena_link_net_peer_record(const ArenaNetLink *l, uint64_t *record) {
	if (l->seats == 2) return record_of(&l->from[peer_seat(l)], record);
	return l->recorded && agreed(l, l->record, record, record_of);
}

void arena_link_net_ended(ArenaNetLink *l) { l->ended = true; }

void arena_link_net_lost(ArenaNetLink *l, const char *why) { breaks(l, why); }
