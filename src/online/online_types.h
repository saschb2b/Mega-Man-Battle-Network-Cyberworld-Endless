/* Public, bounded records only. A player's Pack and folder stay local. */
#ifndef CW_ONLINE_TYPES_H
#define CW_ONLINE_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "protocol.h"

#define ONLINE_EVENT_HISTORY 8

typedef struct {
	uint32_t id;
	uint8_t identity[ONLINE_IDENTITY_BYTES];
	int32_t x, y, z;
	uint8_t facing;
	char name[ONLINE_NAME_BYTES + 1];
	uint8_t roms;   /* the ROMs his game has besides BN6's: ONLINE_ROMS_BN5 (BN5 Team Colonel's), the server's word */
	uint8_t busy;   /* what he is busy with (v13): 0 free, ONLINE_BUSY_BATTLE, ONLINE_BUSY_WATCH, ONLINE_BUSY_TRADE, the server's word */
	uint8_t link;   /* his link's bars as his PING said his round trip (v13): 0 unknown, 1-3 */
} OnlinePeer;

typedef struct {
	uint16_t hp, max_hp;
	uint8_t attack, speed, charge;
	uint16_t souls, crosses, depth;
	uint8_t act;
	uint16_t best_depth;
	uint32_t marks;
	uint8_t board[ONLINE_BOARD_CELLS];
} OnlineProfile;

typedef struct {
	uint32_t serial, player_id, ticks;
	uint8_t kind, value;
} OnlineEvent;

typedef struct {
	bool connected;
	uint32_t seed, player_id, latency_ms, position_serial, position_ticks;
	uint16_t channel, channels;
	uint8_t identity[ONLINE_IDENTITY_BYTES];
	size_t peer_count;
	OnlinePeer peers[ONLINE_MAX_PLAYERS];
	uint32_t card_serial, card_player;
	uint8_t card_identity[ONLINE_IDENTITY_BYTES];
	OnlineProfile card;
	uint32_t event_serial;
	OnlineEvent events[ONLINE_EVENT_HISTORY];
	uint32_t notice_serial;
	uint16_t notice_code;  /* the last NOTICE's reason (protocol.json's codes) */
	uint16_t error_code;   /* the server's refusal (protocol.json's codes), 0 none */
	/* the server's week's net (WELCOME, protocol v6): its seed and number
	 * (weeks since Monday 5 January 1970), 0 none */
	uint32_t week_seed;
	uint16_t week;
	/* the link (issue #153, online_transport.h): quiet a while, or dropped
	 * and being tried again (lost_at: the ticks it went quiet or dropped);
	 * resumes counts the times a resume's WELCOME and first SNAPSHOT came */
	bool quiet, reconnecting;
	uint32_t lost_at, resumes;
	char notice[100], error[128];
} OnlineSnapshot;

#endif
