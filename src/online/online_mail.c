#include "online_mail.h"

#include <stdio.h>

#include "dive_link.h"
#include "online_board.h"
#include "online_trade.h"
#include "online_week.h"
#include "online_transport.h"
#include "square_arena.h"
#include "square_team.h"
#include "square_watch.h"

void online_mail_frame(void) {
	uint8_t m[ONLINE_MAX_MESSAGE];
	size_t n;
	while ((n = online_transport_receive(m, sizeof m)) != 0) {
		bool ok = true;
		/* (the shared dive's, protocol v19's 42 to 53: dive_link.c) */
		if (m[0] >= ONLINE_DIVE_ROOM && m[0] <= ONLINE_WEEK_SPLITS) { dive_link_mail(m, n); continue; }
		switch (m[0]) {
		case ONLINE_TRADE_STATE: ok = online_trade_mail(m, n); break;
		case ONLINE_BOARD: ok = online_board_mail(m, n); break;
		case ONLINE_WEEK_BOARD: ok = online_week_mail(m, n); break;
		case ONLINE_ARENA_STATE: case ONLINE_ARENA_LOADOUT: case ONLINE_ARENA_INPUT: case ONLINE_ARENA_HASH: case ONLINE_ARENA_SEAT:
			/* (a dive's party battle, else the Square's Arena) */
			if (dive_link_on()) dive_link_mail(m, n);
			else square_arena_mail(m, n);
			break;
		case ONLINE_STANDS: case ONLINE_WATCH_STATE: case ONLINE_WATCH_RELAY: case ONLINE_WATCH_KEYS:
			square_watch_mail(m, n);
			break;
		case ONLINE_TEAM_STATE: square_team_mail(m, n); break;
		default: break;
		}
		if (!ok) fprintf(stderr, "online: a malformed message of kind %u set aside\n", m[0]);
	}
}
