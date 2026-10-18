# Net Square protocol v2

`protocol.json` is the server's packaged contract. `protocol.h` exposes
the same constants to the C client; server tests check they agree with
the game generator. All multibyte fields are big-endian. Messages are
unfragmented binary WebSocket frames, at most 512 payload bytes, on
`/presence` with subprotocol `cyberworld.presence.v2`.

| Message | Direction | Payload after the one-byte type |
| --- | --- | --- |
| HELLO (1) | client → server, first | u16 protocol, u16 LAYER_MAKE, u32 LAYER_MAKE_HASH, 4-byte ROM game code, 20-byte ROM SHA-1, 16-byte private guest key, u16 requested channel (0 chooses one), u8 name length, name bytes |
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

HELLO's fixed prefix is 52 bytes, WELCOME is 32, and CARD is 90. The
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

Changing the wire shape bumps the protocol version. Changing the Square
generator must also change the compatible protocol version until it has
its own generator-version field. The run's `LAYER_MAKE` keeps its existing
meaning; the spike does not change the layouts of saved runs.
