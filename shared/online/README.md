# Net Square protocol v7

`protocol.json` is the server's packaged contract. `protocol.h` exposes
the same constants to the C client; server tests check they agree with
the game generator. All multibyte fields are big-endian. Messages are
unfragmented binary WebSocket frames, at most 512 payload bytes, on
`/presence` with subprotocol `cyberworld.presence.v7`.

| Message | Direction | Payload after the one-byte type |
| --- | --- | --- |
| HELLO (1) | client → server, first | u16 protocol, u16 SQUARE_MAKE, u32 SQUARE_HASH, 4-byte ROM game code, 20-byte ROM SHA-1 (BN6's), 4-byte and 20-byte BN5 Team Colonel's game code and SHA-1 (all zeros where the game has not its ROM), 16-byte private guest key, u16 requested channel (0 chooses one), u8 name length, name bytes |
| WELCOME (2) | server → client | u32 world seed, u32 session player ID, u16 tick Hz, u8 room capacity, u16 channel, 16-byte public identity, u16 channel count, u32 the week's seed (never 0), u16 the week's number (weeks since Monday 5 January 1970, `week_epoch`) |
| STATE (3) | client → server | i32 x, i32 y, i32 z in whole world units, u8 facing (0..7) |
| SNAPSHOT (4) | server → client | u8 peer count, then 50 bytes per peer: u32 session ID, 16-byte public identity, three i32 coordinates, u8 facing, 16-byte zero-padded name, u8 ROMs (bit 0, `roms_bn5`: his game has BN5 Team Colonel's ROM; no other bit) |
| ERROR (5) | server → client | u16 reason code, at most 96 printable ASCII bytes |
| PING (6) / PONG (7) | client → server / server → client | u32 client clock value, echoed unchanged |
| PROFILE (8) | client → server | 69-byte public profile, described below |
| INSPECT (9) | client → server | u32 target session ID in the same channel |
| CARD (10) | server → client | u32 session ID, 16-byte public identity, 69-byte public profile |
| SOCIAL (11) | client → server | u8 kind (0 phrase, 1 emote), u8 fixed choice ID |
| EVENT (12) | server → channel, including sender | u32 sender session ID, u8 kind, u8 fixed choice ID |
| CHANNEL (13) | client → server | u16 requested channel (0 chooses one); success sends a fresh WELCOME |
| NOTICE (14) | server → client | u16 reason, at most 96 printable ASCII bytes; an action failed but the connection stays open |
| TRADE (15) | client → server | u8 action (1 propose, 2 answer, 3 accept, 4 cancel, 5 take a board offer), u32 reference (a proposal's or a take's target session ID, else the trade's ID), the chip given (u16 id 1..511, u8 code 0..26, 26 the `*`), the chip taken (the same; zero in a proposal; a take's is the offer's chip) |
| TRADE_STATE (16) | server → client | u8 stage (1 offered, 2 waiting, 3 answered, 4 accepted, 5 done, 6 cancelled), u32 trade ID, u32 the other player's session ID, 16-byte his public identity, my chip and his (u16 id, u8 code each; zero where not yet named) |
| ARENA (17) | client → server | u8 action (1 challenge, 2 accept, 3 leave or decline, 4 over), u32 reference (a challenge's target session ID, else the battle's ID), u16 the sender's latency in ms (0..5000; its round trip to the server with its jitter: the mean and twice the spread of its last pings), u8 rules (a challenge's: 0 `format_bn6`, 1 `format_full`; ignored on the others, 0..1) |
| ARENA_STATE (18) | server → client | u8 stage (1 challenged, 2 waiting, 3 start, 4 ended), u32 battle ID, u32 the other player's session ID, 16-byte his public identity, u8 side (0 the cable's first GBA, 1 the second; START's), u8 input delay in frames (START's), u8 the challenge's rules |
| ARENA_LOADOUT (19) | client → server → opponent | u32 battle ID, u16 length (1..400), that many bytes: the sender's MegaMan as `src/online/arena_wire.h` packs him |
| ARENA_INPUT (20) | client → server → opponent | u32 battle ID, u32 first frame, u8 count (1..8), count u16 GBA keys, one a frame from the first |
| ARENA_HASH (21) | client → server → opponent | u32 battle ID, u32 frame, u64 the two linked cores' state hash after it; frame 0xFFFFFFFF says ready, the hash the pair's at the Custom screen; frame 0xFFFFFFFE, sent just before it, the installs' record its cores were made with (0 under BN6's rules), so that a ready apart is told by its cause (a game that sends none is told the plain stop) |
| OFFER (22) | client → server | u16 chip id (1..511; 0 takes the sender's offer down), u8 code (0..26), u8 want (0..`want_last`, 9: the game's fixed list, 0 anything); all zeros to take it down |
| BOARD (23) | server → client | u8 count (0..8), then 8 bytes an offer: u32 the poster's session ID, u16 chip id, u8 code, u8 want; the recipient's channel's offers he may see, his own among them |
| WEEK_POST (24) | client → server | u16 week, u32 that week's seed, u16 layer (1..`week_layers`, 10), u8 act (that layer's: three layers an act, the Nest the fourth), u8 flags (bit 0 won, only on layer 10; no other bit), u16 minutes, u32 summary hash (not 0) |
| WEEK_BOARD (25) | server → client | u16 this week, u8 count (0..8), u8 last week's count (0..3), u8 the recipient's place this week (0 none), then 22 bytes an entry, the recipient's own best first (zeros where none), then this week's, then last week's: 16-byte zero-padded name, u16 layer, u8 act, u8 flags (bit 0 won), u16 minutes |
| STANDS (26) | server → client | u8 count (0..`stands_max`, 5), then 63 bytes a battle of the recipient's channel (those on, then its last one over): u32 battle ID, u32 the cable's first player's session ID, u32 the second's, two 16-byte zero-padded names in that order, u8 rules, u8 stage (1 the way in, 2 fighting, 3 over with a winner, 4 over without), u8 watching, u8 cheers, then the report: u16 HP and u16 max HP of the first, the same of the second, u16 the last chip each used (0 none), u8 chips each used, u8 winner (0 none, 1 the first, 2 the second) |
| ARENA_STATUS (27) | client → server | u32 battle ID, then the report as STANDS holds it (15 bytes); taken from the cable's first player alone |
| WATCH (28) | client → server | u8 action (1 watch, 2 stop), u32 battle ID |
| WATCH_STATE (29) | server → spectator | u8 stage (1 watching: the battle's setup; 2 over: no more keys come), u32 battle ID, u32 the cable's first player's session ID, u32 the second's, u8 rules, u8 input delay |
| WATCH_RELAY (30) | server → spectator | u8 side (0 the cable's first), then that player's ARENA_LOADOUT or ARENA_HASH as he sent it |
| WATCH_KEYS (31) | server → spectator | u32 battle ID, u8 side, u32 first frame, u16 count (1..`watch_keys_max`, 240), count u16 GBA keys, one a frame from the first |

HELLO's fixed prefix is 76 bytes, WELCOME is 38, CARD is 90, TRADE 12,
TRADE_STATE 32, ARENA 9, ARENA_STATE 29, OFFER 5, WEEK_POST 17,
ARENA_STATUS 20, WATCH 6 and WATCH_STATE 16; a STANDS entry is 63 and
WATCH_KEYS's head 12. The
profile fields are u16 HP, u16 maximum HP, u8 attack, u8 speed, u8 charge,
u16 Souls mask, u16 Crosses mask, u16 depth, u8 act, u16 best depth,
u32 title marks, and 49 row-major NaviCust color cells (a 7×7 grid).
Cells are 0 empty or colors 1..6; these are not BN6's piece indexes.
Maximum HP is 1..9999, HP is 0..maximum, buster stats are 0..5, Souls
uses six bits, Crosses five, and title marks sixteen. Profiles contain
no folder, Pack, currency or save bytes. They are client claims for
display, never authoritative progression or anti-cheat evidence.

Phrase IDs are 0..7 and emote IDs 0..5. The client owns their localized
words and visuals. No arbitrary chat text is accepted. NOTICE codes are
10 channel unavailable, 11 muted, 12 social cooldown and 13 Inspect
target unavailable. ERROR closes the connection; it includes version
and identity mismatch, rate violation and moderation. ERROR 12 tells a
session that a newer connection with the same profile replaced it.

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
all zeros or `bn5_code` and `bn5_sha1` (ERROR 15). The server keeps each
player's ROMs and tells the others in SNAPSHOT; what a pair does
together goes by what both have (docs/ONLINE.md, the pair's common
ground). No Capcom
graphics, audio, text, code or save bytes cross the protocol. Compatibility
metadata is not anti-cheat attestation: a modified client can forge it.

The server holds at most 16 unfinished handshakes, each given five
seconds for the HTTP Upgrade and HELLO together; only joined players
count against its player cap. It has a 15-second idle timeout and a token bucket
of 40 messages replenished at 30 per second. A slow output buffer above
8192 bytes is disconnected. WebSocket pings, pongs and close are handled;
text, fragments, masking violations and oversized frames are rejected.
The client sends positions every 50 ms and pings every two seconds.
Social events have a separate two-event burst with one token restored
every two seconds. Profile updates are accepted at most once per second;
Inspect is limited to four requests per second. Numeric payloads are
validated even when throttled. Moderation is read again from private
local files when they change. `/health` reports configuration and occupancy without
player names, public identities, private keys or IPs.

Trade Central's trades are one chip for one chip between two players
of a channel, neither busy in another trade or a battle. A chip is
BN5's where its id is past `bn6_chip_last` (319) or on one of
`bn6_blank`'s ranges (BN6's blank records, where a game with BN5's ROM
installs BN5's chips); BN5's chips trade only between two players whose
ROMs both have BN5's bit, else the server answers NOTICE 19 (a proposal
refused, an answer ending the trade). The proposer
names the chip given, the other answers with his, and the server
commits only when both accepted the same pair as their TRADE_STATE
showed it, sends done to both and appends the trade to its state
folder's `trades.jsonl` (identities and chips, no names). A stale or
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
(NOTICE 22 otherwise); a BN5 chip is posted only by a game with BN5's
ROM (NOTICE 19). The offer stays while its poster is in the channel: it
goes when he takes it down, leaves the server or the channel, or trades.
The server sends each player of the channel a BOARD whenever its offers
change, and one after every WELCOME; an offer whose chip is BN5's is
left out of the BOARD of a player whose game lacks BN5's ROM. TRADE 5
takes an offer: the reference is the poster, the chip taken the one on
the board (else NOTICE 20, that offer is gone), the pair's common ground
and business as a proposal's; the trade begins answered, both chips
named, and goes on as any (both accept, the server commits). The
poster's game checks that the chip meets his want and cancels it
otherwise.

The week's net (docs/ONLINE.md): every Monday 00:00 UTC a server
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
(docs/ONLINE_ARENA.md): START gives each its side of the cable and the
input delay, from both players' latencies: the lockstep's 3 frames, a tick, then the longer latency in frames
(`server/cyberworld_server/arena.py`, docs/ONLINE_ARENA.md's Smoothed delay).
A challenge waits 30 seconds for its answer. One player of a battle
sends ARENA's over (the cable's first, as the battle ends on its cores);
a leave, a decline or a dropped connection ends it for both, ARENA_STATE
ended telling each. The relayed messages have a bucket of their own,
150 deep and refilled at 120 a second. NOTICE codes 16 (that Navi is
busy), 17 (no longer here, or the battle is over) and 18 (the Arena is
closed on this server) answer an ARENA that cannot be done, and 19 a
challenge to full rules where either game lacks BN5's ROM; a challenge's
rules come back in every ARENA_STATE of its battle.

The stands (protocol v7, docs/ONLINE_ARENA.md, The stands): every
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
battle), 18 (the Arena closed), 19 (full rules need BN5's ROM on the
spectator's game too) and 27 (the stands are full: `watch_most`, 6)
answer a WATCH that cannot be done. A spectator is busy: a challenge to
him is answered NOTICE 16. STANDS goes to every player of a channel as a
battle begins or ends, and at most once a second as watchers, cheers or
the report change, and after every WELCOME; the two players of a battle
under way are told once it is over. A phrase or emote of the channel's
(SOCIAL) while a battle is on counts as a cheer for it, and its EVENT is
held from the battle's two players: they hear nothing of the stands until
their battle is over. ARENA_STATUS is taken from the cable's first player
of a started battle, at most twice a second but for the one naming the
winner. Nothing a spectator sends reaches the battle.

Changing the wire shape bumps the protocol version. HELLO's SQUARE_MAKE
and SQUARE_HASH are the Square generator's own version and its plan's
hash (`src/online/square_plan.h`), checked by the server against
`protocol.json`'s `square_make` and `square_hash`: a change to what a seed
makes bumps SQUARE_MAKE, not the protocol. The run's `LAYER_MAKE` keeps
its own meaning and no longer crosses the wire.
