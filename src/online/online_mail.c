#include "online_mail.h"

#include <stdio.h>

#include "online_trade.h"
#include "online_transport.h"

void online_mail_frame(void) {
	uint8_t m[ONLINE_MAX_MESSAGE];
	size_t n;
	while ((n = online_transport_receive(m, sizeof m)) != 0) {
		bool ok = true;
		switch (m[0]) {
		case ONLINE_TRADE_STATE: ok = online_trade_mail(m, n); break;
		default: break;   /* (the Arena's: ARENA_STATE, LOADOUT, INPUT, HASH) */
		}
		if (!ok) fprintf(stderr, "online: a malformed message of kind %u set aside\n", m[0]);
	}
}
