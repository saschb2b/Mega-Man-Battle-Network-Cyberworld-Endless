/* The network's ArenaLink (arena_link_net.h). ROM-free. */
#include "arena_link_net.h"

#include <inttypes.h>
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
/* a battle of two (else a 2v2's four seats or a party's, each other seat's messages as ARENA_SEAT) */
static bool pair(const ArenaNetLink *l) { return l->seats == 2 && !l->party; }

/* my keys from `from` to the first not yet sent, ONLINE_ARENA_INPUT_MAX a message */
static bool send_from(ArenaNetLink *l, uint32_t from) {
	while (from != l->sent) {
		uint16_t keys[ONLINE_ARENA_INPUT_MAX];
		uint8_t data[ARENA_INPUT_HEAD + 2 * ONLINE_ARENA_INPUT_MAX];
		unsigned n = l->sent - from < ONLINE_ARENA_INPUT_MAX ? l->sent - from : ONLINE_ARENA_INPUT_MAX;
		for (unsigned i = 0; i < n; ++i) keys[i] = l->kept[(from + i) & (ARENA_NET_PEER - 1)];
		size_t size = arena_wire_input(data, sizeof data, l->match, from, keys, n);
		if (!size || !l->out(l->context, data, size)) break;
		from += n;
		if (from > l->flushed) l->flushed = from;
	}
	return from == l->sent;
}

bool arena_link_net_flush(ArenaNetLink *l) {
	return l->flushed == l->sent || (!l->held && send_from(l, l->flushed));
}

/* my keys again from `from` (an ask, a resume: v21): false where they are no longer kept */
static bool resend(ArenaNetLink *l, uint32_t from) {
	if (from >= l->sent) return true;
	if (from < l->link.delay) from = l->link.delay;
	if (l->sent - from > ARENA_NET_PEER) { breaks(l, "the other player's keys came back too late"); return false; }
	if (from < l->flushed) l->resent += l->flushed - from;
	if (from < l->flushed) l->flushed = from;
	arena_link_net_flush(l);
	return true;
}

static bool net_send(ArenaLink *link, uint32_t frame, uint16_t keys) {
	ArenaNetLink *l = of(link);
	if (frame != l->sent) return false;
	/* (held back past what is kept: a link down for that long has lost them) */
	if (l->sent - l->flushed >= ARENA_NET_PEER) { breaks(l, "my keys could not be sent"); return false; }
	l->kept[frame & (ARENA_NET_PEER - 1)] = keys;
	++l->sent;
	if (l->sent - l->flushed >= ARENA_NET_FLUSH) arena_link_net_flush(l);
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
		if (k == my_seat(l) || (l->gone >> k & 1u)) continue;
		if (!word(&l->from[k], &theirs)) return false;
		if (theirs != mine && !apart) { first = theirs; apart = true; }
	}
	*out = first;
	return true;
}

static bool net_poll_hash(ArenaLink *link, uint32_t frame, uint64_t *hash) {
	ArenaNetLink *l = of(link);
	if (pair(l)) return find(l->from[peer_seat(l)].hashes, l->from[peer_seat(l)].nhashes, frame, hash);
	uint64_t mine, first = 0;
	bool apart = false;
	if (!find(l->mine, l->nmine, frame, &mine)) return false;
	for (unsigned k = 0; k < l->seats; ++k) {
		uint64_t theirs;
		if (k == my_seat(l) || (l->gone >> k & 1u)) continue;
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
	++l->ticks;
	if (l->ended) ++l->since_end;
	/* (keys the outbox refused, tried again) */
	arena_link_net_flush(l);
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
	l->sent = l->flushed = delay;
	for (unsigned k = 0; k < ONLINE_SEATS; ++k) l->from[k].next = l->from[k].polled = delay;
	return &l->link;
}

void arena_link_net_seats(ArenaNetLink *l, unsigned seat) {
	l->seats = ONLINE_SEATS;
	l->link.seat = seat < ONLINE_SEATS ? seat : 0;
	l->link.side = seat & 1u;
	l->link.poll_player = net_poll_player;
}

void arena_link_net_party(ArenaNetLink *l, unsigned seat, unsigned n) {
	l->seats = n < 2 ? 2 : n > ONLINE_SEATS ? ONLINE_SEATS : n;
	l->party = true;
	l->link.seat = seat < l->seats ? seat : 0;
	l->link.side = l->link.seat & 1u;   /* (arena_link_seat: a seat below 2 by its side) */
	l->link.poll_player = net_poll_player;
}

void arena_link_net_gone(ArenaNetLink *l, unsigned seat) {
	if (seat < ONLINE_SEATS) l->gone |= 1u << seat;
}

/* seat `seat`'s keys asked for again from the first I lack (v21): at
 * once where `now`, else once ARENA_NET_ASK ticks have passed since the last */
static void ask(ArenaNetLink *l, unsigned seat, bool now) {
	ArenaNetSeat *s = &l->from[seat];
	uint8_t data[ARENA_HASH_BYTES];
	if (l->held || (!now && s->asked && l->ticks - s->asked_at < ARENA_NET_ASK)) return;
	if (!arena_wire_hash(data, l->match, ARENA_NET_HAVE, (uint64_t)seat << 32 | s->next) || !l->out(l->context, data, sizeof data)) return;
	s->asked = true;
	s->asked_at = l->ticks;
	++l->asks;
}

static void keep_hash(ArenaNetLink *l, ArenaNetSeat *s, uint32_t frame, uint64_t hash) {
	/* (an ask for my keys again, v21: a 2v2's or a party's for mine alone) */
	if (frame == ARENA_NET_HAVE) { if ((hash >> 32 & 0xFu) == my_seat(l)) resend(l, (uint32_t)hash); return; }
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
		keep_hash(l, s, frame, hash);
		return true;
	}
	if (!arena_wire_read_input(data, size, &match, &frame, keys, &count) || match != l->match) return false;
	/* (each stream in order, its frames once: a gap's keys dropped and the
	 * first missing asked for again, frames already taken skipped, v21; a
	 * frame past what the link holds breaks it) */
	if (frame > s->next) { ask(l, seat, false); return true; }
	unsigned skip = s->next - frame;
	if (skip >= count) return true;
	if (s->next + (count - skip) - s->polled > ARENA_NET_PEER) { breaks(l, "the other player's keys came too far ahead"); return true; }
	/* (a partner's stream keeps its sixteen bits: his picks, arena_team_custom.h; a party's seats 1-3 each a partner) */
	uint16_t mask = (l->party ? seat >= 1 : seat >= 2) ? ARENA_PARTNER_MASK : ARENA_KEYS_MASK;
	for (unsigned i = skip; i < count; ++i) s->keys[(frame + i) & (ARENA_NET_PEER - 1)] = keys[i] & mask;
	s->next += count - skip;
	s->asked = false;
	return true;
}

bool arena_link_net_take(ArenaNetLink *l, const uint8_t *data, size_t size) {
	unsigned seat;
	const uint8_t *inner;
	size_t n;
	if (arena_wire_seat(data, size, &seat, &inner, &n)) return !pair(l) && take_from(l, seat, inner, n);
	/* (a battle of two: the peer's, as relayed) */
	return pair(l) && take_from(l, peer_seat(l), data, size);
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
	if (pair(l)) return ready_of(&l->from[peer_seat(l)], hash);
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
	if (pair(l)) return record_of(&l->from[peer_seat(l)], record);
	return l->recorded && agreed(l, l->record, record, record_of);
}

void arena_link_net_ended(ArenaNetLink *l) { l->ended = true; }

void arena_link_net_lost(ArenaNetLink *l, const char *why) { breaks(l, why); }

void arena_link_net_hold(ArenaNetLink *l, bool down) { l->held = down; }

/* (the first frame every other seat's game surely had of mine: after the
 * last hash it sent, which it took up only with my keys to that frame) */
static uint32_t surely_had(const ArenaNetLink *l) {
	uint32_t from = l->sent;
	for (unsigned k = 0; k < l->seats; ++k) {
		const ArenaNetSeat *s = &l->from[k];
		if (k == my_seat(l) || (l->gone >> k & 1u)) continue;
		uint32_t had = s->nhashes ? s->hashes[s->nhashes - 1].frame + 1 : l->link.delay;
		if (had < from) from = had;
	}
	/* (a seat that sent no hash for that long: what is kept, its gap asked for if need be) */
	return l->sent - from > ARENA_NET_PEER ? l->sent - ARENA_NET_PEER : from;
}

void arena_link_net_resumed(ArenaNetLink *l) {
	l->held = false;
	uint32_t from = surely_had(l);
	printf("arena: the link back: my keys again from frame %" PRIu32 " (%" PRIu32 " sent)\n", from, l->sent);
	if (!resend(l, from)) return;
	for (unsigned k = 0; k < l->seats; ++k)
		if (k != my_seat(l) && !(l->gone >> k & 1u)) ask(l, k, true);
}
