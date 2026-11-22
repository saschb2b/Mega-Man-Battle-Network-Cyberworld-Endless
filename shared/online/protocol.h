/* The wire contract shared by the game and the ROM-free presence server.
 * server/tests verifies these numbers against protocol.json. */
#ifndef CW_ONLINE_PROTOCOL_H
#define CW_ONLINE_PROTOCOL_H

#define ONLINE_PROTOCOL_VERSION 26
#define ONLINE_MAX_PLAYERS 8
#define ONLINE_NAME_BYTES 16
#define ONLINE_MAX_MESSAGE 512
#define ONLINE_COORDINATE_LIMIT 8192
#define ONLINE_SUBPROTOCOL "cyberworld.presence.v26"
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
#define ONLINE_WEEK_POST_BYTES 18  /* (v19: the acts whose guardian was fought in a party) */
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

/* (protocol v19, issue #589: the shared dive; shared/online/README.md,
 * Protocol v19) */
#define ONLINE_WEEK_PARTY 2           /* WEEK_BOARD's entry flags: a guardian of the dive fought in a party */
#define ONLINE_WEEK_PULLED 4          /* (v22) WEEK_POST's flags and WEEK_BOARD's entry flags: the dive's MegaMan pulled back once by his crew (issue #718) */
#define ONLINE_HELLO_NO_ARENA 8192    /* HELLO's channel word: the game's system cannot run the Arena (a linked fighter) */
#define ONLINE_PEER_DIVING 128        /* SNAPSHOT's per-peer byte, bit 7: he dives (a dive room's peers) */
#define ONLINE_DIVE 41                /* client -> server: u8 action, then that action's fields */
#define ONLINE_DIVE_ROOM 42           /* server -> client: the room entered */
#define ONLINE_DIVE_TRACES 43         /* server -> client: u8 kind, u8 count, entries */
#define ONLINE_DIVE_PLACE 44          /* server -> client: a split's place */
#define ONLINE_DIVE_FEED 45           /* server -> divers and the Square: who fell, who won, a party's guardian down, a flare */
#define ONLINE_DIVERS 46              /* server -> divers and the Square: how many dive, and where */
#define ONLINE_CREW 47                /* client -> server */
#define ONLINE_CREW_STATE 48          /* server -> client */
#define ONLINE_QUEUE 49               /* client -> server: a guardian door's queue */
#define ONLINE_QUEUE_STATE 50         /* server -> client */
#define ONLINE_PARTY_DAMAGE 51        /* linked fighter -> server -> seat 0 (as ARENA_SEAT) */
#define ONLINE_PARTY_HP 52            /* seat 0 -> server -> the linked fighters */
#define ONLINE_WEEK_SPLITS 53         /* server -> client, with WEEK_BOARD */
#define ONLINE_DIVE_ENTER 1           /* DIVE's actions ... */
#define ONLINE_DIVE_LEAVE 2
#define ONLINE_DIVE_END 3
#define ONLINE_DIVE_FOUND 4
#define ONLINE_DIVE_PHRASE 5
#define ONLINE_DIVE_WAVE 6
#define ONLINE_DIVE_SPLIT 7
#define ONLINE_DIVE_VITALS 8          /* v24: MegaMan's HP and max HP, as they change (issue #717) */
#define ONLINE_DIVE_WHERE 9           /* v26: where MegaMan is, ONLINE_WHERE_* (issue #756) */
#define ONLINE_DIVE_ENTER_BYTES 11    /* ... and each one's size, its type byte counted */
#define ONLINE_DIVE_LEAVE_BYTES 2
#define ONLINE_DIVE_END_BYTES 11
#define ONLINE_DIVE_FOUND_BYTES 3
#define ONLINE_DIVE_PHRASE_BYTES 9
#define ONLINE_DIVE_WAVE_BYTES 6
#define ONLINE_DIVE_SPLIT_BYTES 8
#define ONLINE_DIVE_VITALS_BYTES 6
#define ONLINE_DIVE_WHERE_BYTES 3
#define ONLINE_WHERE_LAYER 0          /* on a layer's map */
#define ONLINE_WHERE_HOME 1           /* at home between acts: drawn on no layer */
#define ONLINE_WHERE_BATTLE 2         /* in a battle */
#define ONLINE_DIVE_DELETED 1         /* a dive's end: deleted ... */
#define ONLINE_DIVE_WON 2             /* ... or won */
#define ONLINE_BUSTING_MOST 11        /* BN6's Busting Level: 1 to 10, 11 for S (docs/rom/battle.md) */
#define ONLINE_PARTY_MOST 4           /* a party's or a crew's MegaMen at most */
#define ONLINE_DIVE_ROOM_BYTES 9
#define ONLINE_TRACE_MARKS 1          /* DIVE_TRACES's kinds ... */
#define ONLINE_TRACE_PHRASES 2
#define ONLINE_TRACE_FINDS 3
#define ONLINE_TRACE_OPENED 4
#define ONLINE_TRACE_SPLITS 5
#define ONLINE_TRACE_GONE 6
#define ONLINE_TRACE_HEAD 3           /* ... its head, then each kind's entry */
#define ONLINE_TRACE_MARK_BYTES 25
#define ONLINE_TRACE_PHRASE_BYTES 29
#define ONLINE_TRACE_FIND_BYTES 17
#define ONLINE_TRACE_SPLIT_BYTES 22
#define ONLINE_TRACES_MOST 32         /* a layer's marks, phrases, first finds and splits kept, each */
#define ONLINE_SPLIT_GUARDIAN 1       /* a split's flags: the battle was the layer's guardian */
#define ONLINE_DIVE_PLACE_BYTES 27
#define ONLINE_DIVE_FEED_BYTES 24
#define ONLINE_FEED_DELETED 1         /* DIVE_FEED's kinds */
#define ONLINE_FEED_WON 2
#define ONLINE_FEED_GUARDIAN 3
#define ONLINE_FEED_FLARE 4
#define ONLINE_DIVERS_HEAD 4
#define ONLINE_DIVERS_ENTRY_BYTES 19
#define ONLINE_DIVERS_MOST 16
#define ONLINE_DIVERS_MS 2000         /* DIVERS at most so often, on a change */
#define ONLINE_DIVERS_FIGHTING 1      /* a DIVERS entry's flags: in a party's fight */
#define ONLINE_CREW_BYTES 10
#define ONLINE_CREW_STATE_BYTES 111
#define ONLINE_CREW_MEMBER_BYTES 26
#define ONLINE_CREW_BATTLE 5          /* CREW's actions (v20: 1-4, the asks and ways, gone) */
#define ONLINE_CREW_JOIN 6
#define ONLINE_CREW_BUSY 7           /* v23: a crewmate's battle offered, this game's MegaMan not free to go in */
#define ONLINE_CREW_FORMED 3          /* CREW_STATE's stages (v20: 1-2, the asks', gone) */
#define ONLINE_CREW_SPLIT_UP 4
#define ONLINE_CREW_OFFER 5
#define ONLINE_CREW_ALONE 6
#define ONLINE_CREW_VITALS 7          /* v24: a crewmate's HP, max HP or layer changed (the crew as it stands) */
#define ONLINE_CREW_VITALS_MS 500     /* a game's DIVE vitals at most so often; the server's CREW_STATE vitals at most so often a crew */
#define ONLINE_CREW_JOIN_MS 3000      /* a battle's offer stands so long */
/* v20, the reach on screen since issue #750: in the picture's pixels from
 * MegaMan's feet, BN6's projection (dx + dy across, (dy - dx) / 2 down),
 * within which a crew forms, */
#define ONLINE_CREW_NEAR_WIDE 80
#define ONLINE_CREW_NEAR_HIGH 32
/* ... and within which it holds: inside the 240 by 160 picture both ways */
#define ONLINE_CREW_FAR_WIDE 100
#define ONLINE_CREW_FAR_HIGH 44
#define ONLINE_CREW_FORM_MS 1000      /* near so long before a crew forms */
#define ONLINE_CREW_SPLIT_MS 2000     /* out of reach so long in the room before a member is out */
#define ONLINE_CREW_AWAY_MS 10000     /* ... and elsewhere (a portal taken first, home) */
#define ONLINE_QUEUE_BYTES 5
#define ONLINE_QUEUE_STATE_BYTES 91
#define ONLINE_QUEUE_ENTRY_BYTES 21
#define ONLINE_QUEUE_JOIN 1           /* QUEUE's actions */
#define ONLINE_QUEUE_LEAVE 2
#define ONLINE_QUEUE_START 3
#define ONLINE_QUEUE_PASS 4
#define ONLINE_QUEUE_WAITING 1        /* QUEUE_STATE's stages */
#define ONLINE_QUEUE_STARTED 2
#define ONLINE_QUEUE_OUT 3
#define ONLINE_QUEUE_ALONE 4
#define ONLINE_QUEUE_ARENA 1          /* a queue entry's flags: his system runs the Arena */
#define ONLINE_PARTY_BYTES 9          /* PARTY_DAMAGE and PARTY_HP */
#define ONLINE_ARENA_PARTY 2          /* ARENA_STATE START's format: a party on one field ... */
#define ONLINE_ARENA_LINKED 3         /* ... a linked fighter, his own battle */
#define ONLINE_ARENA_SEAT_GONE 6      /* ARENA_STATE's stage: a party's seat gone (its side byte the seat) */
#define ONLINE_WEEK_SPLITS_BYTES 291
#define ONLINE_NOTICE_DIVE_OUT 30     /* that takes a dive */
#define ONLINE_NOTICE_DIVE_IN 31      /* not on a dive */
#define ONLINE_NOTICE_DIVE_WAIT 32    /* traces too fast */
#define ONLINE_NOTICE_TRACE_GONE 33   /* that phrase is gone */
#define ONLINE_NOTICE_CREW 34         /* a crew's ask refused */
#define ONLINE_NOTICE_QUEUE 35        /* a door's queue refused */
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
