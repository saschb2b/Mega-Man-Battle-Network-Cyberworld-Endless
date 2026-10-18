# Online: the Net Square

[Epic #120](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/120)
puts other players' MegaMen beside the run: a **Net Square** of its own on
a server, reached from a gate in Lan's HP, where MegaMen meet, say a
phrase, read each other's cards, trade a spare chip at Trade Central and
fight a Practice NetBattle of BN6's own. The players' side is in
[GUIDE.md](GUIDE.md#online-the-net-square), the gate's piece of the home
in [HOME.md](HOME.md#11-the-net-squares-gate); this page is how it is
built and what has been shown of it. The Arena has its own,
[ONLINE_ARENA.md](ONLINE_ARENA.md), and the cable research it rests on
[ONLINE_LINK.md](ONLINE_LINK.md).

The official server is `wss://cyberworld.saschb2b.com/presence`, deployed
in Coolify from `deploy/coolify/` ([server/README.md](../server/README.md),
Coolify deployment). No release has shipped the Square yet.

## The design

The dialectic is **the run's integrity against the community's
generosity** (the game-design skill): the Square sits beside the run
and gives variety, never power. It has no battles of the net, no heal, no
shop, no reward and no save; a NetBattle there wins nothing and costs
nothing. The one thing that comes home is a traded chip, one spare for
one spare, which changes what a folder can hold, not how strong
MegaMan is. No menu screen of the project's own leads there: the world
does, a gate in the room the run comes home to, in the manner of
Warframe's Orbiter, with BN6's own chat boxes for every question.

## Where things are

| Part | Sources | Builds and checks |
| --- | --- | --- |
| The game's side | `src/online/`, the gate in `src/director/director_gate.c` and `src/map/lanhp.c` | `build.py` and every target; `build.py test` (`tests/test_online.c`: the Square's plan and hash, `tests/test_arena.c`), `build.py lint` |
| The server | `server/`, its deployment in `deploy/coolify/` | `build.py server`, `build.py server-test`; its own Dockerfile and CI workflow |
| The wire | `shared/online/` ([protocol v3](../shared/online/README.md)) | The server's tests hold its codecs, the C constants, the Square's generator version and hash and the ROM's identity to the client's |

The dependency runs client → wire ← server. The server imports no game
code, runs no emulator, reads no ROM and never sees a save: chips are
numbers to it, MegaMen bytes it bounds. Its Docker image copies only
`server/`, `shared/online/` and the licence.

## A visit

**The gate.** In Lan's HP (`lanhp.c`) a white gate stands at the room's
front, a Mr.Prog beside it. The first step on it asks for the name other
players see, says what the server learns (the PET's address, that name,
MegaMan as he is) and asks before going, the cursor on No; after that
MegaMan asks "To the Net Square,Lan?" each time
(`director_gate.c`, its words in `gate_words.c`; flags `0x1473`-`0x1476`,
[EMULATION.md](EMULATION.md)). The keeper changes the name or the server:
the official one, or another's address, typed once in the one screen
of the Square's own, a field with a keyboard on the pad and by touch and
the system's where there is one (`online_field.c`, `text_input.c`). Yes saves the run as a checkpoint at the gate
(`home_gate_save`), then the gate flickers while the server answers;
B calls it off.

**One core.** The visit runs on the game's own core
(`online_visit.c`): the Square is built into the running BN6, the run's
state (its `Run`, profile, RNG and loot) put aside, and the Square's RAM
thrown away on the way home. Coming home puts the state back and
`director_resume()` re-enters Lan's HP on the live core, MegaMan on the
gate as he left it; nothing is booted again. A game closed in the Square
continues from the gate's checkpoint, as any CONTINUE does. Every end has
MegaMan's word for it at the gate: left, called off, no answer, another
version, another ROM, a name taken, a full Square, barred, removed, the
connection lost.

**The Square.** The server sends a seed; each game plans the same Square
from it (`square_plan.c`): a plaza of ten panels a side and four wings
off it, the arrival with the pad home, Trade Central, the Arena and the
Board, in one of BN6's busy daylight nets (Central, Sky or Green Area;
not Seaside, whose layers stand in another map's slot). The plan's hash
and its generator version (`SQUARE_MAKE`, `SQUARE_HASH`) go in HELLO, so
a server refuses a game that would build another Square. `online_square.c`
builds it with the layers' map builder (`netmap_build`), places the
welcome sign, the channel keeper, Trade Central's clerk at his desk, the
Board's sign and the pad home, and gives the other players' MegaMen
seven NPC scripts of BN6's own, moved from a 100 ms interpolation of the
server's 20 Hz snapshots; their names stand over them. MegaMan walks as
BN6 walks him. Each Square is a channel of eight; the keeper moves
MegaMan to the next or the last.

**Talk.** Every exchange is BN6's chat box with its options, three to a
box (`online_chat.c`, words in `square_words.c`): L for a phrase or an
emote, A facing another MegaMan for his card (HP, act and layer,
Crosses, Souls) and Battle, Wave, Mute, Block; R or the pad for home.
The Square's flags are `0x1480`-`0x14A1`, its own while it lasts.
Phrases and emotes are fixed lists, never free text; mute and block are
the player's, kept by the server's public identity of the other.

## Trade Central

A Mr.Prog at a desk in the north wing trades one spare chip (a copy past
those in the run's three folders) for another player's
(`trade_central.c`, words in `trade_words.c`):

1. MegaMan picks a Navi in the channel and the chip he offers. The picker
   is BN6's own Order Service screen (`trade_order.c`): the shop's
   descriptor pointed, while he is in the Square, at a copy of its text
   archive in the desk's words, its list the run's spares
   ([ROM_DATA.md](ROM_DATA.md), the Order Service).
2. The other player's MegaMan hears the offer, goes to the desk and picks
   a chip back, or says no.
3. Both see the pair and say Deal; the server commits the trade only
   when both accepted the same pair, and writes it to `trades.jsonl`
   (`server/cyberworld_server/trade.py`). A stale or mismatched step,
   a player gone, or five minutes without a step calls it off.
4. Each game keeps the done trade in `trade.pending` and applies it at
   home: on the way back from the Square, or at the next CONTINUE if the
   game was closed there. It runs BN6's own TakeChips and GiveChips on
   the gate's checkpoint (`online_trade_settle`), refuses a swap whose
   chip is no longer spare, deletes the file before it saves, and MegaMan
   says what came and went.

## The Arena

Battle on a card sends a challenge; if the other player takes it, both
games run BN6's Practice NetBattle on **two linked cores each**, the two
MegaMen carried as they are (folder, NaviCust, HP, buster, Crosses), and
only the players' keys cross the server. The Square's core holds still
meanwhile; after the battle each MegaMan stands where he stood and says
how it went. A hash of the pair every 60 frames catches two games apart
and stops both. Nothing is won or lost. The 3DS and the browser build
compile it out (`CW_ARENA`): Battle is not offered there and a challenge
is declined. [ONLINE_ARENA.md](ONLINE_ARENA.md) has the engine, the wire,
the lockstep and the two-client proof.

## The server and the wire

Protocol v3 ([shared/online/README.md](../shared/online/README.md))
is binary over a WebSocket (`cyberworld.presence.v3`). The server refuses
another protocol version, another Square generator or hash, another ROM
code or SHA-1, a name out of bounds and a full Square; it bounds every
message, the players, pending handshakes, each client's traffic and its
output, and relays the Arena's keys under a bucket of its own. Trading
and the Arena each have their switch in its configuration
(`trading`, `arena`). A player's identity is a key of the PET's own,
derived per server from a local secret; moderators ban, mute, kick and
allowlist through the server's command line. Hosting, TLS, backups and
the platforms' transports are [server/README.md](../server/README.md)'s.

## Two games at once

[DEVTOOLS.md](DEVTOOLS.md#the-net-square-two-games-at-once) says how:
a local server (`build.py server-run`), a `network.ini` per data folder or
`--dev online=URL,onlinename=NAME`, and two `tools/play.py` sessions
from `--scene home`; never the official server. A visit plays at the
clock's pace even headless, so the two meet. `tools/online_browser.html`
starts the browser build from home onto the gate, for a browser beside a
native game (the server's `origins` allowing the page's). The Arena's own
proof is in [ONLINE_ARENA.md](ONLINE_ARENA.md#the-two-client-proof), the
cable pair's tool (`build.py online-pair`) in
[ONLINE_LINK.md](ONLINE_LINK.md).

## What has been shown

On this machine, 18 October 2026, the player's Gregar ROM, a local v3
server:

- **The gate and a visit.** The first step's questions (the name field,
  what the server learns, MegaMan's question), the save at the gate, the
  connection, the Square built and entered; the keeper's channel switch
  (the server logged it), the L menu, a card read, R home and the pad
  home, each back on the gate with MegaMan's word. The Square drawn in
  Central, Sky and Green Area.
- **A trade between two games.** Two `tools/play.py` sessions with spare
  chips: an offer picked on the order screen, answered, both Deals, the
  line in `trades.jsonl`, `trade.pending` on both; one game settled on
  its way home, the other, closed in the Square, at its CONTINUE, each
  folder and pack as the trade said.
- **A NetBattle between two games.** The challenge, the way in, the
  battle to its end on both with every hash agreeing, a desync and a
  player leaving each stopping both, a declined challenge
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#the-two-client-proof)).
- **Builds and checks.** Host, aarch64, Windows, 3DS and browser builds
  with `-Werror`; `build.py test` (the Square's plan and hash, the
  Arena's wire and links) and the server's 36 tests; `build.py lint` with
  nothing new.

The transport itself was measured on 15 October 2026, before the gate
replaced the first Network screen (those proof tools went with it; they
are in the history before 433865a8):

- **Hosted WSS:** two native games on the Coolify server with a trusted
  certificate, median round trips 33 and 39 ms; a browser at the
  published site's origin; moderation through the container's command
  line; an identity kept across a redeploy.
- **Many players:** sixteen native games in two full channels, 10,248
  remote positions matching the interpolation; eight ROM-free wire
  clients at about 40 KB/s together.
- **Systems:** a Retroid Pocket Nova on ROCKNIX against the public server
  (median 40 ms); Windows Server 2022 and macOS 15 running the transport's
  sources in CI against the public server; the Android APK in an
  emulator; an untrusted certificate refused on each, with no plaintext
  fallback.

## Not yet

- **The Board and the weekly seed** (the epic's stretch): the Board's
  wing stands with its sign, empty.
- **A referee Mr.Prog** in the Arena's wing, rematches, and the Arena on
  the 3DS and in the browser.
- **On hardware:** the gate, a visit and a trade on the Nova, the Flip 2,
  a New 3DS, Android and iOS; the Arena over a real network rather than
  one machine (the margin a phase between two games' ticks takes from
  its delay).
- **In the browser:** a whole visit with the published server (the
  browser build builds; its online run here was before the gate).
- **The official server:** its Coolify service pointed at
  `deploy/coolify/compose.yaml` and redeployed with v3, `trading` and
  `arena` on in its configuration; a host's retention policy, named
  moderators and its backup and restart checks before a public launch.
- **A pending trade across systems:** `trade.pending` stays beside the
  saves, outside the `.cwsave` carrier; a save moved between a trade and
  its settling leaves the swap on the system the trade was made on.
