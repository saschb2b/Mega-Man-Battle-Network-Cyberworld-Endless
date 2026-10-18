#include "online_mail.h"

#include <stddef.h>
#include <stdint.h>

#include "online_transport.h"
#include "square_arena.h"

void online_mail_frame(void) {
	uint8_t data[ONLINE_MAX_MESSAGE];
	size_t size;
	while ((size = online_transport_receive(data, sizeof data))) {
		switch (data[0]) {
		/* (Trade Central's TRADE_STATE: online_trade_mail) */
		case ONLINE_ARENA_STATE: case ONLINE_ARENA_LOADOUT: case ONLINE_ARENA_INPUT: case ONLINE_ARENA_HASH:
			square_arena_mail(data, size);
			break;
		default: break;
		}
	}
}
