# Presence protocol v1

`protocol.json` is the server's packaged contract. `protocol.h` exposes
the same constants to the C client; server tests check they agree with
the game generator. All multibyte fields are big-endian. Messages are
unfragmented binary WebSocket frames, at most 512 payload bytes, on
`/presence` with subprotocol `cyberworld.presence.v1`.

| Message | Direction | Payload after the one-byte type |
| --- | --- | --- |
| HELLO (1) | client → server, first | u16 protocol, u16 LAYER_MAKE, u32 LAYER_MAKE_HASH, 4-byte ROM game code, 20-byte ROM SHA-1, u8 name length, name bytes |
| WELCOME (2) | server → client | u32 world seed, u32 assigned player ID, u16 tick Hz, u8 room capacity |
| STATE (3) | client → server | i32 x, i32 y, i32 z in whole world units, u8 facing (0..7) |
| SNAPSHOT (4) | server → client | u8 peer count, then 33 bytes per peer: u32 ID, three i32 coordinates, u8 facing, 16-byte zero-padded name |
| ERROR (5) | server → client | u16 reason code, at most 96 printable ASCII bytes |
| PING (6) / PONG (7) | client → server / server → client | u32 client clock value, echoed unchanged |

Names have 1..16 ASCII letters, digits, spaces, underscores or hyphens,
starting with a letter or digit. Coordinates are bounded to ±8192 and
snapshots exclude the recipient. Player IDs belong only to this server
session; the prototype has no persistent identity. WELCOME's capacity is
1..8; at most seven peers are sent to any one client.

HELLO names a verified ROM identity, never supplies a ROM. No Capcom
graphics, audio, text, code or save bytes cross the protocol. Compatibility
metadata is not anti-cheat attestation: a modified client can forge it.

The server allows 32 pending/active connections, five seconds for HTTP
Upgrade and five for HELLO, a 15-second idle timeout, and a token bucket
of 40 messages replenished at 30 per second. A slow output buffer above
8192 bytes is disconnected. WebSocket pings, pongs and close are handled;
text, fragments, masking violations and oversized frames are rejected.
The client sends positions every 50 ms and pings every two seconds.

Changing the wire shape bumps the protocol version. Changing the Square
generator must also change the compatible protocol version until it has
its own generator-version field. The run's `LAYER_MAKE` keeps its existing
meaning; the spike does not change the layouts of saved runs.
