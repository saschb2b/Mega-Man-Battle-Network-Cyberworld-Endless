# Online: the Net Square

[Epic #120](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/120)
puts other players' MegaMen beside the run: a **Net Square** of its own on
a server, reached from a gate in Lan's HP, where MegaMen meet, say a
phrase, read each other's cards, trade a spare chip at Trade Central and
fight a Practice NetBattle of BN6's own (those two with the NetPass's
full set of ROMs, below). The players' side is in
[GUIDE.md](GUIDE.md#online-the-net-square), the gate's piece of the home
in [HOME.md](HOME.md#11-the-net-squares-gate); this page is how it is
built and what has been shown of it. The Arena has its own,
[ONLINE_ARENA.md](ONLINE_ARENA.md), and the cable research it rests on
[ONLINE_LINK.md](ONLINE_LINK.md).

The official server is `wss://cyberworld.saschb2b.com/presence`, deployed
in Coolify from `deploy/coolify/` ([server/README.md](../server/README.md),
Coolify deployment); it speaks the protocol of the branch it was last
deployed from (a game of another protocol version cannot join it, nor a
newer game an older server), trading and the Arena on, until this work
is merged. No release has shipped
the Square yet. Its policy (who runs and moderates it, the rules,
blocking, what it keeps and for how long, how to be removed) is
[server/README.md's The official server](../server/README.md#the-official-server),
summed up in [The official server](#the-official-server) below.

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

## The NetPass

[Issue #131](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/131),
the owner's decision of 20 October 2026: **the full set for what's at
stake**. The Net Square is open to every game: walking, phrases and
emotes, cheering, another MegaMan's card, the referee's report of the
channel's battles, the Board and its posts, the trade board's offers to
read and the week's ranking. Three things need **the full set**, every
ROM this build supports for online play: the Arena (a NetBattle, and
watching one), Trade Central (a trade, and posting or answering a trade
board offer) and the week's DeepNet dive (the greeter's offer and the
dive's post to the Board). The set is BN6's ROM and BN5 Team Colonel's
today, defined once in a table (`src/online/netpass.c`). A future game is
one entry there: the stamps that need it go back to open until a player
has it in, which the release notes and the site announce ahead.

The **NetPass** shows it, and is the one place that names which game
earns which stamp: a key item in Lan's PET (START in the Square), a strip
at the gate and a panel on the second screen. Characters speak only of
stamps, never of ROMs.

### Why

From protocol v4 to v8, BN5's ROM was optional and what two players did
together went by **their common ground**: a card said where the two
PETs differed, a pair where one game lacked BN5's ROM traded BN6's chips
alone, and a NetBattle between them went by **BN6 rules**, the Souls left
home and each BN5 chip in the folder stood in for by the Pack's nearest
BN6 chip (`arena_standin.c`); two games with BN5's ROM fought by **full
rules**. The owner's brainstorm on the stand-ins (issue #131) weighed
ways to ease the mixed NetBattle (the stand-ins, a NetBattle folder, a
loaner folder, stand-ins the player picks) and chose none of them: no
mixed pairs at all.

Through the game-design skill:

- **Dialectic:** one crowd against each player's own collection, as the
  common ground served. The Square stays one crowd for every game; what
  touches a collection (a chip traded, a folder fought with, a dive's
  depth posted) happens where every collection is whole.
- **The critique:** the common ground was **complexity over depth**. Two
  formats, stand-ins, the chips a pair could both hold and the clerk's
  word on a partner's game were rules to learn that gave the player no
  decision to make, and they doubled with every optional game: a second
  one beside BN5 would have made four common grounds where there were
  two.
- **Patterns:** **error prevention** over a good explanation (the mixed
  case never happens, so nothing has to be said about it); a diegetic
  object in the PET rather than a menu of the project's own; **honest
  feedforward** (the gate shows what this visit opens before the Yes); a
  set completed once, with a ceremony (the stamp moment, below).
- **Loop layer:** the session between acts, as the Square's: the pass is
  read at the gate, as a visit begins. It completes once, at the meta
  loop's pace (a ROM added), and changes nothing in a run.
- **What it does not do:** gate the whole hub. A Square for the full set
  alone would empty a small crowd, and each new game would lock out,
  after the fact, players who were in.

**The data** (Umami, 6-20 October 2026, run-start events with the
statistics on): 46 of 53 sessions of release builds had BN5's ROM (87%),
68 of 84 across all builds (81%). The rule costs about one player in
eight NetBattles and trades; that player keeps the Square.

### The stamps

| Stamp | On the pass | Opens | Needs |
| --- | --- | --- | --- |
| Square | meet,cheer | Walking, phrases and emotes, cheering, cards, the referee's report, the Board and its posts, the trade board's offers to read, the week's ranking | BN6's ROM, every game's |
| Arena | NetBattle | Asking for and taking a Practice NetBattle, a rematch, watching one in the stands | The full set |
| Trade | chip swap | A trade at Trade Central, posting or answering a trade board offer | The full set |
| DeepNet | the Board | The week's DeepNet dive: the greeter's offer, the dive's post to the Board | The full set |

`netpass_has` answers for a stamp, `netpass_full` for the set
(`netpass.h`). A pass holds 1 of 4 stamps or 4 of 4: the three that need
the set come together.

### The pass

A 240x160 screen in the project's own pixel art, in the PET's colours
(`tools/site_art.py`'s palette): a green header bar, "NetPass", with the
pass's level on its right ("Square pass", or "★ Full pass" in gold); an
operator card on the left with MegaMan's face, drawn from the player's
own ROM as the screen opens (never stored), "Lan", "MegaMan" and a stamp
bar ("1 of 4", "4 of 4"); the four stamps in a 2x2 grid, a gold stamp
with a check where held, a dashed empty one with "?" where not; and a
bottom panel with the cartridge slots, BN6 in and BN5 in or open
(dashed), beside one line of PET text: "Team Colonel's ROM stamps Arena,
Trade and DeepNet" while BN5's is missing, "Every game in. The whole Net
is yours." once the set is complete.

It shows in three places:

- **In the PET**, as a key item (`0x4A`, among the ids BN6 leaves
  nameless that the run's own items use), always shown. In a run A on it
  opens the pass over the KeyItem screen; A or B closes it, the game's
  keys held meanwhile and its closing button kept from BN6 until it is let
  go ([PET.md](PET.md#the-netpass)). **In the Square, START** opens it:
  BN6's PET stays shut there (`online_chat.c` takes START, as before),
  since what its screens change is the Square's RAM, thrown away on the
  way home, so START, the PET's own button, is the nearest clean way to
  the same screen (`netpass_screen.c`).
- **At the gate** in Lan's HP: while MegaMan asks "To the Net
  Square,Lan?", a strip over the top of the screen shows the pass's level
  and its four stamps, so the player sees what the visit opens before
  saying Yes ([HOME.md](HOME.md#11-the-net-squares-gate)).
- **On the second screen** (the 3DS's bottom screen, Android's second
  display) while MegaMan is in the Square: a NETPASS panel with the pass
  drawn on it.

### What characters say

They speak of stamps. The referee by the ring, to a game without the
Arena stamp, says "NO ARENA STAMP ON YOUR NETPASS!" in BN6's Mr.Prog
voice, and still calls the channel's battles; Battle is offered on a
card only where both games hold the Arena stamp, and "WANNA WATCH?" is
not asked without it. Trade Central's clerk tells a game without the
Trade stamp that it has none, and lists only Navis whose pass holds one;
the board's offers still read. The greeter offers the week's DeepNet dive
only to a full pass; to the rest "THIS WEEK'S DEEP NET IS OPEN!" and "BUT
YOUR NETPASS HAS NO DEEPNET STAMP! SORRY!", and a dive's end waits to be
posted until a visit with the stamp. A card names no game's chips any
more: "His PET holds BN5 chips too!" and "His PET holds BN6 chips only."
went with the common ground; MegaMan reading a card whose pass is not
full says "His NetPass has just the Square stamp.", which is why Battle
is not among its options.

### The stamp moment

The first visit to the Square after the pass becomes full, once MegaMan
is free on the map, the greeter says "WHOA! YOUR NETPASS HAS ROOM FOR
EVERY STAMP NOW!" and "HOLD STILL! STAMP,STAMP,STAMP!"; the pass opens
and the new stamps land one by one, each with a white flash and BN6's
item jingle (sound `0x73`), then the header turns to "★ Full pass" with
BN6's sound `0x95`. Both sounds are BN6's own, read from the player's
ROM and played by the engine's MP2K mixer over the game (`audio_sfx`),
not by the core. A or B while the stamps land lands them all at once; A or B after closes it. It happens
once: the most stamps the game has seen is kept in a small file of its
own in the data folder, `netpass.ini` (`stamps=4`), never in the run's
save: the pass follows the player's ROMs, not a run. A game with BN6's
ROM alone has no moment (its one stamp is every game's).

### Every system, and the New 3DS

The owner's aim (20 October 2026): every stamp earnable on every system.
The browser now battles as the desktops and phones do, its pass and its
stamps theirs ([ONLINE_ARENA.md](ONLINE_ARENA.md#in-the-browser)). The
New 3DS builds the Arena but offers no NetBattle and no watching
(`CW_ARENA`, `square_arena_offered`): a frame of the battle's two GBAs
takes 25.8 ms there against a 60th of a second's 16.7, measured on the
device ([ONLINE_ARENA.md](ONLINE_ARENA.md#the-new-3ds)). The pass is the
same there: its stamps follow the ROMs, as the server's rule does, so
with BN5's ROM it trades and dives, and the Arena stamp shows held.
Battle and Watch are not offered (the referee: "SORRY! NO NETBATTLES FROM
THIS PET!"), and the pass's PET text says so in place of the whole Net's:
"Every game in! No NetBattles on this PET." The second screen's
panel draws the pass's words in the engine's small font, its pixel
letters being a texture's, which a picture drawn into memory (the 3DS's
bottom screen) cannot use.

### On the wire

Protocol v9 holds the rule on the server. HELLO has carried BN5's
identity since v4 (its game code and SHA-1, zeros without it, checked
against `protocol.json` as BN6's is), and every SNAPSHOT peer says each
player's ROMs (a flags byte). The server refuses, with NOTICE 28
(`notice_pass`), a challenge, an accept, a trade's propose, answer or
take, a board post, a WATCH and a WEEK_POST from a session whose HELLO
lacks the full set (`roms_full`), and a challenge or a trade aimed at
one. NOTICE 19, which refused a BN5 chip or full rules between games
that did not both have BN5's ROM, is retired. v9 also drops the rules
byte from ARENA, ARENA_STATE, STANDS and WATCH_STATE: every NetBattle is
by full rules. The trade board still leaves an offer whose chip is BN5's
out of the BOARD sent to a game without BN5's ROM, which could not name
the chip; that game reads the rest. Only identities and numbers cross:
a game without BN5's ROM never receives a byte of it.

The Square itself is built from BN6's own nets (Central, Sky, Green), the
same for every game whatever ROMs it has, and a card names Crosses and
Souls from words of the game's own, never read from BN5's ROM.

## Where things are

| Part | Sources | Builds and checks |
| --- | --- | --- |
| The game's side | `src/online/` (the NetPass's set and stamps in `netpass.c`), the gate in `src/director/director_gate.c` and `src/map/lanhp.c` | `build.py` and every target; `build.py test` (`tests/test_online.c`: the Square's plan and hash, `tests/test_arena.c`), `build.py lint` |
| The server | `server/`, its deployment in `deploy/coolify/` | `build.py server`, `build.py server-test`; its own Dockerfile and CI workflow |
| The wire | `shared/online/` ([protocol v9](../shared/online/README.md)) | The server's tests hold its codecs, the C constants, the Square's generator version and hash and the ROM's identity to the client's |

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
another map's slot). Whatever the area, the Square plays its own music:
BN6's Navi Customizer theme (song 0x1E, `BN6_SONG_NAVI_CUSTOMIZER`, the
owner's pick, 20 October 2026), not the area's. The plan's hash and its generator version
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
- **Check text**: A facing a prop says what it is (the Board also where
  its art shows, over the floor before the rim it stands past: its piece
  is drawn 24 units lower, and a player facing it there read nothing,
  session 74); the Board's posts are
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
box (`online_chat.c`, words in `square_words.c`): L for a phrase, an
emote or a look around, A facing another MegaMan for his card (HP, act and layer,
Crosses, Souls) and Battle (where both passes hold the Arena stamp),
Wave, Mute, Block; R or the pad for home, stepped on as drawn or A on
it (its zone its middle stretched towards the screen's front, as Lan's
HP's portals are: a player stood on its front 21 units from its middle
and nothing asked, session 74). A talk's column of options takes up
and down (`cinema_keys` lets the pad through while MegaMan is held).
The Square's flags are `0x1480`-`0x14C5`, its own while it lasts.

**Finding the wings.** A player ran round an empty Square for ten calls
and found the greeter, the keeper, a walker and the props, but neither
Trade Central's clerks nor the referee (session 74): the wings stand
eight panels or more off the plaza, past what the screen shows of it.
Both stand in every Square (the layout is the same for every seed, only
its props vary), so the way to them is signposted, BN6's way and the
layers' (the game-design skill: signposts over waypoints, by the
landmark everyone sees, words that agree with the map). L's third
option, **Look around**, has MegaMan say where they lie from where he
stands (`square_life_look`, words in `square_life_words.c`): in the
plaza, "Trade Central's to the upper left,Lan!" and "The Arena's to the
lower left!"; anywhere else, which way the landmark is first (as L's
way on a layer, `way_to`), then each wing past it, or "right here" in
its own. The wings stand off the plaza the same way in every Square, so
these words hold for every seed. The greeter's welcome ends on the same
signpost: "TRADE CENTRAL AND THE ARENA ARE PAST THE STATUE!" (the
landmark's name). No map: the Square has no SELECT screen of its own.
Phrases and emotes are fixed lists, never free text; mute and block are
the player's, kept by the server's public identity of the other.

## Trade Central

A Mr.Prog at a desk in the wing up-left of the plaza (-x) trades one
spare chip (a copy past those in the run's three folders) for another
player's (`trade_central.c`, words in `trade_words.c`). Both games hold
the NetPass's Trade stamp (above), so every chip of either game's
registry trades, BN5's too: its ids are the registry's, fixed in
`src/core/xchips_registry.c`, so they name the same chip on every game
with BN5's ROM. To a game without the stamp the clerk says it has none,
and the trade board's offers still read; the server refuses that game a
trade, a post and an answer (NOTICE 28).

1. MegaMan picks a Navi in the channel whose pass holds the Trade stamp
   (the clerk lists no other) and the chip he offers, on the **spares
   list** (below).
2. The other player's MegaMan hears the offer, goes to the desk and picks
   a chip back on the same list (its note says what the offer is), or
   says no.
3. Both see the pair on the **chip trade** (below) and say Deal!; the
   server commits the trade only when both accepted the same pair, and
   writes it to `trades.jsonl` (`server/cyberworld_server/trade.py`). A
   stale or mismatched step, a player gone, or five minutes without a
   step calls it off.
4. Each game keeps the done trade in `trade.pending` the moment the
   server says it is done, wherever MegaMan stands, and applies it at
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

### The two screens

[Issue #145](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/145),
the owner's model the Mega Man Battle Network Legacy Collection's trade
screens: the trade was told in chat boxes, and the exchange itself, one
that can't be undone, was never shown. Two PET screens of our own now
show it, drawn by the engine over the game's picture as the NetPass is
(`trade_screen.c` their course, `trade_screen_draw.c` their picture in
the NetPass's colours and parts, `trade_screen_text.c` their PET text,
`trade_list.c` the list's rows, sorts and filter; the game held under
them, no key reaching the core, their closing A or B kept from BN6 till
it is let go). The clerk's chat boxes stay for the talk around them
(hello, whom, the board, the wants), MegaMan's words in `trade_words.c`.

- **The spares list**, used wherever a chip is picked: an offer to a
  Navi ("Offer a chip"), one back to his ("Offer one back", its note
  "Alpha offers FireBrn1 F"), an answer to a board offer ("Answer an
  offer", only the spares that meet its want, its note "Spares that fit:
  a Fire chip") and a post ("Post an offer"). On the left a card of the
  chip under the cursor: its picture and palette, name, code, element
  icon, power and description, read from the player's ROM as it is drawn
  (`second_art.c`'s readers, the chip registry's for BN5's chips, so
  nothing of Capcom's is kept in a file); on the right MegaMan's spares
  (copies past those his three folders hold), each row its icon, name,
  code and count, the cursor's lit, six at a time with a scroll bar.
  UP and DOWN move (round at the ends), LEFT and RIGHT a page, START
  sorts by BN6's folder editor's first three keys, ID, ABCDE and Code
  (the record's IDSortPos and AlphabetSortPos; the session keeps the
  sort), A picks, B goes back. It replaces BN6's Order Service screen
  (`trade_order.c`, gone) and the chat box's list it fell back on for a
  BN5 chip past BN6's 319, whose list builder would have written past
  its end; the screen lists any id the registry has.
- **The chip trade** ("Chip trade", "With Beta"): both cards, You and
  Them, the arrows between them, and No deal and Deal!, the cursor on No
  deal as "Deal,Lan?" had it on No; B is No deal. After Deal! the cards
  hold, "Waiting on Beta..." under them and the arrows pulsing, B calling
  it off; when the server commits, the cards cross (MegaMan's over the
  top, the other's under it, 40 frames) and land with BN6's jingle
  (`SFX_GOT`, as the NetPass's full pass lands), "Traded!", and A closes
  it; MegaMan then says "Deal! FireBrn1 F is ours once we're home!" as
  before. Called off by the other player or the server meanwhile, it
  closes and MegaMan says the trade's off. MegaMan's own answer or take
  opens it at once; an answer to his offer, or his board offer taken,
  first has his word ("Lan! Beta answered us!", "Lan! Alpha took our
  offer!"), wherever he stands.

The design (the game-design skill, interface): an exchange that can't be
undone gets a screen that shows exactly what changes hands (error
prevention, honest feedforward), with the friction matched to the
stakes: one deliberate press past a default on the safe side, not a
hold or a second question. The list makes the spares visible before
anyone is asked (what is mine to offer, and what fits a want), so the
board's answer is a choice among what fits rather than a guess. The
swap is the session loop's payoff, juiced only as far as the state stays
legible: the cards move, the sound plays once, and what landed is framed
in gold. A second sort answers the same request AsterLand's Order
Service answered (issue #124): ID, ABCDE and Code are BN6's own keys.

### The trade board

A trade needs two players at the desk at once and a blind first offer;
in a channel of a few players, most visits never find out who wants
what. The board says it: each player may put **one offer** up at the
clerk, a spare he **has** (picked on the spares list as a trade's chip
is) and what he **wants** for it, from a fixed list: anything, a Fire,
Aqua, Elec, Wood, Sword, Wind, Break or Cursor chip (BN6's own icon
element, `BN6_CHIP_ELEMENT`), or a Mega (`BN6_CHIP_LIBRARY_TYPE`). The
clerk reads the channel's offers one a box ("Alpha HAS Cannon A! WANTS A
FIRE CHIP!") with Answer it and Next offer under each, and MegaMan's own
to post or take down; the Board in the log-in plaza lists them among its
posts. Answering one lists only the spares that meet its want; the trade begins with both chips named (TRADE's
take, protocol v5), and both say Deal as in any trade. The poster's
game checks the chip against the want it posted and calls off one that
is not (the server never reads a chip's element); MegaMan then says
"Lan! Beta took our offer!" before the Deal. An offer lives on the
server while its poster is in the channel: taken down, a channel
changed, the server left or a trade done, it goes, and no offer is
kept on disk (`trade.py`; `online_board.c` holds the board as last
said, `trade_central.c` the clerk's chats, `trade_words.c` the words).
Posting and answering take the Trade stamp; reading does not. A game
without it reads the clerk's list and the Board's, and the server refuses
its post or answer (NOTICE 28). It is never sent an offer whose chip is
BN5's, which it could not name, and reads the rest.

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

## The week's net

[Issue #127](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/127):
each server gives a net of its own for the week, every player who dives
it dives the same net, and the Board in the log-in plaza lists how deep
they say they got. The owner approved the proposal on 19 October 2026
(each server its own week, the setup locked, as many dives as a player
likes, depths posted as "reported") and decided one thing while it was
built: **a dive keeps a save slot of its own**, so the run in progress
is never touched, and the player goes back to it when the dive ends.
Diving takes the NetPass's DeepNet stamp (above); the Board's ranking
reads on every game. Every diver has BN5's ROM, so the areas a game with
it dresses in BN5's (`run_dress`, by the seed and the ROMs present) are
the same on every dive.

### Its design

Through the game-design skill:

- **Dialectic:** the run's integrity against the community's generosity,
  as the Square's own. The week's net is the pattern "a seedable RNG buys
  community" (Slay the Spire's daily climb): the same net for everyone
  this week, so a depth means something when it is compared.
- **Loop layer:** a session goal beside the normal runs, at the meta
  loop's pace (a week), feeding nothing but the Board. A dive never
  makes a normal run stronger and a normal run never makes a dive
  stronger.
- **Patterns:** a diegetic board (the BBS, in posts); a world that
  offers rather than a menu (the greeter Mr.Prog by the Board asks, BN6's
  own Yes/No with the cursor on No); the same contest for everyone (the
  standard setup and a standard profile); honest feedforward (he says the
  dates and that every Navi dives the same net, the Board says the layers
  are as reported).

What the proposal left open, decided the same way:

1. **What a dive shares with the normal runs.** Nothing that changes
   what a run is or how strong it starts. A dive starts from a *standard
   profile*, made from the player's at its start: his settings and what
   he has been told (MegaMan's tips are not said twice; docs/VOICE.md, a
   button is named once), and none of what runs earned: no threat rungs,
   folders, Cross starts or BeastOut opened, no Spins, programs found or
   compression codes kept, no rival's clearance, no title marks (the
   Secret Area's master follows them, `run_new`, so marks would make
   another net), no records. Anything the dive earns stays in its own
   profile and is let go at the next dive: the critique's "no opportunity
   cost" and "power creep" both stay out, and the comparison stays fair.
   Its end opens nothing (`meta_run_quiet`) and counts in no record of
   the normal runs. Nor does a dive trade: on one, Trade Central's clerk
   says "DIVERS DON'T TRADE! SAME NET,SAME CHIPS FOR ALL!", MegaMan's
   board offer is taken down and an offer that comes in is called off
   unheard of (`trade_central.c`), so every dive plays the standard
   setup's chips and no swap can land in the other slot.
2. **How an unfinished dive goes on.** In the world, as it began: the
   greeter offers it again (Keep diving, Start over, Not now; B is Not
   now). The title's CONTINUE stays the normal run's, so the title keeps
   its one meaning and needs no new row; the owner's rule, no new menu
   screens where the world can do the job, holds. The price is a walk
   from the gate each session and a connection to the server, which the
   dive needs anyway to post.
3. **When the week turns mid-dive.** The dive keeps its week and its
   seed; its end is posted to that week while the server still takes it
   (this week or the last), and is let go after. An unfinished dive of an
   older week gives way: the greeter offers this week's net, which begins
   over it.
4. **Ranking.** By layer, then a win (the short net's Nest), then fewer
   minutes, then the earlier post: a speed tie-break that rewards the
   run's own dialectic, prepare or press on, among the many who win.

### On the server

`server/cyberworld_server/week.py`. A week is numbered from Monday 5
January 1970 (`week_epoch`, 345600 seconds), so every Monday 00:00 UTC
begins one. Its seed is the first four bytes of HMAC-SHA256(the server's
identity secret, "week" and the week's number): each server has its own
week, the same across restarts, and nobody can guess the next. WELCOME
carries the seed and the week's number (protocol v6). WEEK_POST posts a
dive that ended: its week, seed, layer, act, won, minutes and a summary
hash. The server checks only what it can: the week is this one or the
last, the seed is that week's, the layer is 1-10 and the act is that
layer's (three layers an act, the Nest the fourth), a win only on layer
10, the hash not zero; anything else is a NOTICE (23-26). One entry is
kept per identity and week, the best, and each improvement is appended
to the state folder's `weeks.jsonl`, read back at the start. WEEK_BOARD
sends each player this week's top eight, his own best and its place,
and last week's top three, after WELCOME and whenever the Board
changes. A moderator takes an entry down with `--admin hide IDENTITY
[--week N]`, which also refuses that identity's posts for that week.
A depth is a claim: a modified game can post any layer, and nothing is
given for one, so the Board says "as reported".

### In the game

- **The slot** (`src/core/weekly.c`, docs/SAVES.md, The weekly slot):
  `save_slot(true)` puts every progress file of the run after
  `weekly-` in `savedata/`; `weekly.sav` (the normal slot's) holds the
  dive in the slot and a dive's end to post. Starting a dive clears the
  slot, makes its standard profile and the run with `run_new` on the
  week's seed (not `run_new_varied`, which avoids the last run's first
  guardian), `run_setup(RUN_SHORT, FOLDER_STANDARD, 0, 0, 0)`, and the
  game starts it in Lan's room as NEW GAME does. Continuing it loads the
  slot's checkpoint as CONTINUE loads a run. The title always leaves the
  slot first (`weekly_normal_saved`), so its CONTINUE is the normal
  run's whether the dive ended, was reset or the game was closed.
- **The greeter** (`src/online/square_week.c`, words in
  `square_week_words.c`): his archive is his welcome, then, to a pass
  with the DeepNet stamp, the week's post: the net shifted, this week's deep net open, its dates, every Navi
  dives the same net, and "DIVE IN NOW?" (Yes/No, the cursor on No). With
  an unfinished dive of this week, "YOUR DIVE INTO THE DEEP NET ISN'T
  OVER!" and Keep diving, Start over, Not now. On a dive, he wishes
  MegaMan luck; with a trade waiting to go home, he asks for the trade
  home first (the swap is settled into the run the gate saved, never a
  dive). His choice sets the Square's free flag `0x14AF` (with the way
  home's `0x1493` beside it for Start over); the visit ends
  (`VISIT_END_DIVE`) without the gate's resume, and the dive begins or
  goes on. The normal run's gate checkpoint stays as the gate wrote it.
  A game without the stamp is offered no dive, and the server refuses its
  WEEK_POST (NOTICE 28).
- **The post**: a dive's end (`weekly_ended`, after the run's records
  and saves are done in its slot) keeps its layer, act, won, minutes and
  hash, the better of two of the same week. The next visit sends it as
  soon as WELCOME has named the week; the server's answer, a WEEK_BOARD
  or a NOTICE, lets it go. The greeter then says "YOUR DIVE TO LAYER 5 IS
  UP ON THE BOARD!".
- **The Board**: after its channel's posts, "DEEP NET" with the week's
  dates, "Deepest layers,as Navis report them.", the top eight three to
  a box ("1.Alpha:5", a "!" for a win), "Your best is layer 5,#1!", and
  last week's best three.
- **The summary**: a dive's shows its layer, the viruses and Navis
  deleted, the Library without a count added, and "This week" with the
  deepest layer of this week's dives in place of the best ever; no goal
  line.

`tests/test_backup.c` holds the slot carried and put back byte for byte
in a `.cwsave`, its names (and none else) taken; `tests/test_online.c`
WELCOME's week, WEEK_BOARD read and refused, the post's bytes and the
Board's words and dates; `server/tests/test_week.py` the seed, the best
kept, the refusals, the file read back across a restart and a week's
turn, and a moderator's hide.

## The Arena

Battle on a card, offered where both games' passes hold the Arena stamp
(above), sends a challenge; if the other player takes it, both
games run BN6's Practice NetBattle on **two linked cores each**, the two
MegaMen carried as they are (folder, NaviCust, HP, buster, Crosses,
Souls), and only the players' keys cross the server. Every NetBattle is
by **full rules** since v9: BN5's installs in both pair cores of each
game, the installs' record compared, BN5's chips in the folder and a
Soul MegaMan unites with carried to the other GBA
([ONLINE_ARENA.md](ONLINE_ARENA.md#full-rules)). Where more than one of
Folder1, Folder2 and the Extra Folder holds chips, Lan picks the one
carried ([ONLINE_ARENA.md](ONLINE_ARENA.md#one-rule-full-rules)). The
Square's core holds still
meanwhile; after the battle each MegaMan stands where he stood, says
how it went and asks Lan whether to go again with the same player (the
cursor on No): when both say Yes they fight again with the same
folders, over the same ARENA messages (a challenge answered at once). The
referee by the ring calls the bout and keeps the count. A hash of the
pair every 60 frames catches two games apart and stops both. Nothing is
won or lost. It runs on every system but the New 3DS, the browser
among them; the New 3DS builds it but does not offer it (`CW_ARENA`: a
frame of the pair takes half again a 60th of a second there): Battle is
not offered there, whatever the pass, and a challenge is declined. [ONLINE_ARENA.md](ONLINE_ARENA.md) has the engine, the wire,
the lockstep and the two-client proof.

### The stands

[Issue #128](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/128),
the owner's decision of 19 October 2026: the Arena's wing has stands. A
NetBattle in the channel is **watched for real** by a game that can build
its pair (the Arena built there, and the Arena stamp on its pass): the
referee by the ring asks "WANNA WATCH?", and Yes turns the screen to the
battle, its two cores built and run here from both players' keys as the server hands them on,
a second and a half behind them, from the battle's first frame however
late the spectator came (he catches up faster than real time). R shows
the other GBA's side, L cheers, B goes back to where MegaMan stood. The
spectator checks the players' hashes and stops, saying so, where they
disagree. Every game, the New 3DS and a game without the Arena stamp
among them, gets **the report**: the referee calls the channel's battle (who fights whom, each
MegaMan's HP, the last chip each used, how many watch, the winner and
the cheers) and the Board posts it, from what the cable's first game
reads off its own pair and sends now and then. A phrase or emote said
while a battle is on is a cheer, held from the two players until their
battle is over; the referee then tells them. The stands give nothing
toward a run. How it is built, and its design: [ONLINE_ARENA.md, The
stands](ONLINE_ARENA.md#the-stands); the wire (protocol v7: STANDS,
ARENA_STATUS, WATCH and the server's three WATCH messages; v9 took the
rules byte out of STANDS and WATCH_STATE and refuses a WATCH from a game
without the full set, NOTICE 28):
[shared/online/README.md](../shared/online/README.md).

## The server and the wire

Protocol v9 ([shared/online/README.md](../shared/online/README.md))
is binary over a WebSocket (`cyberworld.presence.v9`; v7 added the
stands, v8 the latency each game asks for with rollback on, v9 the
NetPass's rule, above). The server refuses
another protocol version, another Square generator or hash, another ROM
code or SHA-1 (BN6's, and BN5's where one is named), a name out of
bounds and a full Square; it refuses what needs the full set from a
session without it (NOTICE 28: a challenge, an accept, a trade's steps,
a board post, a WATCH, a WEEK_POST, and a challenge or a trade aimed at
such a session); it bounds every
message, the players, pending handshakes, each client's traffic and its
output, and relays the Arena's keys under a bucket of its own. Trading
and the Arena each have their switch in its configuration
(`trading`, `arena`). A player's identity is a key of the PET's own,
derived per server from a local secret; moderators ban, mute, kick and
allowlist through the server's command line, and take a week's Board
entry down (`hide`). Hosting, TLS, backups and
the platforms' transports are [server/README.md](../server/README.md)'s.

### The official server

`wss://cyberworld.saschb2b.com/presence`, run by Saschb2b, who is its
moderator, as a hobby server with no guarantees; its policy is
[server/README.md, The official server](../server/README.md#the-official-server).
Fixed phrases and emotes only; a player blocks another from his card,
and a moderator mutes, kicks, hides a Board entry or bans on what he sees
himself; with no free text there are no reports. The server has no
accounts and writes no IP address: a public identity derived
from the game's per-server token, in the trades' log for 14 days (no
players' log on the official server: `log_days = 0`), the Board for 2 weeks, bans
and mutes until lifted, and daily backups of its key, moderation and
Board for 14 days (`server.ini`'s `trade_days`, `board_weeks`,
`log_days`, `backup_days`), each pruned by the server itself
(`retention.py`) and by its backup service (`backup.py`, a second
service of `deploy/coolify/compose.yaml`). The console's log names no
player. `--admin forget` answers a request to be removed. It follows
`dev` until the release that ships online play, then `main`
([server/README.md, At a release](../server/README.md#at-a-release-the-official-server-follows-main)).

## Two games at once

[DEVTOOLS.md](DEVTOOLS.md#the-net-square-two-games-at-once) says how:
a local server (`build.py server-run`), a `network.ini` per data folder or
`--dev online=URL,onlinename=NAME`, and two `tools/play.py` sessions
from `--scene home`; never the official server. A visit plays at the
clock's pace even headless, so the two meet. `tools/online_browser.html`
starts the browser build from home onto the gate, for a browser beside a
native game (the server's `origins` allowing the page's), with BN5's ROM
too where chosen (the full pass: a NetBattle, `--dev arenaboth` making a
game take a challenge and play it by itself). The Arena's own
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
  FireBrn1 F is in our Pack now!", Beta's Cannon A. Then a BN5 chip's
  offer, which a game without BN5's ROM is not sent (so since v9 too):
  Alpha back with MrkCan1 M (411, the pack's extension)
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
  none: what issue #128's smoothed delay is for (ONLINE_ARENA.md, Smoothed
  delay: through a local relay at that delay, 16 to 24 waits in 100 frames
  before, none after the first five seconds since).
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
- **The pair's common ground, before v9** (the same day, protocol v4, a
  local server; two host builds, one with BN5's ROM beside BN6's or
  both). Two games with BN5: one offered MrkCan1 M (411, the pack's
  extension) from the clerk's chat-box list, the other answered with
  WideSht1 L (204, a blank record) from the order screen, both said Deal,
  `trades.jsonl` held `[411,12]` for `[204,11]`, and each game's CONTINUE
  settled it: "the Pack's 2 to 1 of the one, 0 to 1 of the other" (BN6's
  TakeChips and GiveChips on both kinds of id), MegaMan's "Lan! MrkCan1 M
  is in our Pack now!". Mixed pairs, which v9 no longer allows (issue
  #131, The NetPass above): the clerk's "THAT NAVI TRADES BN6 CHIPS!"; a
  NetBattle by BN6 rules with Folder2 carried (26 hashes agreeing), and
  one with Folder1's two MrkCan1 M stood in for by Pack chips (25). v9
  removed BN6 rules and the stand-ins with them.
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

- **The stands** (20 October 2026, issue #128, protocol v7, a local
  server, three host builds: two players and a spectator). A Practice
  NetBattle watched from the referee's "WANNA WATCH?" to its end on the
  third game: its pair built there agreeing with the players' ready, 41
  hashes checked against the players' 41, the end at the same frame
  (2,506) on all three; a spectator who came 28 seconds late caught up
  from 1,686 frames behind and watched it live to its end, 41 hashes
  checked; a battle by full rules watched to its end by a game with
  BN5's ROM, its pair made with the players' installs' record, 51 hashes
  checked; a spectator whose copy of the keys was changed stopped by
  itself, saying so, while the players fought on. Three cheers from the
  stands, held from the players, on the Board and in the referee's word
  to the winner after. A battle by full rules called by the referee on a
  game without BN5's ROM, the report alone; the browser build, which has
  no Arena, given the same report live (HP, MiniBomb used)
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#the-three-game-proof)).

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
  game with BN6's ROM alone, so a mixed pair (before v9, which leaves
  such a game the Square and no trade or NetBattle), its run made by a host
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
- **The week's net** (19 October 2026, protocol v6, a local server, the
  Linux build in two `tools/play.py` sessions, Alpha from `--scene home
  --run-depth 7 --seed 3`, Beta from `--run-depth 4 --seed 4`, both with
  `--dev quiet,online=...`). Alpha at the greeter: his welcome, "THE NET
  SHIFTED! THIS WEEK'S DEEP NET IS OPEN!", "OCT 19-OCT 25. EVERY NAVI
  DIVES THE SAME NET!", "DIVE IN NOW?" with the cursor on No; Yes, "OFF
  YOU GO! HEAD HOME AND JACK IN!", the visit's end (12) and the log's
  "a dive of week 2963 begun (seed f521fe36)", MegaMan in Lan's room with
  Dad's call; the dev menu's "Depth 1 seed 4112645686", the server's
  week's seed. The normal run's twelve files, hashed after the gate's
  save, unchanged while the dive was played to layer 4 (the dev menu's
  Next layer) and the game was closed there. Opened again: the title's
  CONTINUE "Home", the normal run at the gate; at the greeter "YOUR DIVE
  INTO THE DEEP NET ISN'T OVER!" and Keep diving, Start over, Not now;
  Keep diving took the dive up on layer 4 (RoboDog Comp). On layer 5,
  MegaMan deleted in a battle; the log's "the dive of week 2963 ended on
  layer 5", the title's summary, the title's CONTINUE again the normal
  run's ("Home"), its files hashed before the dive taken up unchanged.
  The next visit: "the week's dive to layer 5 posted", `weeks.jsonl`'s
  line (week 2963, layer 5, act 2, two minutes), and the greeter's "YOUR
  DIVE TO LAYER 5 IS UP ON THE BOARD!". Beta, a second game, at the
  Board: after its channel's posts, the week's heading with "Oct 19-Oct
  25" and "1.Alpha:5". A second dive from the greeter's new offer ended on
  layer 3, the normal run's files unchanged again; on the final build two
  more, to layer 3 and then layer 2, the second's summary "Reached Layer
  2" and "This week Layer 3", the next visit's post of the first ("the
  week's dive to layer 3 posted", the Board's best for Alpha still 5,
  and the greeter's word on it rightly left out). The saves
  of that data folder, both slots, packed by the game's own carrier code
  (`backup_pack`, 22 files) and installed into a fresh folder
  (`backup_restore`, status OK): every file the same but `profile.sav`,
  whose volumes are the receiving device's, as designed. Found on the
  way: the greeter's first flag was Trade Central's (`0x1494`, the clerk
  answered); the dive's flag is the Square's one left free, `0x14AF`. The
  transport took no WEEK_POST until it was named among its own messages.
  An autopilot run (15000 frames, seed 1) ended as runs do.
- **The NetPass** (20 October 2026, issue #131, protocol v9, a local
  server, the Linux build in `tools/play.py`: Alpha and Gamma with BN5's
  ROM beside BN6's, Beta with BN6's alone, `--scene home --run-depth 7`).
  The server's join lines named the ROMs (`"roms":1`, `"roms":0`). Beta
  in the Square: START opened the Square pass ("1 of 4", the BN5 slot
  open, "Team Colonel's ROM stamps Arena, Trade and DeepNet"); the
  referee said "NO ARENA STAMP ON YOUR NETPASS!", the clerk "HMM... NO
  TRADE STAMP ON YOUR NETPASS!", the greeter "THIS WEEK'S DEEP NET IS
  OPEN!" and "BUT YOUR NETPASS HAS NO DEEPNET STAMP! SORRY!" with no
  question. Alpha's first visit: the greeter's "WHOA! YOUR NETPASS HAS
  ROOM FOR EVERY STAMP NOW!", the pass with Arena, Trade and DeepNet
  landing one by one, then "★ Full pass", and `netpass.ini` saying
  `stamps=4`; Alpha's and Gamma's next visits had no moment. Alpha
  reading Beta's card: "His NetPass has just the Square stamp." and Wave,
  Mute, Block, no Battle; Gamma reading Alpha's: Battle first. Gamma's
  challenge, Alpha's Yes, and a NetBattle by full rules (`arenahp=12`)
  to its end at frame 4,944 on both: the same installs' record
  (1,913,709 bytes, 132 hooks) and ready hash, all 82 hashes agreeing,
  rollback on (window 9, delay 2; 469 and 515 rollbacks), Gamma the
  winner. Then a trade between the two: the clerk listing Gamma alone,
  Alpha's spares Cannon A and MrkCan1 M (411, BN5's, in the clerk's
  list), MrkCan1 M offered, Gamma's Cannon A back from the order screen,
  both Deals, `trades.jsonl`'s `[411,12]` for `[1,0]` and
  `trade.pending` on both. A ROM-free client with BN6's identity alone,
  at the same server, was answered NOTICE 28 ("That needs the full
  NetPass") to an ARENA challenge, a TRADE proposal, an OFFER post and a
  WATCH. In a run (seed 7, `--run-depth 2`): the PET's KeyItem listed
  NetPass with its description, A opened the pass, B closed it back to
  the KeyItem screen. The gate's strip over MegaMan's question in both
  states, the second screen's NETPASS panel in the Square, and the pass
  at 240x160, at 640x480 and on a phone held upright (1080x1920, the
  touch controls under it) captured headless.
- **Trade Central's two screens** (20 October 2026, issue #145, a local
  v9 server, the Linux build in `tools/play.py`, Alpha and Beta each with
  BN5's ROM beside BN6's, `--scene home --run-depth 7`, spares from the
  `chip` dev step). Alpha at the clerk, Offer a chip, Beta: the spares
  list "Offer a chip", "To Beta", Cannon A and FireBrn1 F with their
  counts and Cannon's card; FireBrn1 F picked, "SENT!". Beta heard the
  offer, answered at the desk with Offer one back: the list "Offer one
  back", its note "ALPHA OFFERS FIREBRN1 F", MrkCan1 M (411, BN5's)
  picked, and the chip trade at once, You MrkCan1 M, Them FireBrn1 F, the
  cursor on No deal. Alpha heard "Lan! Beta answered us!", then the same
  pair the other way round; Alpha's Deal! ("Waiting on Beta...", B to call
  it off), Beta's Deal!, the cards crossed on both and landed,
  "Traded!"; `trades.jsonl` `[20,5]` for `[411,12]`, `trade.pending` on
  both the moment it was done, then "Deal! FireBrn1 F is ours once we're
  home!" and each game settled on its way home (the Pack's 2 to 1 and 0
  to 1 on each). The board, the next visits: Beta posted MrkCan1 M from
  the list ("Post an offer", "Trade board") for a Fire chip; Alpha, with
  FireBrn2 S given, read "Beta HAS MrkCan1 M! WANTS A FIRE CHIP!",
  Answer it: the list "Answer an offer", "SPARES THAT FIT: A FIRE CHIP",
  FireBrn1 F and FireBrn2 S alone (Cannon A and MrkCan1 M left off);
  FireBrn2 S taken, Alpha's chip trade at once, Beta's after "Lan! Alpha
  took our offer!", Beta's Deal! first this time, the swap on both,
  `[21,18]` for `[411,12]`, both settled at home. The screens captured
  headless with the `trade` dev step (docs/DEVTOOLS.md) at 240x160,
  640x480 and a phone held upright, BN6's FireBrn1 F and BN5's MrkCan1 M
  on the cards, every sort and the swap's frames. And the game font's
  narrow glyphs: BN6's l, i, j, r, I, 1, !, :, ; and the brackets stand in
  their 8-pixel cells with blank columns on their left, which the
  engine's letters, drawn at their advance, read as a gap before them
  ("Team Co lone l's ROM" on the NetPass); each now starts one column in,
  as the round letters do (`gfx.c`), seen on the pass with BN6's ROM
  alone.

**NetBattles in the browser** (20 October 2026, a local v9 server,
headless Chrome, the harness in `.build/`): two browser tabs fought a
full-rules NetBattle to its end, 28 hashes agreeing, one tab rolling
back 39 times; a tab against the host build, the same; and a tab
watched two host builds' battle from the stands, catching up from its
first frame and checking all 60 of their hashes
([ONLINE_ARENA.md](ONLINE_ARENA.md#in-the-browser)). The New 3DS ran
`--dev rollbench` and a battle playing itself over 3dslink: its pair
frame is past a 60th of a second, so it offers none
([ONLINE_ARENA.md](ONLINE_ARENA.md#the-new-3ds)).

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

- **The week's net, what is left:** a dive's summary hash is a format
  check, not evidence. A dive won (the Nest) has not been played to its
  end here; the 3DS's and the browser's dives are untried.
  A diver's game calls off an offer made to it, but the other game
  hears only "The trade's off": the snapshot does not say who is diving.
- **The NetPass, what is left:** seen headless on the host and Linux
  builds only (the KeyItem's A in a run, START in the Square, the gate's
  strip, the second screen's panel, the stamp moment, the referee, the
  clerk, the greeter); on the 3DS, Android, iOS, the browser and the
  handhelds it is built, untried. A pass's stamp moment plays on the
  next visit after BN5's ROM is added, but a game that loses BN5's ROM
  and gets it back does not see it again (`netpass.ini` keeps the most
  stamps seen). Touch has no button of its own on the pass (the touch
  controls' A and B close it). A trade's settling at home and a
  WEEK_POST refused for the stamp were not played again here (the
  server's tests hold the refusal).
- **Trade Central's screens, what is left** (issue #145): seen headless
  on the host and Linux builds, keys from `tools/play.py`, and on a
  phone's screen headless the touch controls' D-pad and A picking a chip,
  A on No deal and B declining (`--taps`); on the 3DS, Android, iOS, the
  browser and the handhelds they are built, untried. The spares list has
  no touch of its own on its rows. A trade called off by
  the other player while the list is up for an answer is found only when
  the answer is sent (the server's NOTICE, and MegaMan's "The trade's
  off"). The second screen shows the NETPASS panel meanwhile, not the
  chips.
- **The Arena on the New 3DS:** measured, not offered
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#the-new-3ds)): a pair frame 25.8 ms
  with its picture, 20.7 without, two GBAs on one ARM11 core. What is
  left is the two cores on two CPU cores at once, the third core taking
  one, which needs a cable driver of our own whose scheduling is the same
  however the threads interleave (a new pair hash on every system, a
  protocol version), and even then a frame is at the edge of a tick:
  10.4 ms a core and 5 for the picture. The stands there are the report
  alone. A NetBattle in the browser was seen against another browser and
  a native game through a local server; against the deployed server and
  a phone's or a handheld's game, untried.
- **Rollback, what is left** (issue #128, ONLINE_ARENA.md, Rollback): a
  NetBattle with rollback between two devices and through the deployed
  server (protocol v9 there once it is redeployed), on Android, iOS,
  Windows and macOS (built, untried; the Nova measured headless); a first
  NetBattle on a system asks with a window of 4 until a battle has kept
  its frame's cost; the guess is the other's last keys held, nothing
  smarter.
- **The stands, what is left:** watching on Android, iOS and the
  handhelds (built, untried), a catch-up's pace in a window and on a
  handheld (measured headless only), a machine too slow to catch up (in
  the code, not seen), and the report's chips past one a hand
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#not-yet)).
- **On hardware:** the gate, a visit and a trade on the Nova, the Flip 2,
  a New 3DS, an Android device (the emulator's visit, trade and
  NetBattle above) and iOS, macOS and Windows itself (Wine's above); the
  Arena's smoothed delay and rollback over a real network (issue #128:
  measured through a local relay, not yet on the deployed server); a phone's app sent to the background
  mid-battle, which the lead's cap should now wait out (ONLINE_ARENA.md,
  Lockstep), untried.
- **In the browser with the published server:** the official server
  allows the site's origin alone, so the browser's visit was checked
  against a local one; a trade and the trade board there are untried.
- **The official server:** its policy, moderator and retention are
  published and its pruning and backups are in the server, tried locally
  (a Compose stack's archive restored byte for byte, 20 October 2026);
  on the Coolify service itself the backup service, a restore, the
  proxy's access log and the switch from `dev` to `main` at the release
  are the owner's ([server/README.md, At a release](../server/README.md#at-a-release-the-official-server-follows-main)),
  untried; a trade and a NetBattle through it.
- **The Souls under full rules, what is left:** a MegaMan worried by
  hits alone in a NetBattle, ToadSoul's flag and FloatShoe on panels a
  NetBattle's field lacks, and BN5's own NetBattle as a reference for
  MrkCan1 and for a DarkChip's after-effect
  ([ONLINE_ARENA.md](ONLINE_ARENA.md#not-yet)).
- **A pending trade across systems:** `trade.pending` stays beside the
  saves, outside the `.cwsave` carrier; a save moved between a trade and
  its settling leaves the swap on the system the trade was made on.
