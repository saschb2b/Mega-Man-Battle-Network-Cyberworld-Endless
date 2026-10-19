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
Coolify deployment); it speaks the protocol of the branch it was last
deployed from (v4 until it is redeployed with v5, which a v4 game cannot
join, nor a v5 game a v4 server), trading and the Arena on, until this
work is merged. No release has shipped
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

## Battle Network 5, there or not: the pair's common ground

BN5's ROM is optional, so a Square holds players with it and without
it, all in one channel: splitting them would halve a small crowd. What
two players do together goes by **their common ground**, the content
both games hold, chosen up front and named, never a bare refusal. It is
the formats of the games the players know, Pokémon's link rules and
Hearthstone's Standard and Wild, where what both bring decides the rules
and the rules are told before the battle begins (the owner's design also
names BN6's link battles between Gregar and Falzar; assumed, not checked
against [SOURCES.md](SOURCES.md)'s sources here). The game-design skill's
dialectic is **one crowd against each player's own collection**; the
pattern, formats with a telegraph.

- **Each player's ROMs are on the wire.** HELLO carries BN5 Team
  Colonel's identity (its game code and SHA-1, zeros without it), checked
  by the server against `protocol.json` exactly as BN6's is; the server
  keeps each player's ROMs and every SNAPSHOT peer says them (a flags
  byte). Only identities and numbers cross: a game without BN5's ROM never
  receives a byte of it, so a pair where one lacks it has to keep to BN6.
- **A card says it.** Where the two games differ, MegaMan's reading of the
  other's card ends with "His PET holds BN5 chips too!" or "His PET holds
  BN6 chips only."
- **Trade Central.** Between two games with BN5's ROM every chip of
  either game's registry trades, BN5's too (its ids are the registry's,
  fixed in `src/core/xchips_registry.c`, so they name the same chip on
  every game with BN5); a mixed pair trades BN6's own chips alone. The
  clerk lists only the Navis MegaMan has a spare for and only the spares
  the pair can both hold, and says so ("THAT NAVI TRADES BN6 CHIPS!", or
  that every spare is BN5's); each game refuses a chip it cannot hold
  when an offer comes and at settling (BN5's ids, on BN6's blank records
  203-220, 291-300 and 314 and past 319, where its ROM is not read), and
  the server refuses a BN5 chip between games that do not both have its
  ROM (NOTICE 19).
- **The Arena.** A challenge names its rules: a format byte in ARENA and
  ARENA_STATE, **full rules** between two games with BN5's ROM, which the
  server holds them to, and **BN6 rules** for any other pair. Under full
  rules each game installs BN5 into both pair cores from its own BN5 ROM,
  the same way (the game core's installs written down as it starts and
  made again in each pair core, their hash in the pair's), and MegaMan
  fights with BN5's chips in his folder and his Souls, a Soul he unites
  with carried by the pair to the other GBA
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#full-rules)). Under BN6's rules he
  leaves his Souls and BN5's chips in his pack at home and fights with a
  folder of BN6's chips: Lan picks it among Folder1, Folder2 and the Extra Folder,
  those holding BN5's chips named as staying home; where none fits,
  MegaMan says so before any challenge is sent
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#formats)).
- The Square is built from BN6's own nets (Central, Sky, Green), the same
  for every game whatever ROMs it has, and a card names Crosses and Souls
  from words of the game's own, never read from BN5's ROM.

## Where things are

| Part | Sources | Builds and checks |
| --- | --- | --- |
| The game's side | `src/online/`, the gate in `src/director/director_gate.c` and `src/map/lanhp.c` | `build.py` and every target; `build.py test` (`tests/test_online.c`: the Square's plan and hash, `tests/test_arena.c`), `build.py lint` |
| The server | `server/`, its deployment in `deploy/coolify/` | `build.py server`, `build.py server-test`; its own Dockerfile and CI workflow |
| The wire | `shared/online/` ([protocol v5](../shared/online/README.md)) | The server's tests hold its codecs, the C constants, the Square's generator version and hash and the ROM's identity to the client's |

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
  GirlNavi tells one of the Square's tips as gossip (where Navis cheer).
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
  to ask for a NetBattle until this game has fought one; then who beat
  whom in the last bout and, from a rematch on, the count with that
  player; how many are watching), are rewritten as they change, as the
  keeper's are. The referee calls each bout over his head as MegaMan
  comes back ("YOU WIN!", "ALPHA WINS!").
- **Check text**: A facing a prop says what it is; the Board's posts are
  the channel, who is here, the last log-in, the last trade and battle,
  the trade board's offers and a tip (Trade Central); the sign names the
  channel.
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
Crosses, Souls, and BN5 where the two games differ) and Battle, Wave,
Mute, Block; R or the pad for home. A talk's column of options takes up
and down (`cinema_keys` lets the pad through while MegaMan is held).
The Square's flags are `0x1480`-`0x14C3`, its own while it lasts.
Phrases and emotes are fixed lists, never free text; mute and block are
the player's, kept by the server's public identity of the other.

## Trade Central

A Mr.Prog at a desk in the wing up-left of the plaza (-x) trades one
spare chip (a copy past those in the run's three folders) for another
player's (`trade_central.c`, words in `trade_words.c`):

1. MegaMan picks a Navi in the channel and the chip he offers, from
   those the pair can both hold (above, the pair's common ground). The
   picker is BN6's own Order Service screen (`trade_order.c`): the shop's
   descriptor pointed, while he is in the Square, at a copy of its text
   archive in the desk's words, its list the run's spares
   ([ROM_DATA.md](ROM_DATA.md), the Order Service). Where a spare is one
   of BN5's past BN6's 319 (two games with BN5's ROM), the spares are a
   list in the clerk's chat box instead, two to a box (flags
   `0x14B0`-`0x14C3`, 20 at most): the order screen's list builder puts
   each chip at its ID order plus 0x140 in a list of 640 entries, so one
   past 319 would be written past its end.
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
   chip is no longer spare or one this game cannot hold, deletes the file
   before it saves, and MegaMan says what came and went. BN6's own
   routines count a chip past 319 in the pack's extension (docs/
   EMULATION.md), so a BN5 chip comes and goes as BN6's do.

One trade a visit: a game keeps one swap until it is home
(`trade.pending` holds one), so after a trade is done the clerk says
"ONE TRADE A VISIT!" and the server refuses that session a second
(NOTICE 21). Before this, a second trade in a visit overwrote the
first's file: its partner got the chip, this game never gave it.

### The trade board

A trade needs two players at the desk at once and a blind first offer;
in a channel of a few players, most visits never find out who wants
what. The board says it: each player may put **one offer** up at the
clerk, a spare he **has** (picked on the order screen as a trade's chip
is) and what he **wants** for it, from a fixed list: anything, a Fire,
Aqua, Elec, Wood, Sword, Wind, Break or Cursor chip (BN6's own icon
element, `BN6_CHIP_ELEMENT`), or a Mega (`BN6_CHIP_LIBRARY_TYPE`). The
clerk reads the channel's offers one a box ("Alpha HAS Cannon A! WANTS A
FIRE CHIP!") with Answer it and Next offer under each, and MegaMan's own
to post or take down; the Board in the log-in plaza lists them among its
posts. Answering one lists only the spares that meet its want and the
pair can both hold; the trade begins with both chips named (TRADE's
take, protocol v5), and both say Deal as in any trade. The poster's
game checks the chip against the want it posted and calls off one that
is not (the server never reads a chip's element); MegaMan then says
"Lan! Beta took our offer!" before the Deal. An offer lives on the
server while its poster is in the channel: taken down, a channel
changed, the server left or a trade done, it goes, and no offer is
kept on disk (`trade.py`; `online_board.c` holds the board as last
said, `trade_central.c` the clerk's chats, `trade_words.c` the words).
Common ground holds: a game without BN5's ROM is never sent an offer
whose chip is BN5's, never posts one, and is never answered with one.

The design (the game-design skill): the dialectic is the Square's, **the
run's integrity against the community's generosity**, and the board
serves the session loop (arrive, see who's here, do one thing together,
go home). Its patterns are a diegetic board and honest feedforward:
what a player wants is said before anyone walks to the desk, so a trade
starts from a match instead of a cold offer, and the Deal still shows
the exact pair. A want is a **kind**, not a chip: a fixed list needs no
free text (the audience has children in it) and many chips meet one
want, so a small crowd still matches. One offer a player is a budget
that keeps the board to eight lines and stops hoarding posts; offers
that live only while their poster is there keep every answer one that
can finish now (no asynchronous market, no appointment to keep). It
stays one spare for one spare and no currency: there is no price to
rise, and Giga chips are not on the list, so the board advertises
variety, not power. One trade a visit follows from the swap kept at
home, and fits the loop: a trade is the visit's one thing.

## The Arena

Battle on a card sends a challenge; if the other player takes it, both
games run BN6's Practice NetBattle on **two linked cores each**, the two
MegaMen carried as they are (folder, NaviCust, HP, buster, Crosses), and
only the players' keys cross the server. The challenge names its rules
and Lan picks the folder carried (above, the pair's common ground). The
Square's core holds still
meanwhile; after the battle each MegaMan stands where he stood, says
how it went and asks Lan whether to go again with the same player (the
cursor on No): when both say Yes they fight again by the same rules and
folders, over the same ARENA messages (a challenge answered at once). The
referee by the ring calls the bout and keeps the count. A hash of the
pair every 60 frames catches two games apart and stops both. Nothing is
won or lost. The 3DS and the browser build
compile it out (`CW_ARENA`): Battle is not offered there and a challenge
is declined. [ONLINE_ARENA.md](ONLINE_ARENA.md) has the engine, the wire,
the lockstep and the two-client proof.

## The server and the wire

Protocol v5 ([shared/online/README.md](../shared/online/README.md))
is binary over a WebSocket (`cyberworld.presence.v5`). The server refuses
another protocol version, another Square generator or hash, another ROM
code or SHA-1 (BN6's, and BN5's where one is named), a name out of
bounds and a full Square; it bounds every
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
- **The trade board** (19 October 2026, protocol v5, a local server, two
  host builds each with BN5's ROM beside BN6's, `tools/play.py` from
  `--scene home --run-depth 7 --seed 3 --dev quiet`; spares from the
  `chip` dev step: Alpha Cannon A, Beta FireBrn1 F and Cannon B). Alpha
  alone in the channel at the clerk: Offer a chip, Trade board, How it
  works; "THE TRADE BOARD! NO OFFERS UP YET!", Post an offer, Cannon A on
  the order screen, Fire chip from the wants, "IT'S UP! Cannon A FOR A
  FIRE CHIP!". Beta at the Board: "Here now: Alpha", "Alpha has Cannon
  A,wants a Fire chip."; at the clerk "1 OFFER UP!", "Alpha HAS Cannon A!
  WANTS A FIRE CHIP!", Answer it, and the order screen listing FireBrn1 F
  alone (Cannon B, his other spare, left off). Alpha heard "Lan! Beta took
  our offer! FireBrn1 F for our Cannon A!", both said Deal, the server's
  `trades.jsonl` held `[20,5]` for `[1,0]`, the clerk then said "YOU
  TRADED ALREADY! ONE TRADE A VISIT!", and each game settled at home:
  Alpha's "the Pack's 4 to 3 of the one, 0 to 1 of the other", "Lan!
  FireBrn1 F is in our Pack now!", Beta's Cannon A. Then the pair's
  common ground: Alpha back with MrkCan1 M (411, the pack's extension)
  posted it from the clerk's chat-box list ("IT'S UP! MrkCan1 M FOR
  ANYTHING!"), and Gamma, a third game with BN6's ROM alone (the
  server's join line `"roms":0`), read the Board beside him: "Here now:
  Alpha", "Trade board: no offers up.". Found on the way:
  alone in a channel, the clerk's words were never written (his first
  state's key was 0, the empty archive's), so the desk said nothing
  until a second player came; fixed. The server's tests hold the rest:
  a BN5 chip's offer kept from a game without BN5 and refused from one,
  an offer gone with its poster, a take of a chip not the one posted,
  the post's second, a session's second trade.
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
  Arena on, and a `cyberworld.presence.v3` upgrade is answered (101). A
  headless game went through the gate to it over TLS, built the Square
  from its seed (120, Central Area) and came home by R to "We're
  home,Lan!" (it is a test instance until a release ships online play).
  The transport's checks below were against its v2.
- **A NetBattle across a real network** (19 October 2026, issue #129).
  Two host builds, each with BN5's ROM beside BN6's, met on the deployed
  server (protocol v5) and fought two Practice NetBattles by full rules
  through it: the server gave a delay of 7 frames from their latencies,
  both pairs made the same ready hash, and each battle went to its end on
  both games with every hash agreeing (17 and 25). The game whose ticks
  ran ahead waited on the other's keys for about one frame in eight (137
  of 1,080 frames, then 127 of 1,540), where one machine's runs waited on
  none: the margin issue #128's rollback is for.
- **The browser** (the same day, issue #129). `tools/online_browser.html`
  served from the repository, a local v5 server allowing its origin and a
  native game in the Square: the browser build went through the gate,
  saw the native game's MegaMan and his name, and R took it home to
  "We're home,Lan!". It could not before: its HELLO's buffer was still
  v3's 52 bytes, and v4's HELLO is 76, so it never opened its WebSocket
  (`online_browser.c`, now sized by `ONLINE_HELLO_BYTES`).
- **Builds and checks.** Host, aarch64, Windows, 3DS and browser builds
  with `-Werror`; `build.py test` (the Square's plan and hash, the
  Arena's wire and links) and the server's 36 tests; `build.py lint` with
  nothing new.
- **The pair's common ground** (the same day, protocol v4, a local
  server; two host builds, one with BN5's ROM beside BN6's or both). The
  server's join lines naming each player's ROMs. Two games with BN5: one
  offered MrkCan1 M (411, the pack's extension) from the clerk's chat-box
  list, the other answered with WideSht1 L (204, a blank record) from the
  order screen, both said Deal, `trades.jsonl` held `[411,12]` for
  `[204,11]`, and each game's CONTINUE settled it: "the Pack's 2 to 1 of
  the one, 0 to 1 of the other" (BN6's TakeChips and GiveChips on both
  kinds of id), MegaMan's "Lan! MrkCan1 M is in our Pack now!". A mixed
  pair: the game with BN5 asking the clerk about the other heard "THAT
  NAVI TRADES BN6 CHIPS!", its BN5 spare kept back. A mixed pair's
  NetBattle: the card's "His PET holds BN5 chips too!", the challenger's
  "BN6 rules,Lan. Which folder goes?" (Folder1, Folder2), the challenged
  game's "BN6 rules: no Souls,and BN6 chips only." before its Yes, then
  "Folder1 has BN5 chips. It stays home." and Folder2 alone offered; the
  battle to its end with Folder2 carried, 26 hashes agreeing, back in the
  Square to "Alright! We won,Lan!!". Folder2 came from the `folder` dev
  step: a run holds Folder1 alone (Not yet, below). Then with Folder1
  alone, two MrkCan1 M in it (the `chip` dev step): "Folder1: 2 BN5
  chips sit out,Lan. Chips from our Pack stand in for them, like Cannon A
  for MrkCan1 M!", Folder1 carried, the log's two stand-ins, the battle
  to its end on both games with 25 hashes agreeing.
- **A NetBattle by full rules** (the same day, a local server, two host
  builds each with BN5's ROM beside BN6's). "Full rules! Let's see if
  Beta takes us on!" and "Full rules: Souls and BN5 chips too!"; both
  games made their pairs with the same installs' record (1,913,709
  bytes and 132 hooks) and the same ready hash; Alpha's MrkCan1 M took
  Beta from 100 to 30 on both screens and the battle went to its end with
  every hash agreeing. Alpha holding every Soul and a folder of
  JustcOne: UNITE on the Custom screen lit KnightSoul, and on both
  screens KnightMan's summon, Alpha in KnightSoul with his three turns and
  his wrecking ball's hits, the battle to its end with every hash
  agreeing. A desync stopped both; a game whose installs differed (one
  core made without BN5's families) stopped both before the first frame
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#full-rules)).
- **Every Soul in a NetBattle** (19 October 2026, issue #130, the same
  harness). Each of the six Souls united by the challenger, then two at
  once (ShadowSoul and TomahawkSoul, NumberSoul and ToadSoul, KnightSoul
  and ColonelSoul, the second player's at the second Custom screen): the
  Soul's Navi summoned and MegaMan in the Soul on both screens, its
  charged shot (ScreenDivide, the slash, the dice bomb, the swing over a
  field of grass, the wrecking ball, the paralysing notes) hitting on
  both, every hash agreeing. Chaos Unison (ColonelSoul's DrkSonic,
  NumberSoul's) and ColonelSoul's button (its Cannon fired as his
  charged shot) too, once two things only one GBA had written were
  carried or marked on both: the Soul's end and the button's chip.
  The ready apart because BN5's installs differ now says so on both
  games ("Huh? Our PETs don't match on BN5...").
- **The rest of the Souls in a NetBattle** (19 October 2026, issue #130,
  the same harness and a full-rules `--dev arena`). A failed Chaos
  Unison: the Dark Soul at the back of the other player's area firing at
  the one who failed, on both screens, once each side's chip list for it
  is made on its own player's GBA and given to both as the battle begins
  (before, the other GBA made it from the wrong folder and the two GBAs'
  battles went apart, both clients' hashes agreeing). Chaos Unison's
  attack for the other four Souls (DarkInvs, DrkLance, DarkDril,
  DarkWide); ShadowSoul's FloatShoe and ToadSoul's flag set on both
  GBAs as each unites; NumberSoul's hand of ten, a CrakShot from its
  second row dealt with its +10; KnightSoul's chip charge (JustcOne
  doubled to 440); a worried MegaMan offered DarkDril at the hand's end,
  picked and used, dark on both; MrkCan1 locking on a MegaMan who steps.
  Every hash agreeing, and the two GBAs of each pair the same, frame by
  frame ([ONLINE_ARENA.md](ONLINE_ARENA.md#full-rules)).
- **A rematch** (19 October 2026, issue #128, a local server, two host
  binaries). After a battle both MegaMen asked "Again with NAME?"; both
  Yeses started the second battle over the same challenge (the second
  game's question held it until its Yes), with the first battle's pair
  hash, to its end, all 36 hashes agreeing; the referees called each
  bout and then 1 to 1. A No (B) turned the other's Yes down at once
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#the-two-client-proof)).

- **Android and Windows** (19 October 2026, protocol v5, a local server
  with seed 120, then the deployed test server once each; issue #129).
  *Android*: the release APK (`build.py android`) in the emulator
  (Android 14, x86-64), BN6's ROM given through the ROMs screen's folder
  picker; NEW GAME, the jack-in at Lan's PC and the touch D-pad to the
  gate's keeper: Another server, `ws://10.0.2.2:8929/presence` typed
  with Android's keyboard, "DONE! IT LEADS TO 10.0.2.2:8929!". On the
  gate "WAIT,WAIT! FIRST, YOUR NAME,PLEASE!", the name field, the
  keeper's question, Yes, and the Square, a host build's MegaMan
  ("Linux") standing in it and each game showing the other's name; the
  D-pad walking there, his card ("HP 100 of 100. Act 3,layer 7!", "His
  PET holds BN5 chips too!"), a Wave ("o/" over MegaMan's head), R and
  Yes home to "We're home,Lan!". Then the keeper's Official Square, his
  questions again for that server, the deployed server over TLS (its
  `/health` counting the one player) and home again. Found on the way:
  the name and address fields were cut off at both sides on a phone held
  upright (the menu canvas is 160 across there and the field 240), and
  Android's keyboard covered their lower half. Fixed: the field asks for
  a canvas 240 across (`platform_own_wide`) and stands in the upper part
  of an upright screen, whole above the keyboard, its keys answering
  taps ("GOT IT! HELLO, Droid!").
  *Windows*: the .exe (`build.py windows`) under Wine 10 headless, its
  data folder's `network.ini` naming the local server: the gate, the
  Square built from seed 120 with the host build's hash (3354384409), R
  and Yes home to "We're home,Lan!", through WinHTTP's WebSocket.
  Then the deployed server over TLS (WinHTTP's own): the same Square and
  hash, and home again. Its first try failed, "Windows WebSocket
  connection or TLS verification failed": Wine's root store was empty
  because the test's Wine prefix had never been written (its container
  ended before wineserver saved the registry, so no crypto provider was
  registered). A prefix made with `wineboot -u` and saved with
  `wineserver -w` fixed it; the game's code was not at fault.
- **A trade and a NetBattle on Android and Windows** (19 October 2026,
  protocol v5, a local server with seed 120, issue #129), each against a
  host build ("Linux", BN6's and BN5's ROMs) in `tools/play.py`; each
  game with BN6's ROM alone, so a mixed pair, its run made by a host
  build with spares from the `chip` dev step (FireBrn1 F, Cannon B) and
  saved at the gate, then copied into its data folder (Android's with
  adb as root). The Square's places were read from a third host build's
  view of its peers.
  *Android*: the release APK in the emulator (Android 14, x86-64, KVM,
  its GPU in software). The trade board: Linux posted Cannon A for a Fire
  chip; Droid walked to Trade Central by the touch D-pad, "THE TRADE
  BOARD! 1 OFFER UP!", "Linux HAS Cannon A! WANTS A FIRE CHIP!", Answer
  it, the order screen listing FireBrn1 F alone (Cannon B left off),
  "Lan! Droid took our offer!" on the host, both Deals, `[20,5]` for
  `[1,0]` in `trades.jsonl`, then the clerk's "YOU TRADED ALREADY!".
  The app, closed in the Square (reinstalled), settled at its CONTINUE
  ("Lan! Cannon A is in our Pack now!", the Pack's 2 to 1 of the one, 2
  to 3 of the other), the host build on its way home (4 to 3, 0 to 1,
  "Lan! FireBrn1 F is in our Pack now!"). A Practice NetBattle by BN6
  rules ("His PET holds BN6 chips only."), delay 6: the same ready hash
  on both (`919efa43fb0d0db9`), the battle to its end at frame 2,809 with
  all 46 hashes the same, Droid the winner. Through the battle the
  emulator's game played 59-61 frames a second, both pair cores in each
  (a frame's update 3.8 ms, 4.5 at most, its longest frame 11.3 ms); the
  emulator's software GPU showed 51-59 of them (its present 14 ms), a
  phone's shows them all. Droid, ahead, waited on the host's keys 219
  ticks of 2,809 (one in 13); the host none.
  *Windows*: the .exe under Wine 10 in a window (Xvfb, the keyboard by
  xdotool). Linux offered Cannon A from the clerk's Offer a chip; Win
  heard "Lan! Linux offers us Cannon A!", walked to the desk, Offer one
  back, FireBrn1 F, both Deals (`[1,0]` for `[20,5]`), and each settled
  on its way home (Win's "Lan! Cannon A is in our Pack now!", 2 to 1 and
  2 to 3; the host's 3 to 2 and 1 to 2). The NetBattle by BN6 rules,
  delay 6: the same ready hash, the battle to its end at frame 2,781 with
  all 46 hashes the same, Linux the winner; 60 frames shown and played a
  second through it (update 4.5 ms, 5.5 at most, the longest frame 11.9
  ms), the way in's five seconds down to 29 shown a second while the pair
  cores boot (its longest frame 110 ms). Win waited 717 ticks, 600 of
  them the ten seconds the host build was held still mid-battle on
  purpose; the challenger gone before the battle began left Win's
  "Huh? They're gone...".
  Found and fixed on the way: Android threw the game's own output away,
  so none of the online, Arena or frame lines could be read there; it
  now goes to logcat (`startup_output`, android/README.md). And a game
  held still for four seconds mid-battle (the host build between
  `tools/play.py` batches; a phone's app sent to the background) stopped
  the battle on both, twice on Android: "my keys could not be sent" and
  "the other player's keys came out of order"; the waiting game now
  stops sampling keys 128 frames ahead and waits
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#lockstep)), which the Windows
  battle above survived. And on the host build's second visit in one
  process its Offer a chip's order screen said BN6's own "I bought a new
  chip! "Cannon A"!!" in Lan's box where the desk's "We'll offer" belongs
  (a word only: the trade settled 3 to 2). The Order Service's script 7,
  MegaMan's word after the Yes, begins with BN6's `EF 00` on flag `0x171D`
  (`BN6_FLAG_NAVI_IN_PET`) and goes to script 20, Lan's, where it is
  clear; `trade_order.c` now says script 20 as its 7. Seen with the flag
  cleared by the `flags` dev step at the desk: "We'll offer "FireBrn1
  F"!" in MegaMan's box. Why the flag was clear on that visit is not
  known yet.

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
- **The Arena on the 3DS and in the browser**; rollback and the stands
  (issue #128).
- **On hardware:** the gate, a visit and a trade on the Nova, the Flip 2,
  a New 3DS, an Android device (the emulator's visit, trade and
  NetBattle above) and iOS, macOS and Windows itself (Wine's above); the
  Arena's stalls over a real network, about one frame in eight on the
  leading game (issue #128); a phone's app sent to the background
  mid-battle, which the lead's cap should now wait out (ONLINE_ARENA.md,
  Lockstep), untried.
- **In the browser with the published server:** the official server
  allows the site's origin alone, so the browser's visit was checked
  against a local one; a trade and the trade board there are untried.
- **The official server:** its Coolify service following `dev` or `main`
  once this work is merged; a trade and a NetBattle through it; a host's
  retention policy, named moderators and its backup and restart checks
  before a public launch.
- **BN5's chips in a mixed NetBattle: a design to refine.** A run has
  Folder1 alone, so under BN6's rules its BN5 chips stand in for that
  battle by chips of the Pack's (ONLINE_ARENA.md, The folder): honest
  and quick, but the owner calls it friction to brainstorm again (a
  NetBattle folder of the PET's, BN6's Folder2 opened online, is the
  other way weighed).
- **The Souls under full rules, what is left:** a MegaMan worried by
  hits alone in a NetBattle, ToadSoul's flag and FloatShoe on panels a
  NetBattle's field lacks, and BN5's own NetBattle as a reference for
  MrkCan1 and for a DarkChip's after-effect
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#not-yet)).
- **A pending trade across systems:** `trade.pending` stays beside the
  saves, outside the `.cwsave` carrier; a save moved between a trade and
  its settling leaves the swap on the system the trade was made on.
