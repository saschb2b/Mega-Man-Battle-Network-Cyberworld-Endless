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
Coolify deployment); it speaks protocol v3 with trading and the Arena on,
built from this work's branch until it is merged. No release has shipped
the Square yet.

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
from it (`square_plan.c`, `SQUARE_MAKE` 2) in one of BN6's busy daylight
nets (Central, Sky or Green Area; not Seaside, whose layers stand in
another map's slot). The plan's hash and its generator version
(`SQUARE_MAKE`, `SQUARE_HASH`) go in HELLO, so a server refuses a game
that would build another Square. `online_square.c` builds it with the
layers' map builder (`netmap_build`) and gives the other players'
MegaMen seven NPC scripts of BN6's own, moved from a 100 ms
interpolation of the server's 20 Hz snapshots; their names stand over
them. MegaMan walks as BN6 walks him. Each Square is a channel of eight;
the keeper moves MegaMan to the next or the last.

### The Square's design

The owner asked for "a lively hub to gather and communicate", after the
fan servers that build custom maps. The lens (the game-design skill):

- **Experience**: fellowship and expression. The Square is a place to be
  seen in and to linger, not a menu with floors.
- **Dialectic: gather against go.** The plaza's landmark pulls players
  together; each wing sends them off to do something with another
  player (trade, battle, read what happened). Every spot is both a place
  to stand together and a reason to be there.
- **Loop layer**: the session between acts: arrive, see who is here, do
  one thing together, read what happened, go home. Variety and
  expression, never run power: nothing in the Square makes a run
  stronger.
- **Patterns**: a centrepiece to gather round (a "weenie"); systemic
  barks, lines built from live facts; diegetic boards and signs; spots
  that answer players together; variation by the server's seed inside
  one skeleton.
- **Inspiration**: the Open Net Battle and Hub OS community servers
  (Navis on waypoint routes whose lines change, BBS boards, check text on
  every object, welcome and guide Progs); BN2's and BN3's own Net
  Squares (Mr.Progs, Navis chatting about what is going on, the BBS,
  BN3's Under Square with Navis gathered round a statue); the Pokemon
  Wi-Fi Plaza's and Club Penguin's dressed central rooms. These are
  recalled from playing them, not checked in docs/SOURCES.md's sources;
  nothing here copies them.

### The layout

A 36 x 36 grid, grown down the screen rather than across (its tile map
184 x 92 to 184 x 94 entries of the 20477 the free space holds):

| Place | Where | What stands there |
| --- | --- | --- |
| The plaza | 13 panels a side, in the middle | The landmark on its middle panel, the nine panels its base spreads over walled off (MegaMan walks round it, never onto its pedestal), four props round it in walled holes; cybertrees past its back rim, either side of the stage's stair; the channel keeper by the way in from the log-in plaza |
| The log-in plaza (+x) | Off a walkway lined by two cybertrees | The pad home at its far end, the greeter Mr.Prog, the Board (a BBS) and the welcome sign past its back rim |
| The stage (-y) | Up a stair two panels wide from the plaza's edge | Two braziers past its back rim; a place to stand and be seen |
| Trade Central (-x) | Off a walkway | BN6's reception desk, its two clerks |
| The Arena (+y) | A walk down the spectators' floor, which turns round before the ring | The ring up its own stair from that floor, the referee Mr.Prog at its foot |

The landmark is the area's own first: Green's giant cybertree among
cybertrees; in Central and Sky the Undernet's statue or the Graveyard's
monument by the seed, ringed by braziers or cybertrees by the seed. By
the seed too, a mirrored pair of cybertrees past the plaza's -x rim, or
a row of three along the Arena's walk. Only Sky's maps have stairs to
cut (Sky Area's, climbing towards grid -y, 32 high): there the stage and
the ring stand raised as BN6's raised floors, by the layers' own rules
(`net_height.c`: nothing within a panel of where a raised room is drawn,
a panel up the screen). Central's and Green's maps have none, so their
stage and ring are flat daises in the same places. The props are BN6's
map objects through the layers' own pieces (`layer_prop_objects`), at
the stage's height on its rim. Sprites: six of the twelve the loader
takes, 0x4614 to 0x46BC of its 0x8800 bytes; NPCs: all sixteen BN6 runs.

`tests/test_online.c` holds every floor panel reached from the arrival
over 64 seeds, a step between heights only on a stair, each stair from
ground floor up to raised floor, no floor where a raised room is drawn,
every place and every walker's waypoint on the floor.

### Its life

`square_life.c`, its words in `square_life_words.c` (docs/VOICE.md):

- **Two walkers**, a GreenNavi and a GirlNavi, puppets as the peers are,
  walk the plan's routes (the GreenNavi round the plaza, four seconds at
  each corner; the GirlNavi from Trade Central across the plaza, clear
  of the landmark's ring, down the Arena's walk and back, six seconds at
  either end), a world unit a frame, each leg along a grid axis (the
  unit tests hold them on ground floor, through no hole). Their place is
  a function of the wall clock (UTC seconds and the frames since), so
  every client whose clock agrees shows them in the same place: two
  clients' logs (with `CYBERWORLD_EMU_DEBUG`, their "walkers at clock"
  lines) name the same positions at the same second. The server sends no
  clock of its own and the wire is unchanged. A faces one: the walker
  stops, turns to MegaMan and says a line built from what the client
  knows: the Navis here by name, how many, the hour, the area, the last
  Navi to log in, the last trade and NetBattle this game made; the
  GirlNavi tells the Square's tips as gossip.
- **A parade** every three minutes of the wall clock, thirty seconds
  long (after the Pokemon Wi-Fi Plaza's spectacle and the fan servers'
  parades): the walkers log out where they are and in on a loop three
  panels round the landmark, BN6's beam where the first does and his
  "Parade time!" over his head, and walk round it two panels apart
  without stopping; then they log back in on their routes. Their talk
  meanwhile is the parade's. Every client whose clock agrees sees the
  same parade, and players who walk with it stand round the landmark,
  where saying something together makes the Square cheer.
- **BN6's log-in beam** (list 7's sprite 0, sound 0x76) where a Navi
  logs in or out, one NPC for all, and the **greeter**'s word over his
  head ("ALPHA LOGGED IN!"). His own words, and the **referee**'s (how
  to ask for a NetBattle, the last one this game fought and how it went,
  how many are watching), are rewritten as they change, as the keeper's
  are.
- **Check text**: A facing a prop says what it is; the Board's posts are
  the channel, who is here, the last log-in, the last trade and battle,
  and two tips; the sign names the channel.
- **Together**: when two players say something or emote within four
  seconds of each other round the landmark (three panels) or on the
  stage, the walker nearer the landmark shows BN6's "!" (list 5's sprite
  7), the other says "!!" and BN6's sparkle sounds; once in ten seconds,
  each client from the events it sees.

None of it takes an event flag: the walkers, beam and mark are puppets,
their talk BN6's talk box from the host (`talk_start`), the Mr.Progs'
words their own archives in the layer's scratch.

**Talk.** Every exchange is BN6's chat box with its options, three to a
box (`online_chat.c`, words in `square_words.c`): L for a phrase or an
emote, A facing another MegaMan for his card (HP, act and layer,
Crosses, Souls) and Battle, Wave, Mute, Block; R or the pad for home.
The Square's flags are `0x1480`-`0x14A1`, its own while it lasts.
Phrases and emotes are fixed lists, never free text; mute and block are
the player's, kept by the server's public identity of the other.

## Trade Central

A Mr.Prog at a desk in the wing up-left of the plaza (-x) trades one
spare chip (a copy past those in the run's three folders) for another
player's (`trade_central.c`, words in `trade_words.c`):

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
- **The Square's second plan and its life** (the same day, later). The
  Square through real visits in all three areas (seeds 120, 121, 122:
  Central with the statue, Sky with its stage and ring raised and
  MegaMan walked up both stairs, Green with the giant cybertree); in two
  games at once: the greeter's "ALPHA LOGGED IN!" over his head as the
  second arrived, the Board's posts naming the channel, who was there and
  the log-in, the referee's words, a walker spoken to (she stopped and
  turned), both games' walkers at the same positions at the same clock
  seconds, both games cheering at once when their players said Hello
  round the statue (the "!" over the walker nearer it), Trade Central's
  desk answering from its wing (to "no spare chips", the run having
  none), and a Practice NetBattle asked for on the Arena's spectators'
  floor, both cores agreeing frame by frame; and a run timed to a
  parade, the two walkers marching round the statue past MegaMan, two
  panels apart, at the positions the clock gives.
- **The official server.** Redeployed from `deploy/coolify/compose.yaml`
  on the same state volume: `/health` says protocol 3, trading and the
  Arena on, and a `cyberworld.presence.v3` upgrade is answered (101). No
  game has visited it since; the transport's checks below were against
  its v2.
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
- **The official server:** a game's visit to it on v3; its Coolify
  service following `dev` or `main` once this work is merged; a host's
  retention policy, named moderators and its backup and restart checks
  before a public launch.
- **A pending trade across systems:** `trade.pending` stays beside the
  saves, outside the `.cwsave` carrier; a save moved between a trade and
  its settling leaves the swap on the system the trade was made on.
