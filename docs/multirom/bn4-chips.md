---
type: Design
title: "BN4's chips in a run"
description: "Phase 4 of BN4 Red Sun in a run, designed and scoped, its pairings (unit 0), its chips in the finds (unit 7), BN5's first batch and ten-bit chip ids (unit 1) built, an id past 511 whole to 602, every BN4 folder chip given a pack entry (ids 635-1023 in the cart's save RAM) and BN4 in the NetPass's full set (unit 8, protocol v17): the census of BN4's 388 chip records against BN6's and BN5's registry (191 new to a run, 6 on BN6's code, the rest in 66 units of BN4's code), chip ids past 511 by ten bits for BN4's chips and BN5's alike (BN5's ShadeMan and BlizzardMan chips at 505-510, 511 held), the design (BN4's own chips found on BN4's ground, by its own records), the online rule, the build plan in units with their proofs, and the owner's calls."
tags: [multirom, bn4, chips, design]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-12T23:55:00Z }
sources:
  - id: bn4-rom
    resource: "Mega Man Battle Network 4: Red Sun (USA), B4WE, SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1"
    title: "The owner's BN4 Red Sun ROM, read as docs/rom/bn4.md says"
  - id: bn6-rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6"
    title: "BN6 Gregar's chip records, attack families and code, the pairs' index of its routine bodies"
  - id: registry
    resource: "src/core/xchips_registry.c at dev 3a4309e1"
    title: "BN5's chips as the registry holds them"
---

# BN4's chips in a run

Phase 4 of [Battle Network 4 in a run](bn4.md): BN4 Red Sun's own chips
in BN6's tables, designed and scoped so building can start. Built so far:
the pairings (unit 0) and BN5's first batch in the free ids (Past 511). The census is read from both ROMs (scratch reads, not kept;
How the census was made, below), the design reasoned with the
game-design skill and checked against [the core loop](../game/core-loop.md),
the scoping by the reverse-engineering skill's Locate and Understand.
What BN5's chips took is the road: [the chip registry](chip-registry.md)
and [the chips' code](chips-code.md).

## Where things stand

**Unit 7, into runs, built: BN4's chips are in the finds** (12 November
2026; The build plan). On BN4's areas the older Net's half of the finds
and the stock is BN4's catalogue, and in the deep it is every game's
read. The catalogue is made as BN5's is ([One net](one-net.md), the older
net's half): the run's chip of each of BN4's (`xchips_bn4_paid`: BN6's of
its name, the registry's of BN5's where that plays, its own row's where
its code plays, BN4's ROM read and the pack has its entry), no DarkChip,
no dark chip of its own (`xreg4_dark`) and no Program Advance;
`src/core/chip_pool.c`'s `CHIP_NET_BN4`, and `CHIP_NET_OLDER` the whole,
BN5's and then BN4's others, a chip once. `loot_layer_nets` names the
layer's catalogue (`loot_older_net`) by the area's game, the deep's the
whole, and every find goes through it: the Net Dealers' stock, Mystery
Data, the vaults', Servers' and gift's picks (`loot_pool_pick`), a held
Soul's kind (`loot_souls.c`), a Chip or BugFrag Trader's list, half each
rarity's (`trader.c`). Nothing names a chip: a unit that comes to play
joins the catalogue the next time the game starts (Boomer1-3 did, merged
while this was built). With all three ROMs the catalogue holds 130 chips,
15 of them BN4's own; with BN6 and BN4, 93 and the same 15.

Two decisions the design left open, taken with the game-design skill.
**The catalogue holds the chips BN6 names too**, as BN5's does: fifteen
chips of BN4's own taking half of an area's finds would be the same
dozen in every dealer's list and every Mystery Data (each some ten times
as likely as a chip of BN6's), where the pattern asked for is identity by
net, not a short list; BN4's own come more often as they are proved.
**The deep draws on the whole, one chip one weight**, not a coin between
the games: one older Net (call 5 of [Battle Network 4 in a
run](bn4.md)), no roll added, and nothing changes there while BN4's ROM
is not read.

**A run without BN4's ROM is what it was, roll for roll.** BN5's catalogue
keeps its chips and their order, the deep's whole is BN5's alone without
BN4, and no pick draws another number. Proved by the autopilot's seeds
1-12 over 15,000 frames with BN6 alone and with BN6 and BN5, this build
against `dev`'s: 24 run logs the same, and their debug logs (every
dealer's stock, every layer's lines) line for line. With BN4 read the same
seeds ran with BN6 and BN4 and with all three, and again on BN4's five
areas (`--net-biome x15` to `x19`, 30,000 frames), each run ended by its
act's guardian as the autopilot's runs end, none by a fault (152
battles begun, 141 won); on BN4's ground dealers stocked Invis,
PanlSht1, Counter1 and 3, WhitWeb1 and 2, Boomer1 and 3, Mystery Data
held Repair, PanlSht2, WhitWeb1, Boomer1 and 3, a gift Counter1 to 3.
`LAYER_MAKE` 115: with BN4's ROM read a layer's stock rolls otherwise on
BN4's areas and in the deep.

**What it measured** (`build.py economy`, its new table The older Net's
half; [Measuring it](../game/economy/measuring.md)). The tier is rolled
before the catalogue is asked, so the chips an act gives, their good
share, the Mystery Data and the zenny cannot move, and did not: with all
three ROMs the report is `dev`'s number for number to its new table (a
held Soul's kind counts BN4's chips of TomahawkMan's kind beside BN5's).
A run as its seed dresses it holds 0 numbers outside the plan's bands,
with all three ROMs and with BN6 and BN4, and finds 0.3 and 0.9 of BN4's
own chips. One number does move on BN4's ground, the codes held: BN4's
chips carry \* more often (38 of its 100 standard chips; 13 of its 15
that play), so a run with BN6 and BN4 holds one chip more in its best
letter from the third act on (9, 12, 16, 19 against 8, 11, 15, 18),
inside the band, and a run made wholly on BN4's ground, which no seed
makes, three more by the fourth act (13, 17, 22 at the Nest), outside it.
Left as BN4's records have it, the design's rule, and the report counts
the seed's run from now on, so the day BN4's \*s push a real run out it
says so (the owner's call 9, below: kept).

**The Library keeps a BN4 chip held.** A chip of BN4's own given to
MegaMan never reached the profile's Library (`chip_pool_older` knew
BN5's rows alone): its page counted the chips both games name and no
more. Seen: PanlSht1 and WhitWeb1 given, BN4's page 9/129 to 11/129,
`dev`'s build 9/126 before and after. The second screen's Library panel
counts the same page.

**Found on the way, and fixed: BN6's menus on BN4's areas.** On a layer
in BN4's tiles the PET's folder screens, a dealer's shop and the Chip
Trader drew as garbled tiles, in 0.20.0 too. BN4's net maps are
256-colour, and `netmap_colours` set that bit each frame wherever BG1
and BG2 held the net's controls, which those menus set too (the folder's
as it opens) or leave as the map had them (a shop, the trader). It now
sets the bit only in the game's own main mode and takes it off in the
menus' ([Net maps](../rom/net-maps.md), The map's BG controls). Seen on
seed 5 with BN6 and BN4 (Town Area's first layer): the folder, FOLDER
EDIT and back to the map; a dealer's shop on Town Area and the countries;
the Chip Trader on ACDC Area, which gave "WhitWeb2 Y" from a list of
BN4's chips alone (a scratch build), an id past 511 through BN6's own
trader.

**Not built here:** the flame's five DarkChips (none plays); a round's
and Guardian Data's Navi chips (unit 6's); the visitors' rewards, which
pay a chip of BN4's that plays since unit 0. A chip of BN4's whose id has
no pack entry (603-634, the Program Advances') stays out of the
catalogue, and a visitor's reward of one pays the stand-in's zenny
(`reward_as` asks `bn6_pack_holds`, since the pack's entries from 635:
the next paragraph).

**Unit 8, online, built: BN4 Red Sun is in the NetPass's full set**
(protocol v17; [The NetPass](../online/netpass.md), A game joins the
set), the owner's call 1. HELLO carries BN4's identity, the server
keeps its bit (`roms_bn4`) and `roms_full` takes it, so a game without
BN4's ROM keeps the Square and loses the Arena, Trade and DeepNet stamps
until it adds it; the greeter says so once to a pass that was full
before, and the stamp moment plays again when the larger set fills. A
NetBattle's folder may hold any chip the game has a record of to id
1023 (`online_chips.h`, both games' `arena_loadout_check`), where it
refused every id past 511 before: a chip of BN4's whose code is not
moved yet sits out of the deck on both cores, as in a run. A trade, an
answer, a board offer and the spares list take only a chip that plays
(`online_chip_plays`), so a pending one never shows. The board leaves
BN4's own chips (from `bn4_chip_first`, 527) out of a game without
BN4's ROM, as BN5's are left out of one without BN5's.

**Every BN4 folder chip has a pack entry** (10 November 2026; [The chip
registry and BN5's chips' code](../rom/chips-code.md), The pack in the
save RAM). Ids 635-1023 count in the cart's save RAM, 5,616 bytes of
which BN6's save library never addresses (its four calls' addresses and
sizes are literals, read; the battery, the PET's screens, a shop, a Chip
Trader and a NetBattle rolled back 872 times watched it for every read
and write: the pack's routines alone touch it). Every state the engine
writes carries it, so a checkpoint, a `.cwsave` and the boot's state
keep the counts with no change to `Run`; a rollback snapshot takes it
beside the core's raw state. mGBA serves that RAM a byte at a time, so
BN6's three routines that reach an order stamp by a halfword go on to
the engine's, which use two bytes. Chip 700 given, listed in the folder
editor's Pack, swapped, through a battle, saved and continued keeps its
counts; a run saved by the build before continues with none of them;
the autopilot's seeds 1-12 are the build before's run log for run log.

**The first unit of BN4's code is moved: PanlSht1-3** (10 November 2026;
[The chip recipe generator](../devtools/chip-recipe-generator.md), The
BN4 path). `tools/xchiprecipe_bn4.py` derives a unit from both ROMs: one
routine of BN4's support state's table (`BN4_CHIP_ROUTINES`, support and
Navi chips alike), which is a spawner of a fourth-kind object in BN6's
support routines' own shape, with that object's type and everything it
reaches. PanlSht's (support 0x26) is the fourth kind's 0x4F and its
shot, the third kind's 0x7B, 1008 bytes in two blocks: 28 calls paired
with BN6's twins, the five generic states of a support chip's object
BN6's own at the same places of its types' tables, the element's place
and a panel's side at BN6's offsets, its sprite copied (list 3), no
decision open but its sounds (below). It
installs where BN4's ROM is read (`src/emu/xchips_code.c`, its room
`FREE_BN4_CHIPS`) at BN6's grown support routines' 0x80 + BN4's
subfamily, 0xA6, which the three rows' records take under BN6's support
family: `XREG_MOVED`, ids 559-561. Against BN4's own battle each passes
over 300 frames, MegaMan the subject, a BN4 MettaurSP of 140 HP the foe:
`bn4_chip_panlsht1` (the panel ahead thrown: 40 HP left),
`bn4_chip_panlsht1_miss` (the foe a row up: 140), `bn4_chip_panlsht2`
(the row's two panels: 20) and `bn4_chip_panlsht3` (the four around
MegaMan, one down the foe's row: 60), the broken panels and the sounds
the same; ten mutations caught. The chip's whole flight runs inside its
time freeze, between two fight frames, so the traces meet at what it
leaves. A shot that deletes its foe parts at the deletion's own effect
(effect 0, each engine's art: the families' known difference), so the
specs' foe survives. What the comparator took: a moved row's subfamily
read as its unit's number, BN6's second wheel's flag in the attack's
element (PanlSht's kind is the Cursor's, as BN5's chips' icons give
theirs), the object's chip and spawn leftovers by its type, the two
copied sounds (`tests/xdiff/legit.json`'s `bn4` section).

**Its sounds, and every BN4 chip's after it, go into BN6's song table
grown** (the owner's call 6, 11 November 2026). PanlSht plays two sounds
BN6 has no song of (0x1C6 as a shot is made, 0xBF as it flies), and
nearly every unit to come has its own (ShotStar's star 0x146, FirePlus's
three): some sixty over the 48 units, where BN6 had eight empty song
slots left, kept for BN5's. So the table is copied into the free space
with 550 more slots and the sound engine's five words that name it are
pointed at the copy (`src/emu/song_table.c`, where BN4's ROM is read;
[Sound](../rom/sound.md), The song table grown). A unit's sound takes
the next slot from 0x1DA (`tools/xchiprecipe_bn4.py` `SOUND_SLOTS`,
`src/audio/xsong.c`'s the same): PanlSht's are 0x1DA and 0x1DB, each
call through a shim that names the slot, a number past a byte.

**ShotStar is moved too** (11 November 2026: support 0x19, id 570, one
chip, a Mega). Its unit is three types and a table, 1172 bytes: its
object (the fourth kind's 0x34), the star (the third kind's 0x56, moved:
its spawner is BN6's by body but the types differ below it, the
generator's guard), the star's kinds' table and its mark (the fourth
kind's 0x35). The star's sprite is no sprite of BN6's and is copied to a
list 3 number of the chips' (0x70), its burst's effect to an id of the
chips' (0xCF); its fall's sound takes the grown table's 0x1DC and its
burst's the slot a BN4 family already has for it. `bn4_chip_shotstar`
passes over 400 frames: thirty stars inside the freeze, three on a 140 HP
Mettaur, 20 left, six mutations caught. The trace keeps 128 songs begun
between two fight frames since (a freeze's sixty-three ran past its 32).

**Repair is moved, the first misc unit** (12 November 2026: id 557).
BN4's misc chips run by MegaMan's state 0x20, which BN6's family 0x1C
is instruction for instruction: a routine by subfamily, the attack ended
in the same frame. BN6's table of them (7) is grown for BN4's units
(`XC4_MISC`), and the generator has a second kind of unit for a routine
there, whatever its shape. Repair's is 60 bytes: each of its side's
obstacles with HP left is given its full HP, an effect over it and a
sound. One number is BN6's: the battle state keeps the side's obstacles
at +0xA0 where BN4 keeps them at +0x68 (the two games' routine that
enters one differs in that alone). Its proof needs an obstacle, so
RockCube, a chip both games name, became a control first
(`bn4_chip_rockcube`: the cube is each engine's own object, nearly the
same code; BN6's holds a name and reads an 8-byte row). `bn4_chip_repair`
passes over 300 frames: the cube set, struck by the Mettaur's wave to
190 HP, repaired to 200 in both, struck again; six mutations caught. A
spec with two chips in one hand gives BN6 its deck (`deck6`) and picks
BN4's in the order its own shuffle dealt them.

**Counter1-3 are moved** (12 November 2026: ids 567-569, misc 0). The
routine lies in the code of the hit it spawns (the third kind's 0x17),
which moves with it, 276 bytes: it walks the other side's players and
strikes each whose counter byte is set, so it only lands while a foe
readies its attack. Three numbers are BN6's (the players' place in the
battle state, twice, and the counter byte's in the collision data), its
hit's sprite is copied (list 5) and its sound takes the grown table's
0x1DD. `bn4_chip_counter1`, `2` and `3` pass to the frame of the hit (a
140 HP Mettaur in its window left 70, 30 and 0 in both) and
`bn4_chip_counter1_idle` outside the window (untouched), six mutations
caught. The comparison took MegaMan's lock-out after a chip (BN4 keeps
it at another place of his AI data) and a chip unit's objects ranked
with the natives for the renderer's places. **Not compared:** what a
counter starts after the hit, each engine's own Full Synchro object; the
specs end at the hit.

**WhitWeb1-3 are moved** (12 November 2026: ids 574-576, misc 8, 9 and
0xA), the first of the units of 300 to 600 bytes. Their three routines
are four instructions each, one after another in the web's code (the
third kind's 0x3D), each handing a row to one common routine: one unit
moves the block (528 bytes) and the second and third chips' rows of the
units' table share its code. The web's sprite and effect are copied, its
sound takes the grown table's 0x1DE. `bn4_chip_whitweb1`, `2` and `3`
pass over 300 frames, the webs in play in the fight's frames throughout
(a 140 HP Mettaur on each row left 100), five mutations caught.

**Boomer1-3 are moved** (12 November 2026: ids 584-586, misc 2): the
boomerang (the third kind's 0x32) with the table of its flights, 636
bytes, its sound at the grown table's 0x1DF. `bn4_chip_boomer1`, `2` and
`3` pass over 300 frames, the boomerang in play from t33 to t96 (a 140
HP Mettaur in its way left 80, 60 and 40), five mutations caught. With
it every sprite a unit draws is BN4's copied, as the families' are,
whether or not BN6 has one of its bytes: an object in play in a fight's
frame is compared by its sprite's frames, which the differential names
by the recipe's rows.

**MokoRus1-3 are moved** (10 November 2026: ids 577-579, misc 1): the
Moko (the third kind's 0x2C), 524 bytes, three of them on the rows the
chip's shuffle draws, set off 10 frames apart; its sprite is the first
of list 1, the viruses' list, the chips copy (their range there 0x7F
down to 0x40), its sound at the grown table's 0x1E0. Its hit strikes
and pushes back, which the two engines ask for by different bits of the
hit (BN4's 0x01 with a push bit, BN6's 0x40 with one): the copy passes
BN6's, 0x61 for BN4's 0x21. `bn4_chip_mokorus1`, `2` and `3` pass over
300 frames, the Mokos in play from t33 to t147 (a 140 HP Mettaur in the
middle row left 80, 50 and 20, struck out of its attack and pushed back
a column in both), five mutations caught.

**The first Navi chip plays: GutsMan** (12 November 2026: id 535, unit
6's first; [The Navi chips](#the-navi-chips), below). BN4 runs a Navi
chip from its support state: its routine spawns an object of the Navi's
own (GutsMan's is the fourth kind's 0x10), which steps MegaMan out,
spawns the Navi as his chip shows him, waits for his word and steps
MegaMan in. That Navi is a first-kind type of his own, not the type he
fights as: GutsMan's is 0x14, 424 bytes. He appears, strikes the ground
at frame 20 of 90 and is gone; his shockwave is the Navi's own object
(the third kind's 0x0B), which the Navi's recipe moved already. BN6 has
BN4's object once for every Navi chip, the same four states and waits,
and steps MegaMan out by an object of its own that knows his Crosses. So
the unit is the chip's GutsMan alone (`tools/xchiprecipe_bn4.py`, a unit
of kind `navi`): his spawner at BN6's grown Navi routines' 0x85, run by
BN6's Navi chip object under BN6's Navi family, as BN5's Navi chips are,
his calls into the Navi's blocks aimed at the Navi's copy. His row is
`XREG_MOVED` with BN6's Roll as the twin whose family it takes.
`bn4_chip_gutsman` passes over 300 frames against BN4's own battle: a
140 HP Mettaur left 60 in both, the same three panels cracked (the
shockwave's draws replayed), the same sounds; six mutations caught. Seen
in play against BN6's Mettaur and against BN4's ThunderMan, both Navis
drawn at once. His SP (536) waits for BN4's computed powers and his DS
(537) for its power by the Holes on the field (below): both rows stay
`XREG_PENDING`.

**BN4's ground kind is no Cursor** (12 November 2026, read from the
three ROMs' records): BN4 numbers its chips' kinds as BN5 numbers its
icon elements but for 8, BN4's ground chips' (CrakOut, DublCrak,
TripCrak, Magnum, Geddon1 and 2, PanlSht1-3 and GutsMan's three: the
chips its GutsSoul takes), where BN5's 8 is the Cursor's. BN5 holds Null
for five of the six of those names it shares and has no such icon, as
BN6 has none. A BN4 record's kind 8 is BN6's Null since
(`xrec_from_bn4`); until then PanlSht1-3 carried the Cursor's icon and
BN6's second wheel's Cursor flag in their attack. Their specs pass as
before.

**SidBmbo1-3 are moved** (11 November 2026: ids 571-573, misc 5): the
bamboo (the third kind's 0x44), 420 bytes, that comes up three columns
ahead; its sprite of list 1 copied, its sound at the grown table's
0x1E1. It is the one misc chip BN4's run waits for, 0x28 frames after
its routine before the attack ends, where BN6's run ends a grown
routine's attack in the same frame: the unit carries the wait
(`XChipUnit.wait`, read from BN4's run by the generator) and its entry
in BN6's grown misc routines is a wrapper the installer writes at the
end of its home, which calls the routine once and counts as BN4's run
does. `bn4_chip_sidbmbo1`, `2` and `3` pass over 300 frames, the bamboo
in play from t33 to t95, MegaMan held in his misc state the same frames
in both (a 140 HP Mettaur left 60 and 30; SidBmbo3's 140 against the
family's 250 HP rank, left 110, since a deletion's effect is each
engine's own art), six mutations caught. With it a unit names only the
effects it moves itself: one an earlier unit moved is that unit's row,
which the installer could otherwise resolve only where a family
happened to hold the same sprite.

**The spread chips are moved, the first attack family** (10 November
2026: HeatShot, Heat-V, HeatSide, Bubbler, Bub-V and BublSide, ids
588-593, BN4's family 0x1E). Most of BN4's chips are neither support nor
misc chips: their record's family is a state of MegaMan's of its own, and
the generator has a third kind of unit for one, `attack`: the state
whole, every subfamily of it, at a number of BN6's grown attack families
the chips of BN4 have for their own (0xD0 on, in the units' order), which
its chips' records take as their family with BN4's own subfamily. BN6
still has the spread family as its Spreadr1-3's (0x25, the same phases,
waits and shapes by subfamily and kind), but with its own arm, a
first-kind object where BN4's is a third-kind one, so BN4's records on
BN6's family part from BN4's battle as the arm appears (tried first: t33,
where Cannon's recorded verdict parts). So the state moves with its arm
(the third kind's 0x02) and its shot (the third kind's 0x12, unseen, a
panel a frame), 1218 bytes with the arm's and the shot's kinds' tables:
38 calls paired, four of them new pairs read in both games (the counter
window, the attack's end, a NaviStats byte, the element's set), the
attack's variables' place and MegaMan's animation's number BN6's, three
arm sprites and the bubbles' and fire's burst effects copied, the shot's
sound at the grown table's 0x200 (the attack units' slots are a block
from there, as units are moved side by side) and the bursts' at the
families' slots for them. Eight specs pass over 300 frames, the arm and
the shot in play in the fight's frames: each chip against a 140 HP
Mettaur (left 100, 70, 40, 90, 60 and 30, the burst's effects on the
panels of its shape), `bn4_chip_heatshot_miss` (the shot flown off the
field) and `bn4_chip_heatshot_struck` (MegaMan struck by the Mettaur's
wave with his arm out: the arm let go in both); eleven mutations caught.
The comparison took a moved family's number, MegaMan's animation by each
art's number, the chips' code room for a pointer a moved object keeps
into its copied table, and one difference of the engines': BN4's struck
state leaves the flag an interrupted attack had set, BN6's clears it
(`tests/xdiff/legit.json`'s `bn4` `attack_families`,
`megaman_animations`, `interrupted_attack`). **Not compared:** the
burst's damage to a second foe (a BN4 spec holds one foe: the burst is
seen by its effects' panels), and a spread chip after a support chip in
one battle (BN6's support run holds MegaMan a frame, and his arm with
him: a RockCube ahead of the shot parted there). **Waiting:** Spreader
(18) is the run's by BN6's own and stays so; DrkSpred (192) is the dark
chips' session's, its kind 0xF of the shot's table and its sound a byte
cannot name among its work; HeatSprd and BubSprd, the family's Program
Advances, play once BN4's recipes are listed (BN4's list read into BN6's
grown one, as BN5's were: not built). Every attack unit to come that
draws the arm shares this copy of it (not built: the second unit's work).

**FlmLine1-3, Thunder1-3 and TwnFng1-3 are moved** (10 November 2026:
ids 595-597, 647-649 and 656-658; BN4's families 0x22, 0x26 and 0x34 at
BN6's grown attack families 0xD1, 0xD6 and 0xD3). Each is its state with
the object it spawns: FlmLine's flame (the third kind's 0x19, three on
the column two ahead), TwnFng's fang (the third kind's 0x51, one to the
row above and one to the row below, then along them), and for Thunder
BN4's own thunder (the third kind's 0x2A): Billy's recipe runs BN6's
object for it, drawn from BN6's art, which a family's rule allows by its
foe, so the chip's unit moves BN4's and its ball is BN4's in every
battle. **The arm is shared:** the Spreads unit's copy of the third
kind's 0x02 serves FlmLine too, which copies the arm's spawner alone (40
bytes) and spawns the owner's number of the dispatch; the kinds every
unit draws are rows of the owner's copy of the table. BN6 has none of
the three states, so MegaMan's animations are given BN6's numbers by the
art: the animations of the same frames' durations in each game's sprite
of him. Eleven specs pass, each object in play in the fight's frames (a
140 HP Mettaur left 70, 20 and 0 by FlmLine1-3, the third compared to
the hit; 100, 80 and 60 by Thunder1-3, the ball creeping 90 frames while
MegaMan is struck; 70, 40 and 10 by TwnFng1-3, used before the Mettaur
leaves its row; a miss each for FlmLine and TwnFng), thirteen mutations
caught. A sound row can now hold in the specs of a chip unit's chips
alone (`chips`: Thunder's ball's 0xAD, which Billy's thunder and
ThunderMan pair otherwise). FlmCros1-3 and BstFang1-3, the families'
Program Advances (their subfamily 1, in the moved states already), are
formed since (The Program Advances are formed, below). Thunder's and TwnFng's ids are past 602:
their pack entries are the save RAM's (ids 635 on), in dev since.

**MagBolt1-3 are moved** (11 November 2026: ids 642-644, BN4's family
0x59 at BN6's grown attack family 0xD5). The state draws the arm (its
kinds 0x23 to 0x25), puts the bolt (the third kind's 0x75) on the panel
ahead after six frames and for 0x1E frames strikes the row ahead with a
hit that only pulls (its bits 0x08); the bolt strikes what stands on its
panel. Three things of the engines differ under it, each read from both
games' code. BN4's rule for the direction an object moved by answers
every panel where BN6's answers a neighbour alone, so the unit copies
BN4's collision panels' update with its rule (`UNIT_COPIED`). A hit that
lands with a push is struck and pushed by bit 0x01 in BN4 and bit 0x40
in BN6 (MokoRus's finding): the bolt's bits are 0x43 in its copy. And a
foe under a slide differs in four leftovers the comparison now reads for
a chip's foe: a blocked slide's step, which BN4 keeps and BN6 zeroes; the
direction byte, which BN4 writes as the panel changes and BN6 at the
slide's end; the same blocked step in the struck-and-pushed state; and a
slide cut on its last step, since BN4's virus runner runs a pushed
object's slide before it takes a struck-and-pushed hit and BN6's takes
the hit first. `bn4_chip_magbolt1` (a 140 HP Mettaur pulled from (5,2)
to (4,2), not beside MegaMan: no hit), `2` (beside him already: hit for
110, the pull blocked) and `3` (pulled beside him and hit for 130,
struck for 0x18 frames in both) pass over 300 frames, eleven mutations
caught. MagShok1-3, the family's Program Advances (its subfamily 3, in
the moved state already), are formed since (The Program Advances are
formed, below).

**PropBom1-3 are moved** (11 November 2026: ids 671-673, BN4's family
0x62 at BN6's grown attack family 0xD4). The state throws the bomb (the
third kind's 0x8C) onto the panel ahead; it has 10 HP, takes its side's
obstacle slot (the engines' own routines, twins but for the slots' place
in each battle state) and crawls along the row, a panel in 32 frames.
Whatever strikes it, a virus stepping onto its panel too, ends it in a
burst that harms nothing; past the far column it blasts the two back
columns for its record's power. BN6's routine for effects over a pattern
of panels spawns the id it is handed, so the copy hands it BN4's
explosion's copy and the blast looks as BN4's does. Four specs pass over
300 frames: `bn4_chip_propbom1` (thrown at fight frame 8 from column 3,
the Mettaur a row away: the blast at t102, 140 HP to 20),
`bn4_chip_propbom1_struck` (the Mettaur's wave ends it at t73),
`bn4_chip_propbom2` (250 HP to 90) and `bn4_chip_propbom3` (the Mettaur
steps onto its panel at t66: the burst, no blast); eleven mutations
caught.

**TimeBomb is moved** (11 November 2026: id 529, BN4's support 0x11 at
BN6's grown support routine 0x91). Its routine spawns a placer (the
fourth kind's 0x27), which takes a column of the foe's side and draws
one of its free rows; the bomb (the third kind's 0x4B) is set there with
50 HP, in its side's obstacle slot. The bomb is an actor, the first
among the chips' objects: its update runs the engine's status step and
the actors' runner with a state table of its own. BN4's runner numbers
an object's own states from 6 and BN6's from 8, so the unit brings the
table in BN6's layout (the engine's eight states BN6's, the bomb's count
the ninth), written in its home, and the copy's one store of state 6
writes 8 ([the chip recipe generator](../devtools/chip-recipe-generator.md),
A moved actor). It counts three steps (60, 60 and 30 frames, a tick
each), then strikes the foe's whole area for its record's power under
BN4's explosions (the blasts' object, the fourth kind's 0x24, moved with
it); a hit taken first ends it in a burst. Three specs pass:
`bn4_chip_timebomb` (420 frames: set on (4,3) at t34, the blast at t242,
a Mettaur's 250 HP to 100), `bn4_chip_timebomb_struck` (another seed
draws (4,2), where the Mettaur's wave ends it at t73) and
`bn4_chip_timebomb_top` (a third seed, (4,1)); eleven mutations caught.
Not reached by a spec: a bomb pushed, which slides by BN6's rule (its
state 5 is the engine's, 0x60000 a frame against BN4's 0x80000), and the
placer's way on with no free panel, which in BN4 itself reads an address
of no memory (`BN4_T4_TIMEBOMB`'s note, `src/emu/bn4_chips.h`). A virus
the blast deletes is not compared yet: each engine's deletion draws its
own explosions (the fourth kind's 0x11), which the differential has no
rule for, so the specs' Mettaur outlives the blast.

**FirePlus is moved** (12 November 2026: id 563, BN4's support 0x4C at
BN6's grown support routine 0xCC; the owner's calls 7 and 12). Its
object lives a frame and has the stage's changer turn every panel to
grass, which is the whole of what it does to a battle: fire on grass
adds its damage again by each engine's own rule. BN4 also starts a
480-frame timer in the user's battle data, read by its HUD's icon
alone. BN6 has no such icon and keeps other fields at those offsets, so
the unit's call for that data is the chips' stub that returns 0 and the
four stores go nowhere: **the timer is not kept and the HUD's icon is
not moved**, the one thing of FirePlus a player of BN4 would miss. The
unit's code holds the other three kinds' paths too (ThunPlus, AquaPowr,
WoodPowr: the timers alone), which no chip of a run reaches (call 12:
their rows wait, `ROWS_WAIT`). The changer's block is copied a third
time with it (Hole's and Elem's units have it): the fourth kind has no
shared type yet. `bn4_chip_fireplus` (used at fight frame 30: eighteen
panels to grass at t34, the changer's sounds) and
`bn4_chip_fireplus_cracked` (the Mettaur's panel cracked before it:
grass with the rest) pass over 300 frames; five mutations caught.

**SeekBom1-3 are moved, a family's unit by subfamily** (11 November
2026: ids 598-600, BN4's family 9 at BN6's grown attack family 0xD2).
BN4's bombs are one family: its state throws by the record's subfamily
through a jump table of ten spawners (MiniBomb's three, Binder's 3,
Ball's 4, Geyser's 5, BlkBomb's 6, BugBomb's 7, VDoll's 8, SeekBom's 9;
DarkBomb is subfamily 2), the chips of three lines of work. A unit of it
copies the state (288 bytes), moves the entries of the subfamilies it
names, each a spawner in a third-kind type's code, and takes those chips
alone; the other entries of its copy are BN6's null routine. Another
subfamily's unit copies the state again at a number of its own
(`ATTACK_SUBS`, `ATTACK_NUMBERS` by the family and the unit's name), so
no unit writes another's table and `UNITS` stays append-only. SeekBom's
bomb (the third kind's 0x8D) seeks the nearest foe ahead and lands on
its panel; a hit spawns the explosions' maker (the fourth kind's 0x11),
which spawns the effect a byte of its own names: no immediate holds the
id, so its call goes through a shim that gives r4 BN4's explosion's copy
(`XvEffectCall`, its pair read from the bomb's two spawns of the maker).
`bn4_chip_seekbom1` (a 140 HP Mettaur in his row, left 60),
`bn4_chip_seekbom2` (a row away: the bomb finds it, 30 left) and
`bn4_chip_seekbom3` (250 HP to 110) pass over 300 frames, ten mutations
caught, the dropped pair among them. Binder, Ball and DarkBomb take
units of their own the same way; the comparison reads BN4's family as
one number, so a second unit of family 9 needs it by the spec's chip.

**What of the attack families' line waits** (11 November 2026).
GutPnch1-3 (support 0x36, ids 668-670): its object spawns a first-kind
object (type 0x42, spawner `0x080C85C2`), which no chip unit moves yet,
and a second body of the user drawn from his own battle sprite by his
Navi and Soul (`0x0800B90A`, a table at `0x08017F98`), for which BN6's
own routine is not found; two routines that run a handler per bit of a
mask over a global at `0x020363F0` (`0x08015B9E`, `0x08015E78`) are not
read.

**The Program Advances are formed** (12 November 2026: fourteen of the
nineteen). BN4's recipes are in BN6's grown list after BN6's and BN5's:
`tools/xreg_bn4.py` reads BN4's list (`BN4_PA_LIST`) and writes the
recipes of the Program Advances whose code is moved (its `PAS`) and
whose parts all play, in BN6's ids (`src/core/xchips_pas_bn4.c`, 19
recipes), and the chips' install lists them where BN4's code is in
(`src/emu/xchips_code.c` pas_stage: one place for both games' recipes,
each game's install writing the whole). BN4's checker was read for it
(`BN4_PA_CHECK`, [BN4 Red Sun](../rom/bn4.md), Its Program Advances): a
recipe's kind is 0, n of one chip with codes in a row, or anything else,
its ids listed in order, where BN6 reads a routine by the kind as an
offset, so BN4's kind 1 is BN6's 4 and nothing of BN6's checker changes.
A Program Advance's row stays `XREG_PA` (no folder chip, no pack entry)
and names its unit as a moved chip's does: its record takes the unit's
attack family with BN4's own subfamily and parameters, or the unit's
routine under BN6's misc family. HeatSprd and BubSprd are the spread
state's subfamilies 1 and 2 with the shot's kinds 0xB and 7 (the burst
over the nine panels round the hit), FlmCros1-3 FlmLine's subfamily 1
(five flames in a cross), BstFang1-3 TwnFng's subfamily 1 (eight
volleys), MagShok1-3 MagBolt's subfamily 3 (three bolts): all in the
units already, no code moved for them. PitRng1-3 is BN4's misc routine
0x0D, a unit of its own (`BN4_PITRNG`): a launcher, the fourth kind's
0x6C, that throws three of Boomer's boomerangs sixteen frames apart: a
misc unit that moves a fourth-kind type and shares a type an earlier
unit moved (the boomerang, Boomer's copy: `SHARED3`), as Meteors' unit
does with ShotStar's star. Twenty specs pass (`bn4_pa_*`), each Program Advance formed
by BN4's checker on BN4's Custom screen and by BN6's on its own from the
same three picks, dealt in order by `deck4` (`BN4_BATTLE_DECK`, found
for it), then used: HeatSprd by its listed recipe and by each of its
three rows of codes (a Spikey's 330 HP left 30), BubSprd by each of its
three, FlmCros1 (30 left), MagShok1-3 (50, 80 and 30 left), PitRng1 and
2 (50 left), BstFang1's eight volleys flown past a foe between them.
Twenty-six mutations are caught: the list's word, a recipe's Program
Advance, part and kind, each family's record (power, subfamily, the
shot's kind) and the code under each (the shot's row, the cross's third
flame, the volleys' count, MagShok's pull, the launcher's count and
wait). What no spec reaches: a Program Advance of 300 or more deletes
every moved virus it is not weak against, and a deletion parts at each
engine's own effect, so BubSprd, FlmCros2 and 3, PitRng3 and BstFang1-3
against a foe in their way are compared to the frame of the hit that
deletes (HP 0 in both), not past it; BubSprd's burst is HeatSprd's code
with another row, FlmCros2 and 3 are FlmCros1's with another power.
The comparison reads a Program Advance's row as a moved chip's, and
the launcher's element at BN6's place (the registries' bn4
object_element, by type). PitHoky1-3 (12 November 2026) are AirHoc's
state with powers of 100, 150 and 200, three AirHoc of codes in a row:
`bn4_pa_pithoky1`-`3` pass to the frame before the Mettaur's deletion,
two mutations caught. **Waiting on their parts:** P.Driver (GunSol1-3) and DarkNeo (DrkLine, BugChain and
Bass; a dark Program Advance, the dark chips' price its own question):
`tools/xreg_bn4.py` says so for each as it writes. Seen on the way,
none of it this unit's: the spread shot's collision data differs at the
frame it strikes an Elmperor (BN4's is struck back by the Elmperor's
body, flags and a tally at its +0x0C and +0x6C on, BN6's is not), a
Wood attack on a Fire virus sets BN6's own multiplier byte (collision
data +0x74) on the attack's object, a counter hit's Full Synchro is an
object of each engine's own, a foe held by `hold=` parts at t1 for a
moved BN4 family, and a struck Navi's AI data counts the damage in BN6
alone (+0x20): each kept out of these specs by the foe chosen, none
masked.

**The units' art room was the scarce one** (11 November 2026). A unit's
sprites were copied whole at the core's start, and an arm's kind is a
sprite of its own, 9 to 14 KB each: the spread unit's five take 49 KB,
and the twelve units to TwnFng 130 KB, against 8 KB of code. The room
(`XC4_ART` to `XC4_END`) was 127 KB of the region, and for a day its
start was moved down (`FREE_BN4_CHIPS` + 0x24000, 176 KB) to hold those
units. It could not hold the list: the plan has some 260 KB free in all
(`tools/free_space.py`). The art's room, below, ended it, and the
split is back at + 0x30000 (the code's room 96 KB).

**The units' third-kind objects run on a page of the dispatch that is
the chips' alone** (11 November 2026: [the generator](../devtools/chip-recipe-generator.md),
The dispatch's numbers). The dispatch's tables are one for every game
and BN5's own numbers were all they had left, 29 for a hundred units; a
type now runs at BN4's own number, nothing is allocated, and no BN5
Navi or chip moved later can meet a chip of BN4's there. The page takes
1.5 KB from the art's room's end (62 slots where it had 63).

**The units' art is no longer installed** (11 November 2026: The art's
room, below): a sprite is put into one room as an object loads it, so
the room holds what is alive at a moment and the remaining units need
no art's room of their own. Every chip spec passes with it, their
mutations are caught, and `bn4_chips_thirty` plays a folder of thirty.

**Meteors1-3 are moved** (10 November 2026: ids 674-676, misc 0x11), the
first unit of the parallel sweep's third line and the first whose chips
sit past id 602. Its routine lies after its controller, the fourth
kind's last type (0x81), which moves with it, 178 bytes: the chip aims
at a panel of MegaMan's row on the other side and the controller lets
five meteors fall there, one every twenty frames. The aim is a routine
BN6 holds too, instruction for instruction with the same words of panel
flags, though nothing in BN6 calls it. The meteors are ShotStar's star,
which ShotStar's unit moved already: the star is shared as the spread
chips' arm is (`tools/xchiprecipe_bn4.py` `SHARED3`), its spawner
copied with Meteors' unit and spawning ShotStar's copy by its number,
so the star's code is in the engine once and Meteors takes no
third-kind dispatch number of its own. `bn4_chip_meteors1`, `2`
and `3` pass with a 140 HP Mettaur on the aimed panel (90 then 40 HP
left, 80 then 20, and 70; each ends before the meteor that deletes it)
and `bn4_chip_meteors1_miss` over 300 frames with the Mettaur off it,
the five meteors bursting on the empty panel; seven mutations caught.
**Not seen in play yet:** ids from 603 have no pack entry, so the chip
step cannot hand one out until the pack's RAM for them lands; the
differential fights with a folder alone.

**Ligtnin1-3, AquaUp1-3 and GreenWd1-3 are moved, one unit** (ids
677-679 on 11 November 2026, 689-694 on 12 November; misc 0x0C). The
nine chips are one routine, a sweep (the third kind's 0x93) that goes
column by column from the user's edge and does one of three things by
the record's first parameter: a geyser from a cracked panel under a foe
(AquaUp, 0), a tower from a grass one (GreenWd, 1), or a bolt on a panel
that holds an obstacle, striking the eight panels around it (Ligtnin,
2). The unit is the sweep, the strike both panel chips share (0x92) and
the bolt (0x8E), each at BN4's own number on the chips' page of the
dispatch, 1144 bytes with the strike's rows, its sprites copied, its
three sounds at the grown table's 0x240 to 0x242 (the third line takes
its slots from 0x240 so the sweep's sessions number apart), and BN4's
own sprite step for a third-kind object (`BN4_T3_SPRITE_STEP`), copied
as BN4's Navis copy it: BN6's nearest updates a sprite in the frame its
animation is set up too, so the geyser's frames ran one ahead of BN4's.
`bn4_chip_ligtnin1`, `2` and `3` pass with a RockCube set first (the
bolt on its panel at t164; a 140 HP Mettaur beside it left 10 by
Ligtnin2) and `bn4_chip_ligtnin1_none` with no obstacle, no bolt. The
differential's BN4 battle gives the foe's panel a type with `panels=`
([the differential](../devtools/differential.md)): `bn4_chip_aquaup1` to
`3` on a cracked panel (110, 140 and 170 off), `bn4_chip_greenwd1` to
`3` on grass (120, 140 and 160), `bn4_chip_aquaup1_none` with no cracked
panel and `bn4_chip_greenwd1_cracked` on a cracked one, neither raising
anything; the stronger two of each fight BN4's Mettaur of 250 HP, as a
lethal hit there would compare each engine's deletion (its sound, which
in BN4 takes the geyser's player 9 in the same frame), not the chip.
Twelve mutations caught across the nine.

**BigHamr1-3 are moved** (12 November 2026: ids 659-661, BN4's support
0x10 at BN6's grown support routine 0x90). Its routine spawns a support
object (the fourth kind's 0x26), which sets a hammer (the third kind's
0x4A) on the panel before MegaMan: an actor of 100 HP, run by the
actors' runner as TimeBomb's bomb is, its own states renumbered from 8
the same way. It waits, then swings: the screen shakes for 30 frames
and the panel before it is struck for the record's power. Three specs
pass: `bn4_chip_bighamr1` and `bn4_chip_bighamr2` against BN4's Mettaur
of 250 HP (to 90 and 30), `bn4_chip_bighamr3` against BN4's Billy of 300
HP set at (4,1), which hops to (4,2) before the swing (to 20); seven
mutations caught. Two things the differential reads of MegaMan as the
battle's frame where his own record runs a frame behind after BN6's
support family (the support attack's end): his sprite's screen place,
which the camera's shake moves for every object alike, and his keys,
which the key update writes whatever his state
([the differential](../devtools/differential.md)). Not reached by a
spec: a lethal swing, whose deletion each engine draws its own way (as
TimeBomb's blast). **GodHammr waits** (`ROWS_WAIT` in
`tools/xchiprecipe_bn4.py`): its hit strikes the whole field, its own
hammer and MegaMan among them, and where BN4's actors' runner finds the
hammer's final damage and plays its hurt sound (0xC8), BN6's finds none,
both deleting the hammer by the hit's flag; BN6's hit pass on the
struck actor is still to read (`bn4_chip_godhammr` at t57).

**Next:** BlakBarr's barrier and BlakWeap (calls 7, 12 and 13: the
other three Plus chips and Marking are left out); Binder,
SandRing and Ball are attack families, whose unit kind the spread
chips brought (the bombs' family by `ATTACK_SUBS`, as SeekBom's unit). BugChain
waits on the owner's call 11 (it copies a side's NaviCust bugs to the
other's record, a NetBattle chip).

**The first chip of BN4's own plays: Invis** (10 November 2026). BN4's
support 0x31 is BN6's subfamily 1, Invisibl's, the same by body down its
callees but for the two engines' struct offsets (a scratch walk of both
ROMs), and both records hold the same 360 frames. Its row is
`XREG_RECORD` with its twin (`tools/xreg_bn4.py`, `ON_BN6`): id 558
takes Invisibl's family, subfamily and lock-on and BN4's parameters,
codes, MB, name and art. `bn4_chip_invis` passes against BN4's own
battle over 500 frames (the Mettaur's waves through him, his blink, the
sound as it ends), three mutations of its record caught. It plays where
BN4's ROM is read: BN4's visitors' rewards pay it where they paid the
stand-in, and the Library counts it. The way for the next: the deep walk
first, then a spec; a chip whose walk differs is BN4's code to move.

**The six "on BN6's code" are not** (10 November 2026, read in both ROMs
and run): the census compared each routine's body and stopped at a twin,
without its callees. Hole and DrkLine make BN4's Hole panel, a panel
BN6 has none of, and are dark chips (below); TimeBomb's object is BN6's
by body but places its bomb by BN4's own routine (`0x080E4DE4`, 55
instructions, against BN6's `0x080E45D0`, 14); LifeAura's and BlakBarr's
barrier reads its kind through a table of BN4's where BN6 takes the
record's parameter (BN4 `0x080E5784`, BN6 `0x080E4E3C`), as every
barrier and panel chip both games name holds another parameter in each
(HolyPanl 6 and 4, Barrier 0 and 1, Barr100 1 and 5). Unit 4 as planned
falls away: each of the six is a unit of unit 5, its code BN4's moved,
or a record on BN6's code with its parameters mapped, whichever its
spec proves. Ball, RollSP and RollDS are not yet read this deep.

**BN4's light and dark chips** (verified in the ROM, 10 November 2026):
a chip's record holds its alignment at +0x15 (`BN4_CHIP_ALIGN`: 0 any,
1 light, 2 dark). As a chip is used (`BN4_CHIP_USE_TEST`, `0x0800D5D6`)
a dark chip plays only where MegaMan's karma (`BN4_NAVI_KARMA`) is under
470 and a light chip only at 470 or over; otherwise the dud plays (a
puff, family 0x20's subfamily 4). 63 records are light (the SP Navi
chips, HolyPanl, Snctuary, the barriers) and 31 dark (Static, Hole,
DrkLine, Muramasa, Anubis, ElemDark, BlakWing, the DS Navi chips, four
past 300). A run takes no karma (The DarkChips, below), so a light chip
plays for any MegaMan, and a dark chip at the dark chips' price, 20 max
HP for a battle it is used in (the owner's call 4); the differential's
BN4 battle is given the karma its chip asks (`karma4`). What Hole's panel is for is not yet read (the
players' account, that a DS Navi chip needs one on the field, is a lead
for unit 6).

**The dark chips' price is in code** (10 November 2026;
[DarkChips](../game/meta/darkchips.md), the owner's call of 10 November
2026, Built). `tools/xreg_bn4.py` writes which rows are dark
(`xreg4_dark_ids`: a record's alignment dark, or BN4's DarkChip mark in
its effect flags, `BN4_CHIP_DARK`): 28 of the 193, the 23 dark-aligned
with a row (Hole, DrkLine, ElemDark, BugCurse, BlakBarr, 17 DS Navi
chips, the Program Advance DarkNeo) and the five DarkChips (DrkCanon,
DarkBomb, DrkVulcn, DrkSpred, DrkStage, whose alignment is 0: BN4 plays
them by its worry, not its karma). One used by MegaMan in BN6's engine
is seen where every DarkChip's use is (`BN6_DARK_AFTER`'s hook, by the
row's id) and the battle costs 20 max HP once. The tool's guard is
lifted: a dark chip may be listed in `MOVED` or `ON_BN6`. The other
eight dark-aligned records have no row: Static, Muramasa, Anubis and
BlakWing are BN6's own chips by name (BN6's code, no price), NumberMan's
and ShadeMan's DS the registry's rows of BN5's, ProtoMan's DS and
BugCharg none. **Not taken from BN4:** a dark MegaMan himself (his
object's +0x94, his dark sprite, what else his karma changes); a dark
chip's spec gives BN4's reference the karma that plays it (`karma4` 0)
and the comparison explains his look from both games' code: his
palette, BN4's dark 2 against BN6's 0, the one difference of him
(`bn4_chip_invis_dark`, [The differential](../devtools/differential.md)).

**The dark units, read** (10 November 2026; [BN4 Red Sun's
tables](../rom/bn4.md), The panel changer and the Hole panel). With the
price in code, each unit of the dark line was derived
(`tools/xchiprecipe_bn4.py --report`) and read in the ROM. None is
moved yet; why each waits:

| Chips | What they are in BN4's code | Waits on |
| --- | --- | --- |
| Static, Muramasa, Anubis, BlakWing | dark-aligned records of names BN6 has | nothing: a run's are BN6's own chips, one chip a name, no row |
| DrkStage (562), Hole (527), DrkLine (528) | one object, the panel changer (the fourth kind's 0x1F), by a kind of its table: Hole a Hole panel ahead, DrkLine a row of them, DrkStage Holes on MegaMan's whole side and poison on the other | **moved** (11 November 2026): one unit of support 0x0B and 0x3B, the changer once, its kinds' table in BN6's numbers, its blink BN6's, its four sounds at 0x280-0x282 and 0x29F; `bn4_chip_hole`, `bn4_chip_drkline` and `bn4_chip_drkstage` PASS by a dark MegaMan, five mutations caught |
| BlakWeap (602) | support 0x4B, 156 bytes: a third-kind object spawned by one call with no twin | its unit (call 7: moved with Marking and the Plus chips, not cut); its record is Marking's kind (no Library number, the blank picture), and not dark |
| DrkCanon (695), DarkBomb (696), DrkVulcn (601), DrkSpred (594) | subfamilies of BN4's cannons (0x0B/3), bombs (0x09/2), vulcans (0x1F/4) and spreads (0x1E/3); DrkCanon's power computed (1046) | DrkVulcn, DarkBomb and DrkSpred **moved** (12 November 2026, below); DrkCanon waits on BN4's computed powers (unit 6, bn4-navi-powers' claim) |
| ElemFlar, ElemIce, ElemLeaf, ElemSand, ElemDark (637, 638, 662, 663, 697) | a state of MegaMan's of their own, 0x60, subfamilies 0-4 | **moved** (12 November 2026, below) |
| DarkNeo (605) | a Program Advance (DrkLine, BugChain, Bass), support 0x46: 1036 bytes, 20 decisions open | its parts: DrkLine above, BugChain (the generator's line) |

**DrkVulcn and the Elem chips are moved** (12 November 2026: ids 601,
637, 638, 662, 663 and 697). DrkVulcn is BN4's vulcan state (its family
0x1F) as BN6's grown attack family 0xFD, its shot the spread chips'
copy, shared, its sound 0xBB at the grown table's slot 0x283; the Elem
chips are BN4's state 0x60 as attack family 0xFE, its subfamily the
chip's, the panel changer copied with it (the fourth kind's 0x9B), its
shot's two sounds (0xA8 and 0x14C) by one add at slots 0x285 and 0x329.
ElemDark's shot (its params' +7 set) takes BN4's third-kind sprite
step (`BN4_T3_SPRITE_STEP`), which the unit copies as Ligtnin's does:
BN6's nearest updates the sprite in the frame its animation is set up,
a frame ahead of BN4's. `bn4_chip_drkvulcn` passes to the seventh
shot's deleting hit (t104: 20 a shot on a 140 HP MettaurSP),
`bn4_chip_elemflar`, `elemice`, `elemleaf` and `elemsand` over 300
frames, and `bn4_chip_elemdark` over 300 by a dark MegaMan against
BN4's 250 HP Mettaur (its 220 would delete a MettaurSP); nineteen
mutations caught.
Three things of BN4's battle are not the run's, and the comparison
treats each as such:

- **BN4's bug at the fight's start after an attack DarkChip**
  (`calm4`, the owner's call of 12 November 2026: left out). Once an
  attack DarkChip is chosen, BN4 sets a NaviCust bug of type 1 in the
  battle NaviStats' +0x0D (`BN4_NAVI_BUG`, the byte its levels' routine
  writes: 0xFF, 0x10 or 0x20): 0xFF after DrkVulcn, read at the fight's
  start as confusion; 0x20 after DarkBomb, which walks MegaMan off his
  panel at t1. The same folder with nothing chosen, or DrkStage chosen,
  gives none; DrkSpred's gave none in its spec. BN4's MegaMan
  starts the fight confused for 0x2D0 frames (his collision +0x12,
  0x2CF at t1, counted down at `BN4_CONFUSE_STEP`) with its mark over
  him. Its source is **not yet traced**; ruled out, each hooked on the
  guest: the NaviCust bug-level add 0x080476E0, flag 1:0xAD's set, the
  NaviStats +0x0D writers 0x0800D756 and 0x0800D78A, the code's only
  literal 0x2D0 (its write at 0x0800D8BE), and every `strh` to +0x12
  from 0x08008000 to 0x08020000 but the countdown. The spec's `calm4`
  clears the bug each battle frame it is set, and sets the confusion's
  timer to 1 the first frame it holds more (the fight's start reads the
  bug before the first frame the comparator steps), so BN4's own step
  ends it and takes its mark away; `confuse6`, its mutation, gives
  BN6's MegaMan the 0x2D0 instead.
- **BN4's dark mood at an attack DarkChip's use** (`mood4`; the owner's
  call of 10 November: nothing of a dark chip's rules but the price). A
  chip whose record's +9 holds 0x20 sets MegaMan's mood 0 as it is used
  (`BN4_DARK_MOOD`, 0x0800D5A6-0x0800D5C8), and BN4's mood setter
  (`BN4_MOOD_SET`) never lifts a 0 again (BN5's the same, BN6's not):
  his emotion stays the dark one, so no counter gives Full Synchro for
  the battle. DrkVulcn's third shot counters the Mettaur at t60, Full
  Synchro in BN6's battle and none in BN4's. The spec's `mood4` hooks
  BN4's code there on the guest and goes on past it and the calm that
  follows (`BN4_DARK_MOOD_PAST`); its mutation turns it off. A dark
  MegaMan (karma4 0, which ElemDark needs in BN4's battle) begins the
  battle at mood 0 by his karma; `mood4` gives him the NaviStats' start
  mood once (`BN4_MOOD_START`, 0x99), as a run takes no karma.
- **Full Synchro's aura**, the first kind's 0x5E, is each engine's own
  object, spawned through twin spawners where MegaMan's emotion is 2
  (BN4 `BN4_SYNCHRO_AURA` into his AI data +0x60, BN6 0x080C6482 into
  +0x5C): its +0's 0x04 (BN6's spawner's alone), its +4 to +7 (each
  caller's leftover r4), its sprite state and the word that keeps it
  are allowed, its being in play and life compared (legit.json's bn4
  allowed, Full Synchro's aura; a mutation drops BN6's).

**BugCurse is moved** (12 November 2026: id 730), BN4's support
routine 0x2A as BN6's grown support routine 0xAA: its object (the
fourth kind's 0x56) casts the curse (the third kind's 0x80, BN4's
third-kind sprite step copied with it, its sound 0x141 at the grown
table's 0x288) on the other side's object it finds. BN4 finds it by
`0x0800C456`, the other side's first of its three objects, a virus
too; BN6's twin of that routine (`0x080103BC`) answers a side's MegaMan
alone, so against viruses it finds none and the curse is never cast.
The call goes to a routine of the project's own instead,
`XC4_FIRST_ACTOR` (`FREE_BN4_CHIP_STUBS`): a side's first of its four
actors in BN6's battle state (`BN6_BATTLE_PLAYERS` + side x
`BN6_BATTLE_PLAYER_STEP`), as BN4's takes its first; BN6's nearest
(`0x080103F8`) was not taken, as it would curse another foe where
several stand. `bn4_chip_bugcurse` passes over 300 frames by a dark
MegaMan, four mutations caught; whether the first or the nearest is
taken among several foes is read in both games' code, not compared, as
the differential refuses a BN4 spec of more than one foe (its opening's
alignment, legit.json's bn4 align). BN4's Gigas (729-733) need BN6's
folder of a spec to hold another chip besides: a folder of 30 Gigas
never deals in BN6.

**AirHoc1-3 are moved** (12 November 2026: ids 650-652), BN4's air
hockey state 0x27 as BN6's grown attack family 0xFB: MegaMan strikes
a puck (the third kind's 0x2E with its kinds' table, `0x080D2768`)
that bounces diagonally off the field's edges and the sides' border,
its spark the fourth kind's 0x28 moved as 0xA0, the spark's sprite
named by a word of the puck's spawn (`SPAWN_WORD_SPRITES`), its
sounds 0xC0 and 0xCC at the grown table's 0x289 and 0x28A. Three
things the copy needed. The puck's move test reads the collision
data's panel as one word (`ldr r0,[r0,#8]`, column and row with the
panel's byte and counter after them); BN6 keeps those at +0xA, where a
word cannot be read, so a call-in (`XvCallIn`, the family's own call
written over the two instructions) goes to `XC4_COLL_PANEL_WORD`
(`FREE_BN4_CHIP_STUBS` + 0x20), which builds the same word from BN6's
two halfwords. The bounce reads a panel's side through a copy of the
panel's address after a pop (`adds r4,r0,#0`, then `ldrb r1,[r4,#1]`),
which the generator's side patch now follows (BN6's +3). MegaMan's
strike is BN4's animation 0x10, BN6's 0x0C in its own art (frames of
2, 2, 2 and 10 in each). `bn4_chip_airhoc1` passes over 300 frames
against the 250 HP Mettaur, three hits of 50 in both; AirHoc2 and 3
to the frame before the Mettaur's deletion; five mutations caught.

**DarkBomb and DrkSpred are moved** (12 November 2026: ids 696 and
594). DarkBomb is the bombs' state's subfamily 2, its own copy of the
state at attack family 0xFC (as SeekBom's subfamily 9 has its own,
`ATTACK_SUBS`): its arm the third kind's 0x2's kind 0x11, its bomb the
third kind's 0x8 with its kinds' table (`0x080CE088`), its blast's
effect by kind (`0x080CE277`: kind 6's 0x2B, moved as BN6's 0xC4), its
blast's hit word 0x0A05FF01 for the hit area's spawn, its sounds 0xA9
and 0xBD at the grown table's 0x286 and 0x287. DrkSpred is the spread
state's subfamily 3 in the Spreads unit (0xD0), its shot the spread
shot's kind 0xF; its burst's sound, 0x8D, is past a byte at the
grown table's 0x200 where the shot's table names a byte, so the site's
call goes through a shim that makes r0 the slot only where the byte is
0x8D (`XvSoundCall.when`, the other kinds' bytes BN6's numbers as
their rows' copies give them). `bn4_chip_darkbomb` passes over 300
frames against BN4's 250 HP Mettaur (its 200 deletes a MettaurSP), and
`bn4_chip_drkspred` to its hit (t37: its 400 deletes any Mettaur, so
its burst's sound comes after the deletion and is proved by the shim's
unit test alone); eight mutations caught.

BN4's Hole panel (its type 0xB) is a plain panel by its flags: nothing
of BN4's damage, movement or panel code treats it otherwise, and a
panel's step never turns it back. **What it is for is a DS Navi chip's
power** (verified in the ROM): `BN4_DS_POWER` counts the Holes on the
whole field, ten at most, and reads the chip's power from its row of
`BN4_DS_POWERS`, a step a Hole: RollDS 30 with none and 80 with ten,
GutsMnDS 100 and 200, FireMnDS 150 and 250, MetlMnDS 200 and 300. Hole
adds one, DrkLine a row, DrkStage MegaMan's whole side (and poisons the
other). Seen in BN4's own battle (scratch runs of the differential's
harness with `karma4` 0, filmed): DrkStage leaves MegaMan's nine panels
Holes and the other side's nine poison, the foe's HP falling; DrkLine
turns his row's six panels Holes, the three on the other side too; and
RollDS used on a field with no Hole plays and takes 90 of a foe's 180
HP, three hits of 30, the table's last column. So no DS chip is refused
for want of a Hole: the players' account that it needs one is the floor
a field with none gives. Six more places test for a Hole (the map's row): a light Navi
standing on one cleanses it, two random pickers ask whether a row has
one, one leaves it out, one attack turns a column of them plain. So the
panel chips and the DS Navi chips are one design: a dark MegaMan gives
turns and max HP to dig Holes, and his DS chips hit up to twice as hard
for them.

**The Hole panel in BN6's engine, built** (11 November 2026; the
owner's call 8): BN6's type 16, `XPANEL_HOLE`, after BN5's lava, sea and
magnet panel ([one Net](one-net.md); `src/emu/xpanels.h`). Call 8 asked
for a panel type of its own, and it has one, past BN6's sixteen: 15, the
last of them, went to BN5's magnet panel with MagnetMan (#391), and BN6's
flags update ORs a panel's type into its flags' bits 0-4, where bit 4 is
also its words' own floor bit, so a type of 16 to 31 always gives that
bit. The Hole's word, plain's, holds it anyway, so at 16 its flags are
plain's as in BN4 and nothing BN6 runs knows the number
([BN6's panel types and who holds each](../rom/battle.md)). A Hole on
BN6's own hole type (1) was weighed and refused: BN4's Hole is stood on
as plain is, BN6's hole is not, and nothing BN6 runs for type 1 is
BN4's. Installed wherever BN4's
ROM is read, with BN5's or without. Its flags are plain's (BN4's word for
it, checked to be BN6's plain one), its tiles BN4's 54, its colours BN4's
plain panel's rims and its hole's three dark colours, which pulse as
BN4's palettes 1 and 5 pulse them, written over grass's in a battle that
shows a Hole and no grass ([BN4's ROM](../rom/bn4.md), The Hole panel
in BN6's engine). It does nothing to what stands on it, as in BN4; BN4's
one rule for it, a light Navi cleansing it, has no karma to read in BN6
and is not taken. BN4's count of a type's panels is BN6's own routine
(`BN6_PANEL_COUNT`), so a DS Navi chip's moved power counts Holes by
calling it with 16. Seen by the `status panel` dev step: four panels
made Holes draw as BN4's on both sides. The three chips that make them
are moved (the table above): each PASSes against BN4's battle, the Holes
on the same panels at the same frame.

**Unit 2, BN4's registry rows, built** (10 November 2026; The build plan):
194 rows, written by `tools/xreg_bn4.py` from the three ROMs into
`src/core/xchips_registry_bn4.c`, a table of their own beside BN5's
(`XChipReg4`, `xreg4_of`, `xreg4_at`), so that what walks BN5's rows meets
none of them. The tool mirrors the engine's pairing (`pair_bn4`) on the
names in BN6's codes: 175 folder chips have no chip of the run's (the
census's 172 less ShadeMan's SP and DS, which BN5's rows pay since, plus
SerchMan V1, CannMode, SwrdMode, FinalGun and BugCurse, the names BN5 has
unregistered; BugCharg none, ProtoMan's DS none) and 19 Program Advances.
The ids, fixed for good: **527-602**, the 76 with a pack entry in the work RAM, go
to the chips that play first in this plan's order (Hole, DrkLine,
TimeBomb, LifeAura, BlakBarr, Ball, RollSP and RollDS; the Navi chips of
GutsMan, ThunMan, WindMan, FireMan, SerchMan, AquaMan and WoodMan; the
fifteen of the units under 300 bytes; then SidBmbo, WhitWeb, MokoRus,
Binder, SandRing, Boomer, BugChain, the spread chips, FlmLine, SeekBom,
DrkVulcn and BlakWeap); **605-623** to the Program Advances (the engine
keeps the bit of one formed from 603, 32 of them); **635-733** to the
other 99 folder chips, in BN4's order. Every row is `XREG_PENDING` (it
sits out: its code is not in BN6's engine) or `XREG_PA`. Each record is
made from BN4's in BN6's layout (`xrec_from_bn4`: its MB, kind and class
from +6 to +8; its kind as BN6's icon element, BN4 numbering its kinds as
BN5 its icon elements; a computed power unshown; inert until its code
plays), its name and description from BN4's archives in BN6's codes
(BN4's punctuation, 0x40 to 0x57, is BN6's 0x58 on, measured over the 64
descriptions it shares word for word with a chip of the run; its box's
lines set in BN6's), its icon in BN6's icons' palette, its picture and
palette copied, 123 KB of art in all. Without BN4's ROM each is a
placeholder that keeps its codes, class and MB ("BN4-82", "Needs BN4 ROM
back to battle"). Seen: BN4's GutsMan and Hole in BN6's folder editor
with BN4's pictures, descriptions and codes (the chip step, `chip 535 G
15`), and Hole's placeholder with BN4's ROM away. Unit tests hold the
rows' rules, a record made from a made-up BN4 record, the placeholder and
the text codes. A reward of one still pays the stand-in's zenny, as it
sits out. Not in this unit: an id past 602 had no pack entry (a second
survey with canaries found the folder editor, the Library and the
NaviCust writing most of the work RAM's stretches that looked free); the
ids from 635 count in the cart's save RAM since (Where things stand).

**Unit 1, ten-bit chip ids, live; ids past 511 in part** (below, The build plan): the
census of BN6's code is read and kept by `tools/chip_ids.py` ([The chip
registry and BN5's chips' code](../rom/chips-code.md), Ten-bit chip
ids): of 625 shapes 128 are code, 77 of them to patch (116 halfwords,
`--table`), 2 a reward's, 6 the link battle's, 37 no chip, the folder
editor's 5 item-key shifts kept (its items, id << 7 | code, hold ids to
511 whatever an entry's form) and 1 the link battle's real-time chip
open. Built: every reader and writer of a folder entry in the engine's
C through `src/core/chip_entry.h` (nine bits still, one number to
change), its converter from nine bits unit-tested, the installer of the
census's table and the preset folders (`src/emu/xchips_wide.c`, inert at
nine bits), the profile's Library to 1023, the wire's chips to 1023
(protocol v16). Tried at ten bits, not shipped: the autopilot's seeds 1-3
the same run log and pictures as at nine, the folder editor the same
picture for picture, every differential spec its nine-bit verdict.

**Ten bits are live** since: `CHIP_ID_BITS` 10, `RUN_MAGIC` "CWED" (a
run of "CWEB" or "CWEC" converted once, [Portable saves](../engine/saves.md)),
the image of 1024 records and four archives each of names and
descriptions, the engine's tables sized by `CHIPS_IDS` and `XREG_IDS`
(1024), the names' and descriptions' routines over four archives, the
ownership marks hooked for ids from 512 (owned without a mark), the link
battle's real-time chip settled (capped at 0x139 before its mask). An id
past 511 plays: `--dev alias=600` makes chip 600 a copy of Cannon's
record, name and description, and the chip step `chip 600 A 30` puts it
in the folder; the Custom screen deals and draws it (Cannon A 40),
picked and fired it deletes a Mettaur; the run saved so (`run.sav`
"CWED") and CONTINUEd deals it again. Proved against nine bits: the
autopilot's seeds 1-12 from fresh data folders the same run log (66
battles) and the same pictures at frames 5000, 10000 and 14990; every
differential spec its nine-bit verdict (421 PASS on dev merged, the two
recorded FAILs and two smoke INCONCLUSIVEs as before), 246 of 246
mutations caught; the server's contract tests take a trade and an offer
of 600 and 1023. The New 3DS's heap modelled (`tools/heap3ds.py`, seeds
1-3): BN6 alone 85.9 MB, none refused; with BN4 94.7 MB, none refused;
with BN5 95.8 MB, a few of the tiles' learning caches refused as on 4
November (docs/engine/emulation/3ds-memory.md), the image 64 KB larger.

**An id past 511 is whole, to 602** (built 9 November 2026; [The chip
registry and BN5's chips' code](../rom/chips-code.md), The pack past 511).
The pack: a survey of the work RAM ([The RAM survey](../devtools/ram-survey.md),
54 runs) found no 6 KB BN6 leaves alone, nor 1 KB outside the persistent
zone, which is full; but the extension's entries of ids 320-410 are
Program Advances' and BN6's pseudo-chips', which no pack counts, so ids
512-602 take them (`bn6_pack_entry`; a canary over them changed by the
power-on clear alone). 91 ids: BN5's remaining chips and BN4's first
units. The folder editor's items are id << 6 | code (35 halfwords), every
reader of a name or a description takes one of four archives (six tables'
literals, two readers rewritten, the chat box's buffer command hooked), a
chip from 512 given enters the profile's Library by an event, where BN6
would set another block's flag, and a reward names one in a wide form
(kind 2 or 3 with bit 13, `reward_chip`) that a hook at BN6's split reads:
no table, no reserved id (511 stays out of the registry's reach all the
same). Proved with chip 600 (`--dev alias=600`, `reward=600`): given,
listed and swapped in the folder editor, counted again as it is left,
named on the results screen and in a Mystery Data's chat (another
archive's text with the chat's hook out), the autopilot's seeds 1-12 the
base build's run logs and pictures.

Since: the ids from 635 to 1023 have their pack entries in the cart's
save RAM (Where things stand); 603-634 are Program Advances', which need
none. Not yet: a
NetBattle's loadout past 511 (unit 8), and BN5's chips' and Souls' moved
code read for entries it splits itself (the differential passes at ten
bits).

A BN4 virus's reward chip pays the run's chip it is (`xchips_bn4_paid`,
unit 0, built 9 November 2026), by three rules in turn, each one chip per
name as the registry's:

1. **By name, BN6's own, whatever the class**: BN6's folder chip of its
   name (BN4's names in BN6's codes first: its dash 0x40, plus 0x46 and
   dot 0x4E are BN6's 0x98, 0x9E and 0xA6, `rom.c`'s `bn4_code`). 78
   chips: the 65 paid before, 3 more now that the punctuation reads
   (M-Cannon, Atk+10, Navi+20), and the 10 BN4 calls Mega where BN6's are
   standard (Muramasa, Guardian, Anubis, Atk+30, BugFix, DblPoint,
   Snctuary, FullCust, NeoVari, SuprVulc).
2. **By name, the registry's own id** (BN5's ROM read): BN5's chip of its
   name where the registry gives it an id of its own and it is no Program
   Advance (no reward names one). 34 chips: the 26 moved (Z Saver,
   NumberMan's V1 and ShadeMan's among them), the 7 playing over BN6's
   code, a DarkChip (DrkLance).
3. **By Navi and version**: BN4's SP and DS (its record's first parameter
   3 and 4, as BN5's; spelt with letters, "ProtoMSP", where BN6 and BN5
   mark the version) whose V1, one or two ids before with its family and
   subfamily, paired: the registry's row of BN5's id as many after the
   V1's, its record of that version; else BN6's own chip of the V1's
   family and subfamily named with the SP mark. 5 chips: NumberMan's SP
   and DS, ShadeMan's SP and DS (the registry's), ProtoMan's SP (BN6's).
   ProtoMan's DS pays none: BN6's Navi chips are V1, EX and SP. Roll's SP
   and DS wait for theirs (unit 4).

A BN4 pseudo-chip or Program Advance pairs none; BN4's Giga FinalGun
neither, BN6's of the name being a pseudo-chip (it waits for an id of its
own). One the run lacks, or that sits out, pays 200 zenny as before (the
owner's call 9 of [Battle Network 4 in a run](bn4.md)). With BN5's ROM,
117 of BN4's chips pay a chip, 52 more than the 65 before; without it 79
(a scratch mirror of the rules over the three ROMs). The eight moved families'
reward rows (Billy, Spikey, Shrimpy, Walla, Elmperor, CirKill, Mettaur,
Melody) name 276 chip entries, of 26 chips, all BN4's own but Guard1-3:
none paid a chip before; with BN5's ROM Mettaur's 38 Guard entries now
do (a scratch tally of the ROM's rows, numbers only). Mettaur is moved for
Roll's summon alone, no visitor: the six visitors' rows name only BN4's
own chips, so a player meets the pairings as BN4's chips come (unit 2 on)
and as more of its families visit. A tournament round
against a BN4 Navi pays a BN6 Navi chip of the round's band as a
stand-in ([the tournament](../game/tournament.md), call 7). BN4's own
chips have their ids since unit 2 (above); none plays yet.

## The art's room

No chip unit's art is installed (11 November 2026). A unit's sprites
are about 13 KB (nine units took 115 KB), BN4 has some 110 units, and
the core's copy holds 127 KB for them (`XC4_ART` to `XC4_END`); its
free space is full and the per-battle RAM has no byte left
([Memory](../engine/emulation/memory.md)). A folder's units cannot be
laid out for a battle either, as BN4's Navis' art is: thirty chips can
be thirty units, about 400 KB, and a NetBattle holds two folders.

So a sprite is put into the room when BN6 loads it for an object
(`src/emu/xchips_bn4_art.h`). BN6 loads every object's sprite through
one routine (`BN6_SPRITE_LOAD`), and a hook on it (`BN6_SPRITE_LOAD_IN`)
sees a row of the units' sprites, copies it from the player's BN4 ROM
(unpacked where BN4 keeps it LZ77'd) and points its list's entry at it
before the routine reads the entry.

The room keeps no record that a state would have to save. It is cut in
slots of 0x800 bytes, and row k (its number over every unit's rows, in
`UNITS`' order: `XC4_ROOM_ROW(k)`) sits at a slot's start + 4 x k, so a
sprite's address names it. The sprites in use are read from the objects
in play, each one's sprite state's pointer (`BN6_SPRITE_DATA`): the
same on both cores of a NetBattle. A sprite an object in play holds is
found at its place; a new one takes the lowest run of slots none in
play lies in, over whatever no object draws any more. Where every slot
is in play the entry stays BN6's placeholder, as BN6 leaves an unloaded
sprite, and the debug log says so. What the slots hold is noted in the
room itself (a u16 a slot), and before the run list's objects run
(`BN6_OBJECT_RUNNER_IN`) a sprite an object in play points at is copied
again where its slots hold another's: after a rollback or a loaded
state the objects are another moment's and the room's bytes are not.

The room needs the sprites alive at one moment, not a folder's: nine of
the largest (14 KB) fit at once. `bn4_chips_thirty` fights eight turns
with thirty of BN4's chips in the folder, every unit's, and never has
more than three rows in play. For a unit nothing changes: the generator
writes each sprite as a row of the room, its `room` the sprite's size.
Not proved yet: a battle on a 3DS, where the Arena's two cores share
one copy of the ROM and so one room (each core puts its own objects'
sprites right before they run; a sprite one core's objects hold and the
other's have dropped is copied twice).

## The census

### BN4's 388 records against a run's chips

Ids 1-388 of `BN4_CHIP_RECORDS`, paired by name (each game's text in
BN6's letters, punctuation mapped: BN4 spells the dash and the plus with
other codes than BN6 and BN5), then by Navi and version where the games
spell a Navi chip otherwise:

| Records | How many | What a run does with them today |
| --- | --- | --- |
| Not chips: no codes or no name (copies of LifeAura, Bass and MegaSoul, blanks) | 21 | nothing |
| BN4's pseudo-chips (class 3): the Souls' sixteen, DarkNeo's boss records, BN4's training tools | 30 | nothing; the Souls come by their own track ([BN4's Souls](bn4-souls.md)) |
| BN6's by name and class | 88 | paid as BN6's own |
| BN6's by name, another class: ten BN6 standard chips BN4 calls Mega (Muramasa, Guardian, Anubis, Atk+30, BugFix, DblPoint, Snctuary, FullCust, NeoVari, SuprVulc), Z Saver and the Giga FinalGun (BN6's of both names are pseudo-chips) | 12 | 200 zenny: `pair_bn4` refuses a pair whose library type differs |
| By Navi and version: ProtoMan SP and DS (BN6's), NumberMan SP and DS (the registry's) | 4 | 200 zenny: BN4 spells "SP" and "DS", BN6 and BN5 a glyph |
| The registry's by name (BN5's): 25 moved, 7 over BN6's code, 3 Program Advances, 1 DarkChip | 36 | 200 zenny (`pair_bn4` pairs BN6's own alone) |
| BN5 has the name, unregistered: SearchMan and ShadeMan V1, CannMode, SwrdMode, BugCurse, BugCharg | 6 | 200 zenny |
| **New to a run** | **191** | 200 zenny |

The 191 by kind: 100 standard chips, 12 Megas, 4 Gigas (RedSun,
SignlRed, BlakBarr, BlueMoon), 51 Navi chips, 5 DarkChips (DrkCanon,
DarkBomb, DrkVulcn, DrkSpred, DrkStage) and 19 Program Advances.

The Navi chips: BN4 holds 60 (20 Navis, V1, SP and DS each). Roll V1,
ProtoMan's three and NumberMan's three are a run's already; SearchMan V1
and ShadeMan V1 have BN5's names but no registry row; the other 51 are
the SP and DS of Roll, SearchMan and ShadeMan and all three of GutsMan,
WindMan, FireMan, ThunderMan, MetalMan, JunkMan, AquaMan, WoodMan,
TopMan, BurnerMan, ColdMan, SparkMan, LaserMan, KendoMan and VideoMan.
Eight of those Navis are moved and in the tournament (GutsMan,
ThunderMan, WindMan, FireMan, SearchMan, AquaMan, Roll and WoodMan:
[BN4's Navis in BN6's engine](bn4-navis.md)): 23 chips.

The Program Advances: BN4's list (`BN4_PA_LIST`, [BN4 Red Sun's
tables](../rom/bn4.md), Its Program Advances) holds 37 recipes of 30
Program Advances. Eight are BN6's own (LifeSrd, PoisPhar, BodyGrd,
GigaCan1-3, TimeBom+, H-Burst), three the registry's (SuprSpr1-3), 19
new: HeatSprd and BubSprd (three of the spread chips in a row of codes),
FlmCros1-3 (FlmLine), PitRng1-3 (Boomer1-3), BstFang1-3 (TwnFng),
PitHoky1-3 (AirHoc1-3), MagShok1-3 (MagBolt), P.Driver (GunSol1, 2 and
3) and DarkNeo (DrkLine, BugChain and Bass or BassAnly). Every one is
formed from chips of the 191 but DarkNeo's Bass.

### Their rarity, codes and MB

BN4's records are BN6's layout but for +6 to +8 (its MB, its kind and its
class where BN6 keeps its icon element, its library type and its MB:
`BN4_CHIP_MB`, `BN4_CHIP_KIND`, `BN4_CHIP_CLASS`). Their numbers are on
BN6's scale:

| | BN4's new standard chips (100) | BN6's standard chips (199) |
| --- | --- | --- |
| Rarity 0 / 1 / 2 / 3 / 4 | 15% / 30% / 41% / 13% / 1% | 20% / 29% / 35% / 12% / 4% |
| Codes: one / three / four | 2 / 62 / 36 | 31 / 116 / 52 |
| With the \* code | 38 | (BN6's own counts in `economy.c`'s reports) |
| MB, median | 28 | 30 |

The Megas are rarity 3 and 4 (BN6's 2 to 4), MB 38 to 80; the Gigas
rarity 4, one code each; the Navi chips one code each, V1 rarity 3, SP
and DS 4, MB 32 to 88; the DarkChips rarity 4, MB 99, four codes with
\*. Nothing needs rescaling: a BN4 chip's rarity is a tier as a BN6
chip's is.

### Their code: BN6's, or BN4's moved

A chip runs by its record's family (+0xB) and subfamily (+0xC). BN4's
family is a state of MegaMan's (`BN4_PLAYER_STATES` by its number),
BN6's an attack family (its table by the number less 0x10), so the
numbers differ; the 88 chips both name pair 16 of BN4's families with
BN6's (BN4's 0x09 bombs BN6's 0x12, 0x0A swords 0x13, 0x0B cannons 0x14,
0x0C support 0x15 and the Navi chips 0x1B, 0x1D recovery 0x20, 0x1F
vulcans 0x17, 0x20 the misc family 0x1C, and nine more of one chip
each). BN4's 0x0C runs its support chips and its Navi chips from one
subfamily table, where BN6 splits them. A new chip whose family and
subfamily a paired chip shares runs on BN6's code with its own record;
for the rest, every routine its dispatch reaches and every object it
spawns (`BN4_SPAWN_1`, `_3`, `_4` with the type in r0) was looked up by
body in the pairs' index of BN6's routines (`tools/xrecipe_pairs.py`),
the walk stopped at a twin.

- **On BN6's code, 6 chips by the census, none by the differential**
  (Where things stand: the walk compared bodies and not callees; kept as
  the census wrote it): Hole and DrkLine (BN4's 0x0C/0x0B is
  BN6's panel changer, support subfamily 5, HolyPanl's and Snctuary's,
  the panel by the record's parameters); TimeBomb (0x0C/0x11,
  TimeBom+'s, BN6's subfamily 10); LifeAura and BlakBarr (0x0C/0x15,
  the barrier's, BN6's subfamily 4: whether BN6's barrier draws their
  auras is the differential's to say); Ball (the bomb family's
  subfamily 4, its routine BN6's by body). RollSP and RollDS run BN6's
  Roll (Navi family 0x1B, 0) with a computed power (1001 and 1023,
  indices into BN4's own table of computed powers, which moves as
  BN5's did: [the chips' code](chips-code.md), Computed powers).
- **BN4's code moved, the rest:** 134 chips in 48 units by family and
  subfamily, the 19 Program Advances with their parts' units, and 51
  Navi chips in 18 units, one a Navi.
- **Already a run's, a pairing away:** 16 (the twelve of another class,
  the four by Navi): no new id, no code.

The code a unit moves, beyond an engine base of 340 routines (13.6 KB)
that four or more units reach and that is ported or paired once, as
BN4's families' 71 routines were (an upper bound: a routine is counted
moved where its body has no twin, and the generator's audits pair more):

| Units | Chips | Code to move | Which |
| --- | --- | --- | --- |
| 8 | 15 | under 300 B each, 1.0 KB | Marking, Repair, Invis, PanlSht1-3, DrkStage, FirePlus/ThunPlus/AquaPowr/WoodPowr, Counter1-3, ShotStar |
| 21 | 62 | 300 to 600 B, 9.8 KB | SidBmbo1-3, WhitWeb1-3 (three units), MokoRus1-3, Binder1-3, SandRing, Boomer1-3, BugChain, the spread chips (HeatShot to BublSide, DrkSpred, HeatSprd, BubSprd), FlmLine1-3 with FlmCros1-3, SeekBom1-3, TwnFng1-3 with BstFang1-3, PropBom1-3, MagBolt1-3 with MagShok1-3, PitRng1-3, Thunder1-3, DrkVulcn, BlakWeap, DrkCanon, GutPnch1-3 |
| 13 | 43 | 600 B to 1 KB, 9.9 KB | PopUp, GunSol1-3 and GunSolEX, DarkBomb, ElemFlar to ElemDark, CircGun1-3, BlueMoon, HawkCut1-3, Meteors1-3, Ligtnin1-3 with AquaUp1-3 and GreenWd1-3 (one unit), P.Driver, AirHoc1-3 with PitHoky1-3, NumbrBl1-3, RollAro1-3, Blizzard/HeatBrth/ElecShok/WoodPwdr |
| 6 | 14 | over 1 KB, 13.6 KB | RedSun, DarkNeo, SignlRed, BigHamr1-3 with GodHammr, MetlGer1-3 |
| 18 | 51 | 15.7 KB with the Navis' own objects | the Navi chips: each spawns its Navi's first-kind type, so a moved Navi's copy serves his chips (as BN5's Navi chips run the Navis' copies) and the chips of the ten Navis not moved wait on Phase 2 |

### Past 511: one answer for BN4's chips and BN5's

BN6's chip id is nine bits: a folder entry is the id in its low 9 bits
and the code in its high 7 ([Chips, folders and the Library](../rom/chips.md)),
a reward entry the id, the code from bit 9 and its kind from bit 14, the
pack's extension, the ownership marks (516 bytes) and their keys (512)
and the Library's flags (`0x1E20` + id, the comparisons' block from
`0x2020`) sized for 512 ([The chip registry and BN5's chips'
code](../rom/chips-code.md), The chip registry), and the wire's
`chip_last` 511 (`shared/online/protocol.json`). The registry has taken
every free id but 10 (365-367, 505-511). Two pieces of work meet that
wall: BN4's 198 (its 191, its 6 of BN5's unregistered names and its
Giga FinalGun) and BN5's own unregistered chips the gap audit counted
(105 named; those a run would give: the Darkloids' and Nebula's 21 Navi
chips, the five Program Advances made with them, Gigas still to trace).
One answer serves both.

The three ways, measured:

- **Reusing ids a run never gives.** BN6's 313 folder ids hold 26 a run
  cannot give by `chip_pool.c`'s own rules (a scratch read of Gregar's
  records): 17 of the other version (Falzar's Navi chips 242-256, Otenko
  and one more, library flag 1) and 9 cut from the US release (no
  picture of their own: 275-280, 311-313). 21 of them lie in BN6's Navi
  chips' range (221-280, which AntiNavi and the Navi chip's freeze test),
  just what a Navi chip wants. But 26 is a seventh of the need, the
  other version's ids are Falzar's should that open track come, and a
  reused id keeps BN6's Library number and flags, which its Library
  screen reads. A stopgap at most; not chosen.
- **Remapping** (a window of ids lent to the other games' chips a save
  holds, the chip's identity, its game and number, kept in the save):
  BN6's format untouched, but the window is 148 ids at most (the
  registry's 138 own and the 10 free), fewer than BN4's chips alone; a
  slot must be taken back when no folder, pack, stock or reward row
  names it; the records written into the core's copy change mid-run, so
  a checkpoint carries the map; a NetBattle's pair cores need one map
  for both folders, built the same in both games. It gives up the
  registry's first rule, the same ids at every boot. Not chosen.
- **A wider id: ten bits, 1024 ids.** Chosen. Static, the registry's
  rule kept, every row of either game from 512 at an id fixed once.
  Its cost is one audit, done once for both games.

What ten bits take. Located, then read (unit 1's census, [The chip
registry and BN5's chips' code](../rom/chips-code.md), Ten-bit chip
ids): in BN6's code (to `0x08150000`) 47 places take the low nine bits
by a shift pair (`lsls #23`, `lsrs #23`), 26 load a literal whose low
half is 0x1FF (18 plain), 119 shift right by 9 and 402 left by 9; of all
of them 128 are code the game reaches, the rest data read as Thumb. The
census's verdicts below stand: the hand past the Custom screen holds id
and code apart, so a chip's attack takes an id, never an entry; the
reward splits its own entry at `BN6_REWARD_SPLIT`; the folder editor
keeps its items as id << 7 | code in a halfword, ten bits id << 6 | code
(the codes to 28 fit six bits).

- **Folders:** an entry becomes the id in its low 10 bits and the code
  from bit 10 (the code needs 5 of the 6 left; bit 15 spare). Every
  reader patched; the ROM's tables of entries re-encoded as the core's
  copy is made (the six preset folders, the 18 shops' stock, the five
  trader pools, Mystery Data's lists, the Program Advances' recipes:
  the tables the registry's audit already listed), the grown recipe list
  written in the new form.
- **Rewards:** a reward entry cannot hold ten bits beside its code and
  kind (17 bits). An id from 512 is written in a wide form: kind 2 or 3
  (HP, BugFrags) with bit 13, which no row's amount reaches, the kind its
  id's block of 256, the id's low byte and the code below; a hook at BN6's
  split of a reward reads it (built; the first plan, a reserved id 511
  and a table by battle, would have made a saved state's reward depend on
  the table's order).
- **The engine's tables:** the ownership marks and their keys, the
  Library's flags (moved past `0x2020`'s block to flags BN6 leaves
  alone), the per-side use counts and the Program Advance bits sized for
  1024; the pack's extension 12 bytes an id from 512 (2.4 KB for BN4's,
  more for BN5's) in RAM BN6 leaves alone: 512-602 in the work RAM's
  extension, 635-1023 in the cart's save RAM (Where things stand).
- **Saves:** `run.state` holds BN6's RAM, its folders in the old form: a
  run saved before is converted once as it loads (its folders' entries
  rewritten, `RUN_MAGIC` bumped with the converter, so no run is lost);
  the profile's Library grows to 1023 (its version bumped, an old one
  read into the larger array); a `.cwsave` carries the profile's version
  ([Moving saves](../engine/saves.md)).
- **Trades and the NetPass:** the wire's `chip_last` to 1023 and the
  protocol's version bumped (two builds that differ never pair). A chip
  is named by its id on every game that registers it, as BN5's are, and
  the board leaves an offer out for a game that cannot name its chip, as
  it does BN5's for a game without BN5's ROM. BN5's new rows are BN5's:
  inside the NetPass's full set already, they trade and fight as BN5's
  chips do. BN4's chips follow call 1 (Online, below).
- **NetBattles:** both games' pair cores install the same build's
  registry and compare the installs' record, as now; the ids change
  nothing there.

**The first batch, in the ten free ids, before ten bits** (built 9
November 2026 for ShadeMan's and BlizzardMan's six, at 505-510, each
`chip_*` spec PASS; BigNoise not yet: [The chips' code](chips-code.md)): BN5's
ShadeMan and BlizzardMan Navi chips, V1, SP and DS (six, their Navis
moved already, a chip unit each as NumbrMan's, the gap audit's M), at
505-510, and BigNoise (Pulsar3 and ShadeMan's SP or DS, BN5's recipe) at
365 once ShadeMan's play. 511 is held for the wide rewards above; 366
and 367 stay spare. BN4's ShadeMan V1, SP and DS are the same Navi's: by
Navi and version (unit 0 below) they pay those rows, so BN4 needs 195
ids, not 198. No chip of BN4's own goes into the free ids: its six on
BN6's code wait for ten bits, which they would otherwise use up.

## The Navi chips

Unit 6: the 23 chips of the eight moved Navis. What the first, GutsMan's
V1, settled for all of them.

- **The unit is the chip's Navi.** Each Navi chip's support routine
  spawns a fourth-kind object of its own (GutsMan's 0x10, FireMan's 0x0F,
  AquaMan's 0x16, WoodMan's 0x70; 0x42 serves six Navis by the record's
  third parameter, ThunMan and SerchMan among them), and each of those
  objects is BN6's one Navi chip object (`BN6_CHIP_NAVI_OBJECT`, the
  fourth kind's 0x10) read side by side: MegaMan stepped out and 0x1E
  frames, the Navi's spawner called and his word waited for, 0x1E frames,
  MegaMan stepped in and 0x1E frames. BN4's object is not moved. BN6's
  stands in for it (`tools/xchiprecipe_bn4_navi.py` `OBJECTS`, each BN4
  object read against it before its unit is written, its waits checked
  by the tool), because stepping MegaMan out is each engine's own work
  with his forms (BN4's fourth-kind 0x0C reads its Souls, BN6's
  first-kind 0x2D its Crosses), and because BN6's Navi family ends the
  attack in the frame of the use, as BN4's support state does, where
  BN6's support family holds MegaMan a frame more. What moves is the first-kind type the object's spawner names, the
  Navi as his chip shows him, at a first-kind number of the chips' own in
  the dispatch (0xA0 on).
- **His attack is the Navi's own.** The chip's Navi spawns the objects
  the Navi fights with, which the Navi's recipe moved
  (`src/emu/xnavi_bn4.c`). The unit moves none of them again: each call
  into a block of the Navi's goes to the Navi's copy, one copy run by
  both, and a routine of BN4's engine a Navi copied (the sprite step,
  FireMan's copy) likewise. So the Navi chips' units install after BN4's
  Navis (`xchips_bn4_navi_install`), and in a core without them a Navi
  chip brings nobody.
- **His sprite, art and sounds are filled as the chip is used.** A BN4
  Navi's battle sprite, his attacks' art and his pool's songs are in the
  core's copy only for the battle he fights in ([BN4's Navis in BN6's
  engine](bn4-navis.md), Battle sprites by battle). His chip is used in
  any battle, against another BN4 Navi too, so the chips have rooms of
  their own (`src/emu/xnavi_bn4_chip.c`): `FREE_BN4_CHIP_SPRITE` (60 KB,
  GutsMan's 60,988 bytes the largest of the moved Navis') and
  `FREE_BN4_CHIP_ART`, the upper half of the battle's art room, laid out
  as the battle's is. Before each frame the engine looks for a BN4 Navi
  chip's object past its turn's wait; thirty frames before BN6 spawns the
  Navi it copies his sprite, the art the battle's Navi does not list
  already and his songs from the player's ROM, points his list entries
  there, and points the pool's slots at his songs while the object lives
  (`xnavi_bn4_sounds_chip`), the battle's own after it. Where the
  battle's rooms hold him already nothing is copied. What each room holds
  is three bytes in the core's copy (`XN_BN4_HELD`), and a battle's fill
  points its Navi's list 2 entry at the battle's room again.
- **Its sound of its own**, a chip's Navi as he appears (BN4's 0xB0),
  is TimeBomb's bomb set down too: one song, one slot of the grown song
  table, 0x1E2, whichever unit copies it. The Navi chips' range
  (0x2C0-0x2FF; each session that moves units takes its slots from a
  range of its own) holds the sounds no other unit plays: none yet.
- **GutPnch1-3 are one of them** (12 November 2026: ids 668-670, BN4's
  support 0x36 at BN6's grown Navi routine 0xB6). The chip is no Navi's
  by its class, but its routine spawns a Navi chip's object (the fourth
  kind's 0x64, GutsMan's object state for state) whose Navi is a GutsMan
  of the chip's own (the first kind's 0x42: he appears on MegaMan's
  panel and punches the panel ahead, 100, 140 or 180, the foe struck and
  pushed back a panel). So it is a unit of kind `navi` with no code of
  the tool's changed, where the support kind had left 19 decisions open
  (MegaMan's stepping out, the HUD's pieces hidden and shown: each
  engine's own, in BN6's object already). Two things were its own. BN4's
  object plays 0xB0 as it steps MegaMan in, which BN6's one Navi object
  does not: the unit carries the sound's slot (`XChipUnit.step_in`) and
  the installer aims the word of BN6's object's states that names its
  last state at a stub in the unit's home, which plays it for this
  unit's number and is BN6's own state for every other chip. And the
  punch's bits are BN6's for struck and pushed in the copy (0x61, as the
  Moko's). `bn4_chip_gutpnch1` to `3` pass over 300 frames (a 140 HP
  Mettaur left 40, one of 250 left 110 and 70, each pushed from (4,2) to
  (5,2)), six mutations caught. Not reached: the frame of the punch
  within the time freeze, which no fight frame counts (a mutation of it
  passed and was replaced).
- **RollAro1-3 too** (12 November 2026: ids 665-667, BN4's support 0x35
  at BN6's grown Navi routine 0xB5). Its object (the fourth kind's 0x63)
  is GutPnch's without the sound, and its Navi is a Roll of the chip's
  own (the first kind's 0x41, the moved Roll's sprite) who appears and
  looses an arrow down MegaMan's row: 50, 70 or 90. Her spawner takes
  her side in r3, which BN4's object hands it and BN6's Navi object does
  not (its r3 is the routine's own address), so the unit's entry is a
  wrapper in its home that loads it from the user BN6 hands in r5
  (`XChipUnit.side`). `bn4_chip_rollaro1` to `3` pass over 300 frames (a
  140 HP Mettaur left 90, 70 and 50), five mutations caught. Not
  reached: the arrow's kind and height, which only show inside the time
  freeze (two mutations of them passed and were replaced).
- **What the differential sees.** The chip runs inside its time freeze,
  so the traces meet at what it leaves: the foe's HP, the panels, the
  frame's sounds, MegaMan's state. The comparator reads BN4's support
  state as BN6's Navi family for such a chip, and BN4's object as BN6's
  type 0x10 ([The differential](../devtools/differential.md), A BN4
  chip's spec). The logs watch the chip's Navi's code and the Navi's
  whose objects he runs, so a draw his attack makes is replayed from
  BN4's. **Not compared:** the freeze's own length and what is drawn in
  it. BN6's object gives every Navi BN4's GutsMan's waits; a BN4 object
  that waits otherwise (WoodMan's: 10 and 20 frames where BN6's waits 30)
  would differ there, which the tool says before its unit is written.
- **SP and DS.** An SP's and a DS's power is one BN4 computes (1002 and
  1024 for GutsMan's: BN4's table, to move as BN5's did). A DS's is read
  from the Hole panels on the field (the dark chips' session's reading,
  `0x08019518`: the Holes on both sides, ten at most, pick a column of a
  row of the table `0x08018B48` by the power less 1023; none gives the
  row's last value, GutsMnDS 100), which BN6 has no panel of yet, and a
  DS is a dark chip, whose cost is in `dev` since PR #351. Until the
  powers are moved both rows stay pending and the unit serves the V1
  alone.
- **Not done:** a NetBattle (BN4 is not in the NetPass's set yet, unit
  8): the pool's override and the rooms' record are one core's, as the
  battle's rooms are.

## The design

- **The experience:** the old guard's arsenal. A veteran drafts the
  chips last held in 2004 (CircGun's crosshair, RollAro's arrows,
  GutPnch, the Elem chips, MagBolt, BigHamr, the element columns) in the
  run he already plays, none of them behind a version or a third clear.
- **The dialectic** stays the run's: what you bring against what you
  find. A BN4 chip is a find that asks the folder's question again (its
  codes, its MB, the Program Advance it might complete), on BN4's ground,
  where the viruses and Navis it answers stand.
- **The loop layers:** the session (finds, stock and rewards on BN4's
  areas and from BN4's visitors), the act (a BN4 Navi's round or Guardian
  Data paying his own chip at the version fought), the meta (the
  Library's older Net count grows: variety, never power).
- **The patterns:** hand-made units in a generated run (each chip as BN4
  made it: its codes, rarity, MB and power read from the player's ROM);
  identity by net (what a net gives tells where you are: BN4's ground
  gives BN4's chips, as BN5's territories give BN5's); an honest
  telegraph (a chip that cannot play never shows: it sits out, never
  enters a pool, and a save without BN4's ROM shows "Needs BN4"); meta as
  variety; a bonus with a drawback for the DarkChips, by the flame's
  price.
- **Against [the core loop](../game/core-loop.md):** it serves its fifth
  lack, variety a veteran feels, inside actions the run has (no walk, no
  screen, no battle and no line added: a chip's name and description are
  BN4's, read from the ROM). Its risk is **complexity over depth**: a
  pool of some 530 chips across three games dilutes the draft, a code or
  a Program Advance harder to complete. The answer is the net's half
  (below): on BN6's ground the draft is BN6's as now, and BN4's chips
  come where BN4 is. No power creep: their rarity is BN6's scale, gated
  by the act's tier as BN6's and BN5's are.

### Which chips join

All 191 and BN4's 6 of BN5's unregistered names but BugCharg, each as
its code is proved, one chip per name as the registry's rule has it: a
chip the run has by name stays the run's (BN6's, else the registry's),
and BN4's of a name of its own is BN4's. BugCharg reads the NaviCust's
bugs, BN5's of the name stays out for that, and BN4's with it. The
Program Advances join with their parts. A chip that sits out (its code
not moved, or its ROM not read) is registered (its id fixed, its
placeholder kept) and never enters a run's pools.

### How they enter a run

- **BN4's visitors' rewards:** `xchips_reward_bn4` gives the registry's
  id where BN4's chip plays, the stand-in's 200 zenny where it sits out,
  as BN5's visitors' rewards do. In the six moved families' rows, 66 of
  the 210 entries above 3/8 HP are chips (a scratch tally): about a third
  of a visitor's picks become a chip where they paid 200 zenny.
- **BN4's Navis:** a round of the tournament against a BN4 Navi pays his
  own Navi chip at the version fought, and his Guardian Data where he
  guards, replacing call 7's stand-in once his chip plays; a Navi beaten
  with dark power gives his DS (Phase 2's rule, [Battle Network 4 in a
  run](bn4.md), What players said).
- **The finds and the stock:** BN4's catalogue (its chips that play,
  never a DarkChip nor a Program Advance) is the older Net's half of the
  finds on BN4's areas, as BN5's catalogue is on BN5's ([One net](one-net.md),
  the older net's half): the Net Dealers' stock, Mystery Data, the vaults',
  Servers' and gift's picks, a trader's list half each rarity's. In the
  deep, the older Net's half draws from the catalogues of the games
  present.
- **AsterLand:** unchanged. Its Chip Trader trades from BN6's own list,
  as the town is BN6's ([AsterLand and the Cyber
  Academy](../world/home/asterland-and-academy.md)).
- **The DarkChips:** BN4's five by BN6's flame on BN4's areas, at BN6's
  price (20 max HP a battle its dark power ran in), as BN4's dark rule
  (its karma) is not taken ([DarkChips](../game/meta/darkchips.md)).
- **The Library:** the profile's Library to 1023; BN4's chips counted in
  the older Net's ("those held of those a run can find now") and listed
  on the second screen under it, BN6's own Library screen by its own
  numbers (its Mega, Giga and P.A. tabs where they have room, as BN5's).

### Rarity and codes

As BN4's records have them (above): the rarity a tier as BN6's, the
codes BN4's (three or four, \* in 38 of 100 standard chips), the MB and
the power BN4's. A trader picks a code the pack lacks three times in four
as for BN6's ([What BN6 does](../game/economy/bn6.md)); a dealer prices by
the tier as BN5's chips are priced.

### The economy's bands

(As built, Where things stand: the four bands of what a run is handed
hold, and the codes held rise by one on BN4's ground, inside their band
for a run a seed makes.) No band moves. Act by act a run finds as many chips as now; what changes
is whose: on BN4's areas the older Net's half is BN4's, whose rarity is
BN6's within a few points (above), so the good chips' share stays inside
its fifth. The visitors' rewards shift a little from zenny to chips (a
third of a BN4 visitor's picks, about 60 zenny less and 0.3 chips more a
BN4 visitor) and the tournament pays a Navi chip for a Navi chip. `build.py
economy` does not see visitors (its runs install no families): the unit
that brings BN4's chips into runs adds them to the report or tallies them
apart, before and after, as 5 November's tally did ([Battle Network 4 in a
run](bn4.md), Its viruses' rewards).

### Online

The rule today is the NetPass's ([The NetPass](../online/netpass.md)):
the Arena, trades and the DeepNet dive need the full set of ROMs (BN6's
and BN5's), so no pair ever lacks a chip the other holds. "A future game
is one entry there", its stamps open again until a player has it in. BN4
is not in the set: its viruses, areas and Navis touch no collection, so
nothing of BN4's crossed. Its chips will.

**Built** (unit 8, above): the owner chose the first way, BN4 in the
full set now, with protocol v17. What follows is the choice as it was put.

Two ways keep a pair whole. BN4 joins the full set (the NetPass's way: no
rule to learn, but every player without BN4's ROM loses the Arena and
trades until he adds it), or **BN4's chips stay home**: they sit out of
every NetBattle on both games' pair cores (the deck's sit-out by id,
which both games know from the folder), are left off the spares list and
the trade board, and the NetPass stays BN6's and BN5's. The second is one
rule ("the oldest chips stay home", said once at the Arena's desk), no
ROM's bytes cross, and no player loses a stamp. The NetPass's own
precedent took its full set at BN5's measured 87% of sessions; BN4's
share is not measured yet (0.20.0 is its first release). Recommended:
they stay home until the run-start events show BN4's share near that,
then BN4 joins the set with the notice the NetPass asks for (the owner's
call 1, below).

### EXTRAS

No switch of its own. **Older nets** off already leaves out every other
game's areas, Navis, visitors and finds ([The extras](../game/meta/extras.md)),
and BN4's chips come only through those, so the switch keeps them out
too: a run with it off finds, buys and earns no BN4 chip. A BN4 chip
already in the pack still plays where BN4's ROM is read: the switch
chooses what the net offers, not what MegaMan holds. A switch apart from
BN5's would split the one older Net of call 5.

## The build plan

Each unit ships on its own; each moved chip proved against BN4's own
battle frame by frame, as BN5's were. Sizes are estimates in working days
of one agent.

0. **Pairings, now (half a day).** `pair_bn4` pairs across a class where
   BN6's chip of the name is a folder chip (ten chips), by Navi and
   version (ProtoMan SP and DS BN6's; NumberMan's the registry's), and
   to the registry's own ids (its 36, Z Saver among them), each playing
   where its code is installed. 50 BN4 rewards stop paying the stand-in.
   Proof: unit tests of the pairing with made-up names (no ROM in CI);
   a scratch tally of the six families' rows before and after; `build.py
   pacing`.
1. **Ten-bit chip ids (3 to 5 days, the largest; serves BN5's rows too).**
   Every site above read in bn6f, patched or shown out of a run's reach,
   listed in docs/rom/chips-code.md as issue #82's audit was; the ROM's
   tables of entries re-encoded; the wide rewards' hook at id 511; the
   pack's extension's RAM found and claimed in `src/emu/free_space.h`;
   the engine's own side; `RUN_MAGIC` bumped with the folders' converter,
   the profile's version, the wire's `chip_last` and the protocol's.
   Proof: `CI=1 build.py test` (the registry's rules to 1024; a chip at
   id 600 given, held in the folder and pack, dealt, used, rewarded,
   counted in the Library, traded by `online_trade_settle`, saved and
   continued); the autopilot's seeds 1-12 the same run log for run log
   as the build before (no BN4 chip in them); `build.py lint`.
2. **BN4's registry rows (2 days, beside 1).** Its 194 rows (195 less
   BugCharg) at ids from
   512 (`XREG_PENDING` until their code plays), each record made from
   BN4's in BN6's layout field by field (`xchips_record.c` with BN4's
   +6 to +8 and its kind's numbering read as its icon's), names,
   descriptions, icons and art read from the player's ROM, placeholders
   "Needs BN4" without it. Proof: unit tests (records from made-up BN4
   records, placeholders, ABC places); a chip in the folder editor with
   BN4's picture and description; a run saved with BN4's chips continued
   without BN4's ROM (placeholders, sitting out, MegaMan's line).
3. **The chips' differential against BN4 and the generator's BN4 path (2
   to 3 days, beside 1). Built** (Where things stand: the specs' MegaMan
   mode, then `tools/xchiprecipe_bn4.py` and its first unit). As planned: A chip spec whose reference is BN4's battle
   (`tools/bn4_lab.py`'s, the chip given in BN4's folder and its key byte
   written for BN4's chip gate, `BN4_CHIP_GATE`, as the Souls' were);
   `tools/xchiprecipe.py` reading BN4 (its families as MegaMan's states,
   0x0C's one table for support and Navi chips, the engine base paired
   or ported once, `tools/bn4_twins.py`'s audits); the census written
   out by the tool into `.build/` (numbers and names, never committed).
   Until 1 lands a spec may give its chip one of the 10 free ids.
4. **The six on BN6's code: fallen away** (Where things stand; each
   joins unit 5). As planned: **(1 day, after 2 and 3).** `XREG_RECORD` over
   their twins: `chip_hole`, `chip_drkline`, `chip_timebomb`,
   `chip_lifeaura`, `chip_blakbarr`, `chip_ball` PASS against BN4's own;
   RollSP and RollDS with BN4's computed powers (`chip_rollsp`,
   `chip_rollds`).
5. **The moved units, cheapest first (about 15 to 20 days in all, in
   parallel).** The 48 units in the table's order, each its `chip_*`
   specs PASS, its mutations caught, its row `XREG_MOVED`. Independent
   by family: several agents at once, each a family, the shared base
   first. The Program Advances with their parts (BN4's recipes after
   BN6's and BN5's in the grown list, BN6's recipe first where two are
   the same; `bn4_pa_*` specs; the per-battle bits past id 383 grown in
   1): **built for fourteen** (Where things stand, The Program Advances
   are formed), five waiting on their parts.
6. **The Navi chips of the eight moved Navis (4 to 6 days, after 3).
   Begun** (The Navi chips, above: GutsMan's V1 plays). As planned:
   Their 23 chips on the Navis' copies, SP and DS by BN4's computed
   powers (`chip_gutsman` and on). Then call 7's stand-in steps aside for
   a round whose Navi's chip plays. The other ten Navis' chips follow
   their Navis (Phase 2).
7. **Into runs: built** (Where things stand), but the flame's DarkChips
   and the Navi chips, which wait for their units. As planned: **(1 to 2
   days, after 2; each chip as it plays).** BN4's
   catalogue in the older Net's half on BN4's areas and the deep, the
   flame's DarkChips, the rounds' and Guardian Data's Navi chips, the
   Library, the second screen; `LAYER_MAKE` bumped (the stock's rolls
   change). Proof: the game-design skill first for any number moved;
   `build.py economy` (0 outside the bands, before and after in the
   commit, visitors counted); `build.py pacing`; `build.py test`
   (`LAYER_MAKE_HASH`); autopilot runs from seeds 1-12 with BN6 and BN4
   and with all three, fresh data folders, the run logs read; captures of
   a BN4 chip in a dealer's stock, on a results screen and as a round's
   prize.
8. **Online and EXTRAS: built** (Where things stand; 1 day, after 7). The rule the owner chooses
   (call 1): the sit-out on both pair cores, the spares list's and the
   board's filter, the Arena desk's one line (a words file), or BN4's
   entry in `netpass.c` with the protocol's bump; `test_extras` with
   Older nets off and BN4 read. Proof: `build.py online-pair` with a BN4
   chip in one folder; a trade refused and offered as the rule says.

In parallel: 0 at once; 1, 2 and 3 side by side (other files: the
engine's id sites, the registry and records, the tools and specs); 4, 6
and the units of 5 as soon as 3's path runs, one agent a family; 7 and 8
once 1 and 2 are in. What can ship first: 0 alone, then 1 with 2 and 4
(six chips and the Navi chips' foundation), each later unit in the
release it is proved by.

## The owner's calls

Asked and answered on 9 November 2026:

1. **Online: BN4 joins the NetPass's full set now**, over the
   recommendation to wait. BN4's chips go into NetBattles and trades
   as they are proved, and BN4 Red Sun's ROM becomes part of the
   common ground the Arena and Trade Central ask of a pair. A player
   without BN4's ROM loses them, so the change is announced ahead as the
   NetPass asks (unit 8; [The NetPass](../online/netpass.md)).
2. **Ten-bit chip ids first, for both games**, as recommended: the
   pairings (unit 0) ship before them, and BN5's ShadeMan and BlizzardMan
   chips take the free ids meanwhile.
3. **All of BN4's chips, as each is proved**, as recommended, with a cut
   list kept open for what the statistics show unused.

Asked and answered on 10 November 2026:

4. **BN4's dark chips cost what every dark chip does:** "Same 20 max HP".
   A battle one of BN4's 31 dark chips is used in is a battle a dark
   power ran in, 20 max HP once a battle
   ([DarkChips](../game/meta/darkchips.md), the owner's call of 10
   November 2026). The light chips play for any MegaMan at no price.
   Built 10 November 2026 (Where things stand, The dark chips' price is
   in code).
5. **LifeAura is one chip with BN6's LifeAur:** "One chip". BN4's 201
   pairs with BN6's 182 by alias (`xreg4_alias`, written by
   `tools/xreg_bn4.py`'s `ALIAS`), takes no row, and its id 530 is
   retired (no release holds an id past 511). The two games' barrier
   tables agree: BN4's kind 6 and BN6's kind 9 are both 1 HP behind a 200
   aura for 3000 frames (docs/rom/bn4.md, Barriers and auras). The rows are 193 since:
   174 folder chips and 19 Program Advances.

Asked and answered on 11 November 2026:

6. **BN4's chips' sounds go into BN6's song table grown:** "Grow song
   table". Chosen over the last eight empty slots, 0x43-0x4A, taken from
   BN5's reserve (call 8 of [Battle Network 4 in a run](bn4.md); Team
   ProtoMan's Navis want them), and over the Navis' pool by battle shared
   with the chips a folder holds. A slot for every sound of another game
   from then on (Where things stand).

Asked and answered on 12 November 2026 (unit 7, Where things stand):

9. **BN4's \*s on BN4's ground: kept as its records have them:** "Keep
   BN4's codes". BN4 gives \* to 38 of its 100 standard chips, and to 13
   of the 15 that play today; a find's code is the record's. Measured, a
   run with BN6 and BN4 holds one chip more in its best letter from the
   third act on, inside the band; a run wholly on BN4's ground would hold
   three more by the fourth act, outside it, but no seed makes one. It is
   what BN4's ground gives, a folder that comes together a little sooner
   there, and `build.py economy` fails the plan the day a seed's run
   leaves the band. Chosen over a find on BN4's ground rolling its code
   among the record's letters first, the \* as often as BN6's chips have
   it.
10. **BN4's dark chips stay out of the finds:** "Keep them out". The
    design keeps the five DarkChips to the flame; BN4's other 23 dark rows
    (Hole, DrkLine, the DS Navi chips) cost 20 max HP a battle one is used
    in (call 4, built), and a dealer or a Mystery Data tells no price. None
    enters the catalogue (`xreg4_dark`); they come by the ways that can say
    the price (a flame's words; a DS by a Navi beaten with dark power,
    Phase 2's rule). Chosen over letting them in as any chip, the card's
    description the only warning.

7. **Marking, the four Plus chips and BlakWeap are moved, not cut:**
   "they are just temporary buffs for elements in battle", "we need
   them" (the owner, 11 November 2026, reversing a "cut" of the same
   day). The question had recommended cutting the six (Marking 556,
   FirePlus, ThunPlus, AquaPowr and WoodPowr 563-566, BlakWeap 602) as
   chips BN4's own players never held. **What was misread:** their
   records (281, 285-288, 289) hold no Library number (+0x1C is 0) and
   one address for the picture, BN4's blank; that says BN4's Library
   does not list them, not that nobody plays them. Their icon is the
   one seventy of BN4's chips share, the blank picture stands in some
   ninety records, and their code is whole: Marking sets 600 frames on
   a per-side record of BN4's (`0x020374B0`), a Plus chip a 480-frame
   timer of its element in the user's battle data and clears the other
   three's (support 0x4C, the element its record's +0x10). Whether a
   shop, trader or reward of BN4's names them was never searched, so
   "never handed out" was an assumption, and the goal is every chip of
   every game ([what a run lacks](gaps.md)). So their ids and rows stay
   (193 rows), and they are moved as every unit is: what reads the
   timers moved with them, a spec where the buff shows, its mutations.
   How they show where BN4 gives them no number and no picture (the
   Library's page, a reward's card) is the one thing left to choose.
12. **AquaPowr, WoodPowr and ThunPlus are left out; FirePlus is moved:**
    "Leave them out" (the owner, 11 November 2026, to the session that
    measured them). Asked because the three do less than call 7 took
    them for. **Verified** in BN4 Red Sun
    (USA, `B4WE`), two ways. By experiment, BN4's own battle run alone
    by the differential (control specs, the guest core): the Plus chip
    used at fight frame 30, a chip of its element at frame 200, inside
    its 480 frames, against a Mettaur of 250 HP, and the same chip with
    no Plus chip before it. AquaPowr then Bubbler takes 50 and Bubbler
    alone 50; WoodPowr then Boomer1 60 and 60; ThunPlus then Thunder1 40
    and 40. FirePlus then HeatShot takes 80 where HeatShot alone takes
    40, and that is the grass FirePlus lays (every panel of the field;
    fire on grass adds its damage again, `0x08012CF2`), no rule of the
    timer. By the code: the chips' object (the fourth kind's 0x7F,
    `0x080EACDC`) sets its element's word of four in the user's battle
    data (+0x3C to +0x42) to 480 and clears the other three; the
    player's tick (`0x08022E3C`) counts them down; and the one reader
    found is the HUD's (`0x08014CC0`: the first word not 0 picks an
    icon's tiles for OBJ VRAM `0x06017480`). Searched: every caller of
    the routine that finds that data by side (`0x0800BFF4`, 26), every
    halfword read or written at those offsets after a load of the
    object's +0x58, every indexed access from 0x3C in
    `0x08007000`-`0x08030000`. Not tried: the buster and the charge shot
    under a timer, a foe's attack against MegaMan under one, a
    NetBattle. So the three are not moved: no unit, their rows left
    out of play and their ids (564 to 566) kept retired, as BugChain's
    is until its call, and nothing of a run hands them out or lists
    them. FirePlus (563) is moved for what it does to a battle, the
    grass; its timer is kept nowhere in BN6 and its HUD icon is not
    moved (Where things stand, FirePlus). Call 7 stands for the other three of its six: FirePlus,
    Marking and BlakWeap. **Two answers were given the same evening.**
    Through the coordinating session the owner was first quoted with
    "Move as BN4 has them", on a recommendation to move the three and
    keep them out of the reward pools; asked directly afterwards, with
    the measurement before him, he answered "Leave them out", and
    confirmed it the same day (11 November 2026) when asked which of
    the two stands.
13. **Marking is left out:** "Leave it out" (the owner, 12 November
    2026). Asked because it does nothing in a plain battle. **Verified**
    in BN4 Red Sun (USA, `B4WE`), two ways. By experiment, BN4's own
    battle run alone by the differential (control specs): Marking used
    at fight frame 30 against a Mettaur of 250 HP, and the same battle
    without it. MegaMan takes the same hits at the same frames (200 at
    t89, t227 and t365), LEFT and UP move him as without it, and no
    object appears; nothing in the trace differs but his misc state for
    a frame. By the code: its routine (`0x08022F5A`, 16 bytes) sets 600
    at the side's record's +0x16 (`0x020374B0` + 0x1B0 a side), and the
    player's tick (`0x08022E3C`) counts that record's timers only where
    its first byte is not 0 (`0x08022E62`), which it is in a plain
    battle. Where it counts, each frame gives the user's collision data
    a status (its +0x1C 2 and flag 8, `0x0800B018`) whose tick
    (`0x0800AF62`) spawns the mark over an object's head, the fourth
    kind's 0x06. What sets the record's first byte, which mode of BN4's,
    was not searched. So Marking is not moved: no unit, its row out of
    play and its id (556) kept, as the three Plus chips of call 12.
    Call 7 stands for FirePlus and BlakWeap.

Open:

8. **BN4's Hole panel: moved into BN6's engine, or left out?** Asked 12
   November 2026. Hole, DrkLine and DrkStage make it, and a DS Navi
   chip's power counts it (Where things stand, The dark units, read):
   from its floor with no Hole to up to double with ten. BN6 has no
   such panel. (a) **Moved**: a panel type of the engine's own at BN6's
   free 15, plain's flags, BN4's tiles and colours copied as BN5's lava
   and sea were, counted by BN4's own routine moved with the DS chips'
   powers. Several days (every background palette is in use), and the
   three panel chips and the DS chips then play as BN4 made them: a
   battle's 20 max HP buys Holes, the Holes buy power. (b) **Left out**:
   a DS chip always hits at its floor (the no-Hole column), Hole and
   DrkLine are cut (they would only make panels plain at 20 max HP), and
   DrkStage is BN4's without its Holes: poison on the other side.
   Recommended: (a), as the DS chips are 17 of the 28 dark rows and
   their powers hinge on it; until it is built the DS chips can be
   proved against BN4's battle with no Hole on the field and play at
   their floor, which (b) would make final. Until answered Hole, DrkLine
   and DrkStage sit out.
11. **BugChain: cut, or its bugs mapped onto BN6's?** Asked 11 November
    2026. By its code (read in the ROM: its object's routine `0x080E668E`)
    BugChain copies seven bytes from one side's battle Navi stats to the
    other's, the NaviCust bugs a side fights under (BN4's record, 0x40
    bytes a side at `0x0203BEC0`, through `0x0800D742`): one side's bugs
    become both sides'. Only a Navi a player steers reads that record, so
    against viruses and the run's guardians the chip changes nothing that
    was found (their code reads none of it: assumed from the readers met,
    not searched to the end); it is a NetBattle chip. BN6 keeps the same
    record in another layout (0x64 bytes a side at `0x0203CE00`,
    `0x08013682`) and its own set of bugs, so moving it means pairing
    each of the seven fields with BN6's by its reader in both games, and
    deciding what a bug BN6 has none of becomes. Recommended: cut it from
    runs' pools and rewards (its row left out of play, id 587 kept
    retired) and take it up with the Arena, where a second player's
    record exists. Until answered it sits out.

Decided here by the data and the standing rules, not asked: no EXTRAS
switch of BN4's own (call 5's one older Net); BN4's DarkChips by BN6's
flame (BN4's karma not taken); BugCharg out (BN5's rule for it); BN4's
rarity and codes as its records hold them (BN6's scale, measured).

## How the census was made

Scratch Python over the three ROMs (in the session's scratchpad, not
kept): names read through each game's text archives
(`BN4_CHIP_NAMES`, BN5's `BN5_CHIP_NAMES_LOW`, BN6's `RomLayout`
`chip_names`) and their punctuation mapped; records through
`BN4_CHIP_RECORDS`, `BN5_CHIP_RECORDS` and BN6's; the registry's rows
parsed from `src/core/xchips_registry.c`; the code split by the families
the name pairs give and, for the rest, `tools/xrecipe_bn4.py`'s
`Pairs4` index of BN6's routine bodies with a walk (`xrecipe_thumb.py`)
from each family's state through its subfamily table and the objects it
spawns; the nine-bit sites by a scan of BN6's Thumb for the shift pairs,
the literal and the shift by 9. Unit 3 makes it a tool. What was not
done: the differential (every "on BN6's code" above is a candidate the
specs confirm), the generator's audits (the code sizes are upper bounds),
a reading of the nine-bit sites (counted, not read), and the PA checker's
trace (`BN4_PA_LIST_REF`'s reader assumed).
