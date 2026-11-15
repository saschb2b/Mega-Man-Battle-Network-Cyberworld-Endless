# Net Square protocol v19

`protocol.json` is the server's packaged contract. `protocol.h` exposes
the same constants to the C client; server tests check they agree with
the game generator. All multibyte fields are big-endian. Messages are
unfragmented binary WebSocket frames, at most 512 payload bytes, on
`/presence` with subprotocol `cyberworld.presence.v19`. The server also
accepts another version's `cyberworld.presence.vN` (echoing it), so that
a game of another version can say its HELLO and hear ERROR 2 or 17, which
side is behind, rather than a refused upgrade (v10).

| Message | Direction | Payload after the one-byte type |
| --- | --- | --- |
| HELLO (1) | client → server, first | u16 protocol, u16 SQUARE_MAKE, u32 SQUARE_HASH, 4-byte ROM game code, 20-byte ROM SHA-1 (BN6's), 4-byte and 20-byte BN5 Team Colonel's game code and SHA-1, then BN4 Red Sun's (v17; each all zeros where the game has not its ROM), 16-byte private guest key, u16 requested channel (0 chooses one; its top bit, `hello_resume` 32768, takes back a place held: below; v18: bit 14, `hello_one_extra` 16384, the game's system reads one extra ROM beside BN6's at most; v19: bit 13, `hello_no_arena` 8192, the game's system cannot run the Arena, a party's linked fighter), u8 name length, name bytes |
| WELCOME (2) | server → client | u32 world seed, u32 session player ID, u16 tick Hz, u8 room capacity, u16 channel, 16-byte public identity, u16 channel count, u32 the week's seed (never 0), u16 the week's number (weeks since Monday 5 January 1970, `week_epoch`) |
| STATE (3) | client → server | i32 x, i32 y, i32 z in whole world units, u8 facing (0..7) |
| SNAPSHOT (4) | server → client | u8 peer count, then 50 bytes per peer: u32 session ID, 16-byte public identity, three i32 coordinates, u8 facing, 16-byte zero-padded name, u8 ROMs and more (bit 0, `roms_bn5`: his game has BN5 Team Colonel's ROM; v17: bit 5, `roms_bn4`, BN4 Red Sun's; v13: bits 1-2, from `busy_shift`, what he is busy with, 0 free, `busy_battle` 1 a NetBattle under way (his own or his team match's), `busy_watch` 2 watching one, `busy_trade` 3 a trade; bits 3-4, from `link_shift`, his link's bars from the round trip his PING said, 0 unknown, 3 under `link_good_ms` (100), 2 under `link_fair_ms` (200), else 1; v18: bit 6, `roms_one_extra`, his system reads one extra ROM at most; v19: bit 7, `peer_diving`, he dives: a dive room's peers, never a Square channel's; every bit has its meaning) |
| ERROR (5) | server → client | u16 reason code, at most 96 printable ASCII bytes |
| PING (6) / PONG (7) | client → server / server → client | u32 client clock value, echoed unchanged (PONG: 5 bytes); since v13 PING is `ping_bytes` (7): the clock, then u16 the sender's round trip in ms as it measures it (0 unknown; a game on the server's own machine says 1), for its link's bars in the others' SNAPSHOT. A 5-byte PING is still answered, its round trip unknown |
| PROFILE (8) | client → server | 69-byte public profile, described below |
| INSPECT (9) | client → server | u32 target session ID in the same channel |
| CARD (10) | server → client | u32 session ID, 16-byte public identity, 69-byte public profile |
| SOCIAL (11) | client → server | u8 kind (0 phrase, 1 emote), u8 fixed choice ID, u32 the session it is aimed at (v14; 0 none, and only an emote of `emote_aimed`'s may name one: 7 bytes, `social_bytes`) |
| EVENT (12) | server → channel, including sender | u32 sender session ID, u8 kind, u8 fixed choice ID, u32 the session it is aimed at (0 none), u8 flags (bit 0, `event_answered`: it answers one the other aimed at its sender within `answer_ms`), u16 ms from this message to the beat it starts on (v14: 14 bytes, `event_bytes`) |
| CHANNEL (13) | client → server | u16 requested channel (0 chooses one); success sends a fresh WELCOME |
| NOTICE (14) | server → client | u16 reason, at most 96 printable ASCII bytes; an action failed but the connection stays open |
| TRADE (15) | client → server | u8 action (1 propose, 2 answer, 3 accept, 4 cancel, 5 take a board offer), u32 reference (a proposal's or a take's target session ID, else the trade's ID), the chip given (u16 id 1..511, u8 code 0..26, 26 the `*`), the chip taken (the same; zero in a proposal; a take's is the offer's chip) |
| TRADE_STATE (16) | server → client | u8 stage (1 offered, 2 waiting, 3 answered, 4 accepted, 5 done, 6 cancelled), u32 trade ID, u32 the other player's session ID, 16-byte his public identity, my chip and his (u16 id, u8 code each; zero where not yet named) |
| ARENA (17) | client → server | u8 action (1 challenge, 2 accept, 3 leave or decline, 4 over, 5 `arena_seen` (v13): the challenge's question up on the challenged's screen, from him alone, once), u32 reference (a challenge's target session ID, else the battle's ID), u16 the sender's latency in ms (0..5000; its round trip to the server with its jitter, the mean and twice the spread of its last pings, less what its rollback window covers: protocol v8), u8 a team challenge's format (v15: `team_relay` 0, `team_side_by_side` 1; else 0, and a lone challenge's is taken as 0) |
| ARENA_STATE (18) | server → client | u8 stage (1 challenged, 2 waiting, 3 start, 4 ended, 5 `arena_seen` (v13): to the challenger, his question is up on the other's screen and its 30 seconds run from now), u32 battle ID, u32 the other player's session ID (a 2v2 partner's: the other side's lead), 16-byte his public identity, u8 side (0 the cable's first GBA, 1 the second; START's; a 2v2's START: the recipient's seat, 0..3), u8 input delay in frames (START's), u8 the format (v15: a team challenge's, CHALLENGED and WAITING; START's 1 a 2v2; v19: a party battle's, `arena_party` 2 on one field, `arena_linked` 3 a linked fighter), four u32 sessions (v15: a 2v2's START, seats 0 to 3: the leads, then side 0's and side 1's partners; v19: a party's, its seats, zeros past them; else zeros); v19's stage 6, `arena_seat_gone`: a party's seat gone, its side byte that seat, the other his session |
| ARENA_LOADOUT (19) | client → server → opponent | u32 battle ID, u16 length (1..400), that many bytes: the sender's MegaMan as `src/online/arena_wire.h` packs him (its own version byte; 2 since v12, his HP beside his max HP: a Team NetBattle's winner goes on with what he has left) |
| ARENA_INPUT (20) | client → server → opponent | u32 battle ID, u32 first frame, u8 count (1..8), count u16 GBA keys, one a frame from the first |
| ARENA_HASH (21) | client → server → opponent | u32 battle ID, u32 frame, u64 the two linked cores' state hash after it; frame 0xFFFFFFFF says ready, the hash the pair's at the Custom screen; frame 0xFFFFFFFE, sent just before it, the installs' record its cores were made with, so that a ready apart is told by its cause; frame 0xFFFFFFFD (v11) a lag note, once a second in the battle: the u64's low 16 bits the sender's least lag of the other's keys over the last second, signed ticks, for the time sync (docs/online/arena/rollback/quicker-on-the-wire.md), never kept for the stands |
| OFFER (22) | client → server | u16 chip id (1..511; 0 takes the sender's offer down), u8 code (0..26), u8 want (0..`want_last`, 9: the game's fixed list, 0 anything); all zeros to take it down |
| BOARD (23) | server → client | u8 count (0..8), then 8 bytes an offer: u32 the poster's session ID, u16 chip id, u8 code, u8 want; the recipient's channel's offers he may see, his own among them |
| WEEK_POST (24) | client → server | u16 week, u32 that week's seed, u16 layer (1..`week_layers`, 10), u8 act (that layer's: three layers an act, the Nest the fourth), u8 flags (bit 0 won, only on layer 10; no other bit), u16 minutes, u32 summary hash (not 0), u8 (v19) the acts whose guardian was fought in a party, bit 0 act 1 to bit 3 act 4, none past the dive's act |
| WEEK_BOARD (25) | server → client | u16 this week, u8 count (0..8), u8 last week's count (0..3), u8 the recipient's place this week (0 none), then 22 bytes an entry, the recipient's own best first (zeros where none), then this week's, then last week's: 16-byte zero-padded name, u16 layer, u8 act, u8 flags (bit 0 won; v19: bit 1 `week_party`, a guardian of the dive fought in a party), u16 minutes |
| STANDS (26) | server → client | u8 count (0..`stands_max`, 5), then 63 bytes a battle (62 before v12) of the recipient's channel (those on, then its last one over): u32 battle ID, u32 the cable's first player's session ID, u32 the second's, two 16-byte zero-padded names in that order, u8 stage (1 the way in, 2 fighting, 3 over with a winner, 4 over without), u8 watching, u8 cheers, then the report: u16 HP and u16 max HP of the first, the same of the second, u16 the last chip each used (0 none), u8 chips each used, u8 winner (0 none, 1 the first, 2 the second), u8 a Team NetBattle's bout (v12; 0 a lone battle; v15: `stands_side_by_side`, 255, a side-by-side battle) |
| ARENA_STATUS (27) | client → server | u32 battle ID, then the report as STANDS holds it (15 bytes); taken from the cable's first player alone |
| WATCH (28) | client → server | u8 action (1 watch, 2 stop), u32 battle ID |
| WATCH_STATE (29) | server → spectator | u8 stage (1 watching: the battle's setup; 2 over: no more keys come), u32 battle ID, u32 the cable's first player's session ID, u32 the second's, u8 input delay, u8 a 2v2 (v15: 1, else 0), u32 seat 2's session, u32 seat 3's (a 2v2's partners; else zeros) |
| WATCH_RELAY (30) | server → spectator | u8 side (0 the cable's first; a 2v2's seat, 0..3), then that player's ARENA_LOADOUT or ARENA_HASH as he sent it |
| WATCH_KEYS (31) | server → spectator | u32 battle ID, u8 side (a 2v2's seat, 0..3; a partner's keys keep their sixteen bits), u32 first frame, u16 count (1..`watch_keys_max`, 240), count u16 GBA keys, one a frame from the first |
| BYE (32) | client → server, last | nothing: the game leaves (the pad, R and home, the game closed), so its place is freed at once; a WebSocket close says the same |
| TEAM (33) | client → server | u8 action (1 ask a player to team up, 2 answer one, 3 split the team or take an ask back, 4 the order), u32 reference (the player asked, the one who asked, 0 for a split, the lead's session for the order), u8 value (an answer's 1 yes; else 0) |
| TEAM_STATE (34) | server → client | u8 stage (1 asked, 2 waiting, 3 formed, 4 split, 5 a match begun, 6 the order shown, 7 a bout called, 8 the bout started, 9 the match over), u32 the partner's session (asked: the asker; waiting: the one asked; split: the other), u32 the team match's ID, u32 the bout under way (its ARENA battle ID, 0 none), four u32 sessions: the recipient's team's two, then the other's, each team's MegaMan up now first (before the order: its captain first; side by side: its captain, the lead), u8 MegaMen standing of the recipient's team, u8 of the other, u8 the bout's number, u8 the result (the match over: 1 won, 2 lost, 3 broken off, 4 drawn; else 0), u16 the staying MegaMan's HP as the last bout's report left it (0 none), u8 the format (v15: 0 the relay, 1 side by side) |
| ARENA_SEAT (35) | server → client | v15, a 2v2's: u8 the sending seat (0..3), then its ARENA_LOADOUT, ARENA_INPUT or ARENA_HASH as it sent it, to the three other seats (v19: a party battle's too, to its other party seats; and a linked fighter's PARTY_DAMAGE, to seat 0 alone) |
| DIVE (41) | client → server | v19: u8 action, then its fields, each action a size of its own (with the type byte): 1 `dive_enter` (u16 week, u32 that week's seed, u8 the layer's depth 1..`week_layers`, u8 its side kind, 0 the main path, else the game's LAYER_* kind, u8 its way, the layer's area id: 11 bytes), 2 `dive_leave` (nothing: 2), 3 `dive_end` (u8 how, `dive_deleted` 1 or `dive_won` 2, i16 x, i16 y, i16 z within `coordinate_limit`, u16 what deleted MegaMan, 0 unknown: 11), 4 `dive_found` (u8 the Mystery Data's index on the layer: 3), 5 `dive_phrase` (u8 a phrase ID, i16 x, y, z: 9), 6 `dive_wave` (u32 a phrase's ID, the server's: 6), 7 `dive_split` (u8 the battle's index on the layer, u16 tenths of a second (not 0), u8 BN6's Busting Level 1..`busting_most`, 11 for S, u8 flags, bit 0 `split_guardian` the layer's guardian, u8 the party's size 1..`party_most`: 8) |
| DIVE_ROOM (42) | server → client | v19, an enter's answer: u16 week, u8 depth, u8 side, u8 way, u16 the room's instance (1 the first), u8 divers in it, the recipient among them (9 bytes) |
| DIVE_TRACES (43) | server → client | v19: u8 kind, u8 count, then count entries of that kind (as many messages as they take): 1 `trace_marks` (16-byte name, i16 x, y, z, u16 cause, u8 how: 25 bytes), 2 `trace_phrases` (u32 ID, 16-byte name, u8 phrase, i16 x, y, z, u16 Waves: 29), 3 `trace_finds` (u8 index, 16-byte name of its first finder this week: 17), 4 `trace_opened` (u8 an index opened this week: 1), 5 `trace_splits` (u8 battle index, u8 party size, u16 tenths, u8 Busting Level, u8 flags, 16-byte name: the quickest bust of that battle at that size: 22), 6 `trace_gone` (u32 a phrase's ID, gone: 4) |
| DIVE_PLACE (44) | server → client | v19, a split's answer: u8 battle index, u8 party size, u16 the tenths sent, u16 their place among this week's busts of that battle at that size (each identity's quickest), u16 how many busted it, u16 the quickest's tenths, 16-byte his name (27 bytes) |
| DIVE_FEED (45) | server → client | v19: u8 kind (`feed_deleted` 1, `feed_won` 2, `feed_guardian` 3 a party's guardian down, `feed_flare` 4 a diver waits at a door), 16-byte name, u8 depth, u8 side, u8 way (a flare's: the door's), u16 cause (a deletion's), u8 party size (a guardian's: 2..4; else 0): 24 bytes |
| DIVERS (46) | server → client | v19: u16 how many dive now, u8 count (0..`divers_most`, 16), then 19 bytes a diver, the deepest first: 16-byte name, u8 depth, u8 act, u8 flags (bit 0 `divers_fighting`, in a party battle) |
| CREW (47) | client → server | v19: u8 action (1 `crew_ask`, 2 `crew_answer`, 3 `crew_split` (or an ask taken back), 4 `crew_way`, 5 `crew_battle`, 6 `crew_join`), u32 reference (the diver asked, the asker, 0, 0, 0, the battle's owner), u8 value (an answer's 1 yes; a battle's its index on the layer; else 0), u8 depth, u8 side, u8 way (`crew_way`'s: the fork's portal he stands on, depth 0 none): 10 bytes |
| CREW_STATE (48) | server → client | v19: u8 stage (1 `crew_asked`, 2 `crew_waiting`, 3 `crew_formed` (the crew as it stands, its ways), 4 `crew_split_up`, 5 `crew_offer` a crewmate's battle offered, 6 `crew_alone` the battle's owner fights alone), u32 the other (asked: the asker; waiting: the one asked; split: who split; offer: the owner), u8 value (an offer's battle index), then four members, zeros past them: u32 session, 16-byte name, u8 depth, u8 side, u8 way (23 bytes each; 99 in all) |
| QUEUE (49) | client → server | v19: u8 action (1 `queue_join`, 2 `queue_leave`, 3 `queue_start`, 4 `queue_pass`), u8 depth, u8 side, u8 way (a join's door: his layer's; else zeros): 5 bytes |
| QUEUE_STATE (50) | server → client | v19: u8 stage (1 `queue_waiting`, 2 `queue_started`, 3 `queue_out`, 4 `queue_alone`), u8 depth, u8 side, u8 way, u8 the recipient's place (1 the leader, 0 none), u8 count, then four entries, zeros past them: u32 session, 16-byte name, u8 flags (bit 0 `queue_arena`, his system runs the Arena): 21 bytes each, 91 in all |
| PARTY_DAMAGE (51) | client → server → seat 0 | v19, a linked fighter's: u32 battle ID, u16 his damage to the guardian so far, u16 his HP (9 bytes); seat 0 hears it as ARENA_SEAT, the sender's seat first |
| PARTY_HP (52) | client → server → linked fighters | v19, seat 0's: u32 battle ID, u16 the guardian's HP, u16 its max HP (9 bytes), to every linked fighter of the battle as sent |
| WEEK_SPLITS (53) | server → client | v19, with every WEEK_BOARD: u16 this week, then sixteen entries, act 1's first, each act's alone then parties of two, three and four: u16 tenths of the week's fastest bust of that act's guardian (0 none), 16-byte name (291 bytes) |

HELLO's fixed prefix is 76 bytes, WELCOME is 38, CARD is 90, TRADE 12,
TRADE_STATE 32, ARENA 9 (8 before v15), ARENA_STATE 45 (28 before v15), OFFER 5, WEEK_POST 18 (17 before v19),
ARENA_STATUS 20, WATCH 6, WATCH_STATE 24 (15 before v15), TEAM 7 and TEAM_STATE 37 (36 before v15); a STANDS entry is 63 (62 before v12) and
WATCH_KEYS's head 12; v19's: DIVE by action as its row says, DIVE_ROOM 9, DIVE_PLACE 27,
DIVE_FEED 24, DIVERS's head 4 and its entry 19, CREW 10, CREW_STATE 99,
QUEUE 5, QUEUE_STATE 91, PARTY_DAMAGE and PARTY_HP 9 each, WEEK_SPLITS
291, DIVE_TRACES's head 3 (each count with its type byte). The
profile fields are u16 HP, u16 maximum HP, u8 attack, u8 speed, u8 charge,
u16 Souls mask, u16 Crosses mask, u16 depth, u8 act, u16 best depth,
u32 title marks, and 49 row-major NaviCust color cells (a 7×7 grid).
Cells are 0 empty or colors 1..6; these are not BN6's piece indexes.
Maximum HP is 1..9999, HP is 0..maximum, buster stats are 0..5, Souls
uses six bits, Crosses five, and title marks sixteen. Profiles contain
no folder, Pack, currency or save bytes. They are client claims for
display, never authoritative progression or anti-cheat evidence.

Phrase IDs are 0..7 and emote IDs 0..11 (v14: Wave, Cheer, Laugh,
Surprised, Thinking, Bow, Got it!, Buster, Phew..., I'm beat, Spin,
Zapped). The client owns their localized words and visuals (MegaMan's
BN6 poses and sounds: docs/online/square/emotes.md). No arbitrary chat text is accepted. NOTICE codes are
10 channel unavailable, 11 muted, 12 social cooldown and 13 Inspect
target unavailable; the shared dive's (v19) 30 `notice_dive_out` (that
takes a dive), 31 `notice_dive_in` (not on a dive), 32
`notice_dive_wait` (traces too fast), 33 `notice_trace_gone` (that
phrase is gone), 34 `notice_crew` (a crew's ask refused) and 35
`notice_queue` (a door's queue refused). ERROR closes the connection; it includes version
and identity mismatch, rate violation and moderation. ERROR 12 tells a
session that a newer connection with the same profile replaced it.
ERROR 2 (another protocol) and 3 (another Square generator) now say the
game is the older side (v10): a game newer than the server, by its
protocol or its SQUARE_MAKE, is answered ERROR 17
(`error_server_older`), so each says where the newer one is. ERROR 16
(`error_maintenance`) turns every HELLO away while the server's
`server.ini` names a maintenance line; its text is that line (at most
`maintenance_bytes`, 40, printable ASCII: "until 14:00 UTC"), which the
gate shows, and `/health` carries it as `maintenance`.

Names have 1..16 ASCII letters, digits, spaces, underscores or hyphens,
starting with a letter or digit. Coordinates are bounded to ±8192 and
snapshots exclude the recipient and other channels. Session IDs belong
only to the current connection. Public identities are the first 16 bytes
of HMAC-SHA256(server identity secret, private guest key); they remain
stable on that server and differ between servers. The private key is
never relayed, logged or stored by the server. TLS protects it in transit;
possession of the key is the guest's identity. Resetting one's profile
can evade an anonymous-profile ban, which is an inherent limitation.
WELCOME's capacity is 1..8; at most seven peers are visible in a channel.
The total player cap is configured separately, up to 256. Local block and
mute refer to public identities rather than reusable session IDs.

HELLO names verified ROM identities, never supplies a ROM: BN6's must
be `protocol.json`'s `rom_code` and `rom_sha1` (ERROR 4), BN5's either
all zeros or `bn5_code` and `bn5_sha1` (ERROR 15), BN4's all zeros or
`bn4_code` and `bn4_sha1` (ERROR 15, v17). The server keeps each
session's ROMs as bits (`roms_bn5`, `roms_bn4`; `roms_one_extra` where
HELLO said its system reads one at most and it named one game or none,
v18) and tells the others in SNAPSHOT. No
Capcom graphics, audio, text, code or save bytes cross the protocol.
Compatibility metadata is not anti-cheat attestation: a modified client
can forge it.

The Net Square is every game's; the Arena, Trade Central and the week's
posts need the full set (protocol v9; docs/online/netpass.md; from v18
Trade Central a full pass, which a system of one extra ROM holds with
one game: Protocol v18, below).
Walking, phrases and emotes, Inspect, channels, STANDS, BOARD and
WEEK_BOARD go to every session. A challenge or an answer (ARENA 1 and
2), a WATCH, a proposal, an answer or a take (TRADE 1, 2 and 5), an
OFFER post and a WEEK_POST need the full set: every bit of `roms_full`
in the session's ROMs, the games this build supports for online play
(`roms_full` is 33 from v17, BN5's bit and BN4's; BN6's identity every
session has).
A session without it is answered NOTICE 28 (`notice_pass`), and so is
one whose challenge, proposal or take aims at such a session; the server
refuses before anything changes. A leave or an over (ARENA 3 and 4), a
trade's accept or cancel, a WATCH stop and an OFFER take-down are never
refused. Two games of the full set have the same chips and the same
rules, so the server keeps no rules per battle and checks no chip
against a pair. A game added to the set brings its identity to HELLO and
its bit to `roms_full`, in a new protocol version. NOTICE 19 (a BN5 chip
or full rules outside a mixed pair's common ground, protocols v4 to v8)
was retired with them; v18 gives the number to `notice_ground`, a
trade's chip outside its pair's common ground (below). NOTICE 29 (`notice_team`, v12) refuses a
challenge between a team and a lone Navi, or within one team, and an ask
to team up where either is on a team already.

Protocol v18 (docs/online/netpass.md, A system with room for one game):
**a full pass is every game the system can read.** A system that reads
one extra ROM beside BN6's at most (the New 3DS) could never name both
games of v17's full set, and says so: HELLO's channel word carries
`hello_one_extra` (16384, beside the resume bit and clear of every
channel), kept with the session's ROMs as `roms_one_extra` (64, bit 6 of
SNAPSHOT's byte, which tells the others) unless the HELLO names both
games after all. Such a session with one game's bit holds a full pass
(`protocol.pass_full`), and **Trade Central takes a full pass** where it
took the full set: TRADE 1, 2 and 5 and an OFFER post from it, and a
proposal or a take aimed at it. The Arena, WATCH and WEEK_POST still
take the full set (`protocol.full_set`, every bit of `roms_full`): two
games of a NetBattle build the same installs, and what a seed makes of a
dive goes by the ROMs read. Two full passes need not hold the same games
now, so **the server checks a trade's chips against the pair**: a chip
is BN4's from `bn4_chip_first`, else BN5's past `bn6_chip_last` or on
`bn6_blank`, else BN6's and every game's (`protocol.chip_game`); a
proposal's chip, an answer's, a take's two and a post's that a game of
the pair lacks the ROM of (a post's: its own game) is answered NOTICE 19
(`notice_ground`), nothing begun, an answer's ending the trade for both
(TRADE_STATE cancelled). The BOARD sent to a game leaves out the offers
of a chip it does not hold, by the same count (a game with BN4's ROM
alone reads BN4's offers, which `chip_bn5`'s count of every id past
BN6's kept from it). No message changed its size. A v17 game refuses a
SNAPSHOT with bit 6 and a v17 server a HELLO with bit 14 (a channel it
has not: ERROR 7), hence the version: client and server go out
together, and either one older says so (ERROR 2 or 17).

Protocol v17 (docs/online/netpass.md, A game joins the set): **BN4 Red
Sun joins the full set.** HELLO grows by BN4's game code and SHA-1 after
BN5's (`hello_bytes` 100), the session's ROMs gain `roms_bn4` (32, bit 5
of SNAPSHOT's byte, past the busy and link bits) and `roms_full` is both
games' bits, so a game without BN4's ROM is answered NOTICE 28 where one
without BN5's was. A chip of BN4's own is one from `bn4_chip_first`
(527, `XREG4_PACK_FIRST`); an offer of one is left out of the BOARD of a
game without BN4's ROM. Which chips a game lets into a NetBattle or a
trade is its own (docs/online/netpass.md, On the wire); the server still
checks no chip against a pair.

Protocol v16 (docs/multirom/bn4-chips.md, Past 511): **chip ids to
1023** (`chip_last`), for ten-bit chip ids, BN4's chips and BN5's yet
unregistered beyond BN6's 512. A chip on the wire was always its own
u16, so no message changed size or shape; what changed is what the
server and the game take for a chip (a trade, an offer on the board, the
stands' last chip), so two builds that differ never pair. The game's
folders keep their entries in their own form inside it
(`src/core/chip_entry.h`); nothing of that crosses the wire.

Protocol v15 (issue #181; docs/online/arena/two-a-side/overview.md,
Over the server). **A team challenge's format**: ARENA's last byte names
it, `team_relay` (0, the relay of v12) or `team_side_by_side` (1);
ARENA_STATE's CHALLENGED and WAITING tell it, and TEAM_STATE tells all
four. A lone challenge's format is taken as 0. **Side by side**: the
challenge's Yes begins the team match as ever (stage 5), no order is
asked (each team's captain leads, his partner beside him: stage 6 at
once), and its one bout is a battle of four **seats**: seat 0 the
challenger's team's captain (the cable's first GBA), 1 the other
captain (the second GBA), 2 and 3 their partners (side 0's, side 1's).
Each of the four sends ARENA accept when ready; START goes to all four
once all four have, its side byte the recipient's seat, its format 1 and
its four sessions the seats', the input delay from the longest of the
four latencies. What a seat sends (ARENA_LOADOUT, ARENA_INPUT,
ARENA_HASH) goes to the three others as ARENA_SEAT (35): the seat, then
the message as sent; a partner's keys keep sixteen bits (his Custom
screen's picks, his Cross and a Program Advance travel in his stream,
bit 15 set: `TEAM_PICK`, `TEAM_CROSS`, `TEAM_PA` in
src/online/arena_team_custom.h). The wire carries his stream as it
carries a lead's keys; inside each game the pair hands it to his side's
GBA, whose own link packets carry it to both GBAs (docs/online/arena/two-a-side/partners-words.md,
The partners' words in BN6's packet): nothing of the wire changed for
it. The cable's first
(seat 0) sends the report and the over, as in a battle of two; its winner
is his team's or the other's, none a draw (TEAM_STATE 9, result 4); a
leave, a decline, a dropped connection or 30 seconds without all four
ready ends it broken off for both teams (result 3). All four are busy
(SNAPSHOT's busy bits, and a challenge, a trade or a team ask to them
is NOTICE 16). The stands keep a 2v2 by its four seats: WATCH_STATE
names the partners' seats, WATCH_RELAY and WATCH_KEYS carry the seat,
the hashes relayed are the cable's first's. STANDS's bout byte says
`stands_side_by_side` (255); its entry names the two leads and their
report, and the referee calls the leads' teams. A game of another version
is never paired into one: the server answers its HELLO with ERROR 2 or
17, as for any version.

Protocol v14 (issue #176; docs/online/square/emotes.md). **An emote aimed**:
SOCIAL names the session it is aimed at, where it is one of
`emote_aimed`'s (35: Wave, Cheer and Bow, the warm ones; a target on a
phrase or another emote is a ProtocolError); a target gone from the
channel, or the sender himself, is no target. **An answer**: an aimed
emote at a player who aimed one at the sender within `answer_ms` (5000)
is flagged `event_answered` for every recipient, and both ways start
afresh. **The cap**: a sender's aimed emotes at one player unanswered
within `aim_window_ms` (30000) past `aim_cap` (2) go to the sender
alone. **The beat**: EVENT's last field is the milliseconds from its
sending to the server's next beat (its monotonic clock in `beat_ms`
steps, 250) at least the channel's farthest one-way trip (half the
largest round trip its PINGs say) and `beat_lead_ms` (60) ahead, at most
`beat_lead_most_ms` (400); each game starts the emote that long after
the EVENT came, less half its own round trip, so emotes sent together
start together. Twelve emotes (`emote_count`), and the burst above.

Protocol v13 (epic #167's third and fourth steps; docs/online/arena/from-square/rematch-in-a-second.md,
A rematch in a second and The Square before a NetBattle). **A bout's
seed is its battle ID**: every game and spectator writes BN6's two random
words from it into both GBAs of the pair as Practice's last confirm is
pressed, so that each bout deals its own hands and rolls; a game of
another version would build another pair (its ready apart), hence the
version. **A rematch keeps the sides**: the server remembers, for each
pair of players, which was the cable's first in the last battle it
started between them, and a new challenge between the same two (not a
team's) gives him side 0 again whoever asks, so that both games go into
the rematch from the pair they kept (ARENA_STATE's START says the side
as ever; WAITING and CHALLENGED still go to the challenger and the
challenged); a player's leaving forgets his pairs. **A challenge seen**:
the challenged's game sends ARENA 5 (`arena_seen`) as its question opens;
the server tells the challenger (ARENA_STATE 5) and counts the 30
seconds for an answer from then; a challenge not yet seen waits
`pending_unseen` (60) seconds. A team match's bout is seen as it is
called. **The Square's marks**: SNAPSHOT's per-peer byte carries what
each player is busy with and his link's bars, from the round trip his
PING says (above): bars only, never a number in the game, the owner's
word; a game shows the fewer of his bars and its own, the longer round
trip deciding a battle's delay.

A dropped link (protocol v10, issue #153; docs/online/square/dropped-link.md, A dropped
link). A session that ends without a goodbye (BYE or a WebSocket
close), its socket gone or silent for 15 seconds, keeps its place for 40
seconds (`server/cyberworld_server/holds.py`): its slot in its channel,
its session ID and its MegaMan, standing where he stood, in the others'
SNAPSHOTs; a trade, a challenge or a battle aimed at it meanwhile is
answered NOTICE 16. What it had under way ends at once, as on any leave:
a trade not committed is cancelled for both, a battle ends for the other
(ARENA_STATE ended), watching stops, its board offer is taken down. A
HELLO from the same identity with the resume bit takes the place back:
the WELCOME names the same session ID and channel (where the old session
is not yet known to be gone, it is told ERROR 12), the session's last
trade end (done or cancelled) is sent again after it, as the link may
have lost it (a game sets aside a TRADE_STATE it has already had), and
nobody else is told anything. One trade a visit holds across it. A HELLO
with the resume bit and no place held (a server restarted, the hold
over) joins its requested channel as any HELLO does; one without the bit
while a place is held is a new visit: the place is given up and the
session arrives anew. A refusal, a moderator's removal or a goodbye
frees the place at once. The game tries again for 30 seconds from the
moment its link went quiet or dropped (src/online/online_transport.h).

The server holds at most 16 unfinished handshakes, each given five
seconds for the HTTP Upgrade and HELLO together; only joined players
count against its player cap. It has a 15-second idle timeout and a token bucket
of 40 messages replenished at 30 per second; STATE has a bucket of its
own, 200 deep and refilled at 25 a second, so a stalled link's backlog of
positions, let through at once as it clears, is no flood (v10). A slow output buffer above
8192 bytes is disconnected. WebSocket pings, pongs and close are handled;
text, fragments, masking violations and oversized frames are rejected.
The client sends positions every 50 ms (every 500 ms once it has heard
nothing for 400 ms) and pings every two seconds.
Social events have a separate burst of `social_burst` (3) with one
restored each `social_refill_ms` (1000; v14, before it two and one every
two seconds); the game keeps the same burst a hair slower, so that its
player never meets NOTICE 12. Profile updates are accepted at most once per second;
Inspect is limited to four requests per second. Numeric payloads are
validated even when throttled. Moderation is read again from private
local files when they change. `/health` reports configuration and occupancy without
player names, public identities, private keys or IPs.

Trade Central's trades are one chip for one chip between two players
of a channel, both with a full pass (NOTICE 28 otherwise), each chip one
both games hold (NOTICE 19 otherwise, v18), neither busy
in another trade or a battle. The proposer names the chip given, the
other answers with his, and the server commits only when both accepted
the same pair as their TRADE_STATE showed it, sends done to both and
appends the trade to its state folder's `trades.jsonl` (identities and
chips, no names). A stale or
mismatched step, either player leaving, or 300 seconds without a step
cancels it for both. Each game applies its side at home; the server
never sees a folder. NOTICE codes 15 (trading is off on this server),
16 (that Navi is busy) and 17 (no longer here, or the trade is over)
answer a TRADE that cannot be done. A session trades once: after its
trade is done, a proposal, a take or a post by it or to it is answered
NOTICE 21 (each game keeps one swap until it is home).

The trade board: each player may have one offer up, a chip he has and
the kind of chip he wants for it, a number the games name (anything, an
element, a Mega); the server never reads a chip's element and checks
only the number's range. A post replaces the last, at most one a second
(NOTICE 22 otherwise), and only a game with the full set posts (NOTICE
28). The offer stays while its poster is in the channel: it goes when
he takes it down, leaves the server or the channel, or trades. The
server sends each player of the channel a BOARD whenever its offers
change, and one after every WELCOME, a game without the full set among
them. A chip is BN5's where its id is past `bn6_chip_last` (319) or on
one of `bn6_blank`'s ranges (BN6's blank records, where a game with
BN5's ROM installs BN5's chips); an offer whose chip is BN5's is left
out of the BOARD of a player whose game lacks BN5's ROM, which could
not name it; from v17 an offer whose chip is BN4's own (from
`bn4_chip_first`) is left out of the BOARD of a player without BN4's ROM.
TRADE 5 takes an offer: the reference is the poster, the
chip taken the one on the board (else NOTICE 20, that offer is gone),
the pass and business as a proposal's; the trade begins answered, both
chips named, and goes on as any (both accept, the server commits). The
poster's game checks that the chip meets his want and cancels it
otherwise.

The week's net (docs/online/): every Monday 00:00 UTC a server
begins a week; its seed is the first four bytes of HMAC-SHA256(the
server's identity secret, "week" and the week's number as a u32), 1
where those are 0, so each server has its own. A WEEK_POST is taken
when its week is this one or the last, its seed that week's, its layer,
act and flags as above and its hash not zero; else NOTICE 23 (that
week's net is closed), 24 (not that week's net), 25 (the dive does not
add up) or 26 (a moderator took that week's post down). The server keeps
one entry per public identity and week, the best (deeper, then won,
then fewer minutes, then the earlier post), and answers every post with
a WEEK_BOARD, to every player where the Board changed. A WEEK_BOARD
follows every WELCOME. Entries are claims for display: the server checks
only their shape, and nothing is given for one.

The Arena's messages are relayed only between the two players of a
started battle, and only in its shape; the server never sees a battle.
Each game runs both players' linked BN6 cores and only their keys cross
(docs/online/arena/): START gives each its side of the cable and the
input delay, from both players' latencies: the lockstep's frame, then the longer latency in frames, 1 at least (v8: the lockstep a frame, not 3, and each game's latency less what its rollback window covers; issue #167: less its window but a frame, the server's tick for a key to be taken no longer added; docs/online/arena/rollback/overview.md and Quicker on the wire)
(`server/cyberworld_server/arena.py`, docs/online/arena/smoothed-delay.md).
A challenge waits 30 seconds for its answer. One player of a battle
sends ARENA's over (the cable's first, as the battle ends on its cores);
a leave, a decline or a dropped connection ends it for both, ARENA_STATE
ended telling each. The relayed messages have a bucket of their own,
150 deep and refilled at 120 a second. NOTICE codes 16 (that Navi is
busy), 17 (no longer here, or the battle is over), 18 (the Arena is
closed on this server) and 28 (a challenge from or to a game without the
full set, or an answer from one) answer an ARENA that cannot be done.
Every battle is by full rules, BN5's chips and Souls among them
(docs/online/arena/): v9's ARENA, ARENA_STATE, STANDS and WATCH_STATE
carry no rules byte.

Team NetBattles (protocol v12, epic #167; docs/online/arena/teams.md, Team
NetBattles; `server/cyberworld_server/teams.py`): a relay of two teams of
two. TEAM 1 asks a player of the channel to team up (he is told stage
asked, the asker waiting); 2 answers it within 30 seconds (formed to
both, or split to the asker); either one's 3, his leaving the channel or
the server, or a dropped link split the team (split to the other). The
one who asked is the team's captain. A challenge (ARENA 1) between
players of two teams is a team challenge, all four free (NOTICE 16
otherwise); one between a team's player and a lone one, or two of one
team, is refused with NOTICE 29 (`notice_team`). The team challenge's
accept begins the match instead of a battle: its ARENA battle ends with
no ARENA_STATE ended, and all four are told stage 5. Each captain sends
TEAM 4 with his lead (his own session or his partner's); the server
shows nothing until both are in (or 30 seconds have passed, a captain
who has not picked leading himself), then tells all four stage 6 and
calls the first bout. A bout is an ARENA battle between the two teams'
MegaMen up (stage 7, its ID in TEAM_STATE): each of its two fighters
sends ARENA 2 when his game is ready, and START goes to both once both
have; stage 8 tells all four it started, and the two others' games
watch it (WATCH). As it ends (ARENA 4 from the cable's first; another
seat's ARENA 4 is ignored, side by side too) its
winner is the cable's first's report (ARENA_STATUS's winner): the
loser's team's next MegaMan comes in, both where it has none; a team
with no MegaMan standing ends the match (stage 9, each told his team's
result, a draw where both ran out in one bout). A bout that ends any
other way (a leave, a drop, a decline, 30 seconds without both
fighters ready) ends the match broken off for both teams (stage 9,
result 3). The staying MegaMan's game carries the HP he has left in his
ARENA_LOADOUT (v2), and both games write it into the battle as it
starts (the pair's hash at ready covers it). A player in a team match is
busy: a challenge, a trade or a team ask to him is answered NOTICE 16.
STANDS's last byte names a team bout's number for the referee and the
Board. The server keeps teams and matches in memory only.

The stands (protocol v7, docs/online/arena/stands.md): every
started battle's loadouts, ready, installs' record, the cable's first's
hashes every 60 frames and both players' keys from frame 0 are kept
while it runs (the frames before a side's first keys, its input delay's,
as released keys; 72,000 frames a side at most, past which and at a
missing frame the battle is no longer watched) and a minute after it. A
player of the channel who is in no battle asks to watch with WATCH; the
server answers WATCH_STATE watching, then hands on both MegaMen, the
record and the ready (WATCH_RELAY), the keys (WATCH_KEYS, the side
behind first) and the hashes once both sides' keys are past them, at
most 2 KB a tick (20 a second) and only while the spectator's output is
under 4 KB; where the battle is over and all is sent, WATCH_STATE over.
NOTICE 17 (that battle is over), 16 (a player of it, or busy in a
battle), 18 (the Arena closed), 28 (a spectator without the full set)
and 27 (the stands are full: `watch_most`, 6) answer a WATCH that cannot
be done. A spectator is busy: a challenge to him is answered NOTICE 16.
STANDS goes to every player of a channel, a game without the full set
among them, as a battle begins or ends, and at most once a second as
watchers, cheers or the report change, and after every WELCOME; the two players of a battle
under way are told once it is over. A phrase or emote of the channel's
(SOCIAL) while a battle is on counts as a cheer for it, and its EVENT is
held from the battle's two players: they hear nothing of the stands until
their battle is over. ARENA_STATUS is taken from the cable's first player
of a started battle, at most twice a second but for the one naming the
winner. Nothing a spectator sends reaches the battle.

Protocol v19 (issue #589, docs/online/shared-dive.md): **the shared dive**, the server's side and
the wire ahead of the game's. The week's net is dived with everyone
diving it at the same time; this version carries it.

**Rooms by layer.** DIVE enter names a layer of this week's or the
last's net (its seed that week's: NOTICE 23 and 24, as WEEK_POST's) and
takes the NetPass's DeepNet stamp, the full set (NOTICE 28). The
session leaves its Square channel as CHANNEL does (its trade, battle,
team and watching end) and stands in a room of that layer, keyed by
(week, seed, depth, side, way): an instance of at most the server's
room capacity (eight), a new one where those are full, numbered from 1
in DIVE_ROOM. A room is a channel of its own past every Square channel,
so STATE, SNAPSHOT, SOCIAL and INSPECT work in it as in the Square (the
layer's world units, bounded as ever), and no Square sees a diver; a
room's SNAPSHOT marks each peer `peer_diving`. DIVE enter again moves
him to another layer's room; DIVE leave, or CHANNEL, goes back to a
Square channel with a fresh WELCOME, the dive's crew, queue and battle
left. A diver's TRADE, OFFER, TEAM, WATCH and ARENA challenge or accept
are answered NOTICE 31 (`notice_dive_in`); DIVE's other actions, CREW
and QUEUE from a session not in a room NOTICE 30 (`notice_dive_out`). A
dropped diver's place is held in no room: his resume's WELCOME names a
Square channel, and his game dives again.

**Traces of the week**, kept by layer (every instance of it) for this
week and the last, pruned past them, each list capped at `traces_most`
(32, oldest out; a battle's busts 64 identities, the slowest out;
a week's layers 512, the one touched longest ago out), appended to the
state folder's `dives.jsonl` and read back at the start as `weeks.jsonl`
is (`server/cyberworld_server/traces.py`). Entering a room sends the
layer's in DIVE_TRACES (marks, phrases, first finds, the opened set,
splits), and each new one goes to every diver of the layer as it
happens: an end's mark (a win only on layer `week_layers`, NOTICE 25
otherwise); a find, its index added to the opened set and, the first
this week, its finder named; a phrase, one a diver a layer (his new one
replaces his last: `trace_gone`) and a Wave at one by its ID, counted
(a phrase gone: NOTICE 33, `notice_trace_gone`); a split, its battle's
quickest bust at that party size kept by identity, answered with the
sender's place (DIVE_PLACE). A muted identity leaves no phrase (NOTICE
11), a hidden one no trace (NOTICE 26), and traces come at SOCIAL's
burst and refill (NOTICE 32, `notice_dive_wait`). A moderator's
`--admin hide` takes that week's marks, phrases and first finds of that
identity off every layer (the opened set names nobody and stays), and
`forget` his traces of every week.

**The feed and the pack.** An end goes to every diver of that week and
every Square session as DIVE_FEED (deleted or won, the name, the depth,
the cause), and so does a door's party battle won (seat 0's report's
winner the first side) with the party's size. A diver joining a door's
queue is a flare to every diver of the week behind him in the same act
(a lower depth, three layers an act, the Nest the fourth), the door's
depth, side and way with it. DIVERS goes to every session after each
WELCOME and, where it changed, at most every `divers_ms` (2000).

**Crews** (`crews.py`), modelled on TEAM: an ask only to a diver of the
same room (NOTICE 17 otherwise; 16 where an ask is out already; 34,
`notice_crew`, where the other is in a crew or the crew holds
`party_most`), answered within 30 seconds; any member asks. A crew lasts
across layers until a member splits up (the rest go on where two or more
stay), leaves the dive or the server, or ends his dive (DIVE end).
`crew_way` tells every member which fork's portal each stands on, so each
game opens it only when all agree. `crew_battle` offers a crewmate's
random battle to his crewmates in the same room; `crew_join` answers
within `crew_join_ms` (3000). When everyone offered has joined, or the
window ends, a party battle starts, the owner seat 0 and the joiners
after him; with nobody, the owner is told `crew_alone`.

**The door's queue and party battles** (`party.py`). QUEUE join names
the door of the sender's layer (NOTICE 35, `notice_queue`, for another,
for a queue of `party_most` already, or for a start or a pass from
anyone but its leader). The first in it leads and starts when he likes;
alone, he is told `queue_alone`. His game's pass, after a minute idle,
puts him last. A start makes a party battle: a battle ID from the
Arena's count and seats in queue order, each told with ARENA_STATE START
(its seat, the delay from the seats' round trips, its format, the
seats' sessions) after QUEUE_STATE started. A seat's format is
`arena_party` where both its system and seat 0's run the Arena, else
`arena_linked` (HELLO's `hello_no_arena`: the New 3DS and the PSP join
as linked fighters, their own battle, their damage into the field's
guardian). Party seats' ARENA_LOADOUT, ARENA_INPUT and ARENA_HASH go to
the other party seats as ARENA_SEAT; a linked fighter's PARTY_DAMAGE
goes to seat 0 as ARENA_SEAT; seat 0's PARTY_HP goes to the linked
fighters. Seat 0 reports with ARENA_STATUS and ends the battle with ARENA
over (ARENA_STATE ended to every seat still in it); a seat's ARENA leave,
its leaving the dive or its dropped link is told to the others as
ARENA_STATE `arena_seat_gone`, and the battle goes on for the rest, each
game finishing its own. Party battles are busy (SNAPSHOT's busy bits,
DIVERS's flag) and need no `arena` switch on the server. The stands do
not show them yet.

**The Board.** WEEK_POST grows its last byte, the acts whose guardian
was fought in a party (none past the dive's act: NOTICE 25), kept with
the best entry; WEEK_BOARD's entry flags carry it as `week_party`.
WEEK_SPLITS follows every WEEK_BOARD and goes to everyone when a
guardian's split changes the week's fastest of its act and size.

A v18 game would refuse a SNAPSHOT with bit 7, a DIVERS or a WEEK_SPLITS
and send a WEEK_POST a byte short, hence the version: client and server
go out together (ERROR 2 or 17 for either one older).

**Saves sent by a code** (docs/engine/saves.md, Sending by code). A
connection to `/presence` whose first message is TRANSFER (36) rather
than HELLO is no visit: it takes no place, no channel and no identity,
and goes no further than its one exchange. It changes no message of a
visit's, so it has no protocol version of its own and bumps none: a
server from before it answers TRANSFER as it answers any first message
that is not a HELLO, with an ERROR, and the game says that server takes
no saves.

| Message | Direction | Payload after the one-byte type |
| --- | --- | --- |
| TRANSFER (36) | client → server, first | u8 `transfer_version` (1), u8 op (`transfer_put` 1, `transfer_get` 2, `transfer_done` 3), 16-byte lookup, u32 size (a put's: the blob's bytes, 1 to `transfer_max`; else 0): `transfer_bytes` (23) |
| TRANSFER_STATE (37) | server → client | u8 status, u32 its value: `transfer_state_bytes` (6) |
| TRANSFER_DATA (38) | either way | the blob's next bytes, 1 to 511, in order |

A put: TRANSFER put, then the server's READY (0); the blob's bytes in
TRANSFER_DATA; then HELD (1, the value the seconds it is held:
`transfer_minutes` 15 as 900) once the last byte is in, and the server
closes. A put whose lookup is held already is answered TAKEN (5): the
game makes another code. A get: TRANSFER get, then FOUND (2, the value
the blob's size) and its bytes in TRANSFER_DATA, or NONE (3) where
nothing is held under that lookup (never held, gone after its 15
minutes, or let go); the game then sends TRANSFER done on the same
connection, once it has opened the blob, and the server answers REMOVED
(7) and lets it go. A blob a get found and no DONE let go stays until its
time is up, so a broken download can be tried again. BUSY (4, the value
the seconds to wait) answers a put while the server holds as much as it
will, a put or a get past the lookups it allows a minute (a put's TAKEN
would otherwise tell which lookups are held), and either while as many
transfers are under way as it takes; OFF (6) answers both on a
server whose `transfers` setting is off. Anything else, a DATA too long
or past the size, or a connection quiet for 15 seconds, ends it with an
ERROR.

The lookup and the blob are the game's: the code a player reads off one
system and types on the other is never sent, and the server can open
neither the blob nor the code from what it holds (docs/engine/saves.md
says how they are made). The server keeps them in memory alone, never in
a file, a log or a backup, and counts the day's held and taken in its
daily totals.

**Saves kept in step** (docs/engine/saves.md, Saves kept in step). Like
TRANSFER, a connection to `/presence` whose first message is SYNC (39)
is one exchange and no visit, and bumps no version: a server from before
it answers SYNC with an ERROR. A slot is a place on the server for one
player's saves, named by a 16-byte slot id and opened by a 32-byte auth,
both made by the game from a secret the systems it links share (the
secret travels between them as a TRANSFER, never to the server). The
server keeps a slot's blob on disk, sealed by the game and unread, with
its generation, a count it raises at each push.

| Message | Direction | Payload after the one-byte type |
| --- | --- | --- |
| SYNC (39) | client → server, first | u8 `sync_version` (1), u8 op (`sync_pull` 1, `sync_push` 2, `sync_drop` 3), 16-byte slot, 32-byte auth, u32 generation, u32 size (a push's: the blob's bytes, 1 to `sync_max`; else 0): `sync_bytes` (59) |
| SYNC_STATE (40) | server → client | u8 status, u32 generation, u32 size (BUSY's: the seconds to wait): `sync_state_bytes` (10) |
| TRANSFER_DATA (38) | either way | the blob's next bytes, 1 to 511, in order |

A pull names the generation the game last had of the slot (0 none): SAME
(0) where it is still the slot's; FOUND (1, the slot's generation and the
blob's size) and the blob in TRANSFER_DATA where the slot has another;
then the server closes. A push names the generation its saves were made
from: READY (4), the blob in TRANSFER_DATA, then STORED (5, the slot's new
generation, one past the old) once the last byte is on disk; CONFLICT (6,
the slot's generation now) where the slot moved on since, and nothing is
written. A push from generation 0 to a slot the server has not got makes
it, the auth its own from then on; to a slot it has, generation 0 is a
CONFLICT like any other. A drop deletes the slot: DROPPED (9). Any op on
a slot the server has not got (never made, dropped, or unused for
`sync_days`, 180) is NONE (2), but a push from 0, and on a slot with
another auth DENIED (3). BUSY (7, the seconds to wait in the size field)
answers where the server keeps as many slots or bytes as it will, makes
no more slots this hour, or has as many exchanges under way as it takes,
and a push to a slot another push is filling; OFF (8) every SYNC on a
server whose `sync` setting is off. Anything else, a DATA too long or past
the size, or a connection quiet for 15 seconds, ends it with an ERROR.

The server stores the auth's SHA-256, never the auth, and can open no
blob: the key that seals it is made from the shared secret too and never
sent. A slot's files are left out of the server's backups, and the game
keeps every save it has on the system, so a slot lost loses only the
step, never the saves. The daily totals count the pulls and pushes and
the slots kept.

Changing the wire shape bumps the protocol version. HELLO's SQUARE_MAKE
and SQUARE_HASH are the Square generator's own version and its plan's
hash (`src/online/square_plan.h`), checked by the server against
`protocol.json`'s `square_make` and `square_hash`: a change to what a seed
makes bumps SQUARE_MAKE, not the protocol. The run's `LAYER_MAKE` keeps
its own meaning and no longer crosses the wire.
