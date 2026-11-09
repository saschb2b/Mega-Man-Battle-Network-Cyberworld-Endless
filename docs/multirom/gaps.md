---
type: Status
title: "What is not ported yet"
description: "The audit of 9 November 2026: what BN5 Team Colonel's ROM holds that a run does not use yet, kind by kind (areas, families, Navis, Souls, DarkChips, chips and Program Advances, NaviCust programs, Liberation, songs, bystanders and towns, finds), each verified and sized, BN4 Red Sun's open phases beside them, and a ranked list for a content-filler version."
tags: [multirom, bn5, bn4, status]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-09T01:02:00Z }
sources:
  - id: bn5-rom
    resource: "Mega Man Battle Network 5: Team Colonel (USA), SHA-1 5f472f78d8de2df01d5039e045c043cb40969a39, read through tools/xrecipe.py's readers by scratch scripts (not in git)"
    title: "The owner's BN5 Team Colonel ROM"
  - id: registry
    resource: "src/core/xchips_registry.c (its rows and its header comment)"
    title: "The chip registry"
  - id: kb
    resource: "docs/multirom/, docs/game/liberation/, docs/game/core-loop.md, docs/rom/bn5-world.md, docs/rom/bn5-battles.md, CHANGELOG.md 0.20.0"
    title: "The knowledge base as of dev at 3a4309e1"
---

# What is not ported yet

The audit for the version after 0.20.0, which the owner wants to be a
content filler: what Battle Network 5 Team Colonel (USA) has that a run
does not use, kind by kind, then BN4 Red Sun's open phases, then a
ranked list. Every claim names how it was checked: a ROM table read (the
readers of `tools/xrecipe.py` on the owner's ROM, from scratch scripts
that write nothing to git), the code that reads it, or the concept that
records it ([Status](status.md) is the checklist this audit extends). A
negative claim names the search that came up empty. Sizes: **S** a day
or less (data, wiring, a trace), **M** a few days (one recipe or chip
unit proved by `build.py xdiff`, or one generator feature), **L** a week
or more (several M, or a system). Each candidate is checked against [the
core loop](../game/core-loop.md): its lacks (pace, a climax that bites,
variety a veteran feels), what it froze (power meta, any change to BN6's
battle) and the owner's calls.

BN5 is the main path: 147 of 166 release runs carried it in October (the
core loop's statistics), so a gap on its areas reaches nine runs in ten.

## Net areas

All six of BN5's net areas dress BN6's (ACDC, Oran, SciLab, End Area, the
Undernet, Nebula Area: [Status](status.md)), with their themes, battle
music, backgrounds, bystanders and map objects. What is left:

| Gap | Verified how | What it takes | What it adds |
| --- | --- | --- | --- |
| **A BN5 layer laid out as its BN6 area's would be**: no stairs or raised rooms, arrow lanes, teleports' gem or Net Dealer's counter on BN5's layers | Status's open row: `netmap_kit` takes the kit of the area it draws in, and BN5's learned maps give none of those pieces | **M**: a kit learned per BN5 area from its own maps (Oran Area's arrow panels and planks lend no tiles today), `LAYER_MAKE` bumped, `build.py tiles` and a walk measure per area | The set pieces BN6's layers have, on nine runs in ten. Lack 5 (variety); must not lengthen the walk (lack 1: the [generator](../world/level-design/generator.md)'s gate) |
| **BN5's comps and homepages** (net groups 0x80-0x8F) dress none of BN6's comp and homepage layers | Status lists the six areas only; the atlas (`--atlas DIR:x0`) draws the groups from 0x80, so their maps are read | **M** per look: learned and tuned as an area's tiles are, under the atlas | "The same town, comp and guardian four runs running" (lack 5) |

## Virus families

None left. Every family BN5 has and BN6 lacks is moved and proved (25:
[Transplanted families](families.md), BN5's families, all of them). Tuby
and the unnamed AI 31-33 are left out because no battle record of BN5's
names them (all 386 of its enemy lists read; AI 31 and 32 have no ids).

## Navis

BN5's Navi ids are its id table's type 1 (`0x08014C8C`, 0x191 ids), an
AI each with six versions (V1, V2, V3, SP, DS, a blank), named by the
archive at `0x0873C978`. Read for this audit: every AI from 1 to 22 has
its own state table in BN5's Navi tables (`0x080F2374`, each pointing
past the generic states 0-5 into its own routines), so each Navi's
battle code is in Team Colonel's ROM. Which the story fights: a byte
scan of the 0x69 story records' enemy lists (`0x08113CD8`, 48 bytes read
past each pointer, so a record's number is approximate and the ids'
presence is what counts) for each Navi's ids.

| AI | Navi | In the story's records | In a run | Gap |
| --- | --- | --- | --- | --- |
| 1-6 | Team ProtoMan's six: ProtoMan, GyroMan, SearchMan, NapalmMan, MagnetMan, Meddy | V1, V2 and DS of each | No | **L**: six recipes by `tools/xnavirecipe.py`'s walk (M each, their objects to sort as the six's were); the owner's call (below) |
| 7-12 | Team Colonel's six | V1, V2, V3 and DS | Guardians of BN5's areas, V1 to SP by band | DS left out by design ([Transplanted Navis](navis.md)) |
| 13-14 | ShadeMan, BlizzardMan | V2, V3 | Dark layers' Darkloids ([Stage 4](../game/liberation/building/stage-4-darkloids.md)) | Their chips (Chips, below) |
| 15-16 | CloudMan, CosmoMan | V2, V3 | No: derived, not moved | **M**: three generator decisions open (their object types, CloudMan's RAM table at `0x0203CA30`, a sprite loaded by a number made otherwise); then proved as BlizzardMan was |
| 17-18 | LarkMan, GridMan | V1, V2, V3 | No | **M** each: recipes for AI 17 and 18; where BN5's net records set their SP versions is the first step, as it placed the six ([BN5's Navis](../game/bosses/bn5-navis.md)) |
| 19 | Bass (V1-V3, SP, DS, BassXX) | V1-V3 | No (BN6's own Bass is the super boss) | **M-L**: left for later where BN5 dresses the Graveyard ([Super bosses](../game/bosses/super-bosses.md)) |
| 20 | Nebula Grey | Its two records, music 0x17 | No | **L**: BN5's final boss; weighed and left out with its theme ([BN5's music in a run](music.md)) |
| 22 | The Dark Soul | | Chaos Unison's failure | None |
| type 2, 23 | DarkMegaMan (`0x18F`), mission 4's Darkloid | | No | **M**: on BN5's chip AI, which the Dark Soul's move brought; left by Stage 4 |

## Souls

Team Colonel's six are whole, Chaos Unison and BN5's mood with them
([Transplanted Souls](souls.md), S0-S7). Team ProtoMan's six are not
offered. Read for this audit: BN5's form tables hold rows for forms 1-6
as for 7-12, each its own loadout (`0x0801CA18`, six bytes), its
element (`0x0800E62C`: form 4 Fire, form 5 Elec) and its passives'
entry (`0x08011BA4`); forms 2 and 9 alone have enter and leave routines.
Their sprites, charged shots and code paths were not checked, and that
Team Colonel never lets MegaMan unite with them is assumed (version
exclusives; to verify in [Sources](../research/sources.md)' references).
BN4's Blue Moon Souls run in Red Sun's ROM the same way
([BN4's Souls](bn4-souls.md)), which makes it likely. **L**: `tools/xsoul.py`
over forms 1-6, after their Navis (above), whose defeat gives a Soul in a
run. The owner's call (below).

## DarkChips

None left: all twelve kinds one chip each, BN5's own seven by its rule
([Status](status.md); [The chips' code](chips-code.md), Their rules in
place). The DS Navi chips of the six play behind BN5's dark gate. The
Darkloids' DS chips are dark-gated too and wait with their chips.

## Chips and Program Advances

The registry holds 257 of BN5's 424 chip ids; all 257 play (no row is
`XREG_PENDING`). Of the rest, 105 have a name in BN5's archive (read for
this audit: the names at `BN5_CHIP_NAMES_LOW`/`HIGH`, the records at
`BN5_CHIP_RECORDS`, against the registry's rows):

| Group | Ids (BN5's) | Verified how | What it takes |
| --- | --- | --- | --- |
| The Darkloids' and Nebula's Navi chips: ShadeMan, BlizMan, CloudMan, CosmoMan, LarkMan, GridMan, Django, each V1, SP and DS | 260-280 | Their records: Mega, attack family 0x41 (the Navi chips'), subfamilies 0x01, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13; the SP's power computed (1014-1019, 1022), the DS dark-gated (+0x15 = 2) | A chip unit each, as NumbrMan's (`xchips_moved.c`): **M** for ShadeMan's and BlizzardMan's (their Navis' objects are moved already), more for the others; Django has no Navi in BN5's id table (its chip's code alone) |
| Team ProtoMan's Navi chips, Roll's, GunDelEX, Otenko | 215, 219, 221-241 | The registry's header leaves out "Team ProtoMan's version chips"; their records family 0x41, subfamilies 0x02, 0x03, 0x06-0x09, 0x19 | With Team ProtoMan's Navis, if ever |
| Program Advances: BigNoise, CsmoPris, WildBird, Football, PileDrvr | 343-349 | BN5's list of recipes (`0x08028020`): each is two of a standard chip and a Navi chip's SP or DS (Pulsar3 and ShadeMan's, Astroid3 and CosmoMan's, Katana1 and LarkMan's, MrkCan2 and GridMan's, GunDelS3 and Django's) | **S** each once its Navi chip is registered: BN6's list grows with BN5's recipes already (`xchips_pas.c`); its code is the Navi chip family's |
| Gigas: FinalGun, Bass, DeltaRay, BugCurse, MetrKnuk, OmegaRkt, BugCharg, Phoenix, DethPhnx | 290, 301-305, 310, 313-314 | The registry's header names BugCharg, Phoenix and DethPhnx out; which of the others Team Colonel hands out, and how, is not traced (assumed version or post-game chips) | Traced first (**S**), then a unit each |
| Pseudo-chips no folder holds: the Mode chips and their like, BatCan1-4, the capsules and other copies, the Liberation teammates' chips (C-Cannon, T-Swing, S-Melody, KCrusher, NumTrap, DieBomb, SplitUp, Team ProtoMan's seven) | 281-289, 315-318, 369-388, 401-422 | Library type 3 or codes A, B, C and * alike; read by name, what each runs not traced (Soul charged shots and Liberation pieces, assumed) | Only 401-422 are felt: Liberation, below |

Two things bound this work:

- **Ids.** BN6's folder entry is a chip and its code in sixteen bits
  (chip | code << 9), so ids end at 511. The registry leaves ten free
  (365-367, 505-511: [The chip registry](chip-registry.md)). ShadeMan's
  and BlizzardMan's six fit; the other fifteen Navi chips and five
  Program Advances do not, nor BN4's chips (Phase 4, being designed now).
  Whatever answers BN4's past 512 answers these too: the two pieces of
  work share one decision.
- **Formed or not.** InfVulc2-3 and Boxer1-3 never form under BN6's codes
  and recipes, by decision ([The chips' code](chips-code.md)); not a gap.

## NaviCust programs

Nothing of BN5's NaviCust is in a run or the project's map (no row in
`docs/rom/navicust.md` or `docs/rom/bn5-*.md`, no name in `bn5.h`).
Located for this audit: BN5's programs' names in its text beside BN6's
(BN5 near `0x75A77E`, BN6 near `0x73CA83`, found by encoding AirShoes,
UnderSht and BugStop), decoded and compared. Seven are BN5's alone:
**Chivalry, AutoRun, HubBatc, SoulClen, MegaVirs, GigaVirs and SoulT+1**
(BN6's alone: ChpShufl, NumbrOpn, Poem, SlipRunr, FldrPak1-2). Their
records, shapes and effects are not located; that SoulT+1 lengthens a
Soul's turns and MegaVirs and GigaVirs harden viruses is assumed from
the names. **S** to locate the records and trace each in romlab, then
**M** each that needs an effect BN6's code lacks (BN6's NaviCust table
grown, the effect hooked). These are the run's own draft and the
[BugFrag workshop](../game/navicust.md)'s stock: horizontal choices
within a run, never power across runs (the owner's call on the buster).
The new offsets get their `docs/rom/` rows and `bn5.h` names when the work
starts.

## Liberation Missions

Built after BN5 ([Liberation Missions](../game/liberation/overview.md)):
the board, every panel kind BN5 places, its items (hearts, Order Points,
keys, chips, zenny), traps, the BonusPanel's roulette, the guardians'
strikes and returns, Team Colonel's commands, BN6's Cross Navis with
Team ProtoMan's commands, the par's tiers, the music and each mission's
look. Left, from the stages' own "Left for later":

| Gap | Where it is recorded | Size |
| --- | --- | --- |
| CloudMan and CosmoMan as Darkloids (acts 4 and on now take BlizzardMan or ShadeMan) | Stage 4 | **M** (Navis, above) |
| The par's chips as BN5's: the Darkloid's SP two under par, his V1 at par. The run pays a Server's chip and one a tier lower, reasoned when the Darkloid was a BN6 Navi | [The par's prize](../game/liberation/building/par-prize.md) | **M**: his chips registered and moved (Chips, above), `lib_prizes.c` |
| Each teammate's own chip on his Custom screen (Colonel's C-Cannon and the rest, BN5's ids 401-422) and his own folder | [Stage 3](../game/liberation/building/stage-3-team.md) | **M**: pseudo-chips traced and moved; ids (above) |
| Colonel's Save and TwinLiberation not traced; SneakAttack on the Darkloid; BombLiberation onto BonusPanels | Stage 3; [The Cross Navis](../game/liberation/building/cross-navis-team.md) | **S** each to trace, **S-M** to build |
| DarkMegaMan, mission 4's Darkloid | Stage 4 | **M** |
| Acts 5 and 6 tuned for their teams (the owner's decision 5); an AreaGrab freezing a Surrounded battle | Stage 3; The Cross Navis | **M**; a bug, **S** |

## Songs

A run plays BN5's 0x13 and 0x14 (its areas' themes), 0x15 and 0x16
(battle and boss), 0x18, 0x1C and 0x24 (a dark layer's and a guardian's
cues) and 0x22 (the board), and the Jukebox all of them ([BN5's music in
a run](music.md); [the PET](../game/pet.md)). Not in a run, by BN5's
own callers ([BN5's maps, songs, text and
objects](../rom/bn5-world.md), Where BN5 plays its songs):

- the towns' 0x03, 0x04, 0x05 and 0x0C, waiting on a town (below);
- weighed and left out: 0x02, 0x06, 0x0D, 0x0E, 0x17, 0x23;
- started by BN5 and never weighed: 0x19 and 0x1F (a battle's win, 0x19
  where the options' bit 2 is set), 0x10 (its first comp's map list),
  0x01, 0x0B, 0x0F, 0x1A, 0x1B, 0x1D, 0x1E. The win's song after a battle
  on BN5's areas is the felt one: **S**, a hook like the one-turn win's
  (`BN6_WIN_SONG`); 0x10 comes with BN5's comps;
- never started by BN5 itself: 0x07, 0x08, 0x09, 0x11, 0x12, 0x20, 0x21.

## Bystanders and towns

Bystanders: none felt missing. Its HeelNavi and orange Navi stand on its
areas, its dealers are the art BN6's dealer already shares, its Mr.Progs
are BN6's; Nebula Area's HeelNavi dealer is left out on purpose
([BN5's maps, songs, text and objects](../rom/bn5-world.md)).

Towns: none. ACDC Town (0x00:0), Oran Isle (0x02:0), the harbour
(0x03:0), the castle town (0x04:0) and the base's room (0x05:0) are
drawn by the atlas and described in [Status](status.md) (Its towns), with
what each would take. **L**: ACDC Town's 256 colours in a range of their
own, its map copied whole with walls and heights, jack-in points and the
buildings' checks; the others as Seaside and Green Town came. Where a
BN5 town stands in a run is the owner's call (below).

## Mystery Data and shop stock

A run reads BN5's battlefield Mystery Data rows (`BN5_FIND_ROWS`) for its
own records; on its areas half the finds and the dealers' stock are BN5's
chips, drawn from the chip pool by tier (`loot.c`'s older share,
[One net](one-net.md)). BN5's own per-area Mystery Data contents and its
dealers' lists are not read (no name in `bn5.h`, no row in `docs/rom/`).
Not recommended: the run's finds follow the economy's plan and bands by
act ([The economy](../game/economy/overview.md)), which BN5's own tables
would fight. More of BN5's chips reach these pools only by the Chips
section above.

## Anything else a BN5 player would miss

- **Team ProtoMan**: Navis, Souls, chips, the version as a whole (open in
  [Status](status.md)); its six's commands already serve BN6's Crosses on
  the board.
- **The DS Navis** (version 4 of each), BN5's post-game: left out by
  design. Their chip AI is moved (the Dark Soul's), so they would cost an
  M each if the design changed; not proposed.

## BN4 Red Sun's gaps

Listed with sizes, not redesigned ([Battle Network 4 in a run](bn4.md),
The plan; [BN4's Navis](bn4-navis.md); [BN4's Souls](bn4-souls.md)):

| Gap | State | Size |
| --- | --- | --- |
| Phase 4, its chips (about 225 names neither game has, Navi chips, Program Advances, Gigas, DarkChips; the registry past 512) | Being designed by another agent | **L** |
| Its remaining Navis: LaserMan (opens the second build), MetalMan, JunkMan, SparkMan, TopMan, ColdMan, BurnerMan, VideoMan, KendoMan, Duo | 8 of 18 moved (GutsMan, ThunderMan, WindMan, FireMan, SearchMan, AquaMan, WoodMan, Roll) | **M** each, LaserMan and Duo more; **L** together |
| Its moved Navis as guardians of its own areas | The eight fight only in the tournament (past the Nest, act 3 in half the runs); a BN4-dressed act ends at a BN6 guardian ([Status](status.md): no `xguard`) | **S-M**: where its records set each, the band's HP, a BN6 Navi chip as his Guardian Data until Phase 4 |
| Its remaining families: VolGear, Gaia, Boomer, MagTect, Dharma, Kilby, Weather, Spidy, Moloko, and Black Earth's NavBlack and NavShado | 7 moved (Billy, Spikey, Shrimpy, Walla, Elmperor, CirKill visiting; Melody and Mettaur for Roll's summon) | **M** each; its areas' own random battles wait on them (call 10) |
| Its Souls, all twelve | None built; ThunSoul first, then AquaSoul and GutsSoul | **M** for ThunSoul, **L** for all; which a run gives is the owner's call there |
| Its areas: a layout kit of its own (one-wide ways ending in studs, squares four by four, hubs on spurs, conveyors, exit arrows), Black Earth not drawn, its bystanders BN6's | [Status](status.md), BN4 Red Sun's areas | **M** |
| The Undernet's faint pale marks | Named in this audit's brief; no concept records them yet (a search of `docs/` for "faint" and "pale mark" found none) | **S** |
| Its music: the boss theme's swell and its mixing level | Call 10 | **S** |

## For a content-filler version

Ranked by what a player would feel, on how many runs, for the cost. A
filler adds variety and choices at the same power: none of these is
meta power, and none touches BN6's battle (the core loop's "Not
proposed").

1. **The Darkloids' chips and BN5's par prize** (**M**). ShadeMan's and
   BlizzardMan's V1, SP and DS registered in the ten free ids, their two
   Navi chip units moved and proved, the dark layer's par paying his SP
   two under par and his V1 at par as BN5's did. Depends on nothing
   open. With BN5, two forks in three hold a dark layer: today its win
   pays a stand-in chosen when the Darkloid was a BN6 Navi.
2. **CloudMan and CosmoMan** (**M**). The three decisions taken, both
   moved and proved; acts 4 and on get their own Darkloids instead of
   repeating the first two. Their chips follow the id decision.
3. **BN4's Navis guard BN4's areas** (**S-M**). Eight Navis already
   proved meet a player only in a tournament past the Nest; a
   BN4-dressed act ends at a BN6 guardian. Depends on placing each by
   BN4's records; no new transplant.
4. **LarkMan and GridMan as BN5 guardians** (**M** each). A second
   guardian for BN5's areas, which now hold one each (End Area two);
   lack 5's "the same guardian four runs running". Depends on their
   recipes and where BN5's records set their SP; WildBird and Football
   after the ids.
5. **BN5's layers with their area's kit** (**M**). Stairs, arrow lanes,
   teleports and the counter on the layers nine runs in ten walk.
   Depends on the generator's kit per area and a `LAYER_MAKE` bump; held
   to the walk's measure (lack 1).
6. **BN5's comps** (**M**, with BN5's comp theme 0x10). Variety where a
   run's comps repeat. Depends on the tile learning per look.
7. **BN5's NaviCust programs, SoulT+1 first** (**S** to trace, **M** to
   build). New picks for the draft and the BugFrag workshop, SoulT+1
   making the run's Souls a build choice. Depends on locating their
   records; through the game-design skill before numbers.
8. **The teammates' own chips on a dark board** (**M**), with Colonel's
   Save traced (**S**). A teammate's battle as BN5's, his Soul and his
   chip. Depends on tracing the pseudo-chips and on ids.
9. **BN4's remaining families** (**L**, M each). Visitors a quarter as
   repetitive, and the way to BN4's areas fighting their own battles.
10. **ThunSoul** (**M**). The first BN4 Soul, proving the path for the
    eleven others.
11. **BN5's win song on its areas** (**S**). A small cue a BN5 player
    hears after every battle there.
12. **Team ProtoMan's Navis** (**L**), then their Souls (**L**): the
    largest pool left, waiting on the owner's call below.

## Needs the owner's call

- **Team ProtoMan from Team Colonel's ROM.** Their six Navis are in Team
  Colonel's ROM and its story fights them, so as foes they are BN5's own;
  but BN5's ProtoMan beside the run's rival ([The rival](../game/rival.md))
  is two ProtoMen, and their Souls are ones Team Colonel's MegaMan never
  takes. Whether a run may give them, and whether ProtoMan comes at all.
- **Where a BN5 town stands in a run**: home is BN6's town after every
  act ([Home](../world/home/overview.md)); a BN5 town dressing it by the
  seed, a place visited, or none.
