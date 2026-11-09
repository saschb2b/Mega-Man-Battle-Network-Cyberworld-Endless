---
type: Design
title: "Team ProtoMan: foes and Souls"
description: "Team ProtoMan's six from BN5 Team Colonel's ROM as foes whose defeat gives their Souls, designed with the game-design skill, its room built (seats, ids, free space, rivals.sav, Soul bits) and no Navi yet: the five as their partners' second guardian where BN5's own net records place them, their versions by act, their Souls' kinds beside Team Colonel's, ProtoSoul from the rival's netbattle rather than a second ProtoMan, the build order by the ROM's code and the owner's questions."
tags: [game, design, bosses, bn5, souls]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-09T12:20:00Z }
sources:
  - id: owner
    resource: "The owner's call of 9 November 2026, recorded in the gap audit of 9 November 2026 (on the bn5-gaps branch: multirom/gaps.md, The owner's calls)"
    title: "Team ProtoMan from Team Colonel's ROM: as enemies and with their Souls"
    author: human:saschb2b
  - id: bn5-rom
    resource: "Mega Man Battle Network 5: Team Colonel (USA), SHA-1 5f472f78d8de2df01d5039e045c043cb40969a39, read through tools/xrecipe.py's readers by scratch scripts and tools/xnavirecipe.py's derive (nothing written to the tree)"
    title: "The owner's BN5 Team Colonel ROM"
  - id: mmkb-souls
    resource: "https://megaman.miraheze.org/wiki/Double_Soul"
    title: "MMKB, Double Soul: each Soul's chip type, charged shot and abilities"
  - id: finalweapon
    resource: "https://finalweapon.net/2023/04/13/mega-man-battle-network-5-version-differences/"
    title: "Final Weapon, Mega Man Battle Network 5: version differences"
  - id: stats
    resource: "docs/game/core-loop.md, What the statistics say (Umami, 6 to 26 October 2026)"
    title: "The players' statistics"
---

# Team ProtoMan: foes and Souls

The owner's call of 9 November 2026, for the content filler after
0.20.0: Team ProtoMan's six Navis (ProtoMan, GyroMan, SearchMan,
NapalmMan, MagnetMan, Meddy), whose battle code Team Colonel's ROM holds,
come into runs as foes, and beating one gives his Soul, beyond what Team
Colonel's MegaMan ever gets. This is the design, reasoned with the
game-design skill and checked against [the core loop](core-loop.md);
its first unit, the room, is built (The build, below); the Navis are not
yet. Every number below was read from the owner's BN5
ROM (its rows in [BN5's battles, records and rewards](../rom/bn5-battles.md), Team ProtoMan's six)
unless it says otherwise.

## What the ROM holds

Team ProtoMan's six are BN5's Navi AI 1-6, its ids table's type 1, six
versions each, beside Team Colonel's six (AI 7-12) that already guard runs
([BN5's Navis](bosses/bn5-navis.md)). Read on 9 November 2026:

| Navi (AI) | HP: V1, V2, V3, SP, DS | Element | His Soul's kind | His partner in BN5's net records | His code (`xnavirecipe.py`'s walk) |
| --- | --- | --- | --- | --- | --- |
| ProtoMan (1) | 300, 1000, 1600, 1800, 2500 | none | Sword | Colonel, Undernet 3 | 10 blocks, 8648 bytes; 16 states, 8 his own; 119 decisions listed |
| GyroMan (2) | 600, 800, 1400, 1800, 3000 | none | Wind | ShadowMan, Oran Area 1 | 8 blocks, 5812 bytes; 14 states, 6 his own; 78 |
| SearchMan (3) | 800, 1000, 1700, 2000, 2100 | none | Cursor | NumberMan, End Area 3 | 8 blocks, 6862 bytes; 14 states, 7 his own; 85 |
| NapalmMan (4) | 800, 1000, 1600, 2000, 2500 | Fire | Fire | TomahawkMan, SciLab 4 | 10 blocks, 5610 bytes; 12 states, 4 his own; 74 |
| MagnetMan (5) | 500, 800, 1800, 2000, 3000 | Elec | Elec | KnightMan, ACDC Area 1 | 10 blocks, 7352 bytes; 14 states, 6 his own; 100 |
| Meddy (6) | 900, 1100, 1500, 1800, 3000 | none | Recovery | ToadMan, End Area 4 | 10 blocks, 7224 bytes; 13 states, 5 her own; 94 |

- **Their battle code is whole.** Each AI has its own state table in BN5's
  Navi tables and `tools/xnavirecipe.py` derives each as it derived Team
  Colonel's (a slot past the Darkloids' for the walk alone). The
  decisions it lists (calls with no BN6 twin yet, sprites, moved types,
  raw pointers), 74 to 119 a Navi, are of the order it lists for Team
  Colonel's six, which are moved (70 to 107 each): each is one Navi's
  work, as each of theirs was.
- **BN5 places them itself.** Its net maps' battle records hold each
  Navi's SP version, with a story condition, on the map he roams once
  his story is over, and they pair Team ProtoMan's with Team Colonel's:
  KnightMan and MagnetMan in ACDC Area 1, ShadowMan and GyroMan in Oran
  Area 1, TomahawkMan and NapalmMan in SciLab 4, NumberMan and SearchMan
  in End Area 3, ToadMan and Meddy in End Area 4, Colonel and ProtoMan
  in the Undernet 3 (the row that places today's guardians). MMKB's
  boss list pairs them the same way across the two versions.
- **Their Souls are in the ROM, switched off.** BN5's form tables hold
  forms 1-6 as they hold 7-12 (a loadout each, an element: NapalmSoul's
  Fire, MagnetSoul's Elec; a passives entry; form 2, GyroSoul, has enter
  and leave routines as NumberSoul's does), and the UNITE check's table
  of kinds gives each a kind (the table above). The Souls' event flags
  for 1-6 are 0xFF, which this version never sets: Team Colonel's
  MegaMan never unites with them (verified in the ROM; the Final Weapon
  guide lists no Team ProtoMan Navi among Team Colonel's bosses either).
- **Their Navi chips are there too**, V1, EX and SP of each, BN5 ids
  0xE0-0xF1, not in the chip registry ([The chip registry](../multirom/chip-registry.md):
  "not Team ProtoMan's version chips"), and the registry has about ten
  free ids left (the gap audit).
- **ProtoMan's V2 is named DarkProt** in BN5's names archive, as
  Colonel's V2 is DarkCol; no story record of the 0x69 names it (a byte
  scan of their enemy lists, approximate), and what Team Colonel's story
  does with ProtoMan is not read here (assumed, not verified: MMKB's BN5
  page ties ProtoMan's darkening to the other version's story).

## The design

**The experience.** "The older net had two teams, and now I meet the
other one." A veteran of BN5 who played Team Colonel never fought
GyroMan or held NapalmSoul; one who played Team ProtoMan never saw
ShadowMan's. A run makes the two versions one game, which is what
[one net](../multirom/one-net.md) promised: one Battle Network.

**The dialectic** stays the meta layer's, what you bring against what you
find, met at an act's end ([BN5's Navis](bosses/bn5-navis.md)): the Soul
is found, and the folder brought decides what it is worth. Team
ProtoMan's kinds are the folder's common ones (Sword, Fire, Elec), where
Team Colonel's are its thin ones (Invisible, Plus, Obstacle), so the
same guardian layer now asks a different question of the folder in half
the runs.

**The loop layers.** The moment (UNITE on BN6's Custom screen, unchanged),
the act (its guardian), the run (the Souls carried into every battle
after). Nothing reaches the meta loop: no Soul is kept between runs.

**The patterns.** The cross-game catalog's *variety, not power* at the
meta (a guardian swapped for his partner adds no Soul to a run, it
changes which); *parity across the nets* (their band, staging and
Guardian Data BN6's guardians'); *costed power* (each Soul still a chip
given up, once a battle); the *honest telegraph* (the act's card, the
dealer's word and the bystanders name him as they name any guardian);
and *the rival as a relationship* for ProtoMan alone (below).

**What it answers in the review.** Lack 5, "variety a veteran feels":
"the same town, comp and guardian four runs running". BN5 is the main
path (147 of 166 release runs), and its areas hold one guardian each (End
Area two): this doubles every one of them with the partner BN5 itself set
beside him, and adds no guardian, no layer and no walk (lack 1 untouched).

### Where they stand

1. **Each guards his partner's area, by a coin.** Where a run's act has
   a BN5 guardian today (a surface act BN5 dresses, in half the runs by
   its own coin), a second coin of the run's seed picks between Team
   Colonel's Navi of that area and his Team ProtoMan partner: KnightMan
   or MagnetMan in ACDC Area, ShadowMan or GyroMan in Oran Area,
   TomahawkMan or NapalmMan in SciLab, and in End Area one of NumberMan,
   ToadMan, SearchMan and Meddy. Even odds, so each of the eleven Souls
   is as likely as the next.
2. **The deep's pool takes the five too.** Acts 5 and 6 draw BN5's Navis
   from one pool by band ([BN5's Navis](bosses/bn5-navis.md), How often):
   it grows from six to eleven, none who already guarded a surface act of
   the run.
3. **Not in act 1.** The first run is the game for half the players (63 of
   119 started one run only, mostly on a phone), and KnightMan in act 1
   cost a run before he was tuned (the core loop's "a new system meets a
   player gentle first"). The partner coin applies from act 2; act 1's
   BN5 guardian stays Team Colonel's.
4. **Not the tournament, not the Darkloids' seat.** The tournament is
   BN4's climax, its entrants proved by its own records ([The tournament](tournament.md));
   a dark layer's Darkloid gives no Soul by design. BN5's ROM places
   them as guardians of its areas, so that is where they go.

### Their versions by act

Taken by the rule BN5's six follow: the lowest of V1 to V3 whose HP
reaches the act's band, V2 at least from act 4, SP on later cycles, his
HP capped at the band's top as he spawns, never raised but by the Net's
clock. The areas' acts are their partners' (`build.py pacing` measured
the partners there):

| Navi | Act 2 (600-700) | Act 3 (800-1000) | Act 4 (1000-1300) | Act 5 (1100-1500) | Act 6 (1200-2000) |
| --- | --- | --- | --- | --- | --- |
| GyroMan (Oran Area: acts 2, 3) | V1, 600 | V2, 800 | V3, 1400 at 1300 | V3, 1400 | V3, 1400 |
| SearchMan (End Area: acts 1, 2) | V1, 800 at 700 | V1, 800 | V2, 1000 | V3, 1700 at 1500 | V3, 1700 |
| NapalmMan (SciLab: acts 3, 4) | V1, 800 at 700 | V1, 800 | V2, 1000 | V3, 1600 at 1500 | V3, 1600 |
| MagnetMan (ACDC Area: acts 1, 2) | V2, 800 at 700 | V2, 800 | V3, 1800 at 1300 | V3, 1800 at 1500 | V3, 1800 |
| Meddy (End Area: acts 1, 2) | V1, 900 at 700 | V1, 900 | V2, 1100 | V2, 1100 | V3, 1500 |

Every one lies in his band, as Team Colonel's do. Their DS versions stay
out, as Team Colonel's do. One to watch: the
stats row's damage word is 30 at every version of GyroMan's (the others
rise from 30 to 120 or 200 by V3); whether his attacks take their power
from it or from his code is read in his differential before his deep
versions are trusted.

### His Soul, and beside Team Colonel's

- **Earned as Team Colonel's are**: his Guardian Data gives his Soul with
  MegaMan's words, for every battle of the run, one a guardian, a held
  one adding nothing ([Souls](meta/souls.md)). A run meets as many BN5
  guardians as before, so it holds as many Souls as before; which ones
  is what widens.
- **Twelve Souls, twelve kinds.** The UNITE check's table gives the
  twelve Souls twelve different kinds, every kind BN5 has: Team
  Colonel's Obstacle, Invisible, Plus, Wood, Break and Aqua, Team
  ProtoMan's Sword, Wind, Cursor, Fire, Elec and Recovery. No chip lights
  two Souls, so the rule "the first held Soul of the chip's kind" never
  has to choose, and BN5's own UNITE code needs only its flags for 1-6.
- **The key with the lock, counted.** By the chip records' kinds, BN6 has
  Sword 24, Fire 19, Elec 20, Wind 12 and Cursor 10 (Recovery is no
  element of BN6's; its Recovery chips light MeddySoul by BN5's kind of
  their name, as `soulkinds.c` reads every chip), and BN5 Sword 33, Elec
  27, Fire 20, Recovery 15, Cursor 13, Wind 11. Where Invisible and Plus
  were thin, these are thick: ProtoSoul's, NapalmSoul's and MagnetSoul's
  kinds sit in most folders already. The lean of a quarter of the finds
  toward a held Soul's kind ([A held Soul's kind](economy/held-soul-kind.md)) stays as it is;
  `build.py economy`'s last table gets the six new kinds.
- **What could go wrong:** a Sword Soul in a Busting folder unites every
  battle. That is BN5's own design, a chip given up for three turns once
  a battle; the statistics' Souls united (a count to add to the
  run-choices event) say whether one Soul crowds the rest out.

### What UNITE with them gives

From MMKB's Double Soul page, to be verified by `tools/xsoul.py`'s walk of
forms 1-6 and held to BN5's battle frame by frame as the six were:

| Soul | Charged shot | Its abilities | What BN6's engine has for it |
| --- | --- | --- | --- |
| ProtoSoul | a WideSword before him | Reflect on B and back; a charged Sword chip hits twice as hard and steps two panels in | the chip charge of the Souls' S5 (ShadowSoul's swords) |
| GyroSoul | a three-panel tornado | Air and Float shoes; a Wind chip spins his propeller, doubling the next chip | ShadowSoul's FloatShoe; enter and leave routines as NumberSoul's |
| SearchSoul | a scope gun, five locked shots through invisibility | enemies' invisibility gone; Search Shuffle reshuffles the Custom screen's unpicked chips | nothing: the shuffle reaches into BN6's Custom screen |
| NapalmSoul | a Fire vulcan down the row | lava harmless, the sea draining; Fire chips +40; a charged Fire chip throws a Napalm Bomb | BN5's lava and sea as BN6's panels 13 and 14 ([One net](../multirom/one-net.md)) |
| MagnetSoul | a magnet bolt that pulls a foe in and paralyses | magnet panels ignored; B and back freezes his row's foes; Elec chips charge for double | the chip charge; whether any of BN6's fields lays a magnet panel: to check |
| MeddySoul | a capsule bomb three panels ahead | the Custom screen's last two places hold capsules that add an effect to a chip | nothing: the capsules reach into BN6's Custom screen |

SearchSoul and MeddySoul change what BN6's Custom screen offers. The
rule is to drive BN6's own screen, never redraw it: their code is moved
as UNITE's was (BN5's own code in BN6's copy of the screen), traced
first, and left out if the trace shows the screen cannot hold it.

### ProtoMan: the real one, and the older net's

A run already has a ProtoMan. [The rival](rival.md) is the real one,
Chaud's, who races MegaMan, keeps the record and faces him in a
netbattle; the Nest copies him as a guardian too, and Chaud says so the
first time: "A copy. It'll never match the real thing." A third
ProtoMan, the older net's copy, guarding the Undernet beside Colonel,
would put BN5's look of the same Navi on the same net as the real one,
and give his Soul from a copy while the real ProtoMan is out there
keeping score. A player who knows both games knows these are one Navi:
the ProtoMan of BN5 is the ProtoMan of BN6, older.

So, settled for the fiction:

- **No older ProtoMan guards anything.** BN5's ProtoMan is not moved in
  this version; his code is the dearest of the six (8648 bytes, 16
  states, 119 decisions) and has no place a player would believe.
- **ProtoSoul is earned from the real one.** In a run where BN5's ROM is
  read, the rival's netbattle won (the third rung, and each later
  netbattle) gives ProtoMan's Soul for the rest of the run. Soul Unison
  is BN5's bond of trust between Navis; beating Chaud's ProtoMan in a
  real fight is the one way in a run to earn it from him, and Chaud's
  word at the win says so in one box. The owner's call holds as said:
  ProtoMan is a foe in every run, and beating him gives his Soul.
- **It gives the netbattle's later rungs a prize.** After the first
  netbattle the rungs restart, tighter, and pay the record alone; now
  each later netbattle won pays ProtoSoul for that run as well.
- **The copies stay copies.** Team ProtoMan's other five are the older
  net's copies, as Team Colonel's are; a copy of a Navi's data from one
  era stands beside a copy from another (BN4's SearchMan in the
  tournament, BN5's as End Area's guardian), since neither is the Navi
  himself. Only ProtoMan is on this net in person.

### On a dark board

A held Soul's Navi joins a Liberation board ([The Cross Navis and the team
picked](liberation/building/cross-navis-team.md)), fought as MegaMan in his Soul. Team ProtoMan's
commands are already traced in BN5's ROM, ids 1-6 (WideSwrd, Napalm,
MagnetBarrier, PanelSearch, BombLiberation, and Meddy's TwinLiberation),
and lent to BN6's Cross Navis by element. A held Team ProtoMan Soul
brings its own Navi with his own command; a Cross Navi of the same
element keeps the one he was lent, and the pick on arrival chooses.

## The build, in order

Cheapest first, by the walk's own measure (decisions listed, then code
and own states); each Navi is M, as each of Team Colonel's was:

| Unit | What | Size |
| --- | --- | --- |
| 1. The room (built) | Seats for the six, DarkProto's among them (`src/emu/xnavi_recipe.h` `XN_SEAT`, `XN_AI_OF`, `XN_ID_OF`; `tools/xnavirecipe.py` `seat`, `ai_of`, `id_of`): BN5's AI n of 1-6 at seat 10 + n (11-16), Navi AI 25 + n (26-31: between the control's 25 and Team Colonel's 32, no track's; the families' AI indexes a namespace of their own). Their ids: the formula's 0x1B0-0x1C3 are BN6's own, the player's forms (its ids table's type 2 from 0x1A0 to 0x1C3, read in the ROM), so ProtoMan, GyroMan, SearchMan and NapalmMan take the Navis' last four seats, 0x1F0-0x1FF, and MagnetMan and Meddy 0x3E4-0x3EB past BN4's 25 seats (`XN_MORE_IDS`, past every id BN6's code tests: [Virus families moved into BN6's battle engine](../rom/virus-families.md)), named by the families' archive as BN4's are. Their homes, code and art in a region of their own, `FREE_TEAM_NAVIS` (`+0x4D0000`-`+0x4EB000`, [The free space map](../engine/emulation/memory.md)), slots 12-17, and their battle sprites at list 2's 0x2B-0x30 (0x20 + seat): NapalmMan's and Meddy's after their room (`+0x4EB000`), GyroMan's in `FREE_TEAM_SPRITES_MORE` (`+0x710800`); ProtoMan's, SearchMan's and MagnetMan's, `0x22000` bytes in all, still need a place. `rivals.sav` to 65 navis, its fourth version ("CWR4", the third's 59 read into it): DarkProto and the five as navis 59-64 (`GUARDIAN_TEAM_FIRST`). Their Souls' bits, BN5's forms 1-6, in `run.souls`' two spare bytes (`soul_held` and `soul_give` for navis 59-64; the six's byte, what BN6's engine and the finds' lean read, unchanged; `RUN_MAGIC` unchanged, the `Run` untouched) | S |
| 2. NapalmMan | His recipe and `build.py xdiff`'s three specs (`napalmman_v1`, `_v1_keys`, `_sp`), mutations failing; his lava on BN6's panel 13 | M |
| 3. GyroMan | The same; his two hooks | M |
| 4. SearchMan | The same | M |
| 5. Meddy | The same; three raw pointers read by hand | M |
| 6. MagnetMan | The same; three raw pointers | M |
| 7. Guardians | The partner coin in `xguardian.c` (from act 2), the deep pool of eleven, their bodies and faces copied as the six's, Guardian Data (his Soul and a chip of his Soul's kind, below), their intro lines in a words file through [the voice rules](../voice/rules.md) | S-M |
| 8. Their Souls | `tools/xsoul.py` over forms 2-6, a Soul at a time with its specs as S2-S5 held the six: NapalmSoul, GyroSoul, MagnetSoul (M each), then SearchSoul and MeddySoul (L each, their Custom-screen code traced first) | L |
| 9. ProtoSoul | Form 1 by the same walk (M), given at the rival's netbattle won with BN5's ROM read, Chaud's box | M |
| 10. The board | A held Soul's Navi with his own command | S |

Their own Navi chips (BN5 ids 0xE0-0xF1, eighteen) wait for the registry
to have room; until then his Guardian Data gives a chip of his Soul's
kind in their place, as Team Colonel's gave before theirs were
registered.

A `Run` change (held Souls) bumps `RUN_MAGIC`; the guardian's pick is the
run's seed's and makes no layer, so `LAYER_MAKE` stays, but a held Soul
leans a layer's finds, so runs holding the new Souls make other stock:
`LAYER_MAKE` by judgement once unit 8 lands.

## The checks

- Each Navi: `build.py xdiff` (v1, v1_keys, sp PASS; `--mutations` each
  FAIL), `tools/xproxy.py --souls` the same every byte with them installed.
- Each Soul: its own specs against BN5's battle (the six's `soul_*`
  pattern), UNITE greyed for a chip of no held Soul's kind.
- `build.py pacing`: 0 past their band, with `--guardian` for each new
  navi; autopilot runs over `--net-biome x0`-`x5`, fresh data folders, the
  run logs read.
- `build.py economy`: the held Soul's lean over the six new kinds, 0 of
  the plan outside its bands.
- `tools/heap3ds.py` with BN5: the art copies' cost on the New 3DS.
- `python3 tools/voice_catalog.py`, `build.py lint`, `build.py test`.
- After release: the statistics' guardian event for each new navi (HP left
  and seconds against the band), and Souls united by kind.

## The owner's questions

**Answered on 9 November 2026: BN5's ProtoMan comes as DarkProto in the
Undernet**, over the recommendation to give no second ProtoMan. He is his
V2's own name in BN5's ROM, Colonel's partner as the Undernet's second
guardian by the same coin as the other five, and he drops ProtoSoul there.
His code is the costliest of the six, so he comes last in the build. The
rival's ProtoMan stays the rival, and his netbattle keeps its record.
The questions below stand as asked; the rest follow their
recommendations.


Each with the recommendation the data gives:

1. **BN5's ProtoMan.** Recommended: no older ProtoMan as a guardian;
   ProtoSoul from the rival's netbattle won, in runs with BN5's ROM.
   The other way: ProtoMan as Colonel's partner in the Undernet, named
   DarkProto (his V2's own name) to tell him from the rival, his Soul
   from that copy.
2. **The partner coin.** Recommended: even odds between Team Colonel's
   Navi and his partner, from act 2 only.
3. **Their Navi chips.** Recommended: a chip of his Soul's kind until the
   registry has room for eighteen more ids, the Darkloids' first.
4. **SearchSoul and MeddySoul**, whose abilities reach into BN6's Custom
   screen. Recommended: built last, after a trace of their Custom-screen
   code, and left out of this version if BN6's screen cannot host them
   without a redraw.
