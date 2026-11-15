/* The wire contract shared by the game and the ROM-free presence server.
 * server/tests verifies these numbers against protocol.json. */
#ifndef CW_ONLINE_PROTOCOL_H
#define CW_ONLINE_PROTOCOL_H

#define ONLINE_PROTOCOL_VERSION 18
#define ONLINE_MAX_PLAYERS 8
#define ONLINE_NAME_BYTES 16
#define ONLINE_MAX_MESSAGE 512
#define ONLINE_COORDINATE_LIMIT 8192
#define ONLINE_SUBPROTOCOL "cyberworld.presence.v18"
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
#define ONLINE_CHIP_LAST 1023
#define ONLINE_BN6_CHIP_LAST 319
#define ONLINE_BN4_CHIP_FIRST 527
#define ONLINE_CODE_LAST 26
#define ONLINE_TRADE_BYTES 12
#define ONLINE_TRADE_STATE_BYTES 32
#define ONLINE_ARENA_BYTES 9
#define ONLINE_ARENA_STATE_BYTES 45
#define ONLINE_ARENA_LOADOUT_MAX 400
#define ONLINE_ARENA_INPUT_MAX 8
#define ONLINE_HELLO_BYTES 100
#define ONLINE_ROMS_BN5 1
#define ONLINE_ROMS_BN4 32
#define ONLINE_ROMS_FULL 33
#define ONLINE_ROMS_ONE_EXTRA 64
#define ONLINE_HELLO_ONE_EXTRA 16384
#define ONLINE_NOTICE_GROUND 19
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
#define ONLINE_WATCH_STATE_BYTES 24
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
#define ONLINE_TEAM_STATE_BYTES 37
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
/* (protocol v15, issue #181: two MegaMen a side, four seats over the
 * server; ARENA and ARENA_STATE carry a team challenge's format, START a
 * 2v2's seat and the four seats' sessions, TEAM_STATE the format,
 * WATCH_STATE the partners' seats) */
#define ONLINE_ARENA_SEAT 35          /* server -> client in a 2v2: u8 seat, then that seat's relayed message as sent */
#define ONLINE_SEATS 4                /* a 2v2's seats: 0 and 1 the GBAs' players, 2 and 3 side 0's and side 1's partners */
#define ONLINE_TEAM_RELAY 0           /* a team challenge's format: the relay, one bout at a time */
#define ONLINE_TEAM_SIDE_BY_SIDE 1    /* ... two MegaMen a side on one field */
#define ONLINE_STANDS_SIDE_BY_SIDE 255 /* STANDS's bout byte for a side-by-side battle (a relay's bout its number) */

/* Saves sent by a code (docs/engine/saves.md, Sending by code): a
 * connection to /presence whose first message is TRANSFER instead of
 * HELLO, with no protocol version of its own (a server without them
 * answers it as any first message that is not a HELLO, an ERROR). The
 * blob is sealed on the game's side; the server holds it, unread, in
 * memory for ONLINE_TRANSFER_MINUTES. */
#define ONLINE_TRANSFER 36            /* client -> server, first: u8 version, u8 op, 16-byte lookup, u32 size (a put's) */
#define ONLINE_TRANSFER_STATE 37      /* server -> client: u8 status, u32 its value */
#define ONLINE_TRANSFER_DATA 38       /* either way: the blob's next bytes, at most ONLINE_MAX_MESSAGE - 1 */
#define ONLINE_TRANSFER_VERSION 1
#define ONLINE_TRANSFER_PUT 1         /* TRANSFER's op: a blob to hold, its DATA after READY */
#define ONLINE_TRANSFER_GET 2         /* ... the blob held under the lookup, FOUND and its DATA */
#define ONLINE_TRANSFER_DONE 3        /* ... after a GET, on the same connection: the blob let go (REMOVED) */
#define ONLINE_TRANSFER_BYTES 23
#define ONLINE_TRANSFER_STATE_BYTES 6
#define ONLINE_TRANSFER_LOOKUP_BYTES 16
#define ONLINE_TRANSFER_MAX 2097152   /* a blob's bytes at most */
#define ONLINE_TRANSFER_MINUTES 15    /* how long the server holds one */
#define ONLINE_TRANSFER_READY 0       /* TRANSFER_STATE's status: send the DATA */
#define ONLINE_TRANSFER_HELD 1        /* ... the blob is held, for the value's seconds */
#define ONLINE_TRANSFER_FOUND 2       /* ... the blob follows, the value its size */
#define ONLINE_TRANSFER_NONE 3        /* ... nothing held under that lookup */
#define ONLINE_TRANSFER_BUSY 4        /* ... the server is full or asked too often: again in the value's seconds */
#define ONLINE_TRANSFER_TAKEN 5       /* ... a put's lookup is in use: another code */
#define ONLINE_TRANSFER_OFF 6         /* ... this server takes no transfers */
#define ONLINE_TRANSFER_REMOVED 7     /* ... DONE: the blob is gone */

/* Saves kept in step (docs/engine/saves.md, Saves kept in step): a
 * connection to /presence whose first message is SYNC, a slot's sealed
 * saves pulled, pushed on the generation they were pulled at, or dropped.
 * The slot and its key are the game's (a secret the linked systems share);
 * the server keeps the blob on disk, unread, ONLINE_SYNC_DAYS after its
 * last use. TRANSFER_DATA carries the bytes either way. */
#define ONLINE_SYNC 39                /* client -> server, first: u8 version, u8 op, 16-byte slot, 32-byte auth, u32 generation, u32 size (a push's) */
#define ONLINE_SYNC_STATE 40          /* server -> client: u8 status, u32 generation, u32 size */
#define ONLINE_SYNC_VERSION 1
#define ONLINE_SYNC_PULL 1            /* SYNC's op: the slot's saves, where newer than the generation named */
#define ONLINE_SYNC_PUSH 2            /* ... saves to the slot, made from the generation named (0: a new slot) */
#define ONLINE_SYNC_DROP 3            /* ... the slot deleted */
#define ONLINE_SYNC_BYTES 59
#define ONLINE_SYNC_STATE_BYTES 10
#define ONLINE_SYNC_SLOT_BYTES 16
#define ONLINE_SYNC_AUTH_BYTES 32
#define ONLINE_SYNC_MAX 2097152       /* a slot's blob's bytes at most */
#define ONLINE_SYNC_DAYS 180          /* a slot unused this long is deleted */
#define ONLINE_SYNC_SAME 0            /* SYNC_STATE's status: the pull's generation is the slot's */
#define ONLINE_SYNC_FOUND 1           /* ... the slot's newer saves follow: its generation, their size */
#define ONLINE_SYNC_NONE 2            /* ... no such slot (never made, dropped, or unused past its days) */
#define ONLINE_SYNC_DENIED 3          /* ... the slot's, but not its auth */
#define ONLINE_SYNC_READY 4           /* ... send the push's DATA */
#define ONLINE_SYNC_STORED 5          /* ... the push is kept: the slot's new generation */
#define ONLINE_SYNC_CONFLICT 6        /* ... the push was made from an older generation: the slot's now */
#define ONLINE_SYNC_BUSY 7            /* ... the server is full or busy: again in the size field's seconds */
#define ONLINE_SYNC_OFF 8             /* ... this server keeps no slots */
#define ONLINE_SYNC_DROPPED 9         /* ... the slot is gone */

#endif
