# Trade extension: fixed contract, not enabled on presence v2

The live presence service still refuses these commands. The extension is
being integrated with durable client storage. `trade.h` and `trade.json` are identical constants;
the server tests hold them together. Every integer is unsigned network
byte order. No ROM, emulator state, complete folder or Pack travels here.

## Compatible negotiation

A client with complete trade support offers the WebSocket subprotocols
`cyberworld.presence.v2.trade.v1,cyberworld.presence.v2`, in that order.
An equipped server selects the first; a presence-only server selects the
second. HELLO and WELCOME retain their exact v2 layouts in both cases.
The client must verify the exact selected protocol; offering the extension
does not mean it was accepted. No implicit reconnect or protocol guessing.

Only a connection that negotiated the extension may send or receive its
messages. The server sends WELCOME, then CAPS, then any unresolved decision
owned by that authenticated guest and the channel's board. CAPS binds the
persistent server identity before any decision can be trusted. Plain v2
clients never receive extension messages, even on a server with trading.

CAPS with enabled=0 disables new READY, OFFER, REQUEST and PREPARE messages
while still allowing STATUS, CANCEL and ACK to settle existing transactions.
Changing this setting never discards a committed decision. The server's
default command line still refuses `trading=true`; negotiated attachment
is exercised through the service API in integration tests until client
checkpoint installation and recovery are complete.

## Messages

| ID | Direction | Payload after the one-byte kind |
| --- | --- | --- |
| 15 CAPS | server → client | Extension version u16, persistent server identity 32 bytes, enabled u8 (0/1) |
| 16 READY | client → server | Run identity 16, revision u32, checkpoint SHA-256 32, rules u8 |
| 17 OFFER | client → server | Have u16, want u16; both zero withdraws, want zero accepts any distinct entry |
| 18 BOARD_REQUEST | client → server | Empty |
| 19 BOARD | server → client | Count u8, then up to eight 37-byte listings in this channel |
| 20 REQUEST | client → server | Target public identity 16, exact outgoing u16 and incoming u16 from the advertised listings |
| 21 DECISION | server → client | Terms 190 bytes, state u8, prepared mask u8, applied mask u8 |
| 22 PREPARE | client → server | Transaction ID 16, confirmation SHA-256 32 |
| 23 CANCEL | client → server | Transaction ID 16 |
| 24 STATUS | client → server | Transaction ID 16 |
| 25 ACK | client → server | Transaction ID 16 |

A listing contains the public identity (16), zero-padded ASCII name (16),
have and want (u16 each), and rules (u8). Names follow the presence
contract. Rules bit 0 means BN5 present and bit 1 means All *; all other
bits are invalid. Each chip entry is the canonical ID in bits 0–8 and
code in bits 9–15. Valid IDs are 1–511 and codes 0–26. These range checks
do not replace the client's local registry and spare-count validation.

The 190-byte terms contain the server identity (32), transaction ID (16),
then left and right sides (71 each). A side is its public identity (16),
run identity (16), revision (u32), checkpoint SHA-256 (32), chip (u16),
rules (u8). A revision must be below `UINT32_MAX`, leaving room for its
successful swap. Identities and checkpoint digests must be nonzero.
Participants and runs must differ; chip entries must differ. All * rules
must match, and All * entries must retain code 26.

Confirmation is SHA-256 of the UTF-8 bytes `Cyberworld trade 1`, one zero
byte, then exactly those terms. `trade-vector.json` is a fixed invented
cross-language vector, checked independently by C and Python. Do not
regenerate it to conceal a mismatch.

States are 0 offered, 1 committed, 2 aborted. Bit 0 in the two masks is
the left participant and bit 1 is the right. A commit requires prepared
mask 3. A noncommit cannot have applied bits. State and masks do not alter
the confirmation digest. Only the connected participant may prepare,
cancel, query or acknowledge its transaction; a payload never supplies
its own authenticated identity. A server response is never a valid
client command.

READY describes a durable carried checkpoint, and PREPARE is sent only
after the user's confirmation and durable local staging. ACK follows
durable installation. Unknown history and mismatched server identity must
keep recovery pending; neither is an abort. See
[the transaction contract](../../docs/ONLINE_TRADE.md) for lifecycle,
failure handling and platform gates.
