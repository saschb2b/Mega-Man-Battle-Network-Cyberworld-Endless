/* The report of a NetBattle for the stands (arena_report.h). */
#include "arena_report.h"

#include <stdio.h>
#include <string.h>

#include <mgba/core/core.h>

#include "bn6.h"
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

static void send(void) {
	uint8_t m[ONLINE_ARENA_STATUS_BYTES];
	if (!stands_wire_status(m, Q.match, &Q.now) || !online_transport_send(m, sizeof m)) return;
	Q.sent = Q.now;
	Q.sent_at = Q.frames;
}

void arena_report_frame(const Arena *a) {
	struct mCore *c = a ? arena_core(a, 0) : NULL;
	if (!Q.match || !c) return;
	++Q.frames;
	for (unsigned side = 0; side < 2; ++side) {
		uint32_t o = c->rawRead32(c, BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP, -1);
		if (o < BN6_EWRAM || o >= BN6_EWRAM_END - BN6_T1_SIZE) continue;
		stands_report_side(&Q.now, &Q.seen, side, (uint16_t)c->rawRead16(c, o + BN6_T1_HP, -1),
			(uint16_t)c->rawRead16(c, o + BN6_T1_MAX_HP, -1), (uint8_t)c->rawRead8(c, o + BN6_T1_ACTION, -1),
			(uint16_t)c->rawRead16(c, o + BN6_T1_CHIP, -1));
	}
	if (Q.frames - Q.sent_at >= REPORT_EVERY && memcmp(&Q.now, &Q.sent, sizeof Q.now)) send();
}

void arena_report_end(unsigned result) {
	if (!Q.match) return;
	Q.now.winner = result == 1 ? 1 : result == 2 ? 2 : 0;
	if (Q.now.winner) send();
	printf("arena: the report: HP %u/%u and %u/%u, chips %u and %u (last %u and %u), winner %u\n", Q.now.hp[0], Q.now.max_hp[0],
		Q.now.hp[1], Q.now.max_hp[1], Q.now.used[0], Q.now.used[1], Q.now.chip[0], Q.now.chip[1], Q.now.winner);
	Q.match = 0;
}

