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

bool arena_sync_hold(ArenaSync *s) {
	if (++s->since < ARENA_SYNC_SPREAD || s->n < ARENA_SYNC_JUDGED || !s->rtt_ms) return false;
	int least = INT16_MAX;
	for (unsigned i = 0; i < s->n; ++i)
		if (s->lag[i] < least) least = s->lag[i];
	if (least <= (int)arena_sync_target(s)) return false;
	arena_sync_shift(s);
	--s->shifted;
	s->since = 0;
	++s->held;
	return true;
}

void arena_sync_shift(ArenaSync *s) {
	/* (my clock a tick later, so the lags already seen a tick less by it) */
	for (unsigned i = 0; i < s->n; ++i) --s->lag[i];
	++s->shifted;
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
