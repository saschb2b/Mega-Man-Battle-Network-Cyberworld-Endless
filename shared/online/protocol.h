/* The wire contract shared by the game and the ROM-free presence server.
 * server/tests verifies these numbers against protocol.json. */
#ifndef CW_ONLINE_PROTOCOL_H
#define CW_ONLINE_PROTOCOL_H

#define ONLINE_PROTOCOL_VERSION 14
#define ONLINE_MAX_PLAYERS 8
#define ONLINE_NAME_BYTES 16
#define ONLINE_MAX_MESSAGE 512
#define ONLINE_COORDINATE_LIMIT 8192
#define ONLINE_SUBPROTOCOL "cyberworld.presence.v14"
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
#define ONLINE_EMOTE_COUNT 12
#define ONLINE_PROFILE_BYTES 69
#define ONLINE_PEER_BYTES 50
#define ONLINE_WELCOME_BYTES 38
#define ONLINE_TRADE 15
#define ONLINE_TRADE_STATE 16
#define ONLINE_ARENA 17
#define ONLINE_ARENA_STATE 18
#define ONLINE_ARENA_LOADOUT 19
#define ONLINE_ARENA_INPUT 20
#define ONLINE_ARENA_HASH 21
#define ONLINE_OFFER 22
#define ONLINE_BOARD 23
#define ONLINE_CHIP_LAST 511
#define ONLINE_BN6_CHIP_LAST 319
#define ONLINE_CODE_LAST 26
#define ONLINE_TRADE_BYTES 12
#define ONLINE_TRADE_STATE_BYTES 32
#define ONLINE_ARENA_BYTES 8
#define ONLINE_ARENA_STATE_BYTES 28
#define ONLINE_ARENA_LOADOUT_MAX 400
#define ONLINE_ARENA_INPUT_MAX 8
#define ONLINE_HELLO_BYTES 76
#define ONLINE_ROMS_BN5 1
#define ONLINE_ROMS_FULL 1
#define ONLINE_NOTICE_PASS 28
#define ONLINE_OFFER_BYTES 5
#define ONLINE_BOARD_ENTRY_BYTES 8
#define ONLINE_WANT_LAST 9
#define ONLINE_WEEK_POST 24
#define ONLINE_WEEK_BOARD 25
#define ONLINE_WEEK_POST_BYTES 17
#define ONLINE_WEEK_ENTRY_BYTES 22
#define ONLINE_WEEK_TOP 8
#define ONLINE_WEEK_LAST_TOP 3
#define ONLINE_WEEK_LAYERS 10
#define ONLINE_WEEK_EPOCH 345600
#define ONLINE_STANDS 26
#define ONLINE_ARENA_STATUS 27
#define ONLINE_WATCH 28
#define ONLINE_WATCH_STATE 29
#define ONLINE_WATCH_RELAY 30
#define ONLINE_WATCH_KEYS 31
#define ONLINE_STANDS_ENTRY_BYTES 63
#define ONLINE_STANDS_MAX 5
#define ONLINE_ARENA_STATUS_BYTES 20
#define ONLINE_WATCH_BYTES 6
#define ONLINE_WATCH_STATE_BYTES 15
#define ONLINE_WATCH_KEYS_HEAD 12
#define ONLINE_WATCH_KEYS_MAX 240
#define ONLINE_WATCH_MOST 6
/* (protocol v10, issue #153: a place held through a dropped link) */
#define ONLINE_BYE 32
#define ONLINE_HELLO_RESUME 32768
#define ONLINE_ERROR_MAINTENANCE 16
#define ONLINE_ERROR_SERVER_OLDER 17
#define ONLINE_MAINTENANCE_BYTES 40
/* (protocol v12, epic #167: Team NetBattles, a relay of two teams of two) */
#define ONLINE_TEAM 33
#define ONLINE_TEAM_STATE 34
#define ONLINE_TEAM_BYTES 7
#define ONLINE_TEAM_STATE_BYTES 36
#define ONLINE_NOTICE_TEAM 29
/* (protocol v13, epic #167 step 4: the Square shows who is busy and the
 * link before a challenge; a challenge seen; a rematch in a second) */
#define ONLINE_BUSY_SHIFT 1      /* SNAPSHOT's per-peer byte: bits 1-2 what he is busy with ... */
#define ONLINE_BUSY_BATTLE 1     /* ... in a NetBattle */
#define ONLINE_BUSY_WATCH 2      /* ... watching one */
#define ONLINE_BUSY_TRADE 3      /* ... in a trade */
#define ONLINE_LINK_SHIFT 3      /* ... bits 3-4 his link's bars (0 unknown, 1-3) */
#define ONLINE_LINK_GOOD_MS 100  /* a round trip under this: three bars */
#define ONLINE_LINK_FAIR_MS 200  /* ... under this: two; past it one */
#define ONLINE_PING_BYTES 7      /* PING: u32 clock, u16 the sender's round trip in ms (0 unknown) */
#define ONLINE_ARENA_SEEN 5      /* ARENA's action and ARENA_STATE's stage: the challenge's question shown */
#define ONLINE_PENDING_UNSEEN 60 /* seconds a challenge not yet seen waits */
/* (protocol v14, issue #176: the Square's emotes performed, aimed and
 * answered, on the server's beat) */
#define ONLINE_SOCIAL_BYTES 7         /* SOCIAL: u8 kind, u8 id, u32 the MegaMan it is aimed at (0 none) */
#define ONLINE_EVENT_BYTES 14         /* EVENT: u32 sender, u8 kind, u8 id, u32 aimed at, u8 flags, u16 ms to its beat */
#define ONLINE_EMOTE_AIMED 35         /* the emotes that may be aimed, a bit an id: Wave, Cheer, Bow */
#define ONLINE_SOCIAL_BURST 3         /* phrases and emotes at once ... */
#define ONLINE_SOCIAL_REFILL_MS 1000  /* ... then one back so often */
#define ONLINE_AIM_CAP 2              /* aimed emotes at one MegaMan unanswered, past which only the sender sees his */
#define ONLINE_AIM_WINDOW_MS 30000    /* ... counted so long */
#define ONLINE_ANSWER_MS 5000         /* an aimed emote answered within this: EVENT's answered flag */
#define ONLINE_BEAT_MS 250            /* the server's beat an emote starts on */
#define ONLINE_BEAT_LEAD_MS 60        /* ... the least lead past the farthest one-way trip of the channel */
#define ONLINE_BEAT_LEAD_MOST_MS 400  /* ... and the most */
#define ONLINE_EVENT_ANSWERED 1       /* EVENT's flags: it answers one aimed at its sender */

#endif
