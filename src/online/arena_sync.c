/* The Arena's time sync (arena_sync.h). ROM-free. */
#include "arena_sync.h"

#include <math.h>
#include <string.h>

#define TICK_MS (1000.0 / 60.0)

void arena_sync_init(ArenaSync *s) { memset(s, 0, sizeof *s); }

void arena_sync_rtt(ArenaSync *s, uint32_t ms) {
	if (ms && (!s->rtt_ms || ms < s->rtt_ms)) s->rtt_ms = ms;
}

void arena_sync_arrived(ArenaSync *s, int32_t lag) {
	if (lag < INT16_MIN) lag = INT16_MIN;
	if (lag > INT16_MAX) lag = INT16_MAX;
	s->lag[s->at] = (int16_t)lag;
	s->at = (s->at + 1) % ARENA_SYNC_WINDOW;
	if (s->n < ARENA_SYNC_WINDOW) ++s->n;
}

unsigned arena_sync_target(const ArenaSync *s) { return (unsigned)ceil(s->rtt_ms / TICK_MS) + 1; }

bool arena_sync_least(const ArenaSync *s, int *lag) {
	if (s->n < ARENA_SYNC_JUDGED) return false;
	int least = INT16_MAX;
	for (unsigned i = 0; i < s->n; ++i)
		if (s->lag[i] < least) least = s->lag[i];
	*lag = least;
	return true;
}

void arena_sync_peer(ArenaSync *s, int lag) {
	if (lag < INT16_MIN) lag = INT16_MIN;
	if (lag > INT16_MAX) lag = INT16_MAX;
	s->peer_lag = (int16_t)lag;
	s->peer_known = true;
}

/* I lead: past the other's lag by ARENA_SYNC_EVEN once he has said it,
 * else past my own round trip and a tick */
static bool leads(const ArenaSync *s) {
	int least;
	if (s->since < (s->peer_known ? ARENA_SYNC_SETTLE : ARENA_SYNC_SPREAD) || !s->rtt_ms || !arena_sync_least(s, &least)) return false;
	return s->peer_known ? least - s->peer_lag >= ARENA_SYNC_EVEN : least > (int)arena_sync_target(s);
}

bool arena_sync_hold(ArenaSync *s, bool quiet) {
	++s->since;
	if (!leads(s)) { s->due = 0; return false; }
	if (!quiet && ++s->due < ARENA_SYNC_LOUD) return false;
	arena_sync_shift(s);
	--s->shifted;
	s->since = 0;
	s->due = 0;
	++s->held;
	return true;
}

void arena_sync_shift(ArenaSync *s) {
	/* (my clock a tick later, so the lags already seen a tick less by it,
	 * and the other's, as he said it, a tick short until he says it anew) */
	for (unsigned i = 0; i < s->n; ++i) --s->lag[i];
	++s->shifted;
	if (s->peer_known) s->since = 0;
}

void arena_pings_note(ArenaPings *p, uint32_t rtt_ms, uint32_t now_ms) {
	if (!rtt_ms || (p->last && now_ms - p->last < ARENA_SYNC_PING_MS)) return;
	p->ms[p->at] = (uint16_t)(rtt_ms > 5000 ? 5000 : rtt_ms);
	p->at = (p->at + 1) % ARENA_SYNC_PINGS;
	if (p->n < ARENA_SYNC_PINGS) ++p->n;
	p->last = now_ms ? now_ms : 1;
}

uint32_t arena_pings_latency(const ArenaPings *p) {
	if (!p->n) return 0;
	double sum = 0, squares = 0;
	for (unsigned i = 0; i < p->n; ++i) sum += p->ms[i];
	double mean = sum / p->n;
	for (unsigned i = 0; i < p->n; ++i) squares += (p->ms[i] - mean) * (p->ms[i] - mean);
	double said = mean + 2 * sqrt(squares / p->n);
	return said > 5000 ? 5000 : (uint32_t)ceil(said);
}

unsigned arena_window_for(double frame_ms) {
	const double budget = TICK_MS * 0.75, load = 0.1, save = 0.05;
	double per = frame_ms * 1.1 + save;
	if (!(frame_ms > 0) || budget <= load + 2 * per) return 0;
	double w = floor((budget - load) / per) - 1;
	return w >= ARENA_WINDOW_MOST ? ARENA_WINDOW_MOST : (unsigned)w;
}

uint32_t arena_ask_ms(uint32_t latency_ms, unsigned window) {
	/* (the window's frames but one: that one is the tick a key waits to be
	 * taken, which the server once added for every pair; no window, a
	 * frame more asked for it) */
	double ask = latency_ms - ((double)window - 1) * TICK_MS;
	return ask > 0 ? (uint32_t)ceil(ask) : 0;
}
