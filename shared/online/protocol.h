/* The wire contract shared by the game and the ROM-free presence server.
 * server/tests verifies these numbers against protocol.json. */
#ifndef CW_ONLINE_PROTOCOL_H
#define CW_ONLINE_PROTOCOL_H

#define ONLINE_PROTOCOL_VERSION 2
#define ONLINE_MAX_PLAYERS 8
#define ONLINE_NAME_BYTES 16
#define ONLINE_MAX_MESSAGE 512
#define ONLINE_COORDINATE_LIMIT 8192
#define ONLINE_SUBPROTOCOL "cyberworld.presence.v2"
#define ONLINE_HELLO 1
#define ONLINE_WELCOME 2
#define ONLINE_STATE 3
#define ONLINE_SNAPSHOT 4
#define ONLINE_ERROR 5
#define ONLINE_PING 6
#define ONLINE_PONG 7
#define ONLINE_PROFILE 8
#define ONLINE_INSPECT 9
#define ONLINE_CARD 10
#define ONLINE_SOCIAL 11
#define ONLINE_EVENT 12
#define ONLINE_CHANNEL 13
#define ONLINE_NOTICE 14
#define ONLINE_IDENTITY_BYTES 16
#define ONLINE_BOARD_CELLS 49
#define ONLINE_PHRASE_COUNT 8
#define ONLINE_EMOTE_COUNT 6
#define ONLINE_PROFILE_BYTES 69
#define ONLINE_PEER_BYTES 49
#define ONLINE_WELCOME_BYTES 32

#endif
