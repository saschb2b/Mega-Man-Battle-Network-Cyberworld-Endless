---
type: Feature
title: "The NetPass"
description: "The NetPass (issue #131, `src/online/netpass.c`): the full set of ROMs, BN6's, BN5's and BN4's (protocol v17), that opens the Arena, trades and the DeepNet dive, its four stamps, its screen, the stamp moment, a game joining the set, the full pass of a system with room for one game (protocol v18: it trades on the pair's common ground) and protocol v9's NOTICE 28."
tags: [online]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-12T20:30:00Z }
---

# The NetPass

[Issue #131](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/131),
the owner's decision of 20 October 2026: **the full set for what's at
stake**. The Net Square is open to every game: walking, phrases and
emotes, cheering, another MegaMan's card, the referee's report of the
channel's battles, the Board and its posts, the trade board's offers to
read and the week's ranking. Three things need **the full set**, every
ROM this build supports for online play: the Arena (a NetBattle, and
watching one), Trade Central (a trade, and posting or answering a trade
board offer) and the week's DeepNet dive (the greeter's offer and the
dive's post to the Board). The set is BN6's ROM, BN5 Team Colonel's and,
from protocol v17, BN4 Red Sun's, defined once in a table
(`src/online/netpass.c`). A future game is one entry there: the stamps
that need it go back to open until a player has it in, which the release
notes and the site announce ahead ([A game joins the set](#a-game-joins-the-set),
below, as BN4 did). A system that cannot read the whole set at once (the
New 3DS, one game's ROM beside BN6's) holds a full pass with the games it
can read, and that pass trades
([A system with room for one game](#a-system-with-room-for-one-game)).

The **NetPass** shows it, and is the one place that names which game
earns which stamp: a key item in Lan's PET (R on the PET's menu in the Square), a strip
at the gate and a panel on the second screen. Characters speak only of
stamps, never of ROMs.

## Why

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

## The stamps

| Stamp | On the pass | Opens | Needs |
| --- | --- | --- | --- |
| Square | meet,cheer | Walking, phrases and emotes, cheering, cards, the referee's report, the Board and its posts, the trade board's offers to read, the week's ranking | BN6's ROM, every game's |
| Arena | NetBattle | Asking for and taking a Practice NetBattle, a rematch, watching one in the stands | The full set |
| Trade | chip swap | A trade at Trade Central, posting or answering a trade board offer | A full pass: the full set, or one game beside BN6's on a system that reads one extra ROM at most |
| DeepNet | the Board | The week's DeepNet dive: the greeter's offer, the dive's post to the Board | The full set |

`netpass_has` answers for a stamp, `netpass_full` for the pass
(`netpass.h`). A pass holds 1 of 4 stamps or 4 of 4 (2 of 4, the Square's
and Trade's, where a system reads one extra ROM at most): the three that need
the set come together.

## The pass

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
  go ([The NetPass](../game/pet.md#the-netpass)). **In the Square** BN6's PET
  opens as everywhere ([The PET in the Square](square/pet.md)), so the
  key item is there too, and **R on the PET's menu** opens the same
  screen (`netpass_screen.c`), named at its band's end, `R:NETPASS`.
  START opened it there until 11 November 2026, while the PET stayed
  shut in the Square.
- **At the gate** in Lan's HP: while MegaMan asks "To the Net
  Square,Lan?", a strip over the top of the screen shows the pass's level
  and its four stamps, so the player sees what the visit opens before
  saying Yes ([11. The Net Square's gate](../world/home/square-gate.md)).
- **On the second screen** (the 3DS's bottom screen, Android's second
  display) while MegaMan is in the Square: a NETPASS panel with the pass
  drawn on it.

## What characters say

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

## The stamp moment

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

## A game joins the set

The owner's call of 9 November 2026 ([BN4's chips in a run](../multirom/bn4-chips.md),
the owner's call 1): **BN4 Red Sun joins the full set now**, over the
recommendation to wait for its share of sessions. Its chips go into
NetBattles and trades as each is proved, and a game without BN4's ROM
keeps the Square and loses the Arena, Trade and DeepNet stamps until it
adds it. Through the game-design skill:

- **Dialectic and loop layer** as the pass's own: one crowd against each
  player's collection, read at the gate as a visit begins.
- **Patterns:** honest feedforward (the pass shows a third slot, open and
  marching, naming the game: "Red Sun's ROM stamps Arena, Trade and
  DeepNet"); a ceremony per completion (the stamp moment comes again when
  the larger set is filled); and an announcement said once, only to the
  player it changes things for.
- **Who says it, and when:** the greeter, who stamps the pass, on the
  first visit after the set grew, to a game whose pass was full and is not
  now; then the pass opens as it stands, its new slot naming the game. Its
  words speak of stamps, never of ROMs, as every character's do: "OH! YOUR
  NETPASS GOT A NEW SLOT!", "ARENA,TRADE AND DEEPNET STAY SHUT TILL IT'S
  FILLED!", "HAVE A LOOK!" (`square_greeter_grew_words`). A game that
  never held the full pass hears nothing new: its pass was saying what it
  lacks already. A game with all three ROMs never hears it either: the
  stamp moment comes in its place, its stamps landing again.
- **What it does not do:** take a stamp from anyone without saying why,
  or say it twice.

`netpass.ini` keeps the set's size beside the stamps (`games=3`); a file
from before v17 has none and counts as the two-game set's. The greeter's
line is due where the file says every stamp for a smaller set and the pass
is not full (`netpass_grew_due`); said, the file says the Square's stamp
alone, so it is said once and the stamp moment comes when the pass fills
(`netpass_stamp_due`: full, and fewer stamps seen or for a smaller set).
Ahead of the release, the notes and the site say it too: the CHANGELOG's
entry for v17, and the site's line on the NetPass.

## Every system, and the New 3DS

The owner's aim (20 October 2026): every stamp a system can use, on every
system. The browser now battles as the desktops and phones do, its pass
and its stamps theirs ([In the browser](arena/rollback/browser.md)). The
New 3DS builds the Arena but offers no NetBattle and no watching
(`CW_ARENA`, `square_arena_offered`): a frame of the battle's two GBAs
takes 25.8 ms there against a 60th of a second's 16.7, measured on the
device ([The New 3DS](arena/rollback/new-3ds.md)). And it reads one extra
ROM beside BN6's at most (`XROMS_MOST`, `src/core/rom.h`, which
`start_3ds.c` reads its ROMs by: both together outgrow its heap,
[The 3DS's memory](../engine/emulation/3ds-memory.md)), where the full
set asks two since protocol v17. For a while on `dev` its pass could
never be full, and it had lost the trades and the dives 0.20.0 gave it
with BN5's ROM; the owner's word of 12 November 2026 was to fix that in
code, which the next section is. The second screen's panel draws the
pass's words in the engine's small font, its pixel letters being a
texture's, which a picture drawn into memory (the 3DS's bottom screen)
cannot use.

## A system with room for one game

**A pass is full when every game of the set that the system can read is
in** (protocol v18). On every system with room for all of them that is
the full set as before, BN6's, BN5's and BN4's ROMs. On a system that
reads one extra ROM at most (`netpass_one_extra`: today the New 3DS
alone) it is BN6's and one of BN5 Team Colonel's or BN4 Red Sun's. What
that pass opens goes stamp by stamp, by what the two games of a pair
must share:

| Stamp | On a system of one extra ROM, one game in | Why |
| --- | --- | --- |
| Square | Held | Every game's |
| Trade | Held, on the pair's common ground | A trade is one chip for one chip, and a chip is a number both games can name or not |
| Arena | Not held | A NetBattle's two games build the same installs, BN4's among them, which the installs' record's hash checks; and the New 3DS offers none in any case |
| DeepNet | Not held | A dive without BN4's ROM is not the dive the Board's others dived (below) |

**The common ground.** Before v18 both games of a trade held the same
set; now a full set may trade with a New 3DS that has BN5's ROM and not
BN4's. So a trade keeps to what both hold, the rule the pass replaced
for the *whole* of online play in v9 and which comes back here for one
chip at a time, where it costs the player no rule to learn: the spares
list for a Navi shows only the chips his game holds too
(`netpass_peer_holds`, by his ROMs as the server says them), the clerk
says "HMM... Alpha'S PET CAN'T HOLD ANY SPARE OF YOURS!" where there is
none, and an offer of BN4's on the board is left out of the BOARD of a
game without BN4's ROM, as ever. The server holds the same line
(`protocol.holds`): a proposal, an answer, a take or a post naming a
chip of a game either side lacks is refused with NOTICE 19
(`notice_ground`), an answer's ending the trade for both. Whose game a
chip is goes by the wire's numbers on both sides: BN4's own from
`bn4_chip_first` (527), BN5's past `bn6_chip_last` and on BN6's blank
records, the rest BN6's and every game's.

**Why the week's dive stays shut.** The dive is the same net only for
games that read the same ROMs. What a seed makes goes by them:
`run_dress` (`src/core/run.c`) dresses an act's area in one of BN5's or
BN4's by a coin of the seed *where that game's areas exist*
(`net_area_def`, `src/core/rom.c`, answers none for a ROM not read), and
BN4's areas lay out otherwise (their pads and platforms); the surface
acts' guardians (`dressed_xguardian`) and the deep's (`deep_xguardian`)
draw on BN4's Navis; the encounter groups' pool
(`src/core/group_pool.c`) counts BN4's families in; and BN4's chips are
among the finds. A dive is made with the older nets on (`weekly_dive`:
`run_new` on the week's seed, the standard setup, no extra off). So a
New 3DS with BN5's ROM alone would dive another net than the Board's
other entries on the same seed, and its depth would not compare. The
DeepNet stamp stays shut there and the server refuses its WEEK_POST as
before (NOTICE 28); no second Board, and the dive draws on what it drew
on. This is the one thing 0.20.0 gave a New 3DS that 0.21.0 does not
give back, and it is the owner's to weigh: it opens again the day a New
3DS reads both ROMs, or the day a dive is made the same whatever ROMs
are read.

**The pass on that system.** Its header says "Trade pass" with one
game in (`netpass_level_trade`: "★ Full pass" is every stamp's, which
this pass is not, though the rule calls it full), its stamp bar "2 of
4", the Square's and Trade's stamps held and the Arena's and DeepNet's
open. The tray shows the slots the system
has room for: BN6's and the game that is in, or, with none in, both
games' open (either fits) and "Team Colonel's or Red Sun's ROM stamps
Trade"; the second game's slot is not drawn once one is in. Its line
then: "Every game this PET holds is in! It trades. No NetBattles or
DeepNet here." (`netpass_complete_one_extra`). The stamp moment lands
the Trade stamp alone, the greeter's word for it "WHOA! YOUR NETPASS
HAS ROOM FOR THE TRADE STAMP NOW!" and "HOLD STILL! STAMP!". A New 3DS
whose pass was full in 0.20.0 (`netpass.ini`: every stamp, for the
two-game set) hears him once, as every game whose stamps the set's
growth took does: "OH! THE ARENA AND DEEPNET STAMPS NEED MORE NOW!",
"MORE THAN THIS PET HAS ROOM FOR! SORRY!", "YOUR TRADE STAMP STAYS! HAVE
A LOOK!" (`square_greeter_grew_words`), then the pass as it stands; the
file then says its two stamps. He says it only where it is true: on
such a system with no game in, `netpass_grew_due` waits until one is
(the other games' line, of a slot to fill for three stamps, would
promise what that system cannot hold). The
referee there says "SORRY! NO NETBATTLES FROM THIS PET!" whatever the
pass holds. To a full set, the card of such a Navi says "His NetPass has
no Arena stamp." (Battle is not among its options), and Trade Central
lists him.

**On the wire** a system says it in HELLO (`hello_one_extra`, a bit of
the channel word beside the resume's), the server keeps it with the
session's ROMs and tells the others in SNAPSHOT (`roms_one_extra`, bit
6). `--dev xroms=1` makes any build act as such a system (the tests and
the captures; it reads its ROMs as ever).

## On the wire

Protocol v18 adds the system of one extra ROM
([above](#a-system-with-room-for-one-game)): `hello_one_extra` (16384)
in HELLO's channel word, `roms_one_extra` (64, bit 6 of SNAPSHOT's byte)
in a session's ROMs, `protocol.pass_full` for Trade Central where
`full_set` stays the Arena's, the stands' and the week's, and NOTICE 19
(`notice_ground`) for a chip outside a trade's common ground. A v17 game
refuses a SNAPSHOT that carries bit 6 ("no other bit") and a v17 server
a HELLO whose channel word carries the new one (a channel it has not),
so the version moved: a game and a server of v17 and v18 tell each
other which side is behind (ERROR 2 or 17), as any two versions do.

Protocol v17 adds BN4 Red Sun: HELLO carries its game code and SHA-1
after BN5's (all zeros without it, ERROR 15 for another), the server
keeps it as `roms_bn4` (bit 5 of SNAPSHOT's byte, clear of its busy and
link bits) and `roms_full` is both bits. A chip of BN4's own (from
`bn4_chip_first`, 527) is left out of the BOARD of a game without BN4's
ROM, as BN5's is of a game without BN5's.

Which chips cross ([BN4's chips in a run](../multirom/bn4-chips.md),
Online): a NetBattle's folder may hold any chip the game has a record of,
BN6's own and the registry's, to id 1023 (`online_chips.h`,
`arena_loadout_check` on both games); a chip of BN4's whose code is not
moved yet sits out of the deck on both cores, as in a run. A trade, its
answer, a board offer and the spares list take only a chip that plays
(`online_chip_plays`): a pending one never shows. Two games of the set
build the same installs, BN4's among them, which the installs' record's
hash checks before a battle begins.

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
