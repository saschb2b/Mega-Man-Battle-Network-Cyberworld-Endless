#include "online_mail.h"

#include <stdio.h>

#include "online_trade.h"
#include "online_transport.h"
#include "square_arena.h"

void online_mail_frame(void) {
	uint8_t m[ONLINE_MAX_MESSAGE];
	size_t n;
	while ((n = online_transport_receive(m, sizeof m)) != 0) {
		bool ok = true;
		switch (m[0]) {
		case ONLINE_TRADE_STATE: ok = online_trade_mail(m, n); break;
		case ONLINE_ARENA_STATE: case ONLINE_ARENA_LOADOUT: case ONLINE_ARENA_INPUT: case ONLINE_ARENA_HASH:
			square_arena_mail(m, n);
			break;
		default: break;
		}
		if (!ok) fprintf(stderr, "online: a malformed message of kind %u set aside\n", m[0]);
	}
}
