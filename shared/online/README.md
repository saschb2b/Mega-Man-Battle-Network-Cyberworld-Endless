# Net Square protocol v3

`protocol.json` is the server's packaged contract. `protocol.h` exposes
the same constants to the C client; server tests check they agree with
the game generator. All multibyte fields are big-endian. Messages are
unfragmented binary WebSocket frames, at most 512 payload bytes, on
`/presence` with subprotocol `cyberworld.presence.v2`.

| Message | Direction | Payload after the one-byte type |
| --- | --- | --- |
| HELLO (1) | client → server, first | u16 protocol, u16 SQUARE_MAKE, u32 SQUARE_HASH, 4-byte ROM game code, 20-byte ROM SHA-1, 16-byte private guest key, u16 requested channel (0 chooses one), u8 name length, name bytes |
| WELCOME (2) | server → client | u32 world seed, u32 session player ID, u16 tick Hz, u8 room capacity, u16 channel, 16-byte public identity, u16 channel count |
| STATE (3) | client → server | i32 x, i32 y, i32 z in whole world units, u8 facing (0..7) |
| SNAPSHOT (4) | server → client | u8 peer count, then 49 bytes per peer: u32 session ID, 16-byte public identity, three i32 coordinates, u8 facing, 16-byte zero-padded name |
| ERROR (5) | server → client | u16 reason code, at most 96 printable ASCII bytes |
| PING (6) / PONG (7) | client → server / server → client | u32 client clock value, echoed unchanged |
| PROFILE (8) | client → server | 69-byte public profile, described below |
| INSPECT (9) | client → server | u32 target session ID in the same channel |
| CARD (10) | server → client | u32 session ID, 16-byte public identity, 69-byte public profile |
| SOCIAL (11) | client → server | u8 kind (0 phrase, 1 emote), u8 fixed choice ID |
| EVENT (12) | server → channel, including sender | u32 sender session ID, u8 kind, u8 fixed choice ID |
| CHANNEL (13) | client → server | u16 requested channel (0 chooses one); success sends a fresh WELCOME |
| NOTICE (14) | server → client | u16 reason, at most 96 printable ASCII bytes; an action failed but the connection stays open |
| TRADE (15) | client → server | u8 action (1 propose, 2 answer, 3 accept, 4 cancel), u32 reference (a proposal's target session ID, else the trade's ID), the chip given (u16 id 1..319, u8 code 0..26, 26 the `*`), the chip taken (the same; zero in a proposal) |
| TRADE_STATE (16) | server → client | u8 stage (1 offered, 2 waiting, 3 answered, 4 accepted, 5 done, 6 cancelled), u32 trade ID, u32 the other player's session ID, 16-byte his public identity, my chip and his (u16 id, u8 code each; zero where not yet named) |
| ARENA (17) | client → server | u8 action (1 challenge, 2 accept, 3 leave or decline, 4 over), u32 reference (a challenge's target session ID, else the battle's ID), u16 the sender's latency in ms (0..5000) |
| ARENA_STATE (18) | server → client | u8 stage (1 challenged, 2 waiting, 3 start, 4 ended), u32 battle ID, u32 the other player's session ID, 16-byte his public identity, u8 side (0 the cable's first GBA, 1 the second; START's), u8 input delay in frames (START's) |
| ARENA_LOADOUT (19) | client → server → opponent | u32 battle ID, u16 length (1..400), that many bytes: the sender's MegaMan as `src/online/arena_wire.h` packs him |
| ARENA_INPUT (20) | client → server → opponent | u32 battle ID, u32 first frame, u8 count (1..8), count u16 GBA keys, one a frame from the first |
| ARENA_HASH (21) | client → server → opponent | u32 battle ID, u32 frame, u64 the two linked cores' state hash after it; frame 0xFFFFFFFF says ready, the hash the pair's at the Custom screen |

HELLO's fixed prefix is 52 bytes, WELCOME is 32, CARD is 90, TRADE 12,
TRADE_STATE 32, ARENA 8 and ARENA_STATE 28. The
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

HELLO names a verified ROM identity, never supplies a ROM. No Capcom
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
of a channel, neither busy in another trade or a battle. The proposer
names the chip given, the other answers with his, and the server
commits only when both accepted the same pair as their TRADE_STATE
showed it, sends done to both and appends the trade to its state
folder's `trades.jsonl` (identities and chips, no names). A stale or
mismatched step, either player leaving, or 300 seconds without a step
cancels it for both. Each game applies its side at home; the server
never sees a folder. NOTICE codes 15 (trading is off on this server),
16 (that Navi is busy) and 17 (no longer here, or the trade is over)
answer a TRADE that cannot be done.

The Arena's messages are relayed only between the two players of a
started battle, and only in its shape; the server never sees a battle.
Each game runs both players' linked BN6 cores and only their keys cross
(docs/ONLINE_ARENA.md): START gives each its side of the cable and the
input delay, from both players' latencies (`server/cyberworld_server/arena.py`).
A challenge waits 30 seconds for its answer. One player of a battle
sends ARENA's over (the cable's first, as the battle ends on its cores);
a leave, a decline or a dropped connection ends it for both, ARENA_STATE
ended telling each. The relayed messages have a bucket of their own,
150 deep and refilled at 120 a second. NOTICE codes 16 (that Navi is
busy), 17 (no longer here, or the battle is over) and 18 (the Arena is
closed on this server) answer an ARENA that cannot be done.

Changing the wire shape bumps the protocol version. HELLO's SQUARE_MAKE
and SQUARE_HASH are the Square generator's own version and its plan's
hash (`src/online/square_plan.h`), checked by the server against
`protocol.json`'s `square_make` and `square_hash`: a change to what a seed
makes bumps SQUARE_MAKE, not the protocol. The run's `LAYER_MAKE` keeps
its own meaning and no longer crosses the wire.
