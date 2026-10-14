/* The wire contract shared by the game and the ROM-free presence server.
 * server/tests verifies these numbers against protocol.json. */
#ifndef CW_ONLINE_PROTOCOL_H
#define CW_ONLINE_PROTOCOL_H

#define ONLINE_PROTOCOL_VERSION 1
#define ONLINE_MAX_PLAYERS 8
#define ONLINE_NAME_BYTES 16
#define ONLINE_MAX_MESSAGE 512
#define ONLINE_COORDINATE_LIMIT 8192
#define ONLINE_SUBPROTOCOL "cyberworld.presence.v1"
#define ONLINE_HELLO 1
#define ONLINE_WELCOME 2
#define ONLINE_STATE 3
#define ONLINE_SNAPSHOT 4
#define ONLINE_ERROR 5
#define ONLINE_PING 6
#define ONLINE_PONG 7

#endif
