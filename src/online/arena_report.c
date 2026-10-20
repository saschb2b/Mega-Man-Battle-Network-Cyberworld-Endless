/* The report of a NetBattle for the stands (arena_report.h). */
#include "arena_report.h"

#include <stdio.h>
#include <string.h>

#include "online_transport.h"

#define REPORT_EVERY 30            /* frames between two reports at most */

static struct {
	uint32_t match, frames, sent_at;
	ArenaReport now, sent;
	ReportSeen seen;
} Q;

void arena_report_begin(uint32_t match) {
	memset(&Q, 0, sizeof Q);
	Q.match = match;
}

static void report_send(void) {
	uint8_t m[ONLINE_ARENA_STATUS_BYTES];
	if (!stands_wire_status(m, Q.match, &Q.now) || !online_transport_send(m, sizeof m)) return;
	Q.sent = Q.now;
	Q.sent_at = Q.frames;
}

void arena_report_frame(const Arena *a) {
	const ArenaReading *r = a ? arena_reading(a) : NULL;
	if (!Q.match || !r) return;
	++Q.frames;
	for (unsigned side = 0; side < 2; ++side)
		if (r->seen[side]) stands_report_side(&Q.now, &Q.seen, side, r->hp[side], r->max_hp[side], r->action[side], r->chip[side]);
	if (Q.frames - Q.sent_at >= REPORT_EVERY && memcmp(&Q.now, &Q.sent, sizeof Q.now)) report_send();
}

void arena_report_end(unsigned result) {
	if (!Q.match) return;
	Q.now.winner = result == 1 ? 1 : result == 2 ? 2 : 0;
	if (Q.now.winner) report_send();
	printf("arena: the report: HP %u/%u and %u/%u, chips %u and %u (last %u and %u), winner %u\n", Q.now.hp[0], Q.now.max_hp[0],
		Q.now.hp[1], Q.now.max_hp[1], Q.now.used[0], Q.now.used[1], Q.now.chip[0], Q.now.chip[1], Q.now.winner);
	Q.match = 0;
}

